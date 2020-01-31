#ifndef __TEST_HASH_SET__
#define __TEST_HASH_SET__

/* Avoid redefining STB_IMAGE
 * This already happens when building rectangle.o
 */
#define STB_IMAGE_IMPLEMENTATION 1

#include <stdio.h>
#include <assert.h>
#include "../src/define.h"
#include "../src/rectangle.h"
#include "../src/hash_set.cpp"

void testHashSet(GameMemory *gm)
{
    v2 dim = {20, 20};
    v3 center = {55, 55, 0};
    Rect *r =  CreateRectangle(gm, center, COLOR_WHITE, dim);
    HashSet *hs = CreateHashSet(gm);
    assert(hs->rootNode == nullptr);

    HashSetInsert(gm, hs, r);
    assert(hs->rootNode != nullptr);
    assert(hs->rootNode->r != nullptr);

    HashSetInsert(gm, hs, r);
    assert(hs->rootNode->left == nullptr);
    assert(hs->rootNode->right == nullptr);

    Rect *rCopy =  CreateRectangle(gm, center, COLOR_WHITE, dim);
    HashSetInsert(gm, hs, rCopy);

    assert(hs->rootNode->left == nullptr);
    assert(hs->rootNode->right == nullptr);

    printf("rNew insert\n");
    center = {40, 40, 0};
    Rect *rNew =  CreateRectangle(gm, center, COLOR_WHITE, dim);
    HashSetInsert(gm, hs, rNew);

    assert(hs->rootNode->left != nullptr);
    assert(hs->rootNode->left->r == rNew);
    assert(hs->rootNode->right == nullptr);

    printf("rNew2 insert\n");
    center = {40, 40, 0};
    dim = {20, 2};
    Rect *rNew2 =  CreateRectangle(gm, center, COLOR_WHITE, dim);
    HashSetInsert(gm, hs, rNew2);
    assert(rNew != rNew2);

    assert(hs->rootNode->left->left != nullptr);
    assert(hs->rootNode->left->right == nullptr);
    assert(hs->rootNode->right == nullptr);

    printf("test -- rNew2 still in hash set\n");
    center = {40, 40, 0};
    dim = {20, 2};
    Rect *rNew3 =  CreateRectangle(gm, center, COLOR_WHITE, dim);
    HashSetInsert(gm, hs, rNew3);

    assert(hs->rootNode->left->left->r == rNew2);
    assert(hs->rootNode->left->right == nullptr);
    assert(hs->rootNode->right == nullptr);

    Rect **rectList = HashSetOrderedToList(gm, hs);

    for(memory_index i = 0; i < ARRAY_LIST_SIZE(rectList); i++)
    {
        printf("%p\n", rectList[i]);
    }
}

void testEmptyHashSet()
{

}

int main()
{
    GameMemory gameMemory; //NOLINT
    u32 tempSize = MEGABYTE(50);
    gameMemory.base = (u8 *)malloc(tempSize);
    gameMemory.maxSize = tempSize;

    testHashSet(&gameMemory);

    return 0;
}
#endif
