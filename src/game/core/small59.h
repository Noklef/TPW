// The complete destructors of the 0x6fda58 family and one string-and-handle class: reimplementation of 0x41e270, 0x41e2d0, 0x41e330, 0x41e470, 0x41e4f0 and 0x41e560.
// Named by their vtable until the classes are known. The compiler's exception frames are left out.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x41e270 / 0x41e2d0 / 0x41e560 (thiscall; the same body): the vtable 0x6fda58 is set, the handle at +0x40 is destroyed (Handle_Dtor), then the 0x6fd9d8 base destructor runs.
void __fastcall Class6fda58_DtorA(u8 *object);
void __fastcall Class6fda58_DtorB(u8 *object);
void __fastcall Class6fda58_DtorC(u8 *object);

// 0x41e330 / 0x41e4f0 (thiscall): the vtable (0x6fda88 / 0x6fdac8) is set and the handle at +0x48 destroyed; then the vtable 0x6fda58, the handle at +0x40 and the base destructor.
void __fastcall Class6fda88_FullDtor(u8 *object);
void __fastcall Class6fdac8_FullDtor(u8 *object);

// 0x41e470 (thiscall): the vtable 0x6fdaa4 is set, the handle at +0x2c destroyed, then the strings at +0x18 and +4 (String_Dtor).
void __fastcall Class6fdaa4_Dtor(u8 *object);

}  // extern "C"
