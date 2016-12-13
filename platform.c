
#include <stdbool.h>
#include <memory.h>
#include <stdint.h>

typedef int64_t int64;
typedef uint64_t uint64;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int8_t int8;
typedef uint8_t uint8;
#ifdef _WIN32
typedef unsigned int uint;
#endif

#define Assert(expr) if (!(expr)) { *(int*)0 = 0; }
#define ZeroStruct(s) memset(&(s), 0, sizeof(s))

#ifdef _WIN32
#	include "win32.c"
#endif
#ifdef __linux__
#	include "linux.c"
#endif
