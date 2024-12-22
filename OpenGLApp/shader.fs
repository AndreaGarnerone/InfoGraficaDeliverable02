#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

// Textures
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;

// Uniform to determine which texture to use
uniform int textureID;

void main()
{
    if (textureID == 1)
        FragColor = texture(texture1, TexCoords);
    else if (textureID == 2)
        FragColor = texture(texture2, TexCoords);
    else if (textureID == 3)
        FragColor = texture(texture3, TexCoords);
    else
        FragColor = vec4(1.0);
}