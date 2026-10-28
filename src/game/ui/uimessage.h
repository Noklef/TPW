// The game's messages passed on to the interface object at 0x7b51f0 (the thing message thunk 0x50b629's handler): reimplementation of
// 0x50ca50. Names invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What the handler calls on 0x7b51f0 (the game's ports by default; tools/fncmp's driver points them at recorders).
struct UiMessageEnv {
    void(__fastcall *checkOnline)(void *, void *, u8 flag);             // Obj7b51f0_CheckOnline 0x481af0
    void(__fastcall *refreshResearch)(void *, void *, u32 group);      // Obj7b51f0_RefreshResearch 0x481520
    void(__fastcall *passThing)(u8 *, void *, u32 handle);             // Obj7b51f0_PassThingTo7cc218 0x481530
    void(__fastcall *guestRemoved)(void *, void *, u32 handle);        // Obj7b51f0_OnGuestRemoved 0x4818c0
    void(__fastcall *guestAdded)(void *, void *, u32 handle);          // Obj7b51f0_OnGuestAdded 0x481920
    void(__fastcall *staff)(void *, void *, u32 handle, u32 added);    // Panel7cc328_OnStaff 0x481960
};
void UiMessage_SetEnv(const UiMessageEnv *env);   // nullptr: the game's

// 0x50ca50 (stdcall (message), pops 4): the message's type (its vtable's slot 0) picks what 0x7b51f0 is told: 0x13 with +8 5 / 6
// Obj7b51f0_CheckOnline(1 / 0); 0x12 Obj7b51f0_RefreshResearch(+8); 0xa Stub_Ret4b; 7 and 8 Obj7b51f0_PassThingTo7cc218, 0x1b
// Obj7b51f0_OnGuestRemoved, 0x1c Obj7b51f0_OnGuestAdded (each of the word +8, its high half the message pointer's: the original builds it
// in its argument's slot); 0xf Panel7cc328_OnStaff(the word +8 (its high half another register's, not read), +0xc). Others: nothing.
void __stdcall UiObject_HandleMessage(u8 *message);
}
