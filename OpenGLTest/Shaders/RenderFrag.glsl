out vec4 FragColor;

in vec2 texCoord;
uniform sampler2D screenTexture;
uniform sampler2D bloomBlur;
uniform sampler2D corruptionTexture;

uniform float time;
uniform float corruptionAmount = 0;

const float exposure = 1;

void main()
{
    const int tileSize = 50;
    const float timeStepSize = 0.05;
    const float maxOffset = 1/100.0;

    // Image corruption
    const float textureAmount = 0.5;
    float realAmount = corruptionAmount;
    if (texture(corruptionTexture, texCoord).r >= 1)
        realAmount = textureAmount;

    vec2 offsetCoord = texCoord;

    ivec2 tile = ivec2(gl_FragCoord.xy) / tileSize;
    int timeStep = int(time / timeStepSize);
    float rand = random13(vec3(tile.xy, float(timeStep)));
    if (rand > 1 - realAmount)
    {
        vec4 decal = random42(tile.xy);
        offsetCoord += ((2 * rand) - vec2(1)) * maxOffset * realAmount;
    }

    // Bloom
    vec3 hdrColor = texture(screenTexture, offsetCoord).rgb;
    vec3 bloomColor = texture(bloomBlur, offsetCoord).rgb;
    hdrColor += bloomColor;
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);

    FragColor = vec4(hdrColor.xyz, 1);
    // FragColor = vec4(texture(corruptionTexture, texCoord).xyz, 1);
}
