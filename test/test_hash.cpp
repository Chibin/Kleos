#include <assert.h>
#include <stdio.h>
#include "../src/math.h"
#include "../src/hash.h"
#include "../src/string.h"
#include "../src/game_memory.h"

/* The is madness
 * Move a specific amount of characters if it's true or false :)
 */
#define BTOA(x) "false\0\0\0true"+!!(x)*8

#define REPORT_TEST(result, func)          \
{                                          \
    if(result)                             \
    {                                      \
        printf("%s passed!\n", func);      \
    }                                      \
    else                                   \
    {                                      \
        printf("%s failed!\n", func);      \
    }                                      \
}

CREATE_HASH_ADD_FUNCTION(HashU32U32, u32, u32);
CREATE_HASH_GET_VALUE_FUCTION(HashU32U32, u32, u32);

CREATE_HASH_ADD_FUNCTION(HashCharU32, const char *, u32);
CREATE_HASH_GET_VALUE_FUCTION(HashCharU32, const char *, u32);

void testIntHash(GameMemory *gm)
{
    b32 failed = false;
    HASH_CREATE(HashU32U32, gm, hash, MAX_HASH_SIZE);

    memory_index count = 200;
    for(memory_index i = 0; i < count; i++)
    {
        u32 key = SafeCastToU32(i);
        u32 value = SafeCastToU32(i+1);
        HashAdd(hash, key, value);
    }
    b32 result = false;
    for(memory_index i = 0; i < count; i++)
    {
        u32 valueOut = HashU32U32GetValue(hash, i);
        result = valueOut == SafeCastToU32(i+1);
        if (result == false)
        {
            printf("failed at index '%zu' for expected value '%d' vs '%d'\n", i, SafeCastToU32(i+1), valueOut);
            break;
        }
    }

    REPORT_TEST(result, __func__);
}

void testCharHash(GameMemory *gm)
{
    HASH_CREATE(HashCharU32, gm, hash, 3);

    memory_index count = 50;
    for(memory_index i = 0; i < count; i++)
    {
        char key[24] = {};
        snprintf(key, sizeof(key), "%s%zu", "a", i);
        u32 value = SafeCastToU32(i+1);
        HashAdd(hash, key, value);
    }
    b32 result = false;
    for(memory_index i = 0; i < count; i++)
    {
        char key[24] = {};
        snprintf(key, sizeof(key), "%s%zu", "a", i);
        u32 valueOut = HashCharU32GetValue(hash, key);
        result = valueOut == (i+1);
        if (result == false)
        {
            printf("failed at key '%s' for expected value '%d' vs '%d'\n", key, SafeCastToU32(i+1), valueOut);
            break;
        }
    }

    REPORT_TEST(result, __func__);
}

void testCharWithaCollission(GameMemory *gm)
{
    HASH_CREATE(HashCharU32, gm, hash, 3);

    const char *multipleKey[] = {"hello", "hella", "food", "hello", "hi", "hello", NULL};
    b32 result = false;
    memory_index i = 0;
    while(multipleKey[i])
    {
        const char *key = multipleKey[i];
        u32 value = SafeCastToU32(i+1);
        HashAdd(hash, key, value);

        u32 valueOut = HashCharU32GetValue(hash, key);
        result = valueOut == value;
        if (result == false)
        {
            printf("failed at key '%s' for expected value '%d' vs '%d'\n", key, SafeCastToU32(i+1), valueOut);
            break;
        }
        i++;
    }

    REPORT_TEST(result, __func__);
}

int main()
{
    GameMemory gameMemory;
    u32 tempSize = MEGABYTE(50);
    gameMemory.base = (u8 *)malloc(tempSize);
    gameMemory.maxSize = tempSize;

    testIntHash(&gameMemory);
    testCharHash(&gameMemory);
    testCharWithaCollission(&gameMemory);

    return 0;
}
