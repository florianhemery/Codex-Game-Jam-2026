#version 330

// Etage vertex du shader eclaire : position monde, normale monde et
// position en espace lumiere (pour la shadow map).

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Uniforms fournis automatiquement par raylib
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

// Matrice vue-projection du soleil (passe d'ombres)
uniform mat4 lightVP;

out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;
out vec4 fragPosLight;

void main()
{
    vec4 world = matModel*vec4(vertexPosition, 1.0);
    fragPosition = world.xyz;
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    // Pas de normalisation ici : le fragment detecte les normales absentes.
    fragNormal = vec3(matNormal*vec4(vertexNormal, 0.0));
    fragPosLight = lightVP*world;

    gl_Position = mvp*vec4(vertexPosition, 1.0);
}
