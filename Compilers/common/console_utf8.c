// console_utf8 -- print the UTF-8 messages right on a Windows console. See console_utf8.h.

#include "console_utf8.h"

#ifdef _WIN32

#include <stdlib.h>

// the two kernel32 calls, declared here instead of pulling in <windows.h>,
// whose macros (IN, OUT, ERROR, ...) collide with names in the compilers
__declspec(dllimport) int          __stdcall SetConsoleOutputCP(unsigned int cp);
__declspec(dllimport) unsigned int __stdcall GetConsoleOutputCP(void);

#define CP_UTF8_ 65001u

static unsigned int saved_cp;

static void restore_cp(void) { SetConsoleOutputCP(saved_cp); }

void console_utf8(void)
{
    saved_cp = GetConsoleOutputCP();           // 0: no console attached
    if (saved_cp == 0 || saved_cp == CP_UTF8_) return;
    if (SetConsoleOutputCP(CP_UTF8_)) atexit(restore_cp);
}

#else

void console_utf8(void) {}

#endif
