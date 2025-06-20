#version 440

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D texture0;

uniform vec3 CamPos;

uniform int NumberLights;

struct LightInfo {
    vec4 Position;   // Light position (x, y, z) + w (Type of light)
    vec4 Colour;     // Light colour (r, g, b) + a (intensity)
};

layout(std140, binding = 0) buffer LightBuffer {
    LightInfo Light[];
};

struct MaterialInfo {
    vec3 Ka; // Ambient reflectivity
    vec3 Kd; // Diffuse reflectivity
    vec3 Ks; // Specular reflectivity
    float Shiness; // Specular shininess factor (phong exponent)
};
uniform MaterialInfo Material;

out vec4 FragColor;

void main() {
    vec3 base_color = texture(texture0, TexCoord).rgb;
    vec3 finalColor = vec3(0,0,0);
    vec3 ambient = vec3(0,0,0);

    for (int i = 0; i < NumberLights; i++) {
        int type = int(Light[i].Position.w);
        vec3 pos = Light[i].Position.xyz;
        vec3 colour = Light[i].Colour.rgb;
        vec3 direction = Light[i].Colour.xyz;

        if (type == 0) { // Point light
            vec3 L = normalize(pos - Position);
            vec3 V = normalize(CamPos - Position);
            vec3 HalfwayVector = normalize(V + L);

            vec3 diffuse = Material.Kd * colour * max(dot(L, Normal), 0.0);
            vec3 specular = Material.Ks * colour * pow(max(dot(HalfwayVector, Normal), 0.0), Material.Shiness);
            finalColor += diffuse + specular;
        } else if (type == 1) { // Ambient light
            ambient += Material.Ka * colour;
        }
    }

    finalColor += ambient;
    FragColor = vec4(finalColor * base_color, 1.0);
}
