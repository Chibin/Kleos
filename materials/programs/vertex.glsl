#version 330 core
layout (location = 0) in vec3 vPositionModelSpace;
layout (location = 1) in vec4 vTestColor;
layout (location = 2) in vec2 vTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform uint type;

out vec2 TexCoord;
out vec4 vColor;

const uint regular   = 0u;
const uint collision = 1u;
const uint hitbox    = 2u;
const uint hurtbox   = 3u;

void main()
{
    TexCoord = vTexCoord;
    vColor = vec4(1, 0, 1, 1);

    if (type == collision) {
        vColor = vec4(0, 0, 1, 0.7);
    }
    else if (type == hitbox) {

        vColor = vec4(0, 1, 0, 0.7);
    }
    else if ( type == hurtbox) {

        vColor = vec4(1, 0, 0, 0.7);
    }


    gl_PointSize = 10.0;
    gl_Position = projection * view * model * vec4(vPositionModelSpace, 1.0);
}
