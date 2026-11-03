// The preview of the dialog [0x7cc1f8]: a thing's footprint drawn on its child, its sign and its name. Reimplementation of 0x4ab1b0; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../gfx/draw.h"
#include "../math/rect.h"

struct SurfaceInfo;

// The calls (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7cc1f8PreviewEnv {
    u8 *(__fastcall *find)(u8 *defs, void *, u16 id);                       // ObjectDefs_Find
    u32(__stdcall *hasKey)(u32 key);                                        // GlobalSave_HasKey
    void(__cdecl *sign)(const u8 *rect, u32 type, s32 row);                 // Sign_PlaceOverRect
    u32(__fastcall *pixels)(u8 *window, void *, u32 a, u32 b);              // Window_ImplSlot7
    u32(__fastcall *unlock)(u8 *window);                                    // Window_ImplSlot6
    void(__cdecl *fillInfo)(u32 a, u32 *out, u32 c, u32 d);                 // Format_FillInfo
    void(__cdecl *setInfo)(SurfaceInfo *info);                              // Display_SetSurfaceInfo
    void(__cdecl *boxFill)(const Rect *rect, DrawColour colour);            // Display_BoxFill
    u32 *(__cdecl *classify)(const u8 *object);                             // MapGrid_ClassifyTypes
    u8 *(__fastcall *record)(const u8 *object, void *, u32 index);          // Records_DataIfTerminated
    u16 *(__cdecl *widen)(const u8 *text);                                  // Language_Widen
    u16 *(__cdecl *wideText)(u32 id);                                       // Language_WideText
};
void Dialog7cc1f8_SetPreviewEnv(const Dialog7cc1f8PreviewEnv *env);

extern "C" {

// 0x4ab1b0 (cdecl (id: the word it points to)): nothing without the child 0x1ea of [0x7cc1f8]. The definition of id (ObjectDefs_Find on 0x7890a0); with id not negative and its dword +0xc4 not positive or its key (word +0) in the global save: the sign over the panel's saved rectangle (the
// definition's +0x490, the panel's depth +0xe0 + 10); the child 0x1eb's pixels (Window_ImplSlot7), when there are any: the format info for its width and height made the display's surface, all of it filled clear, then MapGrid_ClassifyTypes of the definition's grid of dwords (+0x400 columns of 0x10,
// +0x404 rows) each cell not 0 a box of (width / 8, height / 8) from the bottom in the colour its kind gives: 1 and 4 0x801eaaff, 2 0x800fdc32, 3 0x80dc640f, any other the last one (sic); then Window_ImplSlot6; the child 0x1ec's slot 9 the definition's name (the record table [0x78bad0] at +0x90,
// Language_Widen). Otherwise the area cleared likewise, then by the word: above 0 the sign of the definition 0x64 and the text 0x89; -1, 0x65 and 0x86; else 0x66 and 0x87.
void __cdecl Dialog7cc1f8_ShowPreview(const u16 *id);
}
