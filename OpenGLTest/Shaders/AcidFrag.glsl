layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 texCoord;
in vec3 position;
in vec2 lightmapCoord;
  
uniform vec4 mainColor; 

uniform sampler2D mainTexture;
uniform sampler2D lightmap;

uniform float time;
uniform vec2 spriteSize;

void main()
{
    const float noiseSize = 0.12;
    const vec2 noiseDir = vec2(1, 1);
    const float noiseSpeed = 0.05;
    vec2 noiseCoord = position.xy * noiseSize + noiseDir * noiseSpeed * time;

    const float secNoiseSize = 0.25;
    const vec2 secNoiseDir = vec2(-1, 0.5);
    const float secNoiseSpeed = 0.1;
    vec2 secNoiseCoord = position.xy * secNoiseSize + secNoiseDir * secNoiseSpeed * time;

    const float realSurfaceSize = 0.5;
    float surfaceSize = realSurfaceSize / spriteSize.y;
    float surfaceAmount = 1 - ((1 - texCoord.y) / surfaceSize);
    if (surfaceAmount < 0) surfaceAmount = 0;

    const vec4 aColor = vec4(0.7, 0.2, 0, 0.8);
    const vec4 bColor = vec4(0.05, 0.7, 0, 0.2);
    const vec4 surfaceColor = vec4(0.5, 0.7, 0.3, 1);

    float noise = texture(mainTexture, noiseCoord).r * texture(mainTexture, secNoiseCoord).r;
    
    vec4 color = lerp(aColor, bColor, noise);
    color = (1 - surfaceAmount) * color + (surfaceAmount * surfaceColor);

    vec4 light = vec4(texture(lightmap, lightmapCoord).rgb, 1);

    vec4 finalColor = color * light;
    
    FragColor = finalColor;
    BrightColor = getBright(finalColor);
} 