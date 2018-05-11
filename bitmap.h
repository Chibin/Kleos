#ifndef __BITMAP__
#define __BITMAP__

enum ImageType
{
    STB = 0x1,
    SDL = 0x2,
};

struct Bitmap
{
    u32 width;
    u32 height;
    GLenum format;
    u8* data;
    ImageType freeImageType;

    TextureParam textureParam;
    memory_index bitmapID;

    Bitmap *next;
    Bitmap *prev;
};

inline void PushBitmap(Bitmap *oldNode, Bitmap *newNode)
{
    newNode->next = oldNode->next;
    newNode->prev = oldNode;
    oldNode->next->prev = newNode;
    oldNode->next = newNode;
}

inline Bitmap *FindBitmap(Bitmap *sentinelNode, memory_index bitmapID)
{
    for(Bitmap * node = sentinelNode->next; node != sentinelNode; node = node->next)
    {
        if (node->bitmapID == bitmapID)
        {
            return node;
        }
    }

    return nullptr;
}

#endif
