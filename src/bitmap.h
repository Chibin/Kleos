#pragma once

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
