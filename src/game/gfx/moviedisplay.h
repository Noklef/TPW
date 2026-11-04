// A movie's display made ready: the device's interface, the mode, the palette and the pixel tables. Reimplementation of 0x6700d0; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// What it calls (the ports by default; the bench's recorders and stand-ins: their own benches have them).
struct MovieDisplayEnv {
    u32(__fastcall *pickMode)(u8 *player, void *edx);                        // MovieSurface_PickMode
    void(__fastcall *readSize)(u8 *player, void *edx);                       // MovieSurface_ReadSize
    void(__fastcall *readFormat)(void *unused, void *surface, u32 *out);     // MoviePixels_ReadFormat
    void *(__cdecl *malloc)(u32 size);                                       // CRT_malloc
    void(__fastcall *setupTables)(u8 *player);                               // TgqVideo_SetupTables
    void(__fastcall *buildTables)(const u32 *format, u8 *player);            // MoviePixels_BuildTables
};
void MovieDisplay_SetEnv(const MovieDisplayEnv *env);

extern "C" {

// 0x6700d0 (thiscall on the player): nothing with the settings' +0x88 set. No device +0x10 but an object +0xc: the interface 0x7071f0 asked of it into +0x10 and the player's +0x3e4 1. The mode
// +0x4c picked, +0x1c and +0x20 cleared. Without flag 0x800: the size read, the format of the front +0x14 read into +0x2c, and with 8 bits (+0x48) and no flag 0x4000000 a palette of 0x400 bytes
// (+0x28, cleared only after the palette +0x24 is made from it) set on the front. Without flag 0x100000 the volume +0x78 is 100. The pixel tables built, the table 0xfb7f30's 0x400 dwords the
// clamp of k (-0x200..0x1ff) to -0x80..0x7f, plus 0x80, at (k - 0x180) & 0x3ff; +0x3c 1.
void __fastcall MovieDisplay_Prepare(u8 *player);
}
