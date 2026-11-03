// The dialog [0x7ca210] (script 0x751798, handler 0x498c60) and the name buffer 0x7ba058. Reimplementation of 0x498d80 and 0x486280; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7ca210Env {
    s32(__cdecl *atExit)(void(__cdecl *f)());                    // 0x67a7d0
    void(__cdecl *attach)(u8 *parent, u32 id, u32 text);         // HoverLabel_Attach
    void(__fastcall *copyName)(u8 *thing, void *, u8 *buffer, u32 size);   // 0x4e2780 (agent A's)
};
void Dialog7ca210_SetEnv(const Dialog7ca210Env *env);

extern "C" {

// 0x486280 (cdecl (thing); name invented, unconfirmed): 0x4e2780 of the thing given the buffer 0x7ba058 and 0x3fff; returns the buffer.
u8 *__cdecl Thing_NameBuffer7ba058(u8 *thing);

// 0x498d80 (cdecl; name invented, unconfirmed): open already: nothing. Window7c24c8_Close, Global7cb28c_Select (3, 10), the dialog built (not checked), made [0x7c24c8]. Its child 0x4f3af the handler 0x499060 and the hover label the name
// (Thing_NameBuffer7ba058) of the table 0x7cfb90's entry of the game's word +0x1da732; 0x4f3b0 a heading: the bytes 1, 1, font 6, black, the text 0xa0; 0x4f3ae panelparts::rangedNumber (0..10000, step 1), slot 7 [0x7ca1e8] = the money's +0x118;
// 0x4f3b1 Window705a50_SetState (not 0 / 0: [0x7ca208] = Get1da710_51a280, 1). The mode setting 0xfb3b7c, read the first time (the guard 0xfb34c5 & 1), 2: the child 0x4f3ac's slot 5 (0). Advisor_PostId (0x12f).
void __cdecl Dialog7ca210_Open();
}
