// What the C library's start-up does with the command line (MSVC6's own names).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6898f0: parse_cmdline: the words of `command` into `words` (a table ending in null, may be null) and `text` (may be null), counted in *wordCount (with the table's end) and *textSize. The first word ends at
// a closing quote or a blank; in the others 2n backslashes before a quote are n and the quote toggles (a doubled one inside quotes is itself), 2n+1 are n and a quote. The table 0xfb1669 bit 4 marks lead bytes.
void __cdecl CRT_parse_cmdline(const char *command, char **words, char *text, s32 *wordCount, s32 *textSize);

}  // extern "C"
