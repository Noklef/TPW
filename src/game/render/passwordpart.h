// The text part of a password field: its layout with the text masked. Reimplementation of 0x4901d0 (slot 3 of the vtable 0x6ff380); the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The call (the port by default; the bench's recorder).
struct PasswordPartEnv {
    void(__fastcall *setLayout)(u8 *part, void *, u32 flags, const u8 *info);   // TextPart_SetLayout
};
void PasswordPart_SetEnv(const PasswordPartEnv *env);

extern "C" {

// 0x4901d0 (thiscall (flags, info)): without the flag 0x100 TextPart_SetLayout as it is. With it the text [[info +0x10]] copied to 0x7ba058, every unit of that made '*' (0x6d76f0, _wcsset), and TextPart_SetLayout (0x100) with a copy of the 0x18 bytes of info whose +0x10 points to a pointer to 0x7ba058.
void __fastcall PasswordPart_SetLayout(u8 *part, void *, u32 flags, const u8 *info);
}
