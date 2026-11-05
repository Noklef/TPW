// The input filters: objects chained through +8, each passing a message on with its slot 0x18 (or 0x10) or marking the message unhandled (+8 = 0) at the chain's
// end; the DirectInput device holder they read (0x703c80's class, the device at +0xc); the modifier keys and the mouse's double click; the event queue's
// subscription list (0x703744). Reimplementation of 0x633200-0x633db0 and 0x634130-0x6348f0. Names invented: the classes are known by their vtables only.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

extern "C" {

// 0x632350 (thiscall; object, pops 4): the next filter at +8 replaced: the old one released (slot 0), the new one kept (slot 4).
void __fastcall InputFilter_SetNext(u8 *self, void *, u8 *object);

// 0x633200 (thiscall): the handle at +8 closed and cleared, when there is one.
void __fastcall InputFilter_CloseHandle(u8 *self);

// 0x633250 (thiscall; object, pops 4): the object at +0xc replaced: the old one released (+0xc = 0 first; slot 0), the new one kept (slot 4).
void __fastcall InputFilter_SetSink(u8 *self, void *, u8 *object);

// 0x633510 (thiscall; message, pops 4): the byte at message +0x1c kept in the table at +0x1c by message +0x10, the message passed on (slot 0x18).
void __fastcall KeyFilter_OnMessage(u8 *self, void *, u8 *message);

// 0x6336d0 (stdcall; a, b, object): the event queue's callback: the object's slot 0 with a and b.
void __stdcall InputSubs_Dispatch(u32 a, u32 b, u8 *object);

// 0x6336f0 (thiscall): the destructor: vtable 0x703744, then Unsubscribe.
void __fastcall InputSubs_Dtor(u8 *self);
// 0x633700 (thiscall; list, object, pops 8): Unsubscribe, then the object listens on each channel of the list (6 ends it).
void __fastcall InputSubs_Subscribe(u8 *self, void *, const s32 *list, u32 object);
// 0x633750 (thiscall): with a list at +4, the object at +8 stops listening on each of its channels, +4 moving on, and +4 = +8 = 0.
void __fastcall InputSubs_Unsubscribe(u8 *self);

// 0x6337a0 (thiscall; message, pops 4): the handler table at +0xc told of a mouse button (0xf0-0xf3: slots 0x10, 4) or another message (slots 0xc, 0), the context
// at +0x10 last; then the message passed on (slot 0x18).
void __fastcall HandlerFilter_OnMouse(u8 *self, void *, u8 *message);
// 0x633860 (thiscall; message, pops 4): the table's slot 8 (not checked: the table must be there) with the message's word +0xc and dword +0x10; passed on (slot 0x10).
void __fastcall HandlerFilter_OnChar(u8 *self, void *, u8 *message);

// 0x633a50 (thiscall): vtable 0x703c80; the device at +0xc released (COM slot 8).
void __fastcall DeviceHolder_Dtor(u8 *self);
// 0x633a70 (thiscall; result out, device out, pops 8): the device with a reference, in device out (no check of +0xc). A null device out: E_POINTER. Returns result out.
u32 *__fastcall DeviceHolder_GetDevice(u8 *self, void *, u32 *result, u32 *device);
// 0x633aa0 (thiscall; result out, pops 4): the cooperative level wanted (+4) for the window (+0x1c, or the game's): unacquires unless +0x10 is set and nothing changes;
// level 10 for the old window first (+8 records it) when the window changes.
u32 *__fastcall DeviceHolder_SetLevel(u8 *self, void *, u32 *result);
// 0x633b90 (thiscall; result out, pops 4): SetLevel, then Acquire when +0x10 is set. S_FALSE comes back as 0x4ffe0002.
u32 *__fastcall DeviceHolder_Apply(u8 *self, void *, u32 *result);
// 0x633db0 (thiscall): the COM object at *self released, when there is one.
void __fastcall ComHolder_Release(u32 *self);

// 0x634130 (thiscall; holder + 0x10, pops 4): the keyboard's state read into the flags at +0x20: alt 4, control 2, shift 1. Only for this object's own holder
// (+0x1c); a failed read leaves 0. The device's reference is kept (sic).
void __fastcall KeyFilter_ReadModifiers(u8 *self, void *, u8 *holder);

// 0x634220 (thiscall): the mouse filter's constructor: vtable 0x703cc8, +0x30 = 1.0f, the system's mouse parameters at +0x50, the double click time at +0x44; a swapped
// mouse button pair sets +0x60 (and is swapped back on, after the test of SwapMouseButton(0)). Returns the object.
u8 *__fastcall MouseFilter_Ctor(u8 *self);
// 0x634420 (thiscall; message, pops 4): swapped buttons exchanged (0xf0, 0xf1); the second press of a button within the time at +0x4c: flag 0x10; the position at +0x10
// copied into the message; passed on (slot 0x18).
void __fastcall MouseFilter_OnMessage(u8 *self, void *, u8 *message);
// 0x6344f0 (thiscall): the float position at +0x20 clamped to the bounds at +0x34, truncated into +0x10.
void __fastcall MouseFilter_ClampPosition(u8 *self);
// 0x634560 (thiscall on +0x64's second base; unused, message, pops 8): a message of type 7: its two dwords +4, +8 as the bounds' second pair (the first 0) of the object.
void __fastcall MouseFilter_OnBounds(u8 *self, void *, u32 unused, const u32 *message);

// 0x634720 / 0x634730 (thiscall; object, pops 4): the object's slot 0x14 / 0x18 with this filter.
void __fastcall ChainFilter_Attach(u8 *self, void *, u8 *object);
void __fastcall ChainFilter_Detach(u8 *self, void *, u8 *object);

// 0x633160 (thiscall): vtable 0x703c18, the sink at +0xc released (slot 0), the handle closed, vtable 0x703770. 0x633140 (thiscall (flags), pops 4): with bit 0, freed. Returns self.
void __fastcall InputFilter_Dtor(u8 *self);
u8 *__fastcall InputFilter_DeletingDtor(u8 *self, void *, u32 flags);
// 0x633360 (thiscall): the key filter's constructor: vtable 0x703c40, 0x703c38 at +0xc, the table at +0x1c cleared, listening on the channels at 0x703c30.
u8 *__fastcall KeyFilter_Ctor(u8 *self);
// 0x633600 (thiscall; holder + 0x10 as a number, pops 4): the holder when it is this object's +0x11c: the keys, 0xf0 bytes of a 0x100 read, into +0x1c; its +0x120: the fourth dword of a
// 0x10 read into +0x10c. The device's references: GetDevice's each, one released at the end (sic).
void __fastcall KeyFilter_ReadState(u8 *self, void *, u32 holder);
// 0x633a30 (thiscall (flags), pops 4): DeviceHolder_Dtor; with bit 0, freed. Returns self.
u8 *__fastcall DeviceHolder_DeletingDtor(u8 *self, void *, u32 flags);
// 0x633d10 (thiscall; result out, flags, pops 8): exactly one of 1 / 2 and one of 4 / 8, or -1: those bits of +4 set, the other of each pair cleared, then Apply.
u32 *__fastcall DeviceHolder_SetMode(u8 *self, void *, u32 *result, u32 flags);
// 0x633ea0 (thiscall; result out, pops 4): listening at +0x10 on the channels at 0x703c90 with +0xc (null for no object). 0: returned.
u32 *__fastcall InputFilter_SubscribeC90(u8 *self, void *, u32 *result);
// 0x6341d0 (thiscall; result out, holder, pops 8): the holder at +0x1c, the modifiers read from it. 0: returned.
u32 *__fastcall KeyFilter_SetHolder(u8 *self, void *, u32 *result, u8 *holder);
// 0x6345a0 (thiscall; result out, pops 4): listening at +0x68 on the channels at 0x703cc0 with +0x64 (null for no object). 0: returned.
u32 *__fastcall MouseFilter_Subscribe(u8 *self, void *, u32 *result);
// 0x63e7c0 (thiscall): an input filter (vtable 0x703fd8) with a device holder at +0x10 (vtable 0x703ff0). Returns self.
u8 *__fastcall DeviceFilter_Ctor(u8 *self);
// 0x63e840 (thiscall): the block at +0x30 freed, the holder's and the filter's destructors. 0x63e820 (thiscall (flags), pops 4): with bit 0, freed. Returns self.
void __fastcall DeviceFilter_Dtor(u8 *self);
u8 *__fastcall DeviceFilter_DeletingDtor(u8 *self, void *, u32 flags);

// 0x6348e0 / 0x6348f0: the constants 0x707c40 and 0x679440.
u32 __cdecl MouseFilter_Constant1();
u32 __cdecl MouseFilter_Constant2();

}  // extern "C"

void InputFilter_SetCalls(const AddrCall *table, u32 count);
