#include "mat4.h"

extern "C" {

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
