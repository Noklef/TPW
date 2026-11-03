// The track pieces' texture tables (names invented, unconfirmed): each entry 12 bytes, the handle and the file's name; their folders under the game's base path "%s\queue", "%s\rides\gokarts", "%s\rides\wateride", "%s\hoarding"; and the map's reset that loads them.
// Reimplementations of 0x522900, 0x522aa0, 0x522c60, 0x522e40, 0x52f050.
#pragma once

#include "../../../re/types.h"

// The calls (the game's by default; the bench's recorders).
struct TrackTexturesEnv {
    u32(__cdecl *basePath)();                                                     // 0x40ef40
    s32(__cdecl *format)(char *to, u32 format, u32 base);                         // 0x67a650
    u32(__cdecl *openFolder)(u32 name, const char *folder, u32 open);             // Quickload_LoadRide
    u32(__cdecl *load)(const char *folder, u32 name, u32 *width, u32 *height, u32 flags, u32 id, u32 one, u32 a, u32 b, u32 c, u32 d);  // 0x4629d0
    void(__fastcall *cellReset)(u8 *cell, void *, u32 one);                       // MapCell_Reset
    void(__fastcall *trackReset)(u8 *cell);                                       // TrackCell_Reset
    void(__cdecl *setList)(u32 value);                                            // Global816d48_Set
    void(__cdecl *layEntrance)();                                                 // ParkEntrance_Lay
};
void TrackTextures_SetEnv(const TrackTexturesEnv *env);

extern "C" {

// 0x522900: [0x816d68] and [0x816d6c] 1; the eight entries from 0x763388 (the name at +4, byte +9: flag 0x100000 on 0x12423) loaded, ids 0x4268 up.
void __cdecl TrackTextures_LoadQueue();
// 0x522aa0: [0x8179f8] and [0x8179fc] 2; the entries 0x7633e8 up to 0x7634d8 (+4 the name) with flags 0x10422 and ids 0x42cc up: no name: the entry before's handle; a name starting '-': 0.
void __cdecl TrackTextures_LoadGoKarts();
// 0x522c60: the same for 0x7634d8 up to 0x763604, ids 0x4330 up.
void __cdecl TrackTextures_LoadWaterRide();
// 0x522e40: the entries 0x763608 up to 0x763758, flags 0x422, ids 0x4394 up, no '-' case.
void __cdecl TrackTextures_LoadHoarding();

// 0x52f050: the track state's globals cleared, [0x81d740] and [0x81b1b0] the sizes; every cell in `rows` rows of `columns` reset (its map cell and track cell); the list [0x816d48] cleared, the four tables loaded, the park's entrance laid.
void __cdecl Track_ResetMap(s32 columns, s32 rows);
}
