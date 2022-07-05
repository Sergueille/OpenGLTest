layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 texCoord;
in vec3 position;
  
uniform vec4 mainColor;
uniform sampler2D mainTexture;

uniform float time;

void main()
{
    vec4 color = texture(mainTexture, texCoord) * mainColor;
    float sine = sin(time * 5) / 8 + 0.4;
    color += vec4(sine, sine, sine, 0);

    if (color.a < 0.001)
        discard;

    FragColor = color;
    BrightColor = vec4(0);
} 