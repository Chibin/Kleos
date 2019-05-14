#ifndef __CAMERA__
#define __CAMERA__

#include "camera.h"

#define USE_GLM_LOOKAT 0

inline glm::mat4 LookAt(glm::vec3 from, glm::vec3 to)
{
    glm::mat4 result = glm::mat4(1.0f);
    glm::vec3 forward = glm::normalize(from - to);
    /* glm::vec3(0, 1, 0) is arbitrary.
     * We need an arbitrary vector to find the right vector orthogonal to the
     * forward vector.
     */
    glm::vec3 right = glm::cross(glm::normalize(glm::vec3(0, 1, 0)), forward);
    glm::vec3 up = glm::cross(forward, right);
    /*
     * Position = from;
     * [ R 0
     *   U 0
     *   F 0
     *   P 1 ]
     */
    result[0][0] = right.x;
    result[0][1] = right.y;
    result[0][2] = right.z;
    result[1][0] = up.x;
    result[1][1] = up.y;
    result[1][2] = up.z;
    result[2][0] = forward.x;
    result[2][1] = forward.y;
    result[2][2] = forward.z;
    result[3][0] = -from.x;
    result[3][1] = -from.y;
    result[3][2] = -from.z;
    result[3][3] = 1;

    return result;
}

inline Camera *CreateCamera(GameMemory *gm, v3 pos, v3 target, v3 up)
{
    auto *camera = static_cast<Camera *>(AllocateMemory(gm, (sizeof(Camera))));

    // Camera is at, (0,0,0) in World Space
    // eye
    camera->pos = glm::vec3(pos.x, pos.y, pos.z);

    // and looks at the origin
    // center
    camera->target = glm::vec3(target.x, target.y, target.z);

    // Head is up (set to 0,-1,0 to look upside-down)
    camera->up = glm::vec3(up.x, up.y, up.z);

    /*
     * v3 Right, v3 Up, v3 Forward, v3 Position
     * View matrix:
     * [ R U F P ]
     * [ 0 0 0 1 ]
     */
#if USE_GLM_LOOKAT
    camera->view = glm::lookAt(camera->pos, camera->target, camera->up);
#else
    glm::vec3 from = camera->pos;
    glm::vec3 to = camera->target;

    camera->view = LookAt(from, to);
#endif
    return camera;
}

Camera *CreateCamera(GameMemory *gm)
{
    // Camera is at, (0,0,0) in World Space
    v3 pos = { 0, 0, 5 };
    // and looks at the origin
    v3 target = { 0, 0, 0 };
    // Head is up (set to 0,1,0 to look upside-down)
    v3 up = { 0, 0, 1 };
    return CreateCamera(gm, pos, target, up);
}

void CameraZoom(Camera *camera, f32 zoomAmount)
{
    if ((camera->pos[2] + zoomAmount) < 0)
    {
        printf("can no longer zoom out reached max zoom!\n"
               "current camera z-axis value: %f\n",
               camera->pos[2]);
        return;
    }

    camera->pos += glm::vec3(0, 0, zoomAmount);

#if USE_GLM_LOOKAT
    camera->view = glm::lookAt(camera->pos, camera->target, camera->up);
#else
    glm::vec3 from = camera->pos;
    glm::vec3 to = camera->target;

    camera->view = LookAt(from, to);
#endif
}

void CameraUpdateTarget(Camera *camera, glm::vec3 position)
{
    /* we have to update the position and target in order to keep the camera
     * leveled on the x-axis.
     */

    camera->target = glm::vec3(position.x, position.y, 0);
    camera->pos.x = position.x;
    camera->pos.y = position.y;
    /* Intentionally -- at the moment -- not copying the z-axis to keep the
     * camera functionality working.
     */

#if USE_GLM_LOOKAT
    camera->view = glm::lookAt(camera->pos, camera->target, camera->up);
#else
    glm::vec3 from = camera->pos;
    glm::vec3 to = camera->target;

    camera->view = LookAt(from, to);
#endif
}

void CameraUpdateTarget(Camera *camera, float yaw, float pitch)
{
    glm::vec3 cameraTarget;
    cameraTarget.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraTarget.y = sin(glm::radians(pitch));
    cameraTarget.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    camera->target = glm::normalize(cameraTarget);
}

#endif
