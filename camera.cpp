#include "camera.h"
#include <GL/glew.h>
#include <cstdio>
#include <glm/gtc/matrix_transform.hpp>

Camera *CreateCamera()
{
    auto *camera = static_cast<Camera *>(malloc(sizeof(Camera)));

    // Camera is at, (0,0,0) in World Space
    camera->pos = glm::vec3(0, 0, 10);

    // and looks at the origin
    camera->target = glm::vec3(0, 0, 0);

    // Head is up (set to 0,-1,0 to look upside-down)
    camera->up = glm::vec3(0, 1, 0);

    camera->view = glm::lookAt(camera->pos, camera->target, camera->up);
    return camera;
}

void CameraZoomOut(Camera *camera)
{
    printf("down! zooming out\n");

    GLfloat zoomAmount = 4;
    camera->pos += glm::vec3(0, 0, zoomAmount);

    camera->view = glm::lookAt(camera->pos, camera->target, camera->up);
}

void CameraZoomIn(Camera *camera)
{
    printf("up! zooming in\n");
    GLfloat zoomAmount = 4;

    if ((camera->pos[2] - zoomAmount) < 0)
    {
        printf("can no longer zoom out reached max zoom!\n"
               "current camera z-axis value: %f\n",
               camera->pos[2]);
        return;
    }

    camera->pos += glm::vec3(0, -0, -zoomAmount);

    camera->view = glm::lookAt(camera->pos, camera->target, camera->up);
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

    camera->view = glm::lookAt(camera->pos, camera->target, camera->up);
}

void CameraUpdateTarget(Camera *camera, float yaw, float pitch)
{
    glm::vec3 cameraTarget;
    cameraTarget.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraTarget.y = sin(glm::radians(pitch));
    cameraTarget.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    camera->target = glm::normalize(cameraTarget);
}
