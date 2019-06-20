#ifndef __HASH_H__
#define __HASH_H__

#define MAX_HASH_SIZE 10

#define CREATE_STRUCT(T, TKey, TVal)                                                                     \
    struct T                                                                                             \
{                                                                                                        \
    TKey key;                                                                                            \
    TVal val;                                                                                            \
    T *next;                                                                                             \
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

#define HASH_CREATE(T, gameMemory, hash, bCount)                                                         \
    Hash *hash = (Hash *)AllocateMemory(gameMemory, sizeof(Hash));                                       \
    memset(hash, 0, sizeof(Hash));                                                                       \
    {                                                                                                    \
        memory_index size = sizeof(T) * bCount;                                                          \
        hash->bucketCount = bCount;                                                                      \
        hash->list = (T *)AllocateMemory(gameMemory, size);                                              \
        memset(hash->list, 0, size);                                                                     \
        hash->isEmptyList = (b32 *)AllocateMemory(gameMemory, sizeof(b32) * bCount);                     \
        for(memory_index i = 0; i < size; i++)                                                           \
        {                                                                                                \
            hash->isEmptyList[i] = true;                                                                 \
        }                                                                                                \
        hash->gm = gameMemory;                                                                           \
    }

#define HASH_ADD(T, hash, k, value)                                                                      \
    {                                                                                                    \
        memory_index index = KeyToHashIndex(hash, k) % hash->bucketCount;                                \
        T *tmp = &((T *)hash->list)[index];                                                              \
        if (hash->isEmptyList[index] == true)                                                            \
        {                                                                                                \
            SetHash(tmp, k, value, 0);                                                                   \
            hash->isEmptyList[index] = false;                                                            \
        }                                                                                                \
        else                                                                                             \
        {                                                                                                \
            b32 found = false;                                                                           \
            if (KeyCompare(tmp->key, k))                                                                 \
            {                                                                                            \
                SetValue(value, &tmp->val);                                                              \
                found = true;                                                                            \
            }                                                                                            \
            while(tmp->next != 0 && found == false)                                                      \
            {                                                                                            \
                tmp = tmp->next;                                                                         \
                if (KeyCompare(tmp->key, k))                                                             \
                {                                                                                        \
                    SetValue(value, &tmp->val);                                                          \
                    found = true;                                                                        \
                    break;                                                                               \
                }                                                                                        \
            }                                                                                            \
            if (found == false)                                                                          \
            {                                                                                            \
                T *newKeyVal = (T *)AllocateMemory(hash->gm, sizeof(T));                                 \
                memset(newKeyVal, 0, sizeof(T));                                                         \
                               SetHash(newKeyVal, k, value, 0);                                          \
                               tmp->next = newKeyVal;                                                    \
            }                                                                                            \
        }                                                                                                \
    }

#define HASH_GET_VALUE(T, hash, k, result, found)                                                        \
    memory_index index = KeyToHashIndex(hash, k) % hash->bucketCount;                                    \
    T *tmp = &((T *)hash->list)[index];                                                                  \
                                                                                                         \
    if(hash->isEmptyList[index] == false)                                                                \
    {                                                                                                    \
        do                                                                                               \
        {                                                                                                \
            if (KeyCompare(tmp->key, k))                                                                 \
            {                                                                                            \
                result = tmp->val;                                                                       \
                found = true;                                                                            \
                break;                                                                                   \
            }                                                                                            \
                                                                                                         \
        }                                                                                                \
        while((tmp = tmp->next) != 0);                                                                   \
    }                                                                                                    \

#define CREATE_HASH_ADD_FUNCTION(T, keyType, valueType)                                                  \
    void HashAdd(Hash *hash, keyType k, valueType value)                                                 \
    {                                                                                                    \
        HASH_ADD(T, hash, k, value);                                                                     \
    }

#define CREATE_HASH_GET_VALUE_FUCTION_PROTOTYPE( T, keyType, valueType)                                  \
    valueType T##GetValue(Hash *hash, keyType k);

#define CREATE_HASH_GET_VALUE_FUCTION(T, keyType, valueType)                                             \
    valueType T##GetValue(Hash *hash, keyType k)                                                         \
    {                                                                                                    \
        valueType result = {};                                                                           \
        b32 found = false;                                                                               \
        HASH_GET_VALUE(T, hash, k, result, found);                                                       \
        if(!found)                                                                                       \
        {                                                                                                \
            return NULL;                                                                                 \
        }                                                                                                \
        return result;                                                                                   \
    }                                                                                                    \

#define HashGetValue(T, hash, key)                                                                       \
    T##GetValue(hash, key)

#define FOR_EACH_HASH_KEY_VAL_BEGIN(T, varName, hash)                                                    \
{                                                                                                        \
    memory_index i = 0;                                                                                  \
    memory_index hashBucketCount = hash->bucketCount;                                                    \
    while(i < hashBucketCount)                                                                           \
    {                                                                                                    \
        if (hash->isEmptyList[i] == true)                                                                \
        {                                                                                                \
            i++;                                                                                         \
            continue;                                                                                    \
        }                                                                                                \
        for(T *varName = &((T *)hash->list)[i]; varName; varName = varName->next)

#define FOR_EACH_HASH_KEY_VAL_END()                                                                      \
        i++;                                                                                             \
    }                                                                                                    \
}

/* This is needed as we're doing a for loop within a while loop. If we just do
 * a normal break, then it will just end the for loop but will still go
 * through the other buckets of the hash */
#define FOR_EACH_HASH_BREAK()                                                                            \
    i = hashBucketCount + 1;                                                                             \
    break;

#endif

/* Macro used to create the function to delete the hash key value pair */
#define CREATE_HASH_DELETE_FUCTION(T)                                                                    \
    void T##DeleteKeyVal(T *keyVal)

#define HASH_DELETE(T, hash, k)                                                                          \
    {                                                                                                    \
        memory_index index = KeyToHashIndex(hash, k) % hash->bucketCount;                                \
        if (hash->isEmptyList[index] == false)                                                           \
        {                                                                                                \
            b32 found = false;                                                                           \
            T *tmp = &((T *)hash->list)[index];                                                          \
            T *prev = tmp;                                                                               \
            if (KeyCompare(tmp->key, k))                                                                 \
            {                                                                                            \
                found = true;                                                                            \
            }                                                                                            \
            while(tmp->next != 0 && found == false)                                                      \
            {                                                                                            \
                prev = tmp;                                                                              \
                tmp = tmp->next;                                                                         \
                if (KeyCompare(tmp->key, k))                                                             \
                {                                                                                        \
                    found = true;                                                                        \
                    break;                                                                               \
                }                                                                                        \
            }                                                                                            \
            T *toDelete = nullptr;                                                                       \
            if (found == true)                                                                           \
            {                                                                                            \
                /* This must mean that it's the first node or the only node*/                            \
                if (tmp == prev)                                                                         \
                {                                                                                        \
                    if(tmp->next == nullptr)                                                             \
                    {                                                                                    \
                        hash->isEmptyList[index] = true;                                                 \
                    }                                                                                    \
                    else                                                                                 \
                    {                                                                                    \
                        /* TODO: This will leave a memory leak */                                        \
                        *tmp = *tmp->next;                                                               \
                    }                                                                                    \
                }                                                                                        \
                else if(tmp->next != nullptr)                                                            \
                {                                                                                        \
                    toDelete = tmp;                                                                      \
                    prev->next = tmp->next;                                                              \
                }                                                                                        \
                else if (tmp->next == nullptr)                                                           \
                {                                                                                        \
                    prev->next = nullptr;                                                                \
                    toDelete = tmp;                                                                      \
                }                                                                                        \
                T##DeleteKeyVal(toDelete);                                                               \
            }                                                                                            \
        }                                                                                                \
    }
