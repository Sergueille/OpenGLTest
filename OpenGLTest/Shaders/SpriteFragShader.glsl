#version 330 core
out vec4 FragColor;

in vec2 texCoord;
  
uniform vec4 mainColor; 
uniform sampler2D mainTexture;

void main()
{
    vec4 color = texture(mainTexture, texCoord) * mainColor;

    if (color.a < 0.01)
    {
        discard;
    }

    FragColor = color;
} 