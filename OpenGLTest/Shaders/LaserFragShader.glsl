layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 texCoord;
in vec3 position;
  
uniform int laserType;
uniform vec2 intersectionPosition;
uniform sampler2D noise;
uniform float time;

struct LaserData {
    vec4 mainColor;
    vec4 secColor;
    vec2 noiseSpeed;
    float noiseSize;
    float distortionAmount;
    vec3 intersectionColor;
    float intersectionSize;
};

void main()
{
    const LaserData props[2] = LaserData[2](
        LaserData (
            vec4(1.5, 0.9, 1.2, 1),
            vec4(0.7, 0.2, 0.6, 0),
            vec2(0.7),
            0.3,
            0.2,
            vec3(1.1, 0.1, 0.1),
            2
        ),
        LaserData (
            vec4(1.5, 0.4, 0.4, 1),
            vec4(0.5, 0.2, 0.2, 0),
            vec2(0.8),
            0.4,
            0.3,
            vec3(1.5, 1.2, 0.1),
            4
        )
    );

    LaserData data = props[laserType];

    vec4 color;
    vec4 noise = texture(noise, (position.xy * data.noiseSize) + (data.noiseSpeed * time));

    float distorsion = data.distortionAmount * (noise.x - 0.5f);
    vec2 deformedTexCoord = texCoord + vec2(0.0f, distorsion);

    if (deformedTexCoord.y < 0.5)
    {
        color = data.secColor + ((deformedTexCoord.y * 2) * (data.mainColor - data.secColor));
    }
    else
    {
        color = data.mainColor + (((deformedTexCoord.y - 0.5) * 2) * (data.secColor - data.mainColor));
    }

    float intersectionIntensity = 1 - (distance(position.xy, intersectionPosition) / data.intersectionSize);
    if (intersectionIntensity > 0)
        color = vec4(lerp(color.rgb, data.intersectionColor, intersectionIntensity), color.a);

    FragColor = color;
    BrightColor = getBright(color);
} 