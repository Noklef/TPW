// A GUI text token read as a wide string (guitoken.h has the others). Reimplementation of 0x66de4c. The name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// What it calls besides the CRT's strchr (the game's by default; the bench's recorders).
struct GuiTokenWideEnv {
    u32(__cdecl *strlen)(const char *text);                                  // 0x67ff70
    s32(__cdecl *swprintf)(u16 *to, const u16 *format, const char *text);    // CRT_swprintf
};
void GuiTokenWide_SetEnv(const GuiTokenWideEnv *env);

extern "C" {

// 0x66de4c (cdecl (text, out, count: a word)): the token up to a space or the end (none: 0, as GuiToken_ReadString). With an out and the token's length under twice the count: printed into the out with
// the format 0x77c690. Returns past the token.
char *__cdecl GuiToken_ReadWide(char *text, u16 *out, u32 count);
}
