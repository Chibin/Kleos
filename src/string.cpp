#ifndef __STRING__
#define __STRING__

inline b32 IsStringEmpty(char *string)
{
    return strcmp(string, "") == 1;
}

#endif
