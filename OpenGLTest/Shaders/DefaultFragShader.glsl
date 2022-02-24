#version 330 core
out vec4 FragColor;
in vec2 uv;
  
uniform vec4 mainColor; 

void main()
{
    FragColor = vec4(uv, 0.0, 0.0) + mainColor;
} 