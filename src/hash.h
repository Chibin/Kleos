#ifndef __HASH_H__
#define __HASH_H__

#define MAX_HASH_SIZE 10

#define CREATE_STRUCT(T, TKey, TVal) \
    struct T                         \
{                                    \
    TKey key;                        \
    TVal val;                        \
    T *next;                         \
};

#include "string.h"

struct Hash
{
    void *list;
    b32 *isEmptyList;
    memory_index bucketCount;
    struct GameMemory *gm;
};

CREATE_STRUCT(HashU32U32, u32, u32);

/* This is more specific so can't generalize it. */
struct HashCharU32
{
    char key[24];
    u32 val;
    HashCharU32 *next;
};

void SetValue(u32 value, u32 *o_value)
{
    *o_value = value;
}

void SetHash(HashU32U32 *v, u32 key, u32 value, HashU32U32 *next)
{
    v->key = key;
    SetValue(value, &v->val);
    v->next = next;
}

memory_index KeyToHashIndex(Hash *hash, u32 key)
{
    return key % hash->bucketCount;
}

memory_index KeyToHashIndex(Hash *hash, const char *key)
{
    const char *charKey = key;
    memory_index strLen = StringLen(charKey);
    memory_index count = 0;
    u32 hashValue = 0x811c9dc5;
    u32 prime = 0x1000193;

    for (memory_index i = 0; i < strLen; i++)
    {
        hashValue = hashValue ^ (u32)charKey[i];
        hashValue *= prime;
    }
    memory_index index = hashValue % hash->bucketCount;
    return index;
}

void SetHash(HashCharU32 *v, const char *key, u32 value, HashCharU32 *next)
{
    memory_index strLen = StringLen(key);
#if 0
    snprintf(v->key, sizeof(v->key), "%s", key);
#else
    memcpy(v->key, key, sizeof(v->key));
#endif
    SetValue(value, &v->val);
    v->next = next;
}

b32 KeyCompare(u32 a, u32 b)
{
    return a == b;
}

b32 KeyCompare(const char *a, const char *b)
{
    return strcmp(a, b) == 0;
}

#define HASH_CREATE(T, gameMemory, hash, bCount)                                                  \
    Hash *hash = (Hash *)AllocateMemory(gameMemory, sizeof(Hash));                                \
    memset(hash, 0, sizeof(Hash));                                                                \
    {                                                                                             \
        memory_index size = sizeof(T) * bCount;                                                   \
        hash->bucketCount = bCount;                                                               \
        hash->list = (T *)AllocateMemory(gameMemory, size);                                       \
        memset(hash->list, 0, size);                                                              \
        hash->isEmptyList = (b32 *)AllocateMemory(gameMemory, sizeof(b32) * bCount);              \
        for(memory_index i = 0; i < size; i++)                                                    \
        {                                                                                         \
            hash->isEmptyList[i] = true;                                                          \
        }                                                                                         \
        hash->gm = gameMemory;                                                                    \
    }

#define HASH_ADD(T, hash, k, value)                                                               \
    {                                                                                             \
        memory_index index = KeyToHashIndex(hash, k) % hash->bucketCount;                         \
        T *tmp = &((T *)hash->list)[index];                                                       \
        if (hash->isEmptyList[index] == true)                                                     \
        {                                                                                         \
            SetHash(tmp, k, value, 0);                                                            \
            hash->isEmptyList[index] = false;                                                     \
        }                                                                                         \
        else                                                                                      \
        {                                                                                         \
            b32 found = false;                                                                    \
            if (KeyCompare(tmp->key, k))                                                          \
            {                                                                                     \
                SetValue(value, &tmp->val);                                                       \
                found = true;                                                                     \
            }                                                                                     \
            while(tmp->next != 0 && found == false)                                               \
            {                                                                                     \
                tmp = tmp->next;                                                                  \
                if (KeyCompare(tmp->key, k))                                                      \
                {                                                                                 \
                    SetValue(value, &tmp->val);                                                   \
                    found = true;                                                                 \
                    break;                                                                        \
                }                                                                                 \
            }                                                                                     \
            if (found == false)                                                                   \
            {                                                                                     \
                T *newKeyVal = (T *)AllocateMemory(hash->gm, sizeof(T));                          \
                memset(newKeyVal, 0, sizeof(T));                                                  \
                               SetHash(newKeyVal, k, value, 0);                                   \
                               tmp->next = newKeyVal;                                             \
            }                                                                                     \
        }                                                                                         \
    }

#define HASH_GET_VALUE(T, hash, k, result)                                                        \
    {                                                                                             \
        memory_index index = KeyToHashIndex(hash, k) % hash->bucketCount;                         \
        T *tmp = &((T *)hash->list)[index];                                                       \
                                                                                                  \
        b32 found = false;                                                                        \
        if(hash->isEmptyList[index] == false)                                                     \
        {                                                                                         \
            do                                                                                    \
            {                                                                                     \
                if (KeyCompare(tmp->key, k))                                                      \
                {                                                                                 \
                    result = tmp->val;                                                            \
                    found = true;                                                                 \
                    break;                                                                        \
                }                                                                                 \
                                                                                                  \
            }                                                                                     \
            while((tmp = tmp->next) != 0);                                                        \
        }                                                                                         \
        if(!found)                                                                                \
        {                                                                                         \
            printf("not found\n");                                                                \
        }                                                                                         \
    }

#define CREATE_HASH_ADD_FUNCTION(T, keyType, valueType)                                           \
    void HashAdd(Hash *hash, keyType k, valueType value)                                          \
    {                                                                                             \
        HASH_ADD(T, hash, k, value);                                                              \
    }

#define CREATE_HASH_GET_VALUE_FUCTION_PROTOTYPE( T, keyType, valueType)                           \
    valueType T##GetValue(Hash *hash, keyType k);

#define CREATE_HASH_GET_VALUE_FUCTION(T, keyType, valueType)                                      \
    valueType T##GetValue(Hash *hash, keyType k)                                                  \
    {                                                                                             \
        valueType result = {};                                                                    \
        HASH_GET_VALUE(T, hash, k, result);                                                       \
        return result;                                                                            \
    }                                                                                             \

#define HashGetValue(T, hash, key)                                                                \
    T##GetValue(hash, key)

#endif
