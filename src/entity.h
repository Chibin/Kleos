#ifndef __ENTITY__
#define __ENTITY__

#include "math.h"
#include "opengl.h"
#include <glm/glm.hpp>

struct stats
{
    int movementSpeed;
};

struct FrameData
{
    u32 duration;
    v2 dim;
    /* This is relative to your model not the world*/
    v3 pos;

    FrameData *next;
};

struct FrameState
{
    u32 timePassedCurrentFrame;
    glm::mat4 transform;

    FrameData *startFrame;
    FrameData *currentFrame;
};

/* TODO: how do we treat dynamically moving entities vs static ones? */
struct Entity
{
    memory_index id;

    f32 width;
    f32 height;

    f32 minX;
    f32 maxX;
    Vertex *data;

    b32 willAttack;
    FrameState frameState;

    /* TODO: entity may or may not have stats ...
     * how do we deal with this?
     * may be.. when creating an entity, we use the entity id as a key to store
     * some stat value if the entity needs it.
     */
    bool isTraversable;
    bool isPlayer;
};

inline void EntityMoveAttack(Entity *entity)
{
    entity->willAttack = true;
}

Entity *GetNonTraversableEntities(uint32 *out_NumOfNTs);

#endif
