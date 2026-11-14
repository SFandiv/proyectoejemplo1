#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in mat3 TBN;
in vec3 FragPos;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform bool useNormalMap;

uniform vec3 lightPos;
uniform vec3 lightColor;

void main() {
    vec3 normal;
    
    // 1. Decidir qué normal usar basado en si hay textura de relieve
    if (useNormalMap) {
        vec3 tangentNormal = texture(normalMap, TexCoord).rgb;
        tangentNormal = tangentNormal * 2.0 - 1.0;
        normal = normalize(TBN * tangentNormal);
    } else {
        normal = normalize(TBN[2]); 
    }

    // 2. Calcular la luz direccional y ambiental con la normal corregida
    vec3 ambient = 0.1 * lightColor;
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec4 texColor = texture(diffuseMap, TexCoord);
    FragColor = vec4((ambient + diffuse) * texColor.rgb, texColor.a);
}