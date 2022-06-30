out vec3 FragColor;

in vec2 texCoord;
uniform int nbLights;
uniform vec2 lightPos[256]; // Position in UV coordinates
uniform vec3 lightColor[256]; // Color of the light, intensity included
uniform vec2 lightSize[256]; // Size in UV coordinates
uniform vec3 lightAngles[256]; // Rotation, inner angle, outer angle
uniform vec3 lightShadowSize[256]; // Size of the emiter, used for shadows (UV size x, UV size y, world size)

uniform int nbShadowCasters = 0;
uniform vec2 shadowCastersPos[128]; // Position in UV coordinates
uniform float shadowCastersRot[128]; // Rotation
uniform vec2 shadowCastersSize[128]; // Size in UV coordinates

bool GetShadow(int light, vec2 pos, vec2 lightPosOverride);
float GetFinalShadow(int light);

void main()
{
    vec3 res = vec3(0);
    for (int i = 0; i < nbLights; i++)
    {
        vec2 delta = lightPos[i] - texCoord; // Get delta pos
        delta.x /= lightSize[i].x;
        delta.y /= lightSize[i].y;

        float distAttenuation = 1 - length(delta);
        if (distAttenuation <= 0) 
            continue;

        float lightDir = (lightAngles[i].x + 135) * 3.14159265 / 180; // Add 135 don't know why
        vec2 lightVector = vec2(
            cos(lightDir) - sin(lightDir),
			sin(lightDir) + cos(lightDir)
        );

        float deltaAngle = acos(dot(lightVector, delta) / length(delta) / length(lightVector)) * 180 / 3.14159265; // Get angle of delta vector
        float inner = lightAngles[i].y;
        float outer = lightAngles[i].z;
        float angleAttenuation = (outer - deltaAngle) / (outer - inner);
        if (angleAttenuation < 0)
            continue; 
        if (angleAttenuation > 1) angleAttenuation = 1; // Clamp

        float finalAttenuation = distAttenuation * angleAttenuation;

        if (finalAttenuation > 0)
        {
            float shadowAmount = GetFinalShadow(i);

            if (shadowAmount > 0)
                res += lightColor[i] * finalAttenuation * shadowAmount; // Add light
        }
    }

    FragColor = res;
}

vec2 Rotate(vec2 vector, float angle)
{
    float rad = angle * 3.14159265 / 180.0;
    return vec2(
        (cos(rad) * vector.x) - (sin(rad) * vector.y),
        (sin(rad) * vector.x) + (cos(rad) * vector.y)
    );
}

float GetFinalShadow(int light)
{
    vec2 lightPos = vec2(lightPos[light].x, lightPos[light].y);
    vec2 delta = texCoord - lightPos;

    float nbSamples = 11.0 * lightShadowSize[light].z + 2;

    vec2 dir = normalize(delta);
    vec2 tangent = vec2(cross(vec3(dir.xy, 0), vec3(0, 0, 1)));

    float sampleStep = 2.0 / nbSamples;
    float passed = 0;
    float iterations = 0;
    for (float i = -1 ; i <= 1 + 0.00001; i += sampleStep)
    {
        vec2 lightDelta = tangent * i * lightShadowSize[light].xy;
        vec2 newLightPos = lightPos + lightDelta;
        passed += GetShadow(light, texCoord, newLightPos) ? 1 : 0;
        iterations += 1;
    }

    return passed / iterations;
}

bool GetShadow(int light, vec2 pos, vec2 lightPosOverride) 
{
    float a = (pos.y - lightPosOverride.y) / (pos.x - lightPosOverride.x);
    float b = lightPosOverride.y - (a * lightPosOverride.x);

    // Part of code from RectCollider
    for (int i = 0; i < nbShadowCasters; i++)
    {
        float dist =  length(lightPosOverride - shadowCastersPos[i]);
        if (dist > lightSize[light].x * 2 + shadowCastersSize[i].x * 2
            && dist > lightSize[light].y * 2 + shadowCastersSize[i].y * 2)
            continue;

        vec2 halfSize = shadowCastersSize[i] / 2.f;
        float orientation = shadowCastersRot[i];
        vec2 xVect = Rotate(vec2(1, 0), orientation) * halfSize.x;
        vec2 yVect = Rotate(vec2(0, 1), orientation) * halfSize.y;

        vec2[4] points = {
            shadowCastersPos[i] + xVect + yVect,
            shadowCastersPos[i] - xVect + yVect,
            shadowCastersPos[i] - xVect - yVect,
            shadowCastersPos[i] + xVect - yVect
        };

        // For each side
        for (int j = 0; j < 4; j++)
        {
            vec2 point = points[j]; // First point pf the side
            vec2 next = j == 3 ? points[0] : points[j + 1]; // Second point of the side

            float aside = (point.y - next.y) / (point.x - next.x);
            float bside = next.y - (aside * next.x);

            if (aside == a) continue; // Continue if lines are not crossing

            // Get intersection point
            float resx = (bside - b) / (a - aside);
            float resy = (aside * resx) + bside;
            vec2 res = vec2(resx, resy);

            // Check if the intesection point is on the line
            if (point.x == next.x) // Vectical lines
            {
                if ((point.y < next.y && res.y > point.y && res.y < next.y)
                    || (point.y > next.y && res.y < point.y && res.y > next.y))
                {
                    return false; // Intersect
                }
            }
            else // Other lines
            {
                if ((point.x < next.x && res.x > point.x && res.x < next.x)
                    || (point.x > next.x && res.x < point.x && res.x > next.x)) // If on side
                {
                    if ((pos.x > lightPosOverride.x && res.x < pos.x && res.x > lightPosOverride.x)
                        || (pos.x < lightPosOverride.x && res.x > pos.x && res.x < lightPosOverride.x)) // If btw light and pos
                    {
                        return false; // Intersect
                    }
                }
            }
        }
    }

    return true;
}
