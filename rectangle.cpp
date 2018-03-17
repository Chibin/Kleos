#ifndef __RECTANGLE__
#define __RECTANGLE__

#define RECT_SIZE 36

struct Rect {
    Entity *entity;
    GLfloat *vertices;

    real32 width, height;

    v3 startingPosition;

    v3 topLeft;
    v3 topRight;
    v3 bottomLeft;
    v3 bottomRight;

    v4 color;

    v2 uvTextureTopLeft;
    v2 uvTextureTopRight;
    v2 uvTextureBottomLeft;
    v2 uvTextureBottomRight;

    int size;
};

static GLuint g_rectIndices[] = {  // Note that we start from 0!
    0, 1, 3,   // First Triangle
    1, 2, 3    // Second Triangle
};

GLfloat *CreateVertices(Rect *rect);

Rect *CreateRectangle(Entity *entity, v3 startingPosition, v4 color, real32 width, real32 height, bool isTraversable = true)
{
    /* NOTE: opengl default is ccw */

    Rect *tmp = NULL;
    tmp = (Rect*)calloc(1, sizeof(Rect));
    tmp->startingPosition = startingPosition;
    tmp->color = color;
    tmp->entity = entity;
    tmp->entity->position.x = startingPosition.x;
    tmp->entity->position.y = startingPosition.y;
    tmp->entity->position.z = startingPosition.z;
    tmp->entity->isTraversable = isTraversable;
    tmp->size = RECT_SIZE;

    tmp->width = width;
    tmp->height = height;

    /* FIXME: This is starting the drawing from the origin, but not centered at
     * the origin
     */
    tmp->topRight = v3{width, height, 0};
    tmp->bottomRight = v3{width, 0, 0};
    tmp->bottomLeft = v3{0,0,0};
    tmp->topLeft = v3{0, height, 0};

    tmp->vertices = CreateVertices(tmp);

    return tmp;
}

GLfloat *CreateDefaultRectangleVertices(){
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
     */

    GLfloat *vertices = NULL;
    vertices = (GLfloat*)malloc(sizeof(GLfloat) * 36 );

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

GLfloat *CreateVertices(Rect *rect)
{
    /* texCoords + verticesCoords + color*/
    GLfloat *vertices = NULL;
    vertices = (GLfloat*)malloc(sizeof(GLfloat) * RECT_SIZE);

    if (!vertices) {
        DEBUG_PRINT("Failed to malloc!");
    }

    vertices[0] = rect->topRight.x;
    vertices[1] = rect->topRight.y;
    vertices[2] = rect->topRight.z;
    vertices[3] = rect->color.r;
    vertices[4] = rect->color.g;
    vertices[5] = rect->color.b;
    vertices[6] = rect->color.a;

    vertices[7] = 1;
    vertices[8] = 1;

    vertices[9]  = rect->bottomRight.x;
    vertices[10] = rect->bottomRight.y;
    vertices[11] = rect->bottomRight.z;
    vertices[12] = rect->color.r;
    vertices[13] = rect->color.g;
    vertices[14] = rect->color.b;
    vertices[15] = rect->color.a;
    vertices[16] = 1;
    vertices[17] = 0;

    vertices[18] = rect->bottomLeft.x;
    vertices[19] = rect->bottomLeft.y;
    vertices[20] = rect->bottomLeft.z;
    vertices[21] = rect->color.r;
    vertices[22] = rect->color.g;
    vertices[23] = rect->color.b;
    vertices[24] = rect->color.a;
    vertices[25] = 0;
    vertices[26] = 0;

    vertices[27] = rect->topLeft.x;
    vertices[28] = rect->topLeft.y;
    vertices[29] = rect->topLeft.z;
    vertices[30] = rect->color.r;
    vertices[31] = rect->color.g;
    vertices[32] = rect->color.b;
    vertices[33] = rect->color.a;
    vertices[34] = 0;
    vertices[35] = 1;

    return vertices;
}
#endif
