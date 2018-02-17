#ifndef __LOGGER__
#define __LOGGER__

#define CALL_LOCATION_FMT "[FILE: %s:%d <%s>]: "
#define CALL_LOCATION_ARGS __FILE__,__LINE__,__func__
#define DEBUG(...) fprintf(stderr, __VA_ARGS__)

#if 1
#define DEBUG_PRINT(_fmt, ...) DEBUG(CALL_LOCATION_FMT _fmt, CALL_LOCATION_ARGS, __VA_ARGS__)

#else
#define DEBUG_PRINT(_fmt, ...) EmptyDebugPrint(_fmt, __VA_ARGS__);
#endif

void EmptyDebugPrint(const char* msgFmt, ...)
{

}

#endif
