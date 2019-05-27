#ifndef __DEFINE_H__
#define __DEFINE_H__

#ifndef WIN32
/* need to figure out where to put this or do something else */
#define sprintf_s(buffer, buffer_size, stringbuffer, ...) (sprintf(buffer, stringbuffer, __VA_ARGS__))
#endif

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define COLOR_WHITE \
    v4              \
    {               \
        1, 1, 1, 1  \
    }
#define COLOR_RED  \
    v4             \
    {              \
        1, 0, 0, 1 \
    }
#define COLOR_GREEN \
    v4              \
    {               \
        0, 1, 0, 1  \
    }
#define COLOR_BLUE \
    v4             \
    {              \
        0, 0, 1, 1 \
    }
#define COLOR_BLACK \
    v4              \
    {               \
        0, 0, 0, 1  \
    }

#define COLOR_YELLOW\
    v4              \
    {               \
        1, 1, 0, 1  \
    }

#define COLOR_RED_TRANSPARENT   \
    v4                          \
    {                           \
        1, 0, 0, 0.3f           \
    }
#define COLOR_BLUE_TRANSPARENT  \
    v4                          \
    {                           \
        0, 0, 1, 0.3f           \
    }
#define COLOR_GREEN_TRANSPARENT \
    v4                          \
    {                           \
        0, 1, 0, 0.3f           \
    }
#define COLOR_YELLOW_TRANSPARENT\
    v4                          \
    {                           \
        1, 1, 0, 0.3f           \
    }
#define COLOR_BLACK_TRANSPARENT \
    v4                          \
    {                           \
        0, 0, 0, 0.3f           \
    }

#define TRANSPARENCY(T)         \
    v4                          \
    {                           \
        0, 0, 0, T              \
    }

#endif
