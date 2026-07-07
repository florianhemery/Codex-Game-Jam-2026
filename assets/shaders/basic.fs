#version 330

// Shader fragment de base : reproduit le rendu par defaut de raylib
// (texture albedo x couleur materiau x couleur vertex) avec une teinte
// configurable en plus. Servira de socle au futur pipeline PBR.

// Entrees depuis le vertex shader
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Uniforms fournis automatiquement par raylib
uniform sampler2D texture0; // MATERIAL_MAP_ALBEDO
uniform vec4 colDiffuse;    // couleur materiau x tint de DrawModel()

// Teinte configurable (SetShaderValue). Alpha 0 (defaut GL) -> ignoree,
// le shader se comporte alors exactement comme celui par defaut de raylib.
uniform vec4 tintColor;

out vec4 finalColor;

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec4 tint = (tintColor.a > 0.0) ? tintColor : vec4(1.0);

    finalColor = texelColor*colDiffuse*fragColor*tint;
}
