#ifndef __HASH_SET__
#define __HASH_SET__
#include "hash_set.h"
#include "array.h"

HashSetNode *CreateHashSetNode(GameMemory *gm, Rect *r)
{
    HashSetNode *result =
        (HashSetNode*)AllocateMemory0(gm, sizeof(HashSetNode));
    result->r = r;
    result->left = nullptr;
    result->right = nullptr;
    return result;

}

HashSet *CreateHashSet(GameMemory *gm)
{
    HashSet *result = (HashSet *)AllocateMemory0(gm, sizeof(HashSet));
    result->rootNode = nullptr;
    return result;

}

#if 0
HashSet *CreateHashSet(GameMemory *gm, Rect *r)
{
    HashSet *result = (HashSet *)AllocateMemory0(gm, sizeof(HashSet));
    result->rootNode = CreateHashSetNode(gm, r);
    return result;
}
#endif

/* If a is smaller than b, return 1.
 * if b is bigger than a, return -1.
 * if a and b are equal return 0
 * Which is bigger is determined by the max point of a rect.
 * If both have the same lenght max.x, then total area will be the
 * determinant.
 */
int HashSetCompare(Rect *a, Rect *b)
{
    if(a == b)
    {
        return 0;
    }
    else if(a->max.x == b->max.x && a->min.y == b->min.y)
    {
#if 0
        printf("maxx: %f %f\n", a->max.x, b->max.x);
        printf("maxy: %f %f\n", a->max.y, b->max.y);
        printf("minx: %f %f\n", a->min.x, b->min.x);
        printf("miny: %f %f\n", a->min.y, b->min.y);
#endif
        return 0;
    }
    else if(a->max.x < b->max.x)
    {
        return 1;
    }
    else if(a->max.x == b->max.x)
    {
        if(a->dim.x * a->dim.y < b->dim.x * b->dim.y)
        {
            return 1;
        }
        return -1;
    }

    return -1;

}

void HashSetInsertNode(GameMemory *gm, HashSetNode *hsn, Rect * r)
{
    int compare = HashSetCompare(r, hsn->r);
    if(compare == 0)
    {
        return;
    }
    else if(compare == 1)
    {
        if(hsn->left == nullptr)
        {
            hsn->left = CreateHashSetNode(gm, r);
            return;
        }
        else
        {
            HashSetInsertNode(gm, hsn->left, r);
        }
    }
    else /* insert to right */
    {
        if(hsn->right == nullptr)
        {
            hsn->right = CreateHashSetNode(gm, r);
            return;
        }
        else
        {
            HashSetInsertNode(gm, hsn->right, r);
        }
    }
}

void HashSetInsert(GameMemory *gm, HashSet *hs, Rect *r)
{
    if(hs->rootNode == nullptr)
    {
        hs->rootNode = CreateHashSetNode(gm, r);
        return;
    }

    HashSetInsertNode(gm, hs->rootNode, r);
}

void HashSetOrderedToListNode(GameMemory *gm, HashSetNode *hsn, Rect ***arr) {
    if(hsn->left != nullptr)
    {
        HashSetOrderedToListNode(gm, hsn->left, arr);
    }

    ARRAY_PUSH(Rect *, gm, *arr, hsn->r);

    if(hsn->right != nullptr)
    {
        HashSetOrderedToListNode(gm, hsn->right, arr);
    }
}

Rect **HashSetOrderedToList(GameMemory *gm, HashSet *hs)
{

    ARRAY_CREATE(Rect *, gm, arr);

    if(hs->rootNode == nullptr)
    {
        return arr;
    }

    HashSetOrderedToListNode(gm, hs->rootNode, &arr);
    return arr;
}

#endif
