#ifndef __RECTANGLE__
#define __RECTANGLE__

struct Rect {
    Entity entity;

    real32 width, height;

    v3 topLeft;
    v3 topRight;
    v3 bottomLeft;
    v3 bottomRight;
};

Rect *CreateRectangle(v3 startingPosition, real32 width, real32 height)
{
    /* NOTE: opengl default is ccw */

    Rect *tmp = NULL;
    tmp = (Rect*)calloc(1, sizeof(Rect));
    tmp->entity.position = glm::vec3(startingPosition.x,
                                     startingPosition.y,
                                     startingPosition.z);
    tmp->width = width;
    tmp->height = height;

    tmp->bottomLeft = startingPosition;
    tmp->topLeft = startingPosition + v3{0, height, 0};
    tmp->bottomRight = startingPosition + v3{width, 0, 0};
    tmp->topRight = startingPosition + v3{width, height, 0};
    
    return tmp;
}
#endif
