#ifndef __RECTANGLE__
#define __RECTANGLE__

struct Rect {
    Entity *entity;
    GLfloat *vertices;

    real32 width, height;

    v3 startingPosition;

    v3 topLeft;
    v3 topRight;
    v3 bottomLeft;
    v3 bottomRight;

    v2 uvTextureTopLeft;
    v2 uvTextureTopRight;
    v2 uvTextureBottomLeft;
    v2 uvTextureBottomRight;

};

GLfloat *CreateVertices(Rect *rect);

Rect *CreateRectangle(Entity *entity, v3 startingPosition, real32 width, real32 height)
{
    /* NOTE: opengl default is ccw */

    Rect *tmp = NULL;
    tmp = (Rect*)calloc(1, sizeof(Rect));
    tmp->startingPosition = startingPosition;
    tmp->entity = entity;
    tmp->entity->position.x = startingPosition.x;
    tmp->entity->position.y = startingPosition.y;
    tmp->entity->position.z = startingPosition.z;

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
    vertices = (GLfloat*)malloc(sizeof(GLfloat) * 12 );

    vertices[0] = 1;
    vertices[1] = 1;
    vertices[2] = 0;
    vertices[3] = 1;
    vertices[4] = 1;

    vertices[5] = 1;
    vertices[6] = 0;
    vertices[7] = 0;
    vertices[8] = 1;
    vertices[9] = 0;

    vertices[10] = 0;
    vertices[11] = 0;
    vertices[12] = 0;
    vertices[13] = 0;
    vertices[14] = 0;

    vertices[15] = 0;
    vertices[16] = 1;
    vertices[17] = 0;
    vertices[18] = 0;
    vertices[19] = 1;

    return vertices;
}

GLfloat *CreateVertices(Rect *rect)
{
    /* texCoords + verticesCoords */
    int totalSize = 20;

    GLfloat *vertices = NULL;
    vertices = (GLfloat*)malloc(sizeof(GLfloat) * totalSize);

    if (!vertices) {
        DEBUG_PRINT("Failed to malloc!");
    }
    
    vertices[0] = rect->topRight.x;
    vertices[1] = rect->topRight.y;
    vertices[2] = rect->topRight.z;
    vertices[3] = 1;
    vertices[4] = 1;

    vertices[5] = rect->bottomRight.x;
    vertices[6] = rect->bottomRight.y;
    vertices[7] = rect->bottomRight.z;
    vertices[8] = 1;
    vertices[9] = 0;

    vertices[10] = rect->bottomLeft.x;
    vertices[11] = rect->bottomLeft.y;
    vertices[12] = rect->bottomLeft.z;
    vertices[13] = 0;
    vertices[14] = 0;

    vertices[15] = rect->topLeft.x;
    vertices[16] = rect->topLeft.y;
    vertices[17] = rect->topLeft.z;
    vertices[18] = 0;
    vertices[19] = 1;

    rect->topLeft;

    return vertices;
}
#endif
