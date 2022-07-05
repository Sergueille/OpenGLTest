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

vec3 lerp(vec3 a, vec3 b, float x)
{
    return a + (b - a) * x;
}

float random11(float p)
{
    p = fract(p * .1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}

float random12(vec2 p)
{
	vec3 p3 = fract(vec3(p.xyx) * .1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

vec4 random41(float p)
{
	vec4 p4 = fract(vec4(p) * vec4(.1031, .1030, .0973, .1099));
    p4 += dot(p4, p4.wzxy+33.33);
    return fract((p4.xxyz+p4.yzzw)*p4.zywx);
}

vec4 random42(vec2 p)
{
	vec4 p4 = fract(vec4(p.xyxy) * vec4(.1031, .1030, .0973, .1099));
    p4 += dot(p4, p4.wzxy+33.33);
    return fract((p4.xxyz+p4.yzzw)*p4.zywx);
}

float random13(vec3 p3)
{
	p3 = fract(p3 * .1031);
    p3 += dot(p3, p3.zyx + 31.32);
    return fract((p3.x + p3.y) * p3.z);
}

