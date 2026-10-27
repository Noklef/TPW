// The GUI library's text tokens: the fields of a line that its window descriptions (the readers about 0x6608bc-0x66131e) are parsed
// from, each one ended by a space or by the end of the text. Reimplementation of 0x66dbfa, 0x66dc2c, 0x66dcf7, 0x66dd68 and
// 0x66ddd8. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x66dbfa (cdecl (text); name unconfirmed): the text past its spaces and tabs.
char *__cdecl GuiToken_SkipBlanks(char *text);

// 0x66dc2c (cdecl (text, out); name unconfirmed): the token (as GuiToken_ReadWord's, terminated in place meanwhile) looked up by strcmp
// among the first [0x77c59c] (a signed word) names of the window kinds' table 0x77c540 ("WINDOW" 1, "BUTTON" 2, ... "KNOB" 0xb;
// 8 bytes each: the name, its word), the first match's word put in out (when not 0). Returns the byte after the token's end, or 0
// when no name matched.
char *__cdecl GuiToken_ReadKind(char *text, u16 *out);

// 0x66dcf7 / 0x66dd68 (cdecl (text, out); names unconfirmed): the token ended by the first space, or by the text's end (strchr with 0),
// terminated in place while atoi reads it into out (when not 0; a word for 0x66dcf7, a dword for 0x66dd68), then its end put back.
// Returns the byte after the end: past the terminator when the token was the last (sic).
char *__cdecl GuiToken_ReadWord(char *text, u16 *out);
char *__cdecl GuiToken_ReadDword(char *text, u32 *out);

// 0x66ddd8 (cdecl (text, out, count: a word); name unconfirmed): as GuiToken_ReadWord, the token copied to out (when not 0) by strncpy
// with count, sign-extended from 16 bits.
char *__cdecl GuiToken_ReadString(char *text, char *out, u32 count);
}
