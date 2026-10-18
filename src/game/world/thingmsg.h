// The messages things are sent (through the message centre to Thing_HandleMessage 0x50b550):
// reimplementation of their constructors and Type functions in 0x40f240-0x40f920. Each is a vtable
// (Type, then the scalar deleting destructor), +4 cleared and its fields from +8; Type gives the
// message's number, which names the class here.
#pragma once

#include "../../../re/types.h"

extern "C" {

// The constructors (thiscall; each returns msg). Arguments land in the order given unless noted.
u8 *__fastcall ThingMsg01_Ctor(u8 *msg, void *, u32 a);                    // 0x40f290, 0x6fd7a8
u8 *__fastcall ThingMsg02_Ctor(u8 *msg, void *, u32 c, u32 b);             // 0x40f240, 0x6fd7a0: +8 b, +0xc c
u8 *__fastcall ThingMsg06_Ctor(u8 *msg, void *, u16 a);                    // 0x40f330, 0x6fd7c0
u8 *__fastcall ThingMsg07_Ctor(u8 *msg, void *, u16 a);                    // 0x40f380, 0x6fd7c8
u8 *__fastcall ThingMsg08_Ctor(u8 *msg, void *, u16 a);                    // 0x40f3d0, 0x6fd7d0
u8 *__fastcall ThingMsg09_Ctor(u8 *msg, void *, u16 a);                    // 0x40f420, 0x6fd7d8
u8 *__fastcall ThingMsg0a_Ctor(u8 *msg, void *, u16 a);                    // 0x40f470, 0x6fd7e0
u8 *__fastcall ThingMsg0b_Ctor(u8 *msg);                                    // 0x40f4c0, 0x6fd7e8
u8 *__fastcall ThingMsg0c_Ctor(u8 *msg);                                    // 0x40f500, 0x6fd7f0
u8 *__fastcall ThingMsg0d_Ctor(u8 *msg);                                    // 0x40f540, 0x6fd7f8
u8 *__fastcall ThingMsg11_Ctor(u8 *msg, void *, u16 a, u32 b);             // 0x40f580, 0x6fd800
u8 *__fastcall ThingMsg12_Ctor(u8 *msg, void *, u32 a);                    // 0x40f5e0, 0x6fd808
u8 *__fastcall ThingMsg13_Ctor(u8 *msg, void *, u32 a);                    // 0x40f630, 0x6fd810
u8 *__fastcall ThingMsg14_Ctor(u8 *msg, void *, u16 a);                    // 0x40f680, 0x6fd818
u8 *__fastcall ThingMsg15_Ctor(u8 *msg, void *, u8 a);                     // 0x40f6d0, 0x6fd6c8
u8 *__fastcall ThingMsg17_Ctor(u8 *msg, void *, u32 a, u32 b);             // 0x40f6f0, 0x6fd820
// 0x40f740, 0x6fd828: +8 a, +0xc b, +0x10 d, +0x14 c.
u8 *__fastcall ThingMsg18_Ctor(u8 *msg, void *, u32 a, u16 b, u32 c, u32 d);
// 0x40f7a0, 0x6fd830: +8 a, +0xc b, +0x10 c, +0x14..+0x24 d..h, +0x28 i, +0x2a j, +0x2c k.
u8 *__fastcall ThingMsg19_Ctor(u8 *msg, void *, u32 a, u32 b, u8 c, u32 d, u32 e, u32 f, u32 g, u32 h, u16 i,
                               u16 j, u32 k);
u8 *__fastcall ThingMsg1a_Ctor(u8 *msg, void *, u32 a, u32 b, u16 c, u16 d);  // 0x40f840, 0x6fd838
u8 *__fastcall ThingMsg1b_Ctor(u8 *msg, void *, u16 a);                    // 0x40f900, 0x6fd848
u8 *__fastcall ThingMsg1c_Ctor(u8 *msg, void *, u16 a);                    // 0x40f8b0, 0x6fd840
// 0x40f300 (value, pops 4): message 05 (Type 0x40f2d0): +4 = 0 and the base vtable 0x6fd7b8, then the record at +8 copied from the one
// the value points at (Fn_59b550), then the vtable 0x6fd7b0. Returns msg.
u8 *__fastcall Obj_CtorPart8(u8 *object, void *, u32 value);

// The Type functions (thiscall, this unused): the message's number.
u32 __fastcall ThingMsg02_Type(const u8 *msg);  // 0x40f260
u32 __fastcall ThingMsg05_Type(const u8 *msg);  // 0x40f2d0
u32 __fastcall ThingMsg06_Type(const u8 *msg);  // 0x40f350
u32 __fastcall ThingMsg07_Type(const u8 *msg);  // 0x40f3a0
u32 __fastcall ThingMsg08_Type(const u8 *msg);  // 0x40f3f0
u32 __fastcall ThingMsg09_Type(const u8 *msg);  // 0x40f440
u32 __fastcall ThingMsg0a_Type(const u8 *msg);  // 0x40f490
u32 __fastcall ThingMsg0b_Type(const u8 *msg);  // 0x40f4d0
u32 __fastcall ThingMsg0c_Type(const u8 *msg);  // 0x40f510
u32 __fastcall ThingMsg0d_Type(const u8 *msg);  // 0x40f550
u32 __fastcall ThingMsg11_Type(const u8 *msg);  // 0x40f5b0
u32 __fastcall ThingMsg12_Type(const u8 *msg);  // 0x40f600
u32 __fastcall ThingMsg13_Type(const u8 *msg);  // 0x40f650
u32 __fastcall ThingMsg14_Type(const u8 *msg);  // 0x40f6a0
u32 __fastcall ThingMsg17_Type(const u8 *msg);  // 0x40f710
u32 __fastcall ThingMsg18_Type(const u8 *msg);  // 0x40f770
u32 __fastcall ThingMsg19_Type(const u8 *msg);  // 0x40f810
u32 __fastcall ThingMsg1a_Type(const u8 *msg);  // 0x40f880
u32 __fastcall ThingMsg1c_Type(const u8 *msg);  // 0x40f8d0
u32 __fastcall ThingMsg1b_Type(const u8 *msg);  // 0x40f920
}
