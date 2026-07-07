#version 330

// Dome de ciel procedural :
// - degrade zenith/horizon
// - disque solaire + halo (HDR)
// - nuages FBM 2D projetes sur le dome, derive lente
// - etoiles scintillantes (si stars et soleil sous l'horizon)
// - fondu vers fogColor a l'horizon pour raccorder la scene

in vec3 fragDir;

uniform vec3 sunDir;        // direction du soleil VERS la scene
uniform vec3 sunColor;      // HDR
uniform vec3 skyZenith;
uniform vec3 skyHorizon;
uniform float cloudCoverage; // 0..1
uniform vec3 cloudTint;
uniform vec3 fogColor;
uniform float time;
uniform int starsOn;

out vec4 finalColor;

float Hash21(vec2 p)
{
    p = fract(p*vec2(234.34, 435.345));
    p += dot(p, p + 34.23);
    return fract(p.x*p.y);
}

float Hash13(vec3 p)
{
    p = fract(p*0.1031);
    p += dot(p, p.zyx + 31.32);
    return fract((p.x + p.y)*p.z);
}

float Noise2(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f*f*(3.0 - 2.0*f);
    float a = Hash21(i);
    float b = Hash21(i + vec2(1.0, 0.0));
    float c = Hash21(i + vec2(0.0, 1.0));
    float d = Hash21(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

// FBM 5 octaves, decorrelation par translation entre octaves.
float Fbm(vec2 p)
{
    float v = 0.0;
    float amp = 0.5;
    for (int i = 0; i < 5; i++)
    {
        v += amp*Noise2(p);
        p = p*2.03 + vec2(17.3, 9.1);
        amp *= 0.55;
    }
    return v;
}

void main()
{
    vec3 dir = normalize(fragDir);
    vec3 toSun = normalize(-sunDir);

    // Degrade zenith/horizon.
    float h = clamp(dir.y, 0.0, 1.0);
    vec3 col = mix(skyHorizon, skyZenith, pow(h, 0.42));

    // Disque solaire (smoothstep serre) + halo.
    float s = dot(dir, toSun);
    float disc = smoothstep(0.99935, 0.99965, s);
    float halo = pow(clamp(s, 0.0, 1.0), 160.0)*0.5 + pow(clamp(s, 0.0, 1.0), 8.0)*0.08;
    col += sunColor*(disc*9.0 + halo);

    // Nuages proceduraux projetes sur le dome.
    float cloudMask = 0.0;
    vec3 cloudCol = cloudTint;
    if (dir.y > 0.015)
    {
        vec2 cuv = dir.xz/(dir.y + 0.18)*0.9;
        cuv += vec2(0.013, 0.006)*time;
        float f = Fbm(cuv);
        float cut = mix(0.78, 0.18, clamp(cloudCoverage, 0.0, 1.0));
        cloudMask = smoothstep(cut, cut + 0.24, f);
        cloudMask *= smoothstep(0.015, 0.12, dir.y);
        cloudMask = min(cloudMask, 0.96);
        // Variation interne (masses sombres/claires) + illumination cote soleil.
        cloudCol = cloudTint*(0.45 + 0.95*f)*(0.85 + 0.35*pow(clamp(s, 0.0, 1.0), 3.0));
    }
    col = mix(col, cloudCol, cloudMask);

    // Etoiles : visibles seulement si demandees et soleil sous l'horizon,
    // masquees par les nuages.
    float starVis = (starsOn == 1) ? (1.0 - smoothstep(0.0, 0.12, max(toSun.y, 0.0))) : 0.0;
    if (starVis > 0.0 && dir.y > 0.03)
    {
        float hsh = Hash13(floor(dir*230.0));
        float star = step(0.9985, hsh);
        float twinkle = 0.55 + 0.45*sin(time*3.0 + hsh*80.0);
        col += vec3(0.9, 0.95, 1.15)*(star*twinkle*starVis*(1.0 - cloudMask));
    }

    // Raccord brouillard a l'horizon.
    col = mix(fogColor, col, smoothstep(-0.06, 0.09, dir.y));

    finalColor = vec4(col, 1.0);
}
