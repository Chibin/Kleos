#ifndef __ENTITY_MANAGER__
#define __ENTITY_MANAGER__

struct EntityManager {
    /* TODO: We're currently assuming that no entities will be removed,
     * may be later on implement something that will detect things that we
     * remove.
     * may be have an array of removed indexes?! or clean it up such that it's
     * NULL?
     *
     * Static entites vs non-static?
     * How about static interactable entities?
     */
    Entity *entities;
    unsigned int size;

    unsigned int totalAllocatedSpace;

    /* A dictionary/collection of all the entity types?
     * or
     * entity[type_rect] = [array of rects]
     * entities[type_rect][<dictionary> ID] = rect
     */

    /* not sure how to properly use this yet */
    unsigned int *entityIDs;
};

void _allocateMoreMemory(EntityManager *em);

EntityManager* CreateEntityManger()
{
    EntityManager *em = (EntityManager*)malloc(sizeof(EntityManager));
    if (!em) {
        PAUSE_HERE("entity manger is NULL? %s\n", __func__);
    }

    em->size = 0;
    em->entityIDs = NULL;
    em->totalAllocatedSpace = 10; /* arbitrary amount */

    /* TODO: not sure if we should zero the values */
    em->entities = (Entity*)malloc(sizeof(Entity) * em->totalAllocatedSpace);
    em->entities[0].position = glm::vec3(0,0,0);
    memset(em->entities, 0, sizeof(Entity) * em->totalAllocatedSpace);

    /* TODO: This should really be an assert */
    if (!em->entities) {
        PAUSE_HERE("Something bad happend! %s:%d\n", __func__, __LINE__);
    }

    return em;
}

Entity *AddNewEntity(EntityManager *em)
{
    /* Creates a new entity that's zeroed out */
    if (em->totalAllocatedSpace < em->size)
        _allocateMoreMemory(em);

    Entity *entity = &(em->entities[em->size]);

    entity->id = em->size;
    entity->position = glm::vec3(0, 0, 0);

    em->size++;
    return entity;
}

int Append(EntityManager *em, Entity *entity)
{
    /* Returns the index of the last appended value.
     * Using append will assume that you want to copy the data.
     * grow the array size by a quarter of the previous size if we don't have
     * space???
     */
    if (em->totalAllocatedSpace < em->size)
        _allocateMoreMemory(em);

    memcpy(&(em->entities[em->size]), entity, sizeof(Entity));
    em->size++;

    return em->size-1;
}

/* helper functions */
void _allocateMoreMemory(EntityManager *em) {
    em->totalAllocatedSpace = FLOOR(em->totalAllocatedSpace * 0.25);
    Entity *entities = (Entity*)malloc(sizeof(Entity) * em->totalAllocatedSpace);
    memset(em->entities, 0, sizeof(Entity) * em->totalAllocatedSpace);

    if (!entities || !em->entities) {
        PAUSE_HERE("Something bad happend! %s:%d\n", __func__, __LINE__);
    }

    memcpy(entities, em->entities, sizeof(Entity) * em->size);
    free(em->entities);
    em->entities = entities;
}

#endif
