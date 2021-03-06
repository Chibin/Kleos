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

layout (push_constant) uniform PushConstants
{
    mat4 view;
    mat4 projection;
} pushConsts;

layout (location = 0) out vec2 TexCoord;
layout (location = 1) out vec4 vColor;

void main()
{
    TexCoord = vTexCoord;
    vColor = vTestColor;

    gl_Position = pushConsts.projection * pushConsts.view * vec4(vPositionModelSpace, 1.0);

    //gl_Position.y = -gl_Position.y;
    // can gl_Position.z be removed by using `#define GLM_FORCE_DEPTH_ZERO_TO_ONE` in
    // the render code?
    //gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
}

