#ifndef __HASH_SET_H__
#define __HASH_SET_H__

#define STB_IMAGE_IMPLEMENTATION 1

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

Rect **HashSetOrderedToList(GameMemory *gm, HashSet *hs);
void HashSetInsert(GameMemory *gm, HashSet *hs, Rect *r);
HashSet *CreateHashSet(GameMemory *gm);
#endif
