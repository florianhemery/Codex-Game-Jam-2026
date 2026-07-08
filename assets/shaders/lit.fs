#version 330

// Etage fragment du shader eclaire :
// - diffus Lambert + speculaire GGX discret (roughness variable)
// - soleil directionnel avec ombre (PCF 3x3, biais adaptatif)
// - ambiante hemispherique (ciel / sol selon N.y)
// - jusqu'a 16 point lights avec attenuation douce
// - brouillard exp2
// - materiaux terrain proceduraux (splat weights dans fragColor)
// Sortie HDR lineaire : le tonemap se fait dans la passe post.

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;
in vec4 fragPosLight;

// Uniforms fournis automatiquement par raylib
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float useTextureAlbedo;

// Environnement
uniform vec3 viewPos;
uniform vec3 sunDir;        // direction du soleil VERS la scene
uniform vec3 sunColor;      // HDR
uniform vec3 skyAmbient;
uniform vec3 groundAmbient;
uniform vec3 fogColor;
uniform float fogDensity;

// Ombres
uniform sampler2D shadowMap;
uniform float shadowTexel;  // 1/resolution de la shadow map

// Point lights
#define MAX_LIGHTS 16
uniform vec3 lightsPos[MAX_LIGHTS];
uniform vec3 lightsColor[MAX_LIGHTS]; // couleur x intensite (HDR)
uniform int lightsCount;

// Terrain proceduraux Aurelia
uniform float terrainMode;
uniform vec3 biomeTint[4];  // grass, rock, asphalt, sand

out vec4 finalColor;

// --- Bruit procedural (hash / FBM / ridged) ---

float hash21(vec2 p)
{
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 34.45);
    return fract(p.x * p.y);
}

float noise2(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = hash21(i);
    float b = hash21(i + vec2(1.0, 0.0));
    float c = hash21(i + vec2(0.0, 1.0));
    float d = hash21(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float fbm(vec2 p)
{
    float v = 0.0;
    float amp = 0.5;
    for (int i = 0; i < 4; i++)
    {
        v += amp * noise2(p);
        p *= 2.03;
        amp *= 0.5;
    }
    return v;
}

float ridgedNoise(vec2 p)
{
    float n = noise2(p);
    n = 1.0 - abs(2.0 * n - 1.0);
    return n * n;
}

float ridgedFbm(vec2 p)
{
    float v = 0.0;
    float amp = 0.55;
    for (int i = 0; i < 4; i++)
    {
        v += amp * ridgedNoise(p);
        p *= 2.11;
        amp *= 0.5;
    }
    return v;
}

bool isTerrainDraw(vec4 weights)
{
    float wSum = weights.r + weights.g + weights.b + weights.a;
    if (wSum < 0.001)
        return false;
    if (terrainMode > 0.5)
        return true;
    return length(weights.rgb - vec3(1.0)) > 0.01 || weights.a < 0.99;
}

vec3 sampleTriplanarFbm(vec3 worldPos)
{
    vec2 xz = worldPos.xz * 0.05;
    float nX = fbm(vec2(worldPos.y, worldPos.z) * 0.08);
    float nY = fbm(xz);
    float nZ = fbm(vec2(worldPos.x, worldPos.y) * 0.08);
    return vec3(nX, nY, nZ);
}

vec3 terrainGrass(vec2 uv, vec3 N, vec3 tint, float macro)
{
    float fine = fbm(uv * 8.0 + vec2(3.1, 7.2));
    float blade = fbm(uv * 22.0 + vec2(11.0, 5.0));
    vec3 base = vec3(0.16, 0.40, 0.13) * tint;
    base *= 0.82 + 0.28 * fine + 0.08 * blade;
    base *= 0.55 + 0.45 * clamp(N.y, 0.0, 1.0);
    base *= 0.90 + 0.20 * macro;
    return base;
}

vec3 terrainRock(vec2 uv, vec3 tint, float macro)
{
    float ridge = ridgedFbm(uv * 3.5 + vec2(1.7, 4.3));
    float grain = fbm(uv * 14.0 + vec2(9.0, 2.0));
    vec3 dark = vec3(0.30, 0.29, 0.27) * tint;
    vec3 light = vec3(0.52, 0.50, 0.47) * tint;
    vec3 col = mix(dark, light, ridge);
    col *= 0.88 + 0.18 * grain;
    col *= 0.92 + 0.16 * macro;
    return col;
}

vec3 terrainAsphalt(vec2 uv, vec3 tint)
{
    vec2 cell = floor(uv * 36.0);
    float crack = hash21(cell);
    float micro = fbm(uv * 48.0);
    vec3 base = vec3(0.11, 0.11, 0.12) * tint;
    float crackMask = step(0.92, crack);
    base = mix(base, base * 1.35, crackMask * 0.35);
    base *= 0.88 + 0.16 * micro;
    return base;
}

vec3 terrainSand(vec2 uv, vec3 tint, float macro)
{
    float grain = fbm(uv * 20.0 + vec2(4.0, 13.0));
    float ripple = fbm(uv * 5.0 + vec2(17.0, 3.0));
    vec3 base = vec3(0.78, 0.66, 0.44) * tint;
    base *= 0.86 + 0.22 * grain;
    base += vec3(0.04, 0.02, 0.0) * ripple;
    base *= 0.94 + 0.12 * macro;
    return base;
}

vec3 terrainAlbedo(vec3 worldPos, vec2 uv, vec4 weights, vec3 N)
{
    vec3 tri = sampleTriplanarFbm(worldPos);
    float macro = tri.y;

    vec3 grass = terrainGrass(uv, N, biomeTint[0], macro);
    vec3 rock = terrainRock(uv, biomeTint[1], macro);
    vec3 asphalt = terrainAsphalt(uv, biomeTint[2]);
    vec3 sand = terrainSand(uv, biomeTint[3], macro);

    vec3 col = grass * weights.r + rock * weights.g + asphalt * weights.b
        + sand * weights.a;

    // Variation macro altitude / biome
    float alt = worldPos.y;
    col *= 0.92 + 0.10 * sin(alt * 0.04 + macro * 6.28);

    // AO cheap : courbure de hauteur en espace ecran
    float dhdx = dFdx(worldPos.y);
    float dhdz = dFdy(worldPos.y);
    float curv = abs(dhdx) + abs(dhdz);
    float ao = 1.0 - clamp(curv * 10.0, 0.0, 0.38);
    col *= ao;

    return col;
}

float terrainRoughness(vec4 weights)
{
    return weights.r * 0.95 + weights.g * 0.90 + weights.b * 0.70 + weights.a * 0.92;
}

// Facteur d'eclairement solaire [0..1] via shadow map, PCF 3x3.
// Retourne 1.0 hors du frustum lumiere (fondu pres des bords).
float SunVisibility(float ndl)
{
    vec3 proj = fragPosLight.xyz/fragPosLight.w;
    proj = proj*0.5 + 0.5;
    if (proj.z >= 1.0) return 1.0;
    if (proj.x <= 0.0 || proj.x >= 1.0 || proj.y <= 0.0 || proj.y >= 1.0) return 1.0;

    // Biais plus genereux pour limiter l'acne sur surfaces planes et petites pieces.
    float bias = max(0.0025*(1.0 - ndl), 0.00035);

    // PCF 3x3 : noyau fixe pour limiter le scintillement des ombres en mouvement.
    float lit = 0.0;
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            vec2 o = vec2(float(x), float(y)) * shadowTexel * 1.4;
            float depth = texture(shadowMap, proj.xy + o).r;
            lit += (proj.z - bias > depth) ? 0.0 : 1.0;
        }
    }
    lit /= 9.0;

    // Fondu vers "eclaire" pres des bords de la carte (pas de coupure nette).
    vec2 edge = min(proj.xy, vec2(1.0) - proj.xy);
    float border = smoothstep(0.0, 0.05, min(edge.x, edge.y));
    return mix(1.0, lit, border);
}

