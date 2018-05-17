#ifndef __BITMAP__
#define __BITMAP__

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#pragma warning(push)
#pragma warning(disable : 4244)
#include "stb_image.h"
#pragma warning(pop)
#endif


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

inline void ImageToBitmap(Bitmap *bitmap, const char *ImageName)
{
    s32 componentsPerPixel;

    /* PNGs are normally 'flipped'.
     * This happens because OpenGL expects the 0.0 coordinate on the y-axis to
     * be on the bottom side of the image, but images usually have 0.0 at the
     * top of the y-axis
     */
    size_t ImageLen = strlen(ImageName);
    if (ImageLen >= 5 && strcmp(ImageName + ImageLen - 5, ".png"))
        stbi_set_flip_vertically_on_load(true);

    bitmap->data =
        stbi_load(ImageName, (s32 *)&bitmap->width, (s32 *)&bitmap->height, &componentsPerPixel, 0);

    int textureFormat = GL_RGB;
    if (componentsPerPixel == 4)
        textureFormat = GL_RGBA;
    bitmap->format = textureFormat;

    ASSERT(componentsPerPixel == 4);

}

inline void SetBitmap(Bitmap *bitmap, TextureParam textureParam, memory_index bitmapID, const char * imgPath)
{
    ZeroSize(bitmap, sizeof(Bitmap));
    ImageToBitmap(bitmap, imgPath);
    bitmap->textureParam = textureParam;
    bitmap->bitmapID = bitmapID;
}

#endif
