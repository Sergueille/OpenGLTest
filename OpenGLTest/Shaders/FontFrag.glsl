layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;
in vec2 TexCoords;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{    
    if (texture(text, TexCoords).r < 0.1) {
        discard;
    }

    FragColor = vec4(textColor, 1.0);
    BrightColor = getBright(textColor);
}  