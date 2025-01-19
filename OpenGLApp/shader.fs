#version 330 core
out vec4 FragColor;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;    
    float shininess;
}; 

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Inputs from vertex shader
in vec3 FragPos;  
in vec3 Normal; 
in vec2 TexCoords;

// Textures
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;

// Uniform to determine which texture to use
uniform int textureID;

// Lighting uniforms
uniform vec3 viewPos;
uniform Material material;
uniform Light light;

void main()
{
    vec4 texColor;
    if (textureID == 1)
        texColor = texture(texture1, TexCoords);
    else if (textureID == 2)
        texColor = texture(texture2, TexCoords);
    else if (textureID == 3)
        texColor = texture(texture3, TexCoords);
    else
        texColor = vec4(1.0);

        // Lighting calculations

    // Ambient
    vec3 ambient = light.ambient * material.ambient;

    // Diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);  

    // Combine lighting components
    vec3 lighting = ambient + diffuse + specular;

    // Apply lighting to the texture color
    vec3 result = lighting * texColor.rgb;

    // Output final fragment color
    FragColor = vec4(result, texColor.a);
}