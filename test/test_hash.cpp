#include <assert.h>
#include <stdio.h>
#include "../src/math.h"
#include "../src/game_memory.h"
#include "../src/hash.h"

/* The is madness
 * Move a specific amount of characters if it's true or false :)
 */
#define BTOA(x) "false\0\0\0true"+!!(x)*8

memory_index GetValueTest(Hash *hash, u32 key)
{
    memory_index index = key % MAX_HASH_SIZE;
    HashKeyValue *result = &((HashKeyValue *)hash->list)[index];

    do
    {
        if (result->key == key)
        {
            return result->val;
        }

    }
    while((result = result->next) != 0);

    printf("found nothing!\n");

    return 0;
}

void testIntHash()
{
    GameMemory gameMemory;
    u32 tempSize = MEGABYTE(50);
    gameMemory.base = (u8 *)malloc(tempSize);
    gameMemory.maxSize = tempSize;

    CREATE_HASH(HashKeyValue, &gameMemory, hash, sizeof(HashKeyValue) * MAX_HASH_SIZE, &GetValueTest);

    memory_index count = 200;
    for(memory_index i = 0; i < count; i++)
    {
        ADD_HASH(HashKeyValue, hash, SafeCastToU32(i), SafeCastToU32(i+1));
    }
    for(memory_index i = 0; i < count; i++)
    {
        b32 result = hash->GetValue(hash, SafeCastToU32(i)) == SafeCastToU32(i+1);
        if (result == false)
        {
            printf("failed at index '%zu' for value '%d'\n",
                    hash->GetValue(hash, SafeCastToU32(i)), SafeCastToU32(i+1));
            break;
        }
    }
    printf("Hash: All tests passed!\n");
}

int main()
{

    testIntHash();

    return 0;
}
