#pragma once

#include <glm/glm.hpp>
#include "game_memory.h"

struct Camera
{
    glm::vec3 pos;
    glm::vec3 target;
    glm::vec3 up;

    glm::mat4 view;
};

Camera *CreateCamera();
void CameraZoomOut(Camera *camera);
void CameraZoomIn(Camera *camera);
void CameraUpdateTarget(Camera *camera, glm::vec3 position);
void CameraUpdateTarget(Camera *camera, float yaw, float pitch);
inline Camera *CreateCamera(GameMemory *gm, v3 pos, v3 target, v3 up);
