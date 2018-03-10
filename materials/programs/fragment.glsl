#version 330 core
in vec2 TexCoord;

uniform sampler2D tex;

out vec4 color;

void main()
{
    color = texture(tex, TexCoord);
    // color.a = 0.1; alpha channel
}
