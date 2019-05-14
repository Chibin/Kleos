#ifndef __STRING_H__
#define __STRING_H__

memory_index StringLen(const char *from)
{
    memory_index i = 0;
    while(*(from + i) != '\n' && *(from + i) != '\0')
    {
        i++;
    }

    return i;
}

void StringCopy(const char *from, char *to, memory_index len)
{
    while (len --> 0)
    {
        to[len] = from[len];
    }
}

#if 0
char *StringTok(char *source, const char *delimeter, char **context)
{
    static char *g_stringTok = nullptr;

    if (source != nullptr)
    {
        g_stringTok = source;
    }
    char *original

    memory_index sourceLen = StringLen(source);
    memory_index delimLen = StringLen(delimeter);

    memory_index i = 0;
    while (i < sourceLen)
    {
        if (source[i] == delimeter[0])
        {
            b32 delimMatch = true;
            for(memory_index counter = 0; counter < delimLen && i < sourceLen; counter++)
            {
                if(source[i] == delimeter[counter])
                {
                    i++;
                }
                delimMatch = false;
            }
            if (delimMatch)
            {
                source[i] = '\0';
                return g_stringTok;
            }
        }

        i++;
    }

    return nullptr;
}
#endif

inline b32 IsStringEmpty(char *string)
{
    return strcmp(string, "") == 0;
}

#endif
