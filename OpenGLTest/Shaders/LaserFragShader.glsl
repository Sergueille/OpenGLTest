#version 330 core
out vec4 FragColor;

in vec2 texCoord;
in vec3 position;
  
uniform vec4 mainColor; 
uniform vec4 secColor;
uniform sampler2D noise;
uniform float noiseSize;
uniform vec2 noiseSpeed;
uniform float distorsionAmount;
uniform float time;

void main()
{
    vec4 color;
    vec4 noise = texture(noise, (position.xy * noiseSize) + (noiseSpeed * time));

    float distorsion = distorsionAmount * (noise.x - 0.5f);
    vec2 deformedTexCoord = texCoord + vec2(0.0f, distorsion);

    if (deformedTexCoord.y < 0.5)
    {
        color = secColor + ((deformedTexCoord.y * 2) * (mainColor - secColor));
    }
    else
    {
        color = mainColor + (((deformedTexCoord.y - 0.5) * 2) * (secColor - mainColor));
    }

    FragColor = color;
} 