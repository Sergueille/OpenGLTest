out vec4 FragColor;

in vec2 texCoord;
uniform sampler2D screenTexture;
uniform sampler2D bloomBlur;

const float exposure = 1;

void main()
{
    vec3 hdrColor = texture(screenTexture, texCoord).rgb;
    vec3 bloomColor = texture(bloomBlur, texCoord).rgb;
    hdrColor += bloomColor;
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);

    FragColor = vec4(hdrColor.xyz, 1);
}
