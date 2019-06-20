#include "hashtable.h"

SET_VALUE(VkDescriptorSet *);
SET_VALUE(Bitmap *);
SET_VALUE(Entity *);
SET_VALUE(Rect *);
SET_VALUE(Movement *);
SET_VALUE(NPC *);
SET_VALUE(UIInfo *);
SET_VALUE(Stat *);

b32 KeyCompare(Bitmap *a, Bitmap *b)
{
    return a->bitmapID == b->bitmapID;
}

b32 KeyCompare(Bitmap *a, u32 b)
{
    return a->bitmapID == b;
}

b32 KeyCompare(Bitmap *a, const char *b)
{
    return strcmp(a->name, b) == 0;
}

b32 KeyCompare(Entity *a, Entity *b)
{
    return a->id == b->id;
}

memory_index KeyToHashIndex(Hash *hash, Bitmap *key)
{
    return key->bitmapID % hash->bucketCount;
}

memory_index KeyToHashIndex(Hash *hash, Entity *key)
{
    return key->id % hash->bucketCount;
}

CREATE_HASH_FUNCTIONS(HashBitmapVkDescriptorSet, Bitmap *, VkDescriptorSet *);
CREATE_HASH_FUNCTIONS(HashEntityRect, Entity *, Rect *);
CREATE_HASH_FUNCTIONS(HashEntityMovement, Entity *, Movement *);
CREATE_HASH_FUNCTIONS(HashEntityNPC, Entity *, NPC *);
CREATE_HASH_FUNCTIONS(HashSetEntity, Entity *, Entity *);
CREATE_HASH_FUNCTIONS(HashCharUIInfo, const char *, UIInfo *);
CREATE_HASH_FUNCTIONS(HashEntityStat, Entity *, Stat *);
CREATE_HASH_FUNCTIONS(HashBitmapBitmap, Bitmap *, Bitmap *);

CREATE_HASH_GET_VALUE_FUCTION(HashBitmapBitmap, const char *, Bitmap *);
CREATE_HASH_GET_VALUE_FUCTION(HashBitmapBitmap, u32, Bitmap *);

/* Entity */
Entity *CreateAndAddNewEntity(GameMetadata *gameMetadata, GameMemory *gameMemory)
{
    Entity *newEntity = (Entity *)AllocateMemory0(gameMemory, sizeof(Entity));
    newEntity->id = gameMetadata->entityID++;
    HashAdd(gameMetadata->hashSetEntity, newEntity, newEntity);
    return newEntity;
}

CREATE_HASH_DELETE_FUCTION(HashSetEntity)
{
    return;
}

/* NPC */
CREATE_HASH_DELETE_FUCTION(HashEntityNPC)
{
    return;
}

CREATE_HASH_DELETE_FUCTION(HashEntityMovement)
{
    return;
}

CREATE_HASH_DELETE_FUCTION(HashEntityStat)
{
    return;
}
