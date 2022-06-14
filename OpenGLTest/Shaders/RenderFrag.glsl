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

    // Image corruption
    const float textureAmount = 0.8;
    float realAmount = corruptionAmount;
    if (texture(corruptionTexture, texCoord).r >= 1)
        realAmount = textureAmount;

    ivec2 tile = ivec2(gl_FragCoord.xy) / tileSize;
    float rand = random13(vec3(tile.xy, time));
    if (rand > 1 - realAmount)
        discard;

    // Bloom
    vec3 hdrColor = texture(screenTexture, texCoord).rgb;
    vec3 bloomColor = texture(bloomBlur, texCoord).rgb;
    hdrColor += bloomColor;
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);

    FragColor = vec4(hdrColor.xyz, 1);
    // FragColor = vec4(texture(corruptionTexture, texCoord).xyz, 1);
}
