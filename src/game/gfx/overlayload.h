// The overlays made from the .md2 files of a directory, three kinds tried in turn by their loader (0x4891d0): a window part with a name that ends in the extension. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../core/string.h"

// The directory entry the loader passes by value (0x128 bytes; made by 0x489350): only the file's name is read.
struct OverlayFile {
    u8 before[0x20];
    const char *name;
    u8 after[0x104];

    OverlayFile() = default;
    OverlayFile(const OverlayFile &from) { Record_CopyWithText(reinterpret_cast<u8 *>(this), nullptr, reinterpret_cast<const u8 *>(&from)); }
};
static_assert(sizeof(OverlayFile) == 0x128, "the entry is 0x128 bytes");

// Their calls (the ports by default; the bench's recorders: their own benches have them).
struct OverlayLoadEnv {
    u8 *(__cdecl *load)(const char *name, u32 unused, u32 noPart, u32 third, u32 flag);      // Holder_LoadMd2
    void(__cdecl *textures)(u8 *set, u32 probe, u32 unused);                                 // TextureSet_Load
    u8 *(__fastcall *resized)(u8 *overlay, void *, u8 *holder);                              // Overlay_CtorResized
    u8 *(__fastcall *resizedHeight)(u8 *overlay, void *, u8 *holder);                        // Overlay_CtorResizedHeight
    u8 *(__fastcall *plain)(u8 *overlay, void *, u8 *holder);                                // Overlay_Ctor
    u32(__cdecl *registerWindow)(u8 *window, const char *name);                              // GuiWindows_Register
};
void OverlayLoad_SetEnv(const OverlayLoadEnv *env);

extern "C" {

// All cdecl (file by value, unused, directory); return the overlay or 0. The directory is read through String_AssignWide (the pointer the loader has is a String's, sic) and the file's name appended, the holder Holder_LoadMd2
// (that, 0, 0, 1, 0) (none: 0), TextureSet_Load (its +4, 0, 1), an 0x84-byte overlay (the first two write through 0 when that fails, sic). Names invented.
// 0x488d20: when the name without its last [0x7b96bc] chars (String_Left) equals the String 0x7b9ab8, 0x7b9a80 or "!cbut" (Names_Equal): Overlay_CtorResized, +0x12 = 1, registered under that shorter name.
u8 *__cdecl OverlayLoad_Button(OverlayFile file, u32 unused, const String *directory);
// 0x488ee0: the same with only the String 0x7b93a8: Overlay_CtorResizedHeight, +0x12 = 1, registered as "!slider".
u8 *__cdecl OverlayLoad_Slider(OverlayFile file, u32 unused, const String *directory);
// 0x489060 (directory by value: destroyed here): unless the name starts with '!' or is "w_small_shadow.md2" (0x74fe14): Overlay_Ctor, registered under its model's name (holder +4, +0x78, +0x54); +0x12 not set.
u8 *__cdecl OverlayLoad_Plain(OverlayFile file, u32 unused, String directory);
}
