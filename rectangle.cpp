#include "rectangle.h"
#include "game_memory.h"

Rect *CreateRectangle(GameMemory *gm, Entity *entity, v3 startingPosition, v4 color,
                      real32 width, real32 height, bool isTraversable)
{
    /* NOTE: opengl default is ccw */

    Rect *tmp = nullptr;
    tmp = static_cast<Rect *>(AllocateMemory(gm, (1, sizeof(Rect))));
    ZeroSize(tmp, sizeof(Rect));
    tmp->startingPosition = startingPosition;
    tmp->color = color;
    tmp->entity = entity;
    tmp->entity->position.x = startingPosition.x;
    tmp->entity->position.y = startingPosition.y;
    tmp->entity->position.z = startingPosition.z;
    tmp->entity->isTraversable = isTraversable;
    tmp->size = RECT_SIZE;

    tmp->entity->width = width;
    tmp->entity->height = height;
    tmp->width = width;
    tmp->height = height;

    /* FIXME: This is starting the drawing from the origin, but not centered at
     * the origin
     */
    tmp->topRight = v3{ width, height, 0 };
    tmp->bottomRight = v3{ width, 0, 0 };
    tmp->bottomLeft = v3{ 0, 0, 0 };
    tmp->topLeft = v3{ 0, height, 0 };

    tmp->minX = startingPosition.x;
    tmp->minY = startingPosition.y;
    tmp->maxX = startingPosition.x + width;
    tmp->maxY = startingPosition.y + height;

    CreateVertices(tmp);
    tmp->entity->data = tmp->vertices;

    return tmp;
}

GLfloat *CreateDefaultRectangleVertices(GameMemory *gm)
{
    /*
     *   VertexCoords
     *   1,  1, 0,  Top Right
     *   1,  0, 0,  Bottom Right
     *   0,  0, 0,  Bottom Left
     *   0,  1, 0,  Top Left
     *
     *   TextureCoords
     *   1, 1,      Top Right
     *   1, 0,      Bottom Right
     *   0, 0,      Bottom Left
     *   0, 1       Top Left
     *
     *   Texture flipped
     *   1, 0,
     *   1, 1,
     *   0, 1,
     *   0, 0
     */

    GLfloat *vertices = nullptr;
    vertices = static_cast<GLfloat *>(AllocateMemory(gm, (sizeof(GLfloat) * 36)));

    vertices[0] = 1;
    vertices[1] = 1;
    vertices[2] = 0;

    vertices[3] = 0;
    vertices[4] = 0;
    vertices[5] = 0;
    vertices[6] = 0;

    vertices[7] = 1;
    vertices[8] = 1;

    vertices[9] = 1;
    vertices[10] = 0;
    vertices[11] = 0;

    vertices[12] = 0;
    vertices[13] = 0;
    vertices[14] = 0;
    vertices[15] = 0;

    vertices[16] = 1;
    vertices[17] = 0;

    vertices[18] = 0;
    vertices[19] = 0;
    vertices[20] = 0;

    vertices[21] = 0;
    vertices[22] = 0;
    vertices[23] = 0;
    vertices[24] = 0;

    vertices[25] = 0;
    vertices[26] = 0;

    vertices[27] = 0;
    vertices[28] = 1;
    vertices[29] = 0;

    vertices[30] = 0;
    vertices[31] = 0;
    vertices[32] = 0;
    vertices[33] = 0;

    vertices[34] = 0;
    vertices[35] = 1;

    return vertices;
}

void CreateVertices(Rect *rect)
{
    /* texCoords + verticesCoords + color*/
    Vertex *vTopRight = &(rect->vertices[0]);

    vTopRight->position[0] = rect->topRight.x;
    vTopRight->position[1] = rect->topRight.y;
    vTopRight->position[2] = rect->topRight.z;
    vTopRight->color[0] = rect->color.r;
    vTopRight->color[1] = rect->color.g;
    vTopRight->color[2] = rect->color.b;
    vTopRight->color[3] = rect->color.a;

    vTopRight->normal[0] = 0;
    vTopRight->normal[1] = 0;
    vTopRight->normal[2] = 0;

    if (rect->isTextureUpsideDown)
    {
        vTopRight->uv[0] = 1;
        vTopRight->uv[1] = 0;
    }
    else
    {
        vTopRight->uv[0] = 1;
        vTopRight->uv[1] = 1;
    }

    Vertex *vBottomRight = &(rect->vertices[1]);
    vBottomRight->position[0] = rect->bottomRight.x;
    vBottomRight->position[1] = rect->bottomRight.y;
    vBottomRight->position[2] = rect->bottomRight.z;
    vBottomRight->color[0] = rect->color.r;
    vBottomRight->color[1] = rect->color.g;
    vBottomRight->color[2] = rect->color.b;
    vBottomRight->color[3] = rect->color.a;
    vBottomRight->normal[0] = 0;
    vBottomRight->normal[1] = 0;
    vBottomRight->normal[2] = 0;

    if (rect->isTextureUpsideDown)
    {
        vBottomRight->uv[0] = 1;
        vBottomRight->uv[1] = 1;
    }
    else
    {
        vBottomRight->uv[0] = 1;
        vBottomRight->uv[1] = 0;
    }

    Vertex *vBottomLeft = &(rect->vertices[2]);
    vBottomLeft->position[0] = rect->bottomLeft.x;
    vBottomLeft->position[1] = rect->bottomLeft.y;
    vBottomLeft->position[2] = rect->bottomLeft.z;
    vBottomLeft->color[0] = rect->color.r;
    vBottomLeft->color[1] = rect->color.g;
    vBottomLeft->color[2] = rect->color.b;
    vBottomLeft->color[3] = rect->color.a;
    vBottomLeft->normal[0] = 0;
    vBottomLeft->normal[1] = 0;
    vBottomLeft->normal[2] = 0;

    if (rect->isTextureUpsideDown)
    {
        vBottomLeft->uv[0] = 0;
        vBottomLeft->uv[1] = 1;
    }
    else
    {
        vBottomLeft->uv[0] = 0;
        vBottomLeft->uv[1] = 0;
    }

    Vertex *topLeft = &(rect->vertices[3]);

    topLeft->position[0] = rect->topLeft.x;
    topLeft->position[1] = rect->topLeft.y;
    topLeft->position[2] = rect->topLeft.z;
    topLeft->color[0] = rect->color.r;
    topLeft->color[1] = rect->color.g;
    topLeft->color[2] = rect->color.b;
    topLeft->color[3] = rect->color.a;
    topLeft->normal[0] = rect->topLeft.x;
    topLeft->normal[1] = rect->topLeft.y;
    topLeft->normal[2] = rect->topLeft.z;

    if (rect->isTextureUpsideDown)
    {
        topLeft->uv[0] = 0;
        topLeft->uv[1] = 0;
    }
    else
    {
        topLeft->uv[0] = 0;
        topLeft->uv[1] = 1;
    }
}
