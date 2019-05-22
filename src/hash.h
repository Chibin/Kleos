#ifndef __HASH_H__
#define __HASH_H__

#define MAX_HASH_SIZE 10

#include "string.h"

struct HashKeyValue
{
    u32 key;
    u32 val;
    HashKeyValue *next;
};

struct HashKeyCharValueU32
{
    char key[24];
    u32 val;
    HashKeyCharValueU32 *next;
};

void SetValues(HashKeyValue *v, u32 key, u32 value, HashKeyValue *next)
{
    v->key = key;
    v->val = value;
    v->next = next;
}

void SetValues(HashKeyCharValueU32 *v, const char *key, u32 value, HashKeyCharValueU32 *next)
{
    memory_index strLen = StringLen(key);
#if 0
    snprintf(v->key, sizeof(v->key), "%s", key);
#else
    memcpy(v->key, key, sizeof(v->key));
#endif
    v->val = value;
    v->next = next;
}

struct Hash
{
    void *list;
    b32 *isEmptyList;
    memory_index bucketCount;
    memory_index (*KeyToHashIndex)(Hash *, void *key);
    struct GameMemory *gm;
};

b32 KeyCompare(u32 a, u32 b)
{
    return a == b;
}

b32 KeyCompare(const char *a, const char *b)
{
    return strcmp(a, b) == 0;
}

#define CREATE_HASH(T, gameMemory, hash, bCount, functionPtr)                                 \
    Hash *hash = (Hash *)AllocateMemory(gameMemory, sizeof(Hash));                            \
    memset(hash, 0, sizeof(Hash));                                                            \
    {                                                                                         \
        memory_index size = sizeof(T) * bCount;                                               \
        hash->bucketCount = bCount;                                                           \
        hash->list = (T *)AllocateMemory(gameMemory, size);                                   \
        memset(hash->list, 0, size);                                                          \
        hash->isEmptyList = (b32 *)AllocateMemory(gameMemory, sizeof(b32) * bCount);          \
        memset(hash->isEmptyList, 1, size);                                                   \
        hash->KeyToHashIndex = functionPtr;                                                   \
        hash->gm = gameMemory;                                                                \
    }

#define ADD_HASH(T, hash, k, value)                                                           \
    {                                                                                         \
        memory_index index = hash->KeyToHashIndex(hash, (void *)&k) % hash->bucketCount;      \
        T *tmp = &((T *)hash->list)[index];                                                   \
        if (hash->isEmptyList[index] == true)                                                 \
        {                                                                                     \
            SetValues(tmp, k, value, 0);                                                      \
            hash->isEmptyList[index] = false;                                                 \
        }                                                                                     \
        else if (KeyCompare(tmp->key, k))                                                     \
        {                                                                                     \
            SetValues(tmp, k, value, tmp->next);                                                      \
        }                                                                                     \
        else                                                                                  \
        {                                                                                     \
            while(tmp->next != 0)                                                             \
            {                                                                                 \
                tmp = tmp->next;                                                              \
            }                                                                                 \
            T *newKeyVal = (T *)AllocateMemory(hash->gm, sizeof(T));                          \
            SetValues(newKeyVal, k, value, 0);                                                \
            tmp->next = newKeyVal;                                                            \
        }                                                                                     \
    }

#define GET_VALUE_HASH(T, hash, k, result)                                                    \
    typeof(T->val) result = {};                                                               \
    {                                                                                         \
        memory_index index = hash->KeyToHashIndex(hash, (void *)&k) % hash->bucketCount;      \
        T *tmp = &((T *)hash->list)[index];                                                   \
                                                                                              \
        if(hash->isEmptyList[index] == false)                                                 \
        {                                                                                     \
            do                                                                                \
            {                                                                                 \
                if (KeyCompare(tmp->key, k))                                                  \
                {                                                                             \
                    result = tmp->val;                                                        \
                    break;                                                                    \
                }                                                                             \
                                                                                              \
            }                                                                                 \
            while((tmp = tmp->next) != 0);                                                    \
        }                                                                                     \
    }

#endif
