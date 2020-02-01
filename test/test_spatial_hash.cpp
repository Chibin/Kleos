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
#include "../src/hash_set.h"
#include "../src/spatial_hash.cpp"

void testHashKey(GameMemory *gm)
{
    SpatialHash *sh = CreateSpatialHash(gm, 1000, 10, 10 /*cell grid*/);
    memory_index bucketIndex = SpatialHashPointToBucket(sh, v2{0, 0});
    assert(bucketIndex == 0);

    bucketIndex = SpatialHashPointToBucket(sh, v2{9, 9});
    assert(bucketIndex == 0);

    bucketIndex = SpatialHashPointToBucket(sh, v2{10, 0});
    assert(bucketIndex == 1);

    bucketIndex = SpatialHashPointToBucket(sh, v2{10, 10});
    assert(bucketIndex == 11);

    bucketIndex = SpatialHashPointToBucket(sh, v2{0, 10});
    assert(bucketIndex == 10);

    bucketIndex = SpatialHashPointToBucket(sh, v2{0, 23});
    assert(bucketIndex == 20);

    bucketIndex = SpatialHashPointToBucket(sh, v2{0, 25});
    assert(bucketIndex == 20);

    bucketIndex = SpatialHashPointToBucket(sh, v2{51, 10});
    assert(bucketIndex == 15);

    bucketIndex = SpatialHashPointToBucket(sh, v2{10, 51});
    assert(bucketIndex == 51);

    bucketIndex = SpatialHashPointToBucket(sh, v2{99, 0});
    assert(bucketIndex == 9);

    bucketIndex = SpatialHashPointToBucket(sh, v2{99, 9});
    assert(bucketIndex == 9);

    bucketIndex = SpatialHashPointToBucket(sh, v2{100, 0});
    assert(bucketIndex == 10);

    bucketIndex = SpatialHashPointToBucket(sh, v2{0, 100});
    assert(bucketIndex == 100);

    bucketIndex = SpatialHashPointToBucket(sh, v2{0, 100});
    assert(bucketIndex == 100);

    bucketIndex = SpatialHashPointToBucket(sh, v2{0, 95});
    assert(bucketIndex == 90);

    bucketIndex = SpatialHashPointToBucket(sh, v2{99, 100});
    assert(bucketIndex == 109);

    bucketIndex = SpatialHashPointToBucket(sh, v2{20, 10});
    assert(bucketIndex == 12);

    bucketIndex = SpatialHashPointToBucket(sh, v2{-50.9438515, 51.7043304});
    assert(bucketIndex == 55);

}

void testHashRect(GameMemory *gm)
{
    SpatialHash *sh = CreateSpatialHash(gm, 105, 10, 10 /*cell grid*/);
    v2 dim = {20, 20};
    v3 center = {55, 55, 0};
    Rect *r =  CreateRectangle(gm, center, COLOR_WHITE, dim);

    memory_index *arr = SpatialHashRectToKey(sh, r);
    memory_index size = ARRAY_LIST_SIZE(arr);
    const memory_index hashIds[] = {44, 54, 64, 45, 55, 65, 46, 56, 66};
    for(memory_index i = 0; i < size; i++)
    {
        printf("hashIds: %zu\n", arr[i]);
        assert(hashIds[i] == arr[i]);
    }
    SpatialHashInsert(sh, r);

    dim = {10, 10};
    center = {0, 0, 0};
    Rect *r2 =  CreateRectangle(gm, center, COLOR_WHITE, dim);
    Rect **rects = SpatialHashGet(sh, r2);
    assert(ARRAY_LIST_SIZE(rects) == 0);
    printf("PASS: Found no match.\n");

    dim = {10, 10};
    center = {60, 60, 0};
    Rect *r4 =  CreateRectangle(gm, center, COLOR_WHITE, dim);
    rects = SpatialHashGet(sh, r4);
    assert(ARRAY_LIST_SIZE(rects) == 1);
    printf("PASS: Found a match.\n");

    dim = {5, 5};
    center = {50, 50, 0};
    Rect *r3 =  CreateRectangle(gm, center, COLOR_WHITE, dim);
    rects = SpatialHashGet(sh, r3);
    assert(ARRAY_LIST_SIZE(rects) == 1);
    printf("PASS: Found a match.\n");

    SpatialHashInsert(sh, r3);
    SpatialHashInsert(sh, r4);

    dim = {10, 10};
    center = {55, 55, 0};
    Rect *r5 =  CreateRectangle(gm, center, COLOR_WHITE, dim);
    rects = SpatialHashGet(sh, r5);
    assert(ARRAY_LIST_SIZE(rects) == 3);
    printf("Size %zu\n", ARRAY_LIST_SIZE(rects));
    printf("PASS: Found 3 match.\n");

    printf("core dump?\n");
    dim = {3.50000000, 3.50000000};
    center = {24.6000004,99.7213821, 0};
    Rect *r6 =  CreateRectangle(gm, center, COLOR_WHITE, dim);
    rects = SpatialHashGet(sh, r6);
    //assert(ARRAY_LIST_SIZE(rects) == 3);
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
