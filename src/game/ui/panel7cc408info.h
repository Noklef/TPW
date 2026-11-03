// The people panel 0x6fff40's facts about the shown visitor. Reimplementation of 0x4b72a0; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../core/filetime.h"

// The calls (the ports by default; 0x4e2780 agent A's, by address until ported; the bench's recorders: their own benches have them).
struct Panel7cc408InfoEnv {
    s32(__cdecl *format)(u8 *charset, u16 *dest, u32 size, u32 entry, const u32 *arg);        // Charset_FormatWide
    s32(__cdecl *formatPlain)(u8 *charset, u16 *dest, u32 size, u32 entry);                   // the same without an argument
    void(__cdecl *attach)(u8 *parent, u32 id, u32 text);                                      // HoverLabel_Attach
    u32(__fastcall *timeInPark)(const u8 *visitor);                                           // Visitor_TimeInPark
    FileTime *(__fastcall *spanOf)(const u8 *clock, void *, FileTime *out, u32 ticks);        // FunnyTime_SpanOf
    void(__fastcall *copyName)(u8 *thing, void *, u8 *buffer, u32 size);                      // 0x4e2780
    void(__fastcall *notify1)(u8 *window, void *, u32 value);                                 // Window_Notify1
    u8 *(__fastcall *record)(const u8 *object, void *, u32 index);                            // Records_DataIfTerminated
    u16 *(__cdecl *widen)(const u8 *text);                                                    // Language_Widen
};
void Panel7cc408Info_SetEnv(const Panel7cc408InfoEnv *env);

extern "C" {

// 0x4b72a0 (thiscall; slot 5 of the vtable 0x6fff40): none shown ([0x7c2658] clear): its slot 0xb, nothing more. With its dword +0x24 clear, that takes the visitor's +0x1d8 and, not 0, the text 0x1c8 of [0x78ba94] made of it (the buffer 0x7ba058) is the hover label 0x3736 of its window +0x10.
// With a page +0x14, each child found gets its slot 7: 0x3724 the +0x1a0, 0x372d the hours in the park (FunnyTime_SpanOf of [0x7cf83c] + 0x2a0 and 4 times Visitor_TimeInPark, over 36000000000), 0x372e-0x3731 the +0x1c4, +0x1c8, +0x1cc, +0x1d0, 0x3732 the low byte of __ftol (float +0x19c) << 10 over 100;
// the child 0x3733 (not checked) its slot 9: the text of [0x78baa0] at the +0x220, and for 0x10 made of the name (0x4e2780 of the table [0x7cfb90] at the word +0x1dc, 0x14 bytes each; 0x40 bytes) as the argument. Last, of the window's child 0x3734: its child 0x3735's Window_Notify1 (+0x30 less 1, or 0x3e7 for none), and its own slot 9 the text of
// [0x78ba9c] at the +0x30, widened.
void __fastcall Panel7cc408_ShowVisitor(u8 *self);
}
