#version 330 core
out vec4 FragColor;

in vec2 texCoord;
  
uniform vec4 spriteColor; 
uniform sampler2D mainTexture;

void main()
{
    vec4 color = texture(mainTexture, texCoord) * spriteColor;

    if (color.a < 0.2)
    {
        discard;
    }

    FragColor = color;
} 