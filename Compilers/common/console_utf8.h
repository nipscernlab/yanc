// ****************************************************************************
// console_utf8 -- the messages are UTF-8; make a Windows console show them so
// ****************************************************************************
//
// The PT messages carry accents, written as UTF-8 in the sources. A Windows
// console decodes output with its own code page (850 or 1252 by default), so
// "instruções" came out as "instruÃ§Ãµes". console_utf8() switches the
// console to UTF-8 (65001) and puts the old code page back when the program
// exits: the setting belongs to the console window, not to the process, and
// would otherwise stay on for whatever runs next in that window. Output that
// goes to a file or a pipe (Aurora, the regress) is untouched: the code page
// only changes how a console draws the bytes. A no-op off Windows.
// ****************************************************************************

#ifndef CONSOLE_UTF8_H
#define CONSOLE_UTF8_H

void console_utf8(void);

#endif
