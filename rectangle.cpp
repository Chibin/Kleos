#include "rectangle.h"
#include "game_memory.h"


Rect *CreateRectangle(GameMemory *gm, v3 startingPosition, v4 color,
                      real32 width, real32 height)
{
    Rect *tmp = nullptr;
    tmp = static_cast<Rect *>(AllocateMemory(gm, sizeof(Rect)));
    ZeroSize(tmp, sizeof(Rect));

    tmp->startingPosition = startingPosition;
    tmp->color = color;
    tmp->width = width;
    tmp->height = height;
    tmp->size = RECT_SIZE;

    /* FIXME: This is starting the drawing from the origin, but not centered at
     * the origin
     */
    v2 min = { startingPosition.x, startingPosition.y};
    v2 max = {startingPosition.x + width, startingPosition.y + height};
    tmp->topRight = v3{ max.x, max.y, 0 };
    tmp->bottomRight = v3{ max.x, min.y, 0 };
    tmp->bottomLeft = v3{ min.x, min.y, 0 };
    tmp->topLeft = v3{ min.x, max.y, 0 };

    tmp->minX = min.x;
    tmp->minY = min.y;
    tmp->maxX = max.x;
    tmp->maxY = max.y;

    CreateVertices(tmp);

    return tmp;
}

void AssociateEntity(Rect *rect, Entity *entity, bool isTraversable)
{
    rect->entity = entity;
    rect->entity->position.x = rect->startingPosition.x;
    rect->entity->position.y = rect->startingPosition.y;
    rect->entity->position.z = rect->startingPosition.z;
    rect->entity->isTraversable = isTraversable;
    rect->entity->width = rect->width;
    rect->entity->height = rect->height;
    rect->entity->data = rect->vertices;
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
    v3 normal = {0.0f, 0.0f, 0.0f};
    /* texCoords + verticesCoords + color*/
    Vertex *vTopRight = &(rect->vertices[0]);

    vTopRight->vPosition = rect->topRight;
    vTopRight->vColor = rect->color;
    vTopRight->vNormal = normal;
    vTopRight->vUv = v2{1, 1};

    Vertex *vBottomRight = &(rect->vertices[1]);
    vBottomRight->vPosition = rect->bottomRight;
    vBottomRight->vColor = rect->color;
    vBottomRight->vNormal = normal;
    vBottomRight->vUv = v2{1, 0};

    Vertex *vBottomLeft = &(rect->vertices[2]);
    vBottomLeft->vPosition = rect->bottomLeft;
    vBottomLeft->vColor = rect->color;
    vBottomLeft->vNormal = normal;
    vBottomLeft->vUv = v2{0, 0};

    Vertex *topLeft = &(rect->vertices[3]);
    topLeft->vPosition = rect->topLeft;
    topLeft->vColor = rect->color;
    topLeft->vNormal = normal;
    topLeft->vUv = v2{0, 1};
}
