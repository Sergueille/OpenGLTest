layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;
in vec2 TexCoords;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{    
    float alpha = texture(text, TexCoords).r;

    if (alpha < 0.01) {
        discard;
    }

    vec4 color = vec4(textColor, alpha);

    FragColor = color;
    BrightColor = getBright(color);
}  