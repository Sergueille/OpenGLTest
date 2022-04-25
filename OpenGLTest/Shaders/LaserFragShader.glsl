#version 330 core
out vec4 FragColor;

in vec2 texCoord;
  
uniform vec4 mainColor; 
uniform vec4 secColor;

void main()
{
    vec4 color;

    if (texCoord.y < 0.5)
    {
        color = secColor + ((texCoord.y * 2) * (mainColor - secColor));
    }
    else
    {
        color = mainColor + (((texCoord.y - 0.5) * 2) * (secColor - mainColor));
    }

    FragColor = color;
} 