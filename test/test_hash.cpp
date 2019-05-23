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

void testIntHash(GameMemory *gm)
{
    b32 failed = false;
    CREATE_HASH(HashKeyValue, gm, hash, MAX_HASH_SIZE);

    memory_index count = 200;
    for(memory_index i = 0; i < count; i++)
    {
        u32 key = SafeCastToU32(i);
        u32 value = SafeCastToU32(i+1);
        ADD_HASH(HashKeyValue, hash, key, value);
    }
    b32 result = false;
    for(memory_index i = 0; i < count; i++)
    {
        GET_VALUE_HASH(HashKeyValue, hash, i, valueOut);
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
    CREATE_HASH(HashKeyCharValueU32, gm, hash, 3);

    memory_index count = 50;
    for(memory_index i = 0; i < count; i++)
    {
        char key[24] = {};
        snprintf(key, sizeof(key), "%s%zu", "a", i);
        u32 value = SafeCastToU32(i+1);
        ADD_HASH(HashKeyCharValueU32, hash, key, value);
    }
    b32 result = false;
    for(memory_index i = 0; i < count; i++)
    {
        char key[24] = {};
        snprintf(key, sizeof(key), "%s%zu", "a", i);
        GET_VALUE_HASH(HashKeyCharValueU32, hash, key, valueOut);
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
    CREATE_HASH(HashKeyCharValueU32, gm, hash, 3);

    const char *multipleKey[] = {"hello", "hella", "food", "hello", "hi", "hello", NULL};
    b32 result = false;
    memory_index i = 0;
    while(multipleKey[i])
    {
        const char *key = multipleKey[i];
        u32 value = SafeCastToU32(i+1);
        ADD_HASH(HashKeyCharValueU32, hash, key, value);

        GET_VALUE_HASH(HashKeyCharValueU32, hash, key, valueOut);
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
