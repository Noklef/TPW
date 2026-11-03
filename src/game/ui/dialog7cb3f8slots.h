// The dialog [0x7cb3f8]'s four user slots shown (name invented, unconfirmed): reimplementation of 0x4a62b0.
#pragma once

#include "../../../re/types.h"

struct Dialog7cb3f8SlotsEnv {
    u8 *(__cdecl *instance)();                                     // UserSlots_Instance
    u8 *(__fastcall *used)(u8 *records, void *, u32 i);            // Records24_IfUsed
    u32(__fastcall *field14)(const u8 *records, void *, u32 i);    // Records24_Field14
    u16 *(__cdecl *wideText)(u32 index);                           // Language_WideText
    u32(__fastcall *setShown)(u8 *window, void *, u32 shown);      // Window_SetShown
};
void Dialog7cb3f8Slots_SetEnv(const Dialog7cb3f8SlotsEnv *env);

extern "C" {

// 0x4a62b0 (cdecl), with a dialog [0x7cb3f8]: for its children 0x7a14, 0x7a19, 0x7a1a, 0x7a1b as slots 0-3: the child 0x7a15's text (slot 9): the record's +8 with a used record, else the text 0x1a5; its child 0x7a16 shown as there is a record; the child 0x7a17 shown as the record's
// Records24_Field14 (0 without one).
void __cdecl Dialog7cb3f8_RefreshSlots();
}
