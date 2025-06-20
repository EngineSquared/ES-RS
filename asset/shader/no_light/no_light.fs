#version 440

in vec3 Position;  // Usually in world or view space
in vec3 Normal;    // Normal vector, should be normalized in vertex shader ideally

struct MaterialInfo {
    vec3 Ka;  // Ambient reflectivity
    vec3 Kd;  // Diffuse reflectivity
    vec3 Ks;  // Specular reflectivity
    float Shiness;  // Specular exponent (phong)
};
uniform MaterialInfo Material;

// Light properties
uniform vec3 CamPos;    // Camera position in world space

out vec4 FragColor;

void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(vec3(0.0, 0.0, 1.0)); // Assuming light direction is along Z-axis

    // Ambient term - white ambient light means ambient light color = vec3(1.0)
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
    FragColor = vec4(result, 1.0);
}
