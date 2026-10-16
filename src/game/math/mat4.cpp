#include "mat4.h"

#include <cstdint>
#include <initializer_list>

extern "C" {

void __cdecl Mat4_InitGlobals() {
    for (u32 address : {0x87b090u, 0x87b110u, 0x87aff0u, 0x87b030u, 0x87af68u, 0x87afa8u, 0x87b158u, 0x87b198u})
        Mat4_Identity(reinterpret_cast<u32 *>((uintptr_t)address));
}

void __fastcall Mat4_Identity(u32 *m) {
    for (int i = 0; i < 16; ++i)
        m[i] = 0;
    m[15] = m[10] = m[5] = m[0] = 0x3f800000;
}

void __fastcall Mat4_Assign(u32 *m, void *, const u32 *from) {
    for (int i = 0; i < 16; ++i)
        m[i] = from[i];
}
}
