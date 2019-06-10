#ifndef __MOUSE_H__
#define __MOUSE_H__

enum MouseType
{
    LEFT_SINGLE_CLICK = 0,
    LEFT_DOUBLE_CLICK,
    LEFT_DRAG_CLICK,
    LEFT_MOUSE_DRAG, /* This is the intermedietary stage, when we don't know
                        whether it's going to be a single clikck or a dragged
                        click */
    RIGHT_SINGLE_CLICK,
    RIGHT_DOUBLE_CLICK,
    RIGHT_DRAG_CLICK,
    RIGHT_MOUSE_DRAG,
    DO_NOT_USE_ANYMORE_MOUSE,
};

struct MouseInfo
{
    MouseType mouseType;
    /* we need 2 v2 for drag information */
    v2 leftScreenCoordinates[2];
    v2 rightScreenCoordinates[2];
    b32 isNew;
};

#endif
