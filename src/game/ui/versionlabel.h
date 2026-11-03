// The small window with the text "v 2.0" (name invented, unconfirmed): reimplementation of 0x48b020.
#pragma once

#include "../../../re/types.h"

struct VersionLabelEnv {
    void(__cdecl *measure)(const u16 *text, const s32 *rect, s8 font, s8 hAlign, s8 vAlign, u32 fit, s32 *box);  // GuiText_Measure
    u8 *(__fastcall *create)(u8 *parent, void *, u32 kind, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);  // Window_CreateChild
    u8 *(__fastcall *part)(u8 *part, void *, u32 value);    // TextPart_Ctor
    void(__fastcall *setImpl)(u8 *window, void *, u8 *impl);    // Window_SetImpl
    void(__fastcall *set4c)(u8 *window, void *, u8 *object);    // Window_Set4c
    void(__fastcall *notify)(u8 *window, void *, u32 a);    // Window_NotifyImpl100
};
void VersionLabel_SetEnv(const VersionLabelEnv *env);

extern "C" {

// 0x48b020 (cdecl (front)): the text formatted from 0x750080 (2, 0) and measured in font 6 in a box of 0x7d00 square; a child of kind 1, flags 3, id 0x8cfd55 of [0xfaa5d8] (front not 0) or of [[0xf82884] + 4],
// its bottom right at (0x7c0, 0x5c0) and its size the text's over the GUI's scales [0xfaa5c4], [0xfaa5c0] (s16; width << 11, height * 0x600); a new TextPart (0x1a4 bytes, none if the allocation fails) as its impl,
// the font entry of [0x78b534] (0x78b5dc, 0x138 each) as its +0x4c, and the text sent.
void __cdecl VersionLabel_Create(u8 front);
}
