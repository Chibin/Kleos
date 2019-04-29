#ifndef __BITMAP__
#define __BITMAP__
#include "bitmap.h"
#include "math.h"

inline void PushBitmap(Bitmap *oldNode, Bitmap *newNode)
{
    newNode->next = oldNode->next;
    newNode->prev = oldNode;
    oldNode->next->prev = newNode;
    oldNode->next = newNode;
}

inline Bitmap *FindBitmap(Bitmap *sentinelNode, memory_index bitmapID)
{
    for (Bitmap *node = sentinelNode->next; node != sentinelNode; node = node->next)
    {
        if (node->bitmapID == bitmapID)
        {
            return node;
        }
    }

    ASSERT(!"FOUND NOTHING");
    return nullptr;
}

inline Bitmap *FindBitmap(Bitmap *sentinelNode, const char *bitmapName)
{
    for (Bitmap *node = sentinelNode->next; node != sentinelNode; node = node->next)
    {
        if (strcmp(node->name, bitmapName) == 0)
        {
            return node;
        }
    }

    ASSERT(!"FOUND NOTHING");
    return nullptr;
}

inline void ImageToBitmap(Bitmap *bitmap, const char *ImageName)
{
    s32 componentsPerPixel = 0;

    /* PNGs are normally 'flipped'.
     * This happens because OpenGL expects the 0.0 coordinate on the y-axis to
     * be on the bottom side of the image, but images usually have 0.0 at the
     * top of the y-axis
     */
    size_t ImageLen = strlen(ImageName);
    if ((ImageLen >= 5) && (strcmp(ImageName + ImageLen - 5, ".png") != 0))
    {
        stbi_set_flip_vertically_on_load(1);
    }

    bitmap->data =
        stbi_load(ImageName, (s32 *)&bitmap->width, (s32 *)&bitmap->height, &componentsPerPixel, 0);

    int textureFormat = GL_RGB;
    if (componentsPerPixel == 4)
    {
        textureFormat = GL_RGBA;
    }
    bitmap->format = textureFormat;

    ASSERT(componentsPerPixel == 4);
}

inline void SetBitmap(Bitmap *bitmap, const char *name, TextureParam textureParam, memory_index bitmapID, const char *imgPath)
{
    ZeroSize(bitmap, sizeof(Bitmap));
    ImageToBitmap(bitmap, imgPath);
    snprintf(bitmap->name, sizeof(bitmap->name), "%s", name);
    bitmap->textureParam = textureParam;
    bitmap->bitmapID = bitmapID;
}
#endif
