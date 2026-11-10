// The picture in the online dialog [0x7cc654] drawn. Reimplementation of 0x4bc270; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// What it calls (the ports by default; the bench's recorders and stand-in: their own benches have them).
struct Dialog7cc654PictureEnv {
    void *(__fastcall *surfaceCtor)(void *self);                                                                      // SurfaceInfo_Ctor
    u8 *(__fastcall *findChild)(u8 *window, void *, u32 id);                                                          // Window_FindChild
    u8 *(__fastcall *lock)(u8 *window, void *, u32 *width, u32 *height);                                              // Window_ImplSlot7
    void(__cdecl *fillInfo)(u8 *pixels, void *info, u32 width, u32 height);                                           // Format_FillInfo
    void(__fastcall *infoFromFile)(void *codec, void *, void *out, const char *path, u32, u32);                       // ImageCodec_InfoFromFile
    void(__fastcall *loadFromFile)(void *codec, void *, void *out, void *to, const char *path, u32, u32);             // ImageCodec_LoadFromFile
    void(__fastcall *arrayInit)(void *array, void *, u32 width, u32 height, u32 depth);                               // Array3D_Init
    void(__fastcall *shrink)(void *image, void *, u8 *to, u32 width, u32 height, u32 how);                            // Image_ShrinkInto
    void(__fastcall *unlock)(u8 *window);                                                                             // Window_ImplSlot6
    void(__fastcall *arrayFree)(void *array);                                                                         // Array3D_Free
};
void Dialog7cc654Picture_SetEnv(const Dialog7cc654PictureEnv *env);

extern "C" {

// 0x4bc270 (cdecl): the child 0xee4 of [0x7cc654] locked (none: nothing more). Its format read; the file named at +0x30 of [0x7cc660] (none: 0x6fe5b0) asked and loaded (the to place 0x7cc628). The
// picture [0x7cc640] of [0x7cc644] by [0x7cc648] copied into a new array of 4 bytes each, the bytes turned about (0xff, the third, the second, the first; the fourth left out). By the mode [0x8bd554]
// the array shrunk into the window (mode 2 passes 2, mode 3 passes 1; another mode: not). Unlocked and the array freed.
void __cdecl Dialog7cc654_ShowPicture();
}

#include "../core/gamedata.h"

// The JPEG codec's vtable (its slots ports: tpw.exe's loader points them there), stored by the picture's loader on the stack.
GAME_DATA(void *[4], JpegCodec_vtable, 0x00700140, 0x10);
