#version 440

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D texture0;

struct MaterialInfo {
    vec3 Ka;  // Ambient reflectivity
    vec3 Kd;  // Diffuse reflectivity
    vec3 Ks;  // Specular reflectivity
    float Shiness;  // Specular exponent (phong)
};
uniform MaterialInfo Material;

uniform vec3 CamPos;

out vec4 FragColor;

void main() {
    vec4 texColor = texture(texture0, TexCoord);
    if (texColor.a < 0.1)
        discard;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(vec3(0.0, 0.0, 1.0)); // Directional light along Z-axis

    // Ambient term
    vec3 ambientLightColor = vec3(1.0);
    vec3 ambient = ambientLightColor * Material.Ka;

    // Diffuse term
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * Material.Kd;

    // Specular term (Phong)
    vec3 viewDir = normalize(CamPos - Position);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = 0.0;
    if(diff > 0.0)
        spec = pow(max(dot(viewDir, reflectDir), 0.0), Material.Shiness);
    vec3 specular = spec * Material.Ks;

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result * texColor.rgb, texColor.a);
}
