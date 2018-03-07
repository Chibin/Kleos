#ifndef __ENTITY__
#define __ENTITY__

struct stats {
    int movementSpeed;
};

/* TODO: how do we treat dynamically moving entities vs static ones? */
struct Entity {
    unsigned int id;
    glm::vec3 position;
    /* TODO: entity may or may not have stats ...
     * how do we deal with this?
     * may be.. when creating an entity, we use the entity id as a key to store
     * some stat value if the entity needs it.
     */
};

inline
void EntityMoveUp(Entity *entity) {
    /* TODO: Replace this later with speed */
    entity->position += glm::vec3(0, 0.1, 0);
}

inline
void EntityMoveDown(Entity *entity) {
    entity->position += glm::vec3(0, -0.1, 0);
}

inline
void EntityMoveLeft(Entity *entity) {
    entity->position += glm::vec3(-0.1, 0, 0);
}

inline
void EntityMoveRight(Entity *entity) {
    entity->position += glm::vec3(0.1, 0, 0);
}
#endif
