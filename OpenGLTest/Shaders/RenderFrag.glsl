out vec4 FragColor;

in vec2 texCoord;
uniform sampler2D screenTexture;
uniform sampler2D bloomBlur;

uniform float time;
uniform float corruptionAmount = 0;

const float exposure = 1;

void main()
{
    const int tileSize = 50;
    ivec2 tile = ivec2(gl_FragCoord.xy) / tileSize;

    float rand = random13(vec3(tile.xy, time));
    
    if (rand > 1 - corruptionAmount)
        discard;

    vec3 hdrColor = texture(screenTexture, texCoord).rgb;
    vec3 bloomColor = texture(bloomBlur, texCoord).rgb;
    hdrColor += bloomColor;
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);

    FragColor = vec4(hdrColor.xyz, 1);
}
