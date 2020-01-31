#ifndef __TEST_SPACIAL_HASH__
#define __TEST_SPACIAL_HASH__

/* Avoid redefining STB_IMAGE
 * This already happens when building rectangle.o
 */
#define STB_IMAGE_IMPLEMENTATION 1

#include <stdio.h>
#include <assert.h>
#include "../src/define.h"
#include "../src/rectangle.h"
#include "../src/spacial_hash.cpp"

void testHashKey(GameMemory *gm)
{
    SpacialHash *sh = CreateSpacialHash(gm, 100, 10, 10 /*cell grid*/);
    memory_index bucketIndex = SpacialHashPointToBucket(sh, v2{0, 0});
    assert(bucketIndex == 0);

    bucketIndex = SpacialHashPointToBucket(sh, v2{9, 9});
    assert(bucketIndex == 0);

    bucketIndex = SpacialHashPointToBucket(sh, v2{10, 0});
    assert(bucketIndex == 1);

    bucketIndex = SpacialHashPointToBucket(sh, v2{10, 10});
    assert(bucketIndex == 11);

    bucketIndex = SpacialHashPointToBucket(sh, v2{0, 10});
    assert(bucketIndex == 10);

    bucketIndex = SpacialHashPointToBucket(sh, v2{0, 23});
    assert(bucketIndex == 20);

    bucketIndex = SpacialHashPointToBucket(sh, v2{0, 25});
    assert(bucketIndex == 20);

    bucketIndex = SpacialHashPointToBucket(sh, v2{51, 10});
    assert(bucketIndex == 15);

    bucketIndex = SpacialHashPointToBucket(sh, v2{10, 51});
    assert(bucketIndex == 51);

    bucketIndex = SpacialHashPointToBucket(sh, v2{99, 0});
    assert(bucketIndex == 9);

    bucketIndex = SpacialHashPointToBucket(sh, v2{99, 9});
    assert(bucketIndex == 9);

    bucketIndex = SpacialHashPointToBucket(sh, v2{100, 0});
    assert(bucketIndex == 10);

    bucketIndex = SpacialHashPointToBucket(sh, v2{0, 100});
    assert(bucketIndex == 100);

    bucketIndex = SpacialHashPointToBucket(sh, v2{0, 100});
    assert(bucketIndex == 100);

    bucketIndex = SpacialHashPointToBucket(sh, v2{0, 95});
    assert(bucketIndex == 90);

    bucketIndex = SpacialHashPointToBucket(sh, v2{99, 100});
    assert(bucketIndex == 109);

    bucketIndex = SpacialHashPointToBucket(sh, v2{20, 10});
    assert(bucketIndex == 12);
}

void testHashRect(GameMemory *gm)
{
    SpacialHash *sh = CreateSpacialHash(gm, 100, 10, 10 /*cell grid*/);
    v2 dim = {20, 20};
    v3 center = {55, 55, 0};
    Rect *r =  CreateRectangle(gm, center, COLOR_WHITE, dim);

    memory_index *arr = SpacialHashRectToKey(sh, r);
    memory_index size = ARRAY_LIST_SIZE(arr);
    const memory_index hashIds[] = {44, 54, 64, 45, 55, 65, 46, 56, 66};
    for(memory_index i = 0; i < size; i++)
    {
        printf("hashIds: %zu\n", arr[i]);
        assert(hashIds[i] == arr[i]);
    }
    SpacialHashInsert(sh, r);

    dim = {10, 10};
    center = {0, 0, 0};
    Rect *r2 =  CreateRectangle(gm, center, COLOR_WHITE, dim);
    Rect **rects = SpacialHashGet(sh, r2);
    assert(ARRAY_LIST_SIZE(rects) == 0);
    printf("PASS: Found no match.\n");

    dim = {10, 10};
    center = {60, 60, 0};
    Rect *r4 =  CreateRectangle(gm, center, COLOR_WHITE, dim);
    rects = SpacialHashGet(sh, r4);
    assert(ARRAY_LIST_SIZE(rects) == 1);
    printf("PASS: Found a match.\n");

    dim = {5, 5};
    center = {50, 50, 0};
    Rect *r3 =  CreateRectangle(gm, center, COLOR_WHITE, dim);
    rects = SpacialHashGet(sh, r3);
    assert(ARRAY_LIST_SIZE(rects) == 1);
    printf("PASS: Found a match.\n");

    SpacialHashInsert(sh, r3);
    SpacialHashInsert(sh, r4);

    dim = {10, 10};
    center = {55, 55, 0};
    Rect *r5 =  CreateRectangle(gm, center, COLOR_WHITE, dim);
    rects = SpacialHashGet(sh, r5);
    assert(ARRAY_LIST_SIZE(rects) == 3);
    printf("Size %zu\n", ARRAY_LIST_SIZE(rects));
    printf("PASS: Found 3 match.\n");


    //ARRAY_FREE(arr) /*to be implemented */
    //ARRAY_FREE(rects) /*to be implemented */
}

int main()
{
    GameMemory gameMemory;
    u32 tempSize = MEGABYTE(50);
    gameMemory.base = (u8 *)malloc(tempSize);
    gameMemory.maxSize = tempSize;

    testHashKey(&gameMemory);
    testHashRect(&gameMemory);
    printf("Done\n");

    return 0;
}
#endif
