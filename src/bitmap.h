#ifndef __BITMAP_H__
#define __BITMAP_H__
#include "math.h"
#include "opengl.h"
#include "sdl_common.h"

#include <GL/glew.h>
#define GL3_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glu.h>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_MALLOC(sz) RequestToReservedMemory(sz)
/* We also need to take care of STBI_FREE(p) */
#pragma warning(push)
#pragma warning(disable : 4244)
#include <stb_image.h>
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
    u8 *data;
    ImageType freeImageType;

    TextureParam textureParam;
    memory_index bitmapID;

    Bitmap *next;
    Bitmap *prev;
};
#endif
