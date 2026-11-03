// The rows object at [0x7c2534] (the options lists: +0 the rows' handler, +4 their count, +0x10 the parent window): reimplementation of 0x492f60. The names are invented.
#pragma once

#include "../../../re/types.h"

// Its calls (the ports by default; the bench's recorders: their own benches have them).
struct Panel7c2534Env {
    void(__cdecl *measure)(const u16 *text, const s32 *rect, s8 font, s8 hAlign, s8 vAlign, u32 fit, s32 *box);   // GuiText_Measure
    u16(__fastcall *getRect)(u8 *window, void *, u8 *rect, u32 inner);       // Window_GetRect
    u8 *(__fastcall *create)(u8 *parent, void *, u32 kind, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);   // Window_CreateChild
    u16(__fastcall *setHandler)(u8 *window, void *, u32 handler);            // Window_SetHandler
    u16 *(__cdecl *dup)(const u16 *text);                                    // TextPool_Dup
    u32(__fastcall *setDwordA0)(u8 *window, void *, u32 index, u32 value);   // Window_SetDwordA0
    void *(__cdecl *alloc)(u32 size);                                        // GameHeap_Alloc
    u8 *(__fastcall *textPart)(u8 *part, void *, u32 value);                 // TextPart6ff530_Ctor
    void(__fastcall *setImpl)(u8 *window, void *, u8 *impl);                 // Window_SetImpl
    u8 *(__cdecl *font)(u32 index);                                          // Language_Font
    void(__fastcall *set4c)(u8 *window, void *, u8 *object);                 // Window_Set4c
    void(__fastcall *notifyC00)(u8 *window, void *, u32 a, u32 b);           // Window_NotifyC00
    void(__fastcall *notify40000)(u8 *window, void *, u32 a, u32 b, u32 c, u32 d);   // Window_Notify40000
};
void Panel7c2534_SetEnv(const Panel7c2534Env *env);

extern "C" {

// 0x492f60 (thiscall (top, text, id, font, red, green, blue), pops 0x1c; called twelve times by 0x48c1xx-0x48c5xx; name invented): the text measured in 0x800 x 0x600 (GuiText_Measure with that font, box zeroed first); its width
// (box +8 - +0 + 1) << 0xb over the word [0xfaa5c4], its height (box +0xc - +4 + 5) * 0x600 over the word [0xfaa5c0] (idiv both: fractions cut, a zero faults, sic). A child of this +0x10 (kind 1, flags 1, the id) centred across that window's
// inner rectangle, from top down its height. With a child: this +0's handler, the dword 1 of +0xa0 its own copy of the text (TextPool_Dup), a new TextPart6ff530_Ctor part (none when the heap has none), that font, Window_NotifyC00 (1, 1),
// its slot 0x24 the text, the colour with alpha 0xff (Window_Notify40000) and this +4 up one. Returns top + height.
s32 __fastcall Panel7c2534_AddRow(u8 *self, void *, s32 top, const u16 *text, u32 id, u32 font, u32 red, u32 green, u32 blue);
}
