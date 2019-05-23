#ifndef __HASHTABLE__
#define __HASHTABLE__

#include "bitmap.h"
#include "renderer/vulkan/vulkan.h"

#define MAX_HASH 17
struct HashBitmapVkDescriptorSet {
    Bitmap *key;
    VkDescriptorSet *val;
    HashBitmapVkDescriptorSet *next;
};

struct HashBitmapBitmap
{
    Bitmap *key;
    Bitmap *val;
    HashBitmapBitmap *next;
};
#endif
