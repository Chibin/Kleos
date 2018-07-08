#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 vPositionModelSpace;
layout (location = 1) in vec4 vTestColor;
layout (location = 2) in vec2 vTexCoord;

layout (binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

layout (location = 0) out vec2 TexCoord;
layout (location = 1) out vec4 vColor;

void main()
{
    TexCoord = vTexCoord;
    vColor = vec4(vTexCoord.x, vTexCoord.y, 0, 1);

    gl_Position = ubo.projection * ubo.view * vec4(vPositionModelSpace, 1.0);
    //gl_Position =  vec4(vPositionModelSpace, 1.0);
}

