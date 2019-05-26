#pragma once
#include <stdio.h>

#define CALL_LOCATION_FMT "[FILE: %s:%d <%s>]: "
#define CALL_LOCATION_ARGS __FILE__, __LINE__, __func__
#define DEBUG(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)

#if 1
#define DEBUG_PRINT(fmt, ...) \
    DEBUG(CALL_LOCATION_FMT fmt, CALL_LOCATION_ARGS, __VA_ARGS__)

#else
#define DEBUG_PRINT(fmt, ...) EmptyDebugPrint(fmt, __VA_ARGS__)
#endif

inline void EmptyDebugPrint(const char * /*_unused*/, ...) // NOLINT
{
}

#define __ERROR_PRINT_INT(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__);
#define ERROR_PRINT(...) __ERROR_PRINT_INT(__VA_ARGS__, "\n");

#if WIN32
#define PAUSE_HERE(fmt, ...)                                             \
    printf(fmt, __VA_ARGS__);                                            \
    printf("Pausing all activity. Press enter in console to resume.\n"); \
    getchar();
#else
#define PAUSE_HERE(fmt, ...)                                             \
    printf(fmt, ##__VA_ARGS__);                                          \
    printf("Pausing all activity. Press enter in console to resume.\n"); \
    getchar();
#endif
