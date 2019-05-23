#include "hashtable.h"
void SetValue(VkDescriptorSet *value, VkDescriptorSet **o_value)
{
    *o_value = value;
}

b32 KeyCompare(Bitmap *a, Bitmap *b)
{
    return a->bitmapID == b->bitmapID;
}

void SetHash(
        HashKeyBitmapValueVkDescriptorSet *v,
        Bitmap *key,
        VkDescriptorSet *value,
        HashKeyBitmapValueVkDescriptorSet *next)
{
    v->key = key;
    v->val = value;
    v->next = next;
}

memory_index KeyToHashIndex(Hash *hash, Bitmap *key)
{
    return key->bitmapID % hash->bucketCount;
}
