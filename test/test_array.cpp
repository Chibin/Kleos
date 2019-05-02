#include <assert.h>
#include <stdio.h>
#include "../src/math.h"
#include "../src/game_memory.h"
#include "../src/array.h"

void testInt()
{
    GameMemory gameMemory;
    u32 tempSize = MEGABYTE(50);
    gameMemory.base = (u8 *)malloc(tempSize);
    gameMemory.maxSize = tempSize;

    ARRAY_CREATE(u32, &gameMemory, arr);

    u32 count = 100;
    for(memory_index i = 0; i < count; i++)
    {
        ARRAY_PUSH(u32, &gameMemory, arr, i);
    }

    for(u32 i = 0; i < count; i++)
    {
        printf("out: %d, expected: %d\n", arr[i], i);
        assert(i == arr[i]);
    }
}

int main()
{

    testInt();

    return 0;
}
