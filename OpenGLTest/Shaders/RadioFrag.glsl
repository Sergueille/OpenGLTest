layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;
layout (location = 2) out vec4 CorruptionAmount;

in vec2 texCoord;
in vec3 position;
  
uniform vec4 mainColor; 

uniform bool hasTexture = false;
uniform sampler2D mainTexture;

uniform bool isLit = false;
uniform sampler2D lightmap;

void main()
{
    CorruptionAmount = vec4(1);
    FragColor = vec4(0, 0, 0, 0.1);
} 
