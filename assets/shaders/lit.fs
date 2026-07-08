#version 330

// Etage fragment du shader eclaire :
// - diffus Lambert + speculaire GGX discret (roughness fixe)
// - soleil directionnel avec ombre (PCF 3x3, biais adaptatif)
// - ambiante hemispherique (ciel / sol selon N.y)
// - jusqu'a 16 point lights avec attenuation douce
// - brouillard exp2
// Sortie HDR lineaire : le tonemap se fait dans la passe post.

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;
in vec4 fragPosLight;

// Uniforms fournis automatiquement par raylib
uniform sampler2D texture0;
uniform vec4 colDiffuse;

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

out vec4 finalColor;

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
    vec4 albedo = texture(texture0, fragTexCoord)*colDiffuse*fragColor;
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

    vec3 L = normalize(-sunDir);
    float ndl = max(dot(N, L), 0.0);
    float visibility = (ndl > 0.0) ? SunVisibility(ndl) : 0.0;

    // Ambiante hemispherique.
    vec3 ambient = mix(groundAmbient, skyAmbient, N.y*0.5 + 0.5);

    // Soleil : diffus Lambert.
    vec3 direct = sunColor*(ndl*visibility);

    // Speculaire GGX discret, roughness fixe.
    float rough = 0.85;
    float a = rough*rough;
    float a2 = a*a;
    vec3 H = normalize(L + V);
    float ndh = max(dot(N, H), 0.0);
    float ndv = max(dot(N, V), 0.0);
    float dd = ndh*ndh*(a2 - 1.0) + 1.0;
    float ndf = a2/(3.14159*dd*dd);
    float k = a*0.5;
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
