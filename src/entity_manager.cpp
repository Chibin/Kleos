#ifndef __ENTITY_MANAGER__
#define __ENTITY_MANAGER__

#include "entity.h"
#include "game_memory.h"
#include "game_time.h"
#include "logger.h"

struct EntityManager
{
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
    u32 size;

    u32 totalAllocatedSpace;

    /* A dictionary/collection of all the entity types?
     * or
     * entity[type_rect] = [array of rects]
     * entities[type_rect][<dictionary> ID] = rect
     */

    /* not sure how to properly use this yet */
    u32 *entityIDs;
};

void _allocateMoreMemory(GameMemory *gm, EntityManager *em);

EntityManager *CreateEntityManger(GameMemory *gm)
{
    START_DEBUG_TIMING();

    auto *em = (EntityManager *)(AllocateMemory(gm, (sizeof(EntityManager))));

    ASSERT(em != nullptr);

    em->size = 0;
    em->entityIDs = nullptr;
    em->totalAllocatedSpace = 100; /* arbitrary amount */

    /* TODO: not sure if we should zero the values */
    em->entities =
        static_cast<Entity *>(AllocateMemory(gm, (sizeof(Entity) * em->totalAllocatedSpace)));

    /* probably don't need this */
    //memset(em->entities, 0, sizeof(Entity) * em->totalAllocatedSpace);

    /* TODO: This should really be an assert */
    ASSERT(em->entities != nullptr);

    END_DEBUG_TIMING();

    return em;
}

Entity *AddNewEntity(GameMemory *gm, EntityManager *em, v3 position = v3{ 0, 0, 0 })
{
    /* Creates a new entity that's zeroed out */
    if (em->totalAllocatedSpace <= em->size)
    {
        _allocateMoreMemory(gm, em);
    }

    Entity *entity = &(em->entities[em->size]);

    entity->id = em->size;
    entity->movement.position = glm::vec3(position.x, position.y, position.z);

    em->size++;
    return entity;
}

int Append(GameMemory *gm, EntityManager *em, Entity *entity)
{
    /* Returns the index of the last appended value.
     * Using append will assume that you want to copy the data.
     * grow the array size by a quarter of the previous size if we don't have
     * space???
     */
    if (em->totalAllocatedSpace < em->size)
    {
        _allocateMoreMemory(gm, em);
    }

    entity->id = em->size;
    memcpy(&(em->entities[em->size]), entity, sizeof(Entity));
    em->size++;

    return em->size - 1;
}

#pragma warning(push)
#pragma warning(disable : 4100)
void _freeMemory(EntityManager *em)
{
    /* TODO: This is a bit complicated since everything is done as a stack...
     * We can't retrieve back the "Freed" memory becausee it's going to be in
     * the middle of the stack.
     * This would count as a memory leak, unless we make our memory management
     * better.
     */
     //free(em->entities); // we can't do this! and should be done through our memory manager.
}
#pragma warning(pop)

/* helper functions */
void _allocateMoreMemory(GameMemory *gm, EntityManager *em)
{
    /* TODO: This is broken. freeing the memory is not a good idea because
     * there might be pointers pointed to the old memory that we plan on
     * deleting.
     */

    u32 newTotalAllocatedSpace =
        em->totalAllocatedSpace + FLOOR(em->totalAllocatedSpace * 0.25);

    auto *entities =
        static_cast<Entity *>(AllocateMemory(gm, (sizeof(Entity) * newTotalAllocatedSpace)));
    //(Entity *)ReallocMemory(gm, em->entities, (sizeof(Entity) * newTotalAllocatedSpace));

    if (entities == nullptr)
    {
        PAUSE_HERE("Something bad happend! %s:%d\n", __func__, __LINE__);
    }

    memcpy(entities, em->entities, sizeof(Entity) * em->totalAllocatedSpace);
    /* TODO: Free memory */
    _freeMemory(em);

    em->totalAllocatedSpace = newTotalAllocatedSpace;
    em->entities = entities;
}

struct EntityDynamicArray
{
    Entity *firstEntity;
    Entity *nextEntity;
    Entity *lastEntity;

    int size;
    int allocatedSize; /*not in bytes but amount of entities */
    Entity **entities;

    /* should probably be in the entity manager */
    int *indexOfEmptySlots;
};

EntityDynamicArray *CreateEntityDynamicArray(GameMemory *gm)
{
    auto *eda =
        static_cast<EntityDynamicArray *>(AllocateMemory(gm, (sizeof(EntityDynamicArray))));
    memset(eda, 0, sizeof(EntityDynamicArray));
    eda->allocatedSize = 20000;
    eda->entities =
        static_cast<Entity **>(AllocateMemory(gm, (sizeof(Entity *) * eda->allocatedSize)));

    return eda;
}

void DeleteEntityDynamicArray(EntityDynamicArray *eda)
{
    if (eda == nullptr)
    {
        return;
    }

    memset(eda->entities, 0, sizeof(Entity *) * eda->allocatedSize);
    eda->size = 0;
}

void PushBack(GameMemory *gm, EntityDynamicArray *eda, Entity *entity)
{
    float expansionRate = 0.25;

    if (eda->allocatedSize <= eda->size)
    {
        uint32 newTotalSpace =
            eda->allocatedSize +
            static_cast<unsigned int>(eda->allocatedSize * expansionRate);

        Entity *tmp = *eda->entities;
        unsigned int oldSize = eda->size;

        *eda->entities =
            static_cast<Entity *>(AllocateMemory(gm, (sizeof(Entity *) * newTotalSpace)));
        memset(*eda->entities, 0, sizeof(Entity) * newTotalSpace);
        memcpy(*eda->entities, tmp, sizeof(Entity) * oldSize);

        free(tmp);

        eda->allocatedSize = newTotalSpace;
    }

    eda->entities[eda->size] = entity;
    eda->size++;
}

/* TODO: Realloc */

#endif
