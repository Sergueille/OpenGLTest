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
};

void main()
{
    const LaserData props[2] = LaserData[2](
        LaserData (
            vec4(1.5, 0.9, 1.2, 1),
            vec4(0.7, 0.2, 0.6, 0),
            vec2(0.7),
            0.3,
            0.2
        ),
        LaserData (
            vec4(0.8, 0.8, 1, 1),
            vec4(0.3, 0.8, 1, 0),
            vec2(0.7),
            0.3,
            0.2
        )
    );

    LaserData data = props[laserType];

    const vec3 intersectionColor = vec3(1, 0, 0);
    const float intersectionSize = 2;

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

    float intersectionIntensity = 1 - (distance(position.xy, intersectionPosition) / intersectionSize);
    if (intersectionIntensity > 0)
        color = vec4(lerp(color.rgb, intersectionColor, intersectionIntensity), color.a);

    FragColor = color;
    BrightColor = getBright(color);
} 