#ifndef __HASHTABLE__
#define __HASHTABLE__

#include "hash.h"
#include "entity.h"
#include "rectangle.h"
#include "npc.h"
#include "bitmap.h"
#include "movement.h"
#include "renderer/vulkan/vulkan.h"

#define MAX_HASH 17

#define CREATE_HASH_FUNCTIONS(T, TKey, TVal) \
    CREATE_HASH_ADD_FUNCTION(T, TKey, TVal); \
    CREATE_HASH_GET_VALUE_FUCTION(T, TKey, TVal);

CREATE_STRUCT(HashBitmapVkDescriptorSet, Bitmap *, VkDescriptorSet *);
CREATE_STRUCT(HashBitmapBitmap, Bitmap *, Bitmap *);
CREATE_STRUCT(HashEntityRect, Entity *, Rect *);
CREATE_STRUCT(HashEntityMovement, Entity *, Movement *);
CREATE_STRUCT(HashEntityNPC, Entity *, NPC *);
CREATE_STRUCT(HashSetEntity, Entity *, Entity *);

CREATE_HASH_GET_VALUE_FUCTION_PROTOTYPE(HashEntityMovement, Entity *, Movement *);
#endif
