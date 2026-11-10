// The input system at 0xfa6e40 (vtable 0x703710, base 0x7037e8) and its parts: reimplementation of 0x617610-0x618c80 and the wrappers 0x5f5590-0x5f5620. All names invented, unconfirmed.
// The library at 0x631870-0x634ee0 is not ported: it goes through the env. "out" is a result code's place, returned.
// Fields: +0x10 the part 0x703c14, +0x54 a stop flag, +0x58 and +0x5c the two devices (their +0x10 the part that polls), +0x60, +0x64, +0x68 (0x24, 0x74, 0x44 bytes) the chain, +0x6c-+0x78 the system's own objects.
#pragma once

#include "../../../re/types.h"

struct EventQueue;

struct InputSystemEnv {
    using Ctor = u8 *(__fastcall *)(u8 *);
    using Plain = void(__fastcall *)(u8 *);
    using Arg = void(__fastcall *)(u8 *, void *, u32);
    using Out = u32 *(__fastcall *)(u8 *, void *, u32 *);
    using OutArg = u32 *(__fastcall *)(u8 *, void *, u32 *, u32);
    Ctor c631870, c633360, c633900, c634220, c6345d0, c634ee0;
    Ctor classCa0;   // Class703ca0_Ctor (a recorder in the bench: its vtable's slot 2 is the library's)
    Plain f631fc0, f632020, f6323e0, f6336f0, f633160, f633920, f634310;
    Arg f633250, f632350;
    Out f6326e0, f633b90, f633340, f633ea0, f6345a0, f634a30;
    OutArg f6328b0, f6328d0, f6335b0, f633560, f6341d0, f633d10;
    void(__fastcall *f633700)(u8 *, void *, u32, u8 *);
    void(__fastcall *post)(EventQueue *, void *, u32, const void *);   // EventQueue_Post
    s32(__cdecl *pump)();   // App_PumpMessages
};
void InputSystem_SetEnv(const InputSystemEnv *env);  // nullptr: the game's

