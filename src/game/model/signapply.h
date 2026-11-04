// A sign's text drawn onto a model's sign textures: reimplementation of 0x467a80.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

// The game's calls by address (tests/fncmp/signapply.py gives recorders); GetDC and ReleaseDC go through their import slots.
void Sign_SetApplyCalls(const AddrCall *table, u32 count);

// 0x467a80 (cdecl; Ride_LoadResource, 0x4682b0): Renderer_Construct on a local sign renderer, Renderer_Teardown at each end; no text: 0.
// text2 given: the two lines as they are. Else text is split at the space nearest its middle (half, half - 1, half + 1, half - 2, ...), its whole on the first line without one.
// The model (object +8's +4): of its +0x36 records (+0x54, 0x10 bytes; entries +0x50, 8 bytes) flag 8 is the first texture, flag 0x10 the second, the last of each kind:
// the entry's byte +1 gets 4, the record's texture +0xc taken, its bit 1 clear asks for the saved pairs. Else one without flag 2, word +0xa 1 and its +0xc "sign1.tga" or "sign2.tga"
// (_stricmp), when that kind has none yet: a block of 0x2c zeroed bytes shared (GlobalMem_Alloc; the second's at +0x16) has words +6, +8 0x80, is its +0xc, the record gets 0xa or 0x12,
// the entry 0x480, the saved pairs asked for. Other than two found: flag 2 and entry flag 0x80 off where they were found, the block freed, 0.
// Saved pairs asked: Object_MakeSavedPairs130 on object +8's +0x1c or else +0x20 of the two indices (-1 for none) and 2, the indices at each texture's +0x12 and their first dwords' bit 1 set whatever it answers; 0: as above.
// Then the parts holding the two entries (faces +0x6c, 0x10 bytes, count word +0x5a) RideBase_TidyUVs'd with 0.02: an entry on no part means every part without flag 0x200 (+0x70, 0xa0 bytes,
// below word +0x44; else +0x74, 0x58 bytes); the second only if its part differs. The folder without a last '\'; the style given, or the folder's last name;
// "<folder>\<language>_<style>.sgn" if VFS_FileExists, else "<folder>\<style>.sgn": SignStyle_Load, 0x5ec680 (fonts), SignStyle_Render on the window 0x7a130c's DC of
// {line 1, line 2, the two +0x12s, 0x80, 0x80, 2 for 0x8bd554 == 2, else 1 for 3, else 2}. The model's +0x30 gets 0x8000, object +4 without bit 0 gets 0x8000000; 1.
// With two of one kind the other texture is what the stack had: the last place looked at in the text, or its length (both sic, kept here; with no such scan the original's stack garbage).
// The original's Debug_Printf for one found is left out.
extern "C" {
u32 __cdecl Sign_Apply(u8 *object, const char *folder, const char *style, const char *fonts, const u16 *text, const u16 *text2);
}

// 0x4682b0 (cdecl (object, text, text2); name invented, unconfirmed): the object's sign applied. No definition (Object_GetDef) or no index at +0x20: 0, and no folder at [0xf816c8]: 0. Fonts: the folder and "Data\Fonts" (the backslash between when the folder has none). The holder [0x7a4610 + 4 * index]. With signs on ([0x78d8d8]): the definition's
// wide name (+0x390) in ANSI as the style, Sign_Apply(holder, the definition's +0x28c, the style, the fonts, the texts) its result; off: 1. A holder without bit 0 at +4 gets bit 0x8000000.
extern "C" {
u32 __cdecl Sign_ApplyObject(u8 *object, const u16 *text, const u16 *text2);
}
