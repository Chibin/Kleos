#ifndef __HASH_SET_H__
#define __HASH_SET_H__

#include "game_memory.h"
#include "rectangle.h"

struct HashSetNode
{
    Rect *r;

    HashSetNode *left;
    HashSetNode *right;
};

struct HashSet
{
    HashSetNode *rootNode;
};
#endif
