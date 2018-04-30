#version 330 core
precision highp float;
in vec2 TexCoord;
in vec4 vColor;

uniform sampler2D tex;

out vec4 color;

void main()
{
    color = texture(tex, TexCoord) * vColor;
    if (color.a < 0.01) {
        discard;
    }
}
