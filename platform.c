
#include <stdbool.h>
#include <memory.h>

#define ZeroStruct(s) memset(&(s), 0, sizeof(s))

#ifdef _WIN32
#	include "win32.c"
#endif
#ifdef __linux__
#	include "linux.c"
#endif
