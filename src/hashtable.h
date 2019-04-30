#ifndef __HASHTABLE__
#define __HASHTABLE__

#include "bitmap.h"
#include "renderer/vulkan/vulkan.h"

#define MAX_HASH 17
struct BitmapDescriptorMap {
    VkDescriptorSet *hashTable[MAX_HASH];
};

#endif
