#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 TexCoord;
layout (location = 1) in vec4 vColor;

layout (binding = 1) uniform sampler2D tex;

layout (location = 0) out vec4 color;

void main()
{
    color = texture(tex, TexCoord) + vColor;
    if (texture(tex, TexCoord).a < 0.01)
    {
        discard;
    }
}

