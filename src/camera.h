#pragma once

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdio.h>

#include "game_memory.h"

struct Camera
{
    glm::vec3 pos;
    glm::vec3 target;
    glm::vec3 up;

    glm::mat4 view;
};

Camera *CreateCamera(GameMemory *gm);
inline Camera *CreateCamera(GameMemory *gm, v3 pos, v3 target, v3 up);
void CameraZoom(Camera *camera, f32 zoomAmount);
void CameraUpdateTarget(Camera *camera, glm::vec3 position);
void CameraUpdateTarget(Camera *camera, float yaw, float pitch);
