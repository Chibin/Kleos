#include "hashtable.h"

void HashInsert(BitmapDescriptorMap *b2d, Bitmap *key, VkDescriptorSet *value)
{
    ASSERT(b2d->hashTable[key->bitmapID % MAX_HASH] == NULL);
    b2d->hashTable[key->bitmapID % MAX_HASH] = value;
}

void HashUpdate(BitmapDescriptorMap *b2d, Bitmap *key, VkDescriptorSet *value)
{
    ASSERT(b2d->hashTable[key->bitmapID % MAX_HASH] != NULL);
    b2d->hashTable[key->bitmapID % MAX_HASH] = value;
}

VkDescriptorSet *GetHashValue(BitmapDescriptorMap *b2d, Bitmap *key)
{
    ASSERT(b2d->hashTable[key->bitmapID % MAX_HASH] != NULL);

    return b2d->hashTable[key->bitmapID % MAX_HASH];
}
