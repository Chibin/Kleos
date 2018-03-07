#version 330 core
layout (location = 0) in vec3 vPositionModelSpace;
layout (location = 1) in vec2 vTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;

void main()
{
    TexCoord = vTexCoord;
    gl_PointSize = 10.0;
    gl_Position = projection * view * model * vec4(vPositionModelSpace, 1.0);
}
