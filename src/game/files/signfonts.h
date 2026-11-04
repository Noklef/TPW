// A sign style's two font files, extracted from the game's files and added to Windows for the run. Name invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// SignFonts_Add's calls: the ports by default; the bench's recorder.
struct SignFontsEnv {
    u32(__cdecl *extract)(const char *from, const char *to);   // VFS_ExtractFile (0x5c5010)
};
void SignFonts_SetEnv(const SignFontsEnv *env);

extern "C" {

// 0x5ec680 (thiscall; one unused argument; pops 4): the font named at +0xc4, then, when it differs, the one at +0x254: extracted from [0xf86558]\name to [0xf816e8]\[0xf86750]\name, added
// (AddFontResourceA), its path put at the end of the vector at 0x789c18 unless there. Returns 0 at the first that fails, otherwise 1.
u8 __fastcall SignFonts_Add(const u8 *style, void *, u32);

}  // extern "C"
