#ifndef __ARRAY_H__
#define __ARRAY_H__

#define ARRAY_INIT_SIZE 10

#define ARRAY_RAW(arr) (memory_index *)((memory_index *)arr - 2)
#define ARRAY_LIST_CAPACITY(arr) (*((memory_index *)((memory_index *)arr - 2)))
#define ARRAY_LIST_SIZE(arr) (*((memory_index *)((memory_index *)arr - 1)))

#define ARRAY_CREATE(T, gameMemory, arr)                                                          \
    /* Use curly to create a local scope for the values */                                        \
    T *arr = nullptr;                                                                             \
    {                                                                                             \
        memory_index *raw = (memory_index *)AllocateMemory(gameMemory, sizeof(memory_index) * 2); \
        raw[0] = 0; /*capacity*/                                                                  \
        raw[1] = 0; /*size*/                                                                      \
        /* This is done so that we can figure out the capacity and size */                        \
        arr = (T *)&raw[2];                                                                       \
    }

#define ARRAY_PUSH(T, gameMemory, arr, value)                                 \
    {                                                                         \
        memory_index *raw = ARRAY_RAW(arr);                                   \
        if (raw[0] == 0)                                                      \
        {                                                                     \
            raw = (memory_index *)ReallocMemory(                              \
                    gameMemory,                                               \
                    raw,                                                      \
                    0,                                                        \
                    sizeof(memory_index) * 2 + sizeof(T) * ARRAY_INIT_SIZE);  \
            raw[0] = 10;                                                      \
            arr = (T *)&raw[2];                                               \
        }                                                                     \
        else if (raw[1] > raw[0])                                             \
        {                                                                     \
            memory_index newCapacity = SafeCastToU32(raw[0] * 0.25f);         \
            raw = (memory_index *)ReallocMemory(                              \
                    gameMemory,                                               \
                    raw,                                                      \
                    sizeof(memory_index) + raw[0] * sizeof(T),                \
                    sizeof(memory_index) * 2 + sizeof(T) * newCapacity);      \
            raw[0] = newCapacity;                                             \
            arr = (T *)&raw[2];                                               \
            /* Realloc should take care of the size value being transfered */ \
        }                                                                     \
                                                                              \
        T *data = (T *)&raw[2];                                               \
        data[raw[1]] = value;                                                 \
        raw[1]++;                                                             \
    }

#endif
