#version 330 core
in vec2 TexCoord;
in vec4 vColor;

uniform sampler2D tex;

out vec4 color;

void main()
{
    color = texture(tex, TexCoord);
//    if (vColor.a > 0) {
//        color.rgb += vColor.rgb;
//    }

    // color.a = 0.1; alpha channel
    //color = vColor;
}
