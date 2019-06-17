#ifndef __HASHTABLE__
#define __HASHTABLE__

#include "hash.h"
#include "entity.h"
#include "rectangle.h"
#include "npc.h"
#include "bitmap.h"
#include "movement.h"
#include "stats.h"
#include "ui.h"
#include "renderer/vulkan/vulkan.h"

#define MAX_HASH 17

#define CREATE_SET_HASH_FUNCTION(T, TKey, TVal)   \
    void SetHash(                                 \
            T *v,                                 \
            TKey key,                             \
            TVal value,                           \
            T *next)                              \
    {                                             \
        v->key = key;                             \
        v->val = value;                           \
        v->next = next;                           \
    }                                             \

#define SET_VALUE(T)                              \
void SetValue(T value, T *o_value)                \
{                                                 \
    *o_value = value;                             \
}

#define CREATE_HASH_FUNCTIONS(T, TKey, TVal)      \
    CREATE_SET_HASH_FUNCTION(T, TKey, TVal);      \
    CREATE_HASH_ADD_FUNCTION(T, TKey, TVal);      \
    CREATE_HASH_GET_VALUE_FUCTION(T, TKey, TVal);

CREATE_STRUCT(HashBitmapVkDescriptorSet, Bitmap *, VkDescriptorSet *);
CREATE_STRUCT(HashBitmapBitmap, Bitmap *, Bitmap *);
CREATE_STRUCT(HashEntityRect, Entity *, Rect *);
CREATE_STRUCT(HashEntityMovement, Entity *, Movement *);
CREATE_STRUCT(HashEntityNPC, Entity *, NPC *);
CREATE_STRUCT(HashSetEntity, Entity *, Entity *);
CREATE_STRUCT(HashCharUIInfo, const char *, UIInfo *);
CREATE_STRUCT(HashEntityStat, Entity *, Stat *);

CREATE_HASH_GET_VALUE_FUCTION_PROTOTYPE(HashEntityMovement, Entity *, Movement *);
#endif
