#include "hashtable.h"
void SetValue(VkDescriptorSet *value, VkDescriptorSet **o_value)
{
    *o_value = value;
}

void SetValue(Bitmap *value, Bitmap **o_value)
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

void SetHash(
        HashBitmapVkDescriptorSet *v,
        Bitmap *key,
        VkDescriptorSet *value,
        HashBitmapVkDescriptorSet *next)
{
    v->key = key;
    v->val = value;
    v->next = next;
}

void SetHash(
        HashBitmapBitmap *v,
        Bitmap *key,
        Bitmap *value,
        HashBitmapBitmap *next)
{
    v->key = key;
    v->val = value;
    v->next = next;
}

memory_index KeyToHashIndex(Hash *hash, Bitmap *key)
{
    return key->bitmapID % hash->bucketCount;
}

CREATE_HASH_ADD_FUNCTION(HashBitmapVkDescriptorSet, Bitmap *, VkDescriptorSet *);
CREATE_HASH_GET_VALUE_FUCTION(HashBitmapVkDescriptorSet, Bitmap *, VkDescriptorSet *);

CREATE_HASH_ADD_FUNCTION(HashBitmapBitmap, Bitmap *, Bitmap *);
CREATE_HASH_GET_VALUE_FUCTION(HashBitmapBitmap, Bitmap *, Bitmap *);
CREATE_HASH_GET_VALUE_FUCTION(HashBitmapBitmap, const char *, Bitmap *);
CREATE_HASH_GET_VALUE_FUCTION(HashBitmapBitmap, u32, Bitmap *);
