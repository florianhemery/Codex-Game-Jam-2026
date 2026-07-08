#version 330

// Post-process plein ecran (entree HDR lineaire -> sortie ecran) :
// 1. blur radial de vitesse (echantillons convergeant vers le centre)
// 2. aberration chromatique legere (decalage R/B radial)
// 3. exposition + tonemap ACES
// 4. etalonnage (teinte + saturation)
// 5. grain de film subtil
// 6. vignette

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;  // scene HDR

uniform float exposure;
uniform vec3 gradeTint;
uniform float saturation;
uniform float vignette;
uniform float aberration;
uniform float grainAmount;
uniform float time;
uniform float speedBlur;     // 0..~1.4 (boost nitro)

out vec4 finalColor;

// Approximation ACES (Narkowicz 2015).
vec3 Aces(vec3 x)
{
    return clamp((x*(2.51*x + 0.03))/(x*(2.43*x + 0.59) + 0.14), 0.0, 1.0);
}

float Hash(vec2 p)
{
    return fract(sin(dot(p, vec2(12.9898, 78.233)))*43758.5453);
}

void main()
{
    vec2 uv = fragTexCoord;
    vec2 toCenter = vec2(0.5) - uv;
    float distC = length(toCenter);

    // Blur radial + aberration chromatique par canal, en HDR.
    vec3 hdr = vec3(0.0);
    float total = 0.0;
    float reach = speedBlur*0.08*distC;
    vec2 ca = toCenter*aberration*(1.0 + distC*2.0);
    for (int i = 0; i < 8; i++)
    {
        float t = float(i)/7.0;
        vec2 p = uv + toCenter*(reach*t);
        float w = 1.0 - 0.55*t;
        hdr.r += texture(texture0, p + ca).r*w;
        hdr.g += texture(texture0, p).g*w;
        hdr.b += texture(texture0, p - ca).b*w;
        total += w;
    }
    hdr /= total;

    // Bloom leger sur les zones lumineuses (HDR, avant tonemap).
    vec3 bloom = vec3(0.0);
    const float bloomRadius = 0.0035;
    bloom += max(texture(texture0, uv + vec2(bloomRadius, 0.0)).rgb - 0.82, 0.0);
    bloom += max(texture(texture0, uv - vec2(bloomRadius, 0.0)).rgb - 0.82, 0.0);
    bloom += max(texture(texture0, uv + vec2(0.0, bloomRadius)).rgb - 0.82, 0.0);
    bloom += max(texture(texture0, uv - vec2(0.0, bloomRadius)).rgb - 0.82, 0.0);
    hdr += bloom * 0.22;

    // Exposition + tonemap ACES.
    vec3 color = Aces(hdr*exposure);

    // Etalonnage : teinte puis saturation autour de la luminance.
    color *= gradeTint;
    float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
    color = clamp(mix(vec3(luma), color, saturation), 0.0, 1.0);

    // Grain fixe (sans animation temporelle) pour eviter le scintillement.
    float g = Hash(uv * vec2(917.0, 533.0)) - 0.5;
    color += vec3(g*grainAmount);

    // Vignette.
    color *= 1.0 - vignette*smoothstep(0.32, 0.85, distC);

    finalColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
