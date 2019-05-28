#ifndef __ENTITY__
#define __ENTITY__

#include "math.h"
#include "opengl.h"
#include <glm/glm.hpp>

struct Movement {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
};

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

    Movement movement;
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
    /* not sure if we'll use this */
    void (*draw)();
};

inline void EntityMoveUp(Entity *entity)
{
    /* TODO: Replace this later with speed (time based)*/
    entity->movement.acceleration = glm::vec3(0, 9.81, 0);
    entity->movement.velocity = glm::vec3(0, 15, 0);
}

inline void EntityMoveDown(Entity *entity)
{
    entity->movement.velocity += glm::vec3(0, -0.01, 0);
}

inline void EntityMoveLeft(Entity *entity)
{
    entity->movement.velocity.x += -5.0f;
}

inline void EntityMoveRight(Entity *entity)
{
    entity->movement.velocity.x += 5.0f;
}

inline void EntityMoveAttack(Entity *entity)
{
    entity->willAttack = true;
}

Entity *GetNonTraversableEntities(uint32 *out_NumOfNTs);

#endif
