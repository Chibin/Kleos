#ifndef __ENTITY__
#define __ENTITY__

#include "math.h"
#include "opengl.h"
#include <glm/glm.hpp>

struct stats
{
    int movementSpeed;
};

/* TODO: how do we treat dynamically moving entities vs static ones? */
struct Entity
{
    unsigned int id;

    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    real32 width;
    real32 height;

    int type;
    real32 minX;
    real32 maxX;
    Vertex *data;

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
    entity->acceleration = glm::vec3(0, 9.81, 0);
    entity->velocity = glm::vec3(0, 15, 0);
}

inline void EntityMoveDown(Entity *entity)
{
    entity->velocity += glm::vec3(0, -0.01, 0);
}

inline void EntityMoveLeft(Entity *entity)
{
    entity->velocity.x += -5.0f;
}

inline void EntityMoveRight(Entity *entity)
{
    entity->velocity.x += 5.0f;
}

Entity *GetNonTraversableEntities(uint32 *out_NumOfNTs);

#endif