extern "C" {

// 0x617610 (thiscall (flags), pops 4): InputSystem_Dtor, freed for bit 0. Returns the object. As 0x617de0, 0x618240, 0x6189b0, 0x617950 for their destructors.
u8 *__fastcall InputSystem_DeletingDtor(u8 *o, void *, u32 flags);
// 0x617630 (thiscall): vtable 0x703710; +0x80's slot 1 (1) when set; +0x84 = 0x6fef90; +0x78, +0x74, +0x70, +0x6c's slot 0 when set; InputBase_Dtor.
void __fastcall InputSystem_Dtor(u8 *o);
// 0x6176e0 (thiscall (a, limit, c), pops 12): a and c into +0x6c's +0xc and +0x10; 0x631fc0 on +0x68; the count its slot 7 answers, at most a limit when there is one; that many times its slot 8 (&local, +0x70), both
// reread each time; 0x632020. Returns the count.
u32 __fastcall InputSystem_Update(u8 *o, void *, u32 a, u32 limit, u32 c);
// 0x617760 (thiscall (out), pops 4): InputBase_Init, then +0x6c, +0x70, +0x74, +0x78 created, the first failure the result. 0x633250 on +0x74 (+0x60); 0x6328b0 on +0x10 (&local, +0x74) the result; not failed, 0x632350 on +0x70 (+0x6c).
u32 *__fastcall InputSystem_Init(u8 *o, void *, u32 *out);
// 0x617860 (thiscall (out), pops 4): no +0x6c: the object's slot 2 (&local), failure the result. InputBase_OpenDevice5c the result (whatever it is, 0x6335b0 on +0x70 (&local, +0x5c) follows).
u32 *__fastcall InputSystem_OpenDevice5c(u8 *o, void *, u32 *out);
// 0x6178c0 (thiscall (out), pops 4): as InputSystem_OpenDevice5c with InputBase_OpenDevice58, 0x633560 and +0x58.
u32 *__fastcall InputSystem_OpenDevice58(u8 *o, void *, u32 *out);
// 0x617950 (thiscall (flags), pops 4): 0x6336f0, freed for bit 0. Returns the object.
u8 *__fastcall Class703744_DeletingDtor(u8 *o, void *, u32 flags);
// 0x617bd0 (cdecl (out, object)): null object 0x80004003; 0x124 bytes and 0x633360 (nothing got or answered: 0xcffe0100); the object set and 0. Returns out.
u32 *__cdecl InputSystem_CreateDevice70(u32 *out, u8 **object);
// 0x617c70 (cdecl (out, object)): null object 0x80004003; 0x38 bytes (none: 0xcffe0100): Class703c18_Ctor, the part at +0x10 (vtable 0x7037b8, list at +0x14 with one 0x14-byte block or a null, +0x20 its head), the block at +0x24, 0x633900 on +0x34,
// vtables 0x7037a0 and 0x70379c; 0x633340 the result: not failed, the object set, else its slot 2 (1).
u32 *__cdecl InputSystem_CreateDevice74(u32 *out, u8 **object);
// 0x617de0 (thiscall (flags), pops 4), the thunk 0x6181b0 from +0x10.
u8 *__fastcall InputDevice74_DeletingDtor(u8 *o, void *, u32 flags);
u8 *__fastcall InputDevice74_DeletingDtorThunk(u8 *part, void *, u32 flags);
// 0x617e00 (thiscall): vtables; 0x633920 on +0x34; +0x28's block freed, +0x28-+0x30 cleared; the part +0x10 as Class7037b8_Dtor; 0x633160.
void __fastcall InputDevice74_Dtor(u8 *o);
// 0x618240 (thiscall (flags), pops 4).
u8 *__fastcall InputBase_DeletingDtor(u8 *o, void *, u32 flags);
// 0x618260 (thiscall): vtable 0x7037e8; InputBase_CloseDevice5c, InputBase_CloseDevice58; +0x68, +0x64, +0x60, +0x5c, +0x58's slot 0 when set; 0x6323e0 on +0x10; 0x6336f0 on +4; vtable 0x703748.
void __fastcall InputBase_Dtor(u8 *o);
// 0x618330 (thiscall (out), pops 4): 0x6326e0 on +0x10, +0x60, +0x64, +0x68 created, the first failure the result; not failed, 0x632350 on +0x64 (+0x68) and on +0x60 (+0x64), 0x633700 on +4 (0x7037e0, the object).
u32 *__fastcall InputBase_Init(u8 *o, void *, u32 *out);
// 0x618400 (thiscall (out), pops 4): set already 0xcffe0102. +0x5c created, 0x6328b0 on +0x10 (&local, +0x5c): failure the result. 0x633250 on it (+0x60); 0x633b90 on its +0x10; 0.
u32 *__fastcall InputBase_OpenDevice5c(u8 *o, void *, u32 *out);
// 0x618490 (thiscall (out), pops 4): +0x5c set: 0x6328d0 on +0x10 (&local, it); still set, cleared and its slot 0. out 0.
u32 *__fastcall InputBase_CloseDevice5c(u8 *o, void *, u32 *out);
// 0x6184d0 (thiscall (out), pops 4): the same for +0x58.
u32 *__fastcall InputBase_CloseDevice58(u8 *o, void *, u32 *out);
// 0x618510 (thiscall (out), pops 4): as InputBase_OpenDevice5c for +0x58; 0x6341d0 on +0x60 (&local, +0x58) after 0x633250.
u32 *__fastcall InputBase_OpenDevice58(u8 *o, void *, u32 *out);
// 0x6185b0 (thiscall (out, value), pops 8): Obj_Set10 on both devices' +0x10; InputBase_PollAll.
u32 *__fastcall InputBase_SetDeviceValue(u8 *o, void *, u32 *out, u32 value);
// 0x618610 (thiscall (unused, event), pops 8): kind 4: OutputDebugStringA(0x7780d0), ShowCursor(no [event+4]), and with it and +0x54 clear InputBase_SetDeviceValue. 6: +0x54 = 1. 7: 0x633d10 on each device's +0x10 (&local, flags: [event+0x10] set:
// [0x703c78] | [0x703c70] on both; else [0x703c74] | [0x703c70] on +0x58 and [0x703c74] | [0x703c6c] on +0x5c); InputBase_PollAll, +0x54 = 0.
void __fastcall InputBase_OnEvent(u8 *o, void *, u32 unused, const u32 *event);
// 0x618730 (thiscall (out), pops 4): InputBase_Poll on each device's +0x10 (none: null); out 0x4ffe0002 (sic: not what they answered).
u32 *__fastcall InputBase_PollAll(u8 *o, void *, u32 *out);
// 0x618780 (thiscall (out, part), pops 8): null 0x4ffe0002. 0x633b90 on it the result; not failed and not 0x4ffe0002: EventQueue_Post(0xfa7cb0, 5, {0xb, part}) (a 0x14-byte local, the rest unset).
u32 *__fastcall InputBase_Poll(u8 *o, void *, u32 *out, u8 *part);
// 0x6187f0 (cdecl (out, object)): null object 0x80004003; 0x24 bytes, Class703ca0_Ctor (none: 0xcffe0100); 0x633ea0 the result: not failed, the object set, else its slot 2 (1).
u32 *__cdecl InputBase_CreateDevice60(u32 *out, u8 **object);
// 0x6188c0 (cdecl (out, object)): as that with 0x74 bytes, 0x634220, its fields (+0x64-+0x70, vtables 0x703810 and 0x703808), 0x6345a0.
u32 *__cdecl InputBase_CreateDevice64(u32 *out, u8 **object);
// 0x6189b0 (thiscall (flags), pops 4), the thunk 0x618c80 from +0x64.
u8 *__fastcall InputChain64_DeletingDtor(u8 *o, void *, u32 flags);
u8 *__fastcall InputChain64_DeletingDtorThunk(u8 *part, void *, u32 flags);
// 0x6189d0 (thiscall): vtables 0x703810, 0x703808; 0x6336f0 on +0x68; +0x64's vtable 0x703748; 0x634310.
void __fastcall InputChain64_Dtor(u8 *o);
// 0x618a40 (cdecl (out, object)): as InputSystem_CreateDevice70 with 0x44 bytes and 0x631870.
u32 *__cdecl InputBase_CreateChain68(u32 *out, u8 **object);
// 0x618ae0 (cdecl (out, object)): as InputBase_CreateDevice60 with 0x5c bytes, 0x6345d0 and 0x634a30.
u32 *__cdecl InputBase_CreateDevice5c(u32 *out, u8 **object);
// 0x618bb0 (cdecl (out, object)): the same with 0x48 bytes and 0x634ee0.
u32 *__cdecl InputBase_CreateDevice58(u32 *out, u8 **object);

// 0x5f5590 (cdecl (a, limit, c, no pump)): no fourth argument: App_PumpMessages. InputSystem_Update on the system.
u32 __cdecl Input_Update(u32 a, u32 limit, u32 c, u32 noPump);
// 0x5f55c0 (cdecl (out)): InputSystem_OpenDevice58 on the system. Returns out.
u32 *__cdecl Input_OpenDevice58(u32 *out);
// 0x5f55e0 (cdecl): InputBase_CloseDevice58 on the system.
void __cdecl Input_CloseDevice58();
// 0x5f5600 (cdecl (out)): InputSystem_OpenDevice5c on the system. Returns out.
u32 *__cdecl Input_OpenDevice5c(u32 *out);
// 0x5f5620 (cdecl): InputBase_CloseDevice5c on the system.
void __cdecl Input_CloseDevice5c();

}  // extern "C"

#include "gamedata.h"

// The vtables hold only ports: tpw.exe's loader points their slots at them (.docs/09-globals.md).
GAME_DATA(void *[7], Class703710_vtable, 0x00703710, 0x1c);
GAME_DATA(void *[5], Class7037a0_vtable, 0x007037a0, 0x14);
GAME_DATA(void *[1], Class7037b4_vtable, 0x007037b4, 0x4);
GAME_DATA(void *[1], Class7037b8_vtable, 0x007037b8, 0x4);
GAME_DATA(void *[7], Class7037e8_vtable, 0x007037e8, 0x1c);
GAME_DATA(void *[7], InputChain64_vtable, 0x00703810, 0x1c);
