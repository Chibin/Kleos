#ifndef __ENTITY__
#define __ENTITY__

struct stats {
    int movementSpeed;
};

/* TODO: how do we treat dynamically moving entities vs static ones? */
struct Entity {
    unsigned int id;

    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;

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

inline
void EntityMoveUp(Entity *entity) {
    /* TODO: Replace this later with speed (time based)*/
    entity->acceleration += glm::vec3(0, 10, 0);
    entity->velocity += glm::vec3(0, 15, 0);
}

inline
void EntityMoveDown(Entity *entity) {
    entity->velocity += glm::vec3(0, -0.01, 0);
}

inline
void EntityMoveLeft(Entity *entity) {
    entity->velocity += glm::vec3(-0.01, 0, 0);
}

inline
void EntityMoveRight(Entity *entity) {
    entity->velocity += glm::vec3(0.01, 0, 0);
}

Entity *GetNonTraversableEntities(uint32 *out_NumOfNTs)
{
    *out_NumOfNTs = 0;
    return NULL;
}

#endif
