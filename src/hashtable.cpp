#include "hashtable.h"

#define CREAT_SET_HASH_FUNCTION(T, TKey, TVal) \
    void SetHash(                              \
            T *v,                              \
            TKey key,                          \
            TVal value,                        \
            T *next)                           \
    {                                          \
        v->key = key;                          \
        v->val = value;                        \
        v->next = next;                        \
    }                                          \

void SetValue(VkDescriptorSet *value, VkDescriptorSet **o_value)
{
    *o_value = value;
}

void SetValue(Bitmap *value, Bitmap **o_value)
{
    *o_value = value;
}

void SetValue(Rect *value, Rect **o_value)
{
    *o_value = value;
}

void SetValue(Movement *value, Movement **o_value)
{
    *o_value = value;
}

void SetValue(NPC *value, NPC **o_value)
{
    *o_value = value;
}

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

CREAT_SET_HASH_FUNCTION(HashBitmapVkDescriptorSet, Bitmap *, VkDescriptorSet *);
CREAT_SET_HASH_FUNCTION(HashEntityRect, Entity *, Rect *);
CREAT_SET_HASH_FUNCTION(HashBitmapBitmap, Bitmap *, Bitmap *);
CREAT_SET_HASH_FUNCTION(HashEntityMovement, Entity *, Movement *);
CREAT_SET_HASH_FUNCTION(HashEntityNPC, Entity *, NPC *);

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

CREATE_HASH_FUNCTIONS(HashBitmapBitmap, Bitmap *, Bitmap *);
CREATE_HASH_GET_VALUE_FUCTION(HashBitmapBitmap, const char *, Bitmap *);
CREATE_HASH_GET_VALUE_FUCTION(HashBitmapBitmap, u32, Bitmap *);
