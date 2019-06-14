#ifndef __UI_H__
#define __UI_H__

struct UIText
{
    const char *text;
    v2 basePosition;
    f32 scaleFactor;
};

struct UIInfo
{
    AABB range;
    v4 color;
    Bitmap *bitmap;
    b32 skipFilter;

    UIText uiText;
};
#endif
