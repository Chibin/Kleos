#ifndef __HASH_H__
#define __HASH_H__

#define MAX_HASH_SIZE 10

#include "math.h"
#include "game_memory.h"

struct HashKeyValue
{
    u32 key;
    u32 val;
    HashKeyValue *next;
};

struct Hash
{
    void *list;
    memory_index (*GetValue)(Hash *, u32);
    GameMemory *gm;
};

#define CREATE_HASH(T, gameMemory, hash, size, functionPtr)        \
    Hash *hash = (Hash *)AllocateMemory(gameMemory, sizeof(Hash)); \
    memset(hash, 0, sizeof(Hash));                                 \
    {                                                              \
        hash->list = (T *)AllocateMemory(gameMemory, size);        \
        memset(hash->list, 0, size);                               \
        hash->GetValue = functionPtr;                              \
        hash->gm = gameMemory;                                     \
    }

#define ADD_HASH(T, hash, k, value)                                \
{                                                                  \
    memory_index index = k % MAX_HASH_SIZE;                        \
    T *tmp = &((T *)hash->list)[index];                            \
    if (tmp->key != k && tmp->val != 0)                            \
    {                                                              \
        while(tmp->next != 0)                                      \
        {                                                          \
            tmp = tmp->next;                                       \
        }                                                          \
        T *newKeyVal = (T *)AllocateMemory(hash->gm, sizeof(T));   \
        newKeyVal->key = k;                                        \
        newKeyVal->val = value;                                    \
        newKeyVal->next = 0;                                       \
        tmp->next = newKeyVal;                                     \
    }                                                              \
    else                                                           \
    {                                                              \
        tmp->key = k;                                              \
        tmp->val = value;                                          \
        tmp->next =  0;                                            \
    }                                                              \
}

#endif
