#version 330 core

const float bloomMin = 1;

vec3 kernelFilter(float kernel[9], float offset, vec2 texCoord, sampler2D tex)
{
    vec2 offsets[9] = vec2[](
        vec2(-offset, offset),
        vec2(0,       offset),
        vec2(offset,  offset),
        vec2(-offset, 0),
        vec2(0,       0),
        vec2(offset,  0),
        vec2(-offset, -offset),
        vec2(0,       -offset),
        vec2(offset,  -offset)
    );

    float kernelSum = 0;
    vec3 result = vec3(0);
    for (int i = 0; i < 9; i++)
    {
        vec3 sample = texture(tex, texCoord + offsets[i]).rgb;
        result += sample * kernel[i];
        kernelSum += kernel[i];
    }

    return result / kernelSum;
}

vec4 getBright(vec3 color)
{
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > bloomMin)
        return vec4(color.rgb, 1.0);
    else
        return vec4(0.0, 0.0, 0.0, 1.0);
}

vec4 getBright(vec4 color)
{
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > bloomMin)
        return vec4(color.rgb, 1.0);
    else
        return vec4(0.0, 0.0, 0.0, 1.0);
}

vec4 lerp(vec4 a, vec4 b, float x)
{
    return a + (b - a) * x;
}
