#pragma once

#include <glm/glm.hpp>

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
