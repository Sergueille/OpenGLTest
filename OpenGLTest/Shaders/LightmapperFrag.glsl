out vec3 FragColor;

in vec2 texCoord;
uniform int nbLights;
uniform vec2 lightPos[256];
uniform vec3 lightColor[256];
uniform vec2 lightSize[256];

void main()
{
    vec3 res = vec3(0);
    for (int i = 0; i < nbLights; i++)
    {
        vec2 delta = lightPos[i] - texCoord;        
        delta.x /= lightSize[i].x;
        delta.y /= lightSize[i].y;

        float dist = sqrt(delta.x * delta.x + delta.y * delta.y);
        float distAttenuation = 1 - dist;

        if (distAttenuation > 0)
        {
            res += lightColor[i] * distAttenuation;
        }
    }

    FragColor = res;
}
