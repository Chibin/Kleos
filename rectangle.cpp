#ifndef __RECTANGLE__
#define __RECTANGLE__

struct rectangle {
    entity Entity;

    real32 width, height;
    v3 topLeft;
    v3 topRight;
    v3 bottomLeft;
    v3 bottomRight;
};

rectangle *CreateRectangle(v3 startingPosition, real32 width, real32 height)
{
    /* NOTE: opengl default is ccw */

    rectangle *tmp = NULL;
    tmp = (rectangle*)calloc(1, sizeof(rectangle));
    tmp->Entity.position = glm::vec3(startingPosition.x,
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