void main()
{
    vec3 V = normalize(viewPos - fragPosition);

    // Normale : certaines primitives raylib n'emettent pas de normale fiable,
    // on reconstruit alors une normale geometrique en espace ecran.
    vec3 N = fragNormal;
    if (dot(N, N) < 0.01)
    {
        N = cross(dFdx(fragPosition), dFdy(fragPosition));
        if (dot(N, V) < 0.0) N = -N;
    }
    N = normalize(N);

    vec4 albedo;
    float rough = 0.85;

    vec4 splatW = fragColor;
    float wSum = splatW.r + splatW.g + splatW.b + splatW.a;

    if (isTerrainDraw(splatW))
    {
        splatW /= wSum;
        albedo.rgb = terrainAlbedo(fragPosition, fragTexCoord, splatW, N);
        albedo.a = 1.0;
        rough = terrainRoughness(splatW);
    }
    else if (useTextureAlbedo < 0.5) {
        // DrawCube sous BeginShaderMode(lit) : couleur via colDiffuse.
        albedo.rgb = colDiffuse.rgb * fragColor.rgb;
        albedo.a = colDiffuse.a * fragColor.a;
    } else if (length(fragColor.rgb - vec3(1.0)) > 0.01) {
        albedo.rgb = fragColor.rgb;
        albedo.a = fragColor.a;
    } else {
        albedo = texture(texture0, fragTexCoord) * colDiffuse * fragColor;
    }

    vec3 L = normalize(-sunDir);
    float ndl = max(dot(N, L), 0.0);
    float visibility = (ndl > 0.0) ? SunVisibility(ndl) : 0.0;

    // Ambiante hemispherique.
    vec3 ambient = mix(groundAmbient, skyAmbient, N.y*0.5 + 0.5);

    // Soleil : diffus Lambert.
    vec3 direct = sunColor*(ndl*visibility);

    // Speculaire GGX discret, roughness variable (terrain ou defaut).
    float a = rough*rough;
    float a2 = a*a;
    vec3 H = normalize(L + V);
    float ndh = max(dot(N, H), 0.0);
    float ndv = max(dot(N, V), 0.0);
    float dd = ndh*ndh*(a2 - 1.0) + 1.0;
    float ndf = a2/(3.14159*dd*dd);
    float k = rough*0.5;
    float vis = 1.0/max((ndl*(1.0 - k) + k)*(ndv*(1.0 - k) + k), 1e-4);
    float fres = 0.04 + 0.96*pow(1.0 - max(dot(H, V), 0.0), 5.0);
    vec3 spec = sunColor*(ndf*vis*fres*0.03*ndl*visibility);

    // Point lights : Lambert + attenuation 1/(1 + 0.12 d + 0.045 d^2).
    vec3 points = vec3(0.0);
    for (int i = 0; i < lightsCount; i++)
    {
        vec3 toL = lightsPos[i] - fragPosition;
        float dist = length(toL);
        vec3 Lp = toL/max(dist, 1e-3);
        float att = 1.0/(1.0 + 0.12*dist + 0.045*dist*dist);
        points += lightsColor[i]*(max(dot(N, Lp), 0.0)*att);
    }

    vec3 color = albedo.rgb*(ambient + direct + points) + spec;

    // Brouillard exp2 vers fogColor.
    float dist = length(fragPosition - viewPos);
    float fogF = exp2(-fogDensity*fogDensity*dist*dist*1.442695);
    color = mix(fogColor, color, clamp(fogF, 0.0, 1.0));

    finalColor = vec4(color, albedo.a);
}
