#ifndef __CAMERA__
#define __CAMERA__

struct Camera {
    glm::vec3 pos;
    glm::vec3 target;
    glm::vec3 up;

    glm::mat4 view;
};


Camera *CreateCamera()
{
    Camera *camera = (Camera*)malloc(sizeof(Camera));

    // Camera is at, (0,0,0) in World Space
    camera->pos = glm::vec3(0, 0, 10);

    // and looks at the origin
    camera->target = glm::vec3(0, 0, 0);

    // Head is up (set to 0,-1,0 to look upside-down)
    camera->up = glm::vec3(0, 1, 0); 

    camera->view = glm::lookAt(
            camera->pos,
            camera->target,
            camera->up);
    return camera;
}

void CameraZoomOut(Camera *camera)
{
    printf("down! zooming out\n");

    camera->pos += glm::vec3(0, 0, 1);

    camera->view = glm::lookAt(
            camera->pos,
            camera->target,
            camera->up);
}

void CameraZoomIn(Camera *camera)
{
    printf("up! zooming in\n");

    if ( (camera->pos[2] - 1) <  0.1) {
        printf("can no longer zoom out reached max zoom!\n"
               "current camera z-axis value: %f\n", camera->pos[2]);
        return;
    }

    camera->pos += glm::vec3(0, -0, -1);

    camera->view = glm::lookAt(
            camera->pos,
            camera->target,
            camera->up);
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
