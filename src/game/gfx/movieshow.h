// A movie's frame shown: the target locked, the frame's kind decoded into it, the target unlocked. Reimplementation of 0x670c20; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// What it calls (the ports by default; the bench's recorders and stand-ins: each has its own bench, or is another's to port).
struct MovieShowEnv {
    void(__fastcall *readByKind)(u8 *player, const u8 *data);                          // MoviePlayer_ReadByKind
    void(__fastcall *placeView)(u8 *player);                                           // MoviePlayer_PlaceView
    void(__fastcall *createForMode)(u8 *player);                                       // MovieSurface_CreateForMode
    u32(__fastcall *restore)(u8 *player);                                              // MovieSurface_Restore
    void(__cdecl *tgqQuality)(s32 quality);                                            // TgqVideo_SetQuality
    void(__fastcall *tgq32)(u8 *player, const u8 *data, u8 *out, u32 unused, u32 pitch);   // TgqVideo_DecodeFrame32
    void(__fastcall *tgq16)(u8 *player, const u8 *data, u8 *out, u32 unused, u32 pitch);   // TgqVideo_DecodeFrame16
    void(__cdecl *tqiQuality)(s32 *out, s32 quality);                                  // TqiVideo_SetQuality
    void(__fastcall *tqiYuy2)(u8 *player, const u8 *data, u8 *out, u32 unused, u32 pitch); // TqiVideo_DecodeFrameYuy2
    void(__fastcall *tqi16)(u8 *player, const u8 *data, u8 *out, u32 unused, u32 pitch);   // TqiVideo_DecodeFrame16
    void(__fastcall *tqi32)(u8 *player, const u8 *data, u8 *out, u32 unused, u32 pitch);   // 0x6718a0
    void(__fastcall *allocate)(u8 *player, const u8 *data);                            // MovieFrame_Allocate
    void(__fastcall *buildPixels)(u8 *player);                                         // MoviePlayer_BuildPixels
    void(__fastcall *decode)(u8 *player, const u8 *data);                              // MovieFrame_Decode
    void(__fastcall *draw)(u8 *player, u8 *out, u32 unused, u32 pitch);                // MoviePlayer_DrawFrame
    void(__fastcall *madVideo)(u8 *player, const u8 *data, u8 *out, u32 pitch, u32 kind);  // 0x6771a0
};
void MovieShow_SetEnv(const MovieShowEnv *env);

extern "C" {

// 0x670c20 (fastcall (player, entry)): the entry's kind in +0x3a8; the frame's header read and the view placed. Unless the settings' flag 0x800 (there the target is theirs: +0x4c, +0x50, the
// width doubled, again with flag 0x40): the surfaces made, lost ones restored (not: nothing more), and the target by the mode +0x4c locked: 1 the surface of flag 0x400 (flag 0x8000000 the
// back, else the front) or the overlay's, 2 +0x20 with +0x54 else the table +0x648 and the width doubled, 4 the overlay's; a lock refused: nothing more. Another mode: no target, and the data for
// a pitch (sic). By the kind 1-7 the frame decoded (1 and 2 drawn at the place's start; 3 and 4 at the view's place, its pitch doubled by the flags 0x30, with mode 1 and flag 0x400 only for 4).
// Last, unless flag 0x800, the target unlocked: a failure counted in +0x2c, +0x30, +0x34 or +0x38.
void __fastcall MovieShow_Frame(u8 *player, u8 *entry);
}
