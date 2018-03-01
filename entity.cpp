#ifndef __ENTITY__
#define __ENTITY__

struct stats {
    int movementSpeed;
};

struct entity {
    unsigned int id;
    glm::vec3 position;
    /* TODO: entity may or may not have stats ...
     * how do we deal with this?
     * may be.. when creating an entity, we use the entity id as a key to store
     * some stat value if the entity needs it.
     */
};

inline
void EntityMoveUp(entity *Entity) {
    /* TODO: Replace this later with speed */
    Entity->position += glm::vec3(0, 0.1, 0);
}

inline
void EntityMoveDown(entity *Entity) {
    Entity->position += glm::vec3(0, -0.1, 0);
}

inline
void EntityMoveLeft(entity *Entity) {
    Entity->position += glm::vec3(-0.1, 0, 0);
}

inline
void EntityMoveRight(entity *Entity) {
    Entity->position += glm::vec3(0.1, 0, 0);
}
#endif
