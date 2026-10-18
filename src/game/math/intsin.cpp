#include "intsin.h"

#include "x87.h"

namespace {

s32 entry(u32 index) { return (s32)(reinterpret_cast<const u32 *>(0x00877358)[index & 0x1ff] << 2); }

}  // namespace

extern "C" {

s32 __cdecl IntSin(u32 angle) { return entry(angle); }

s32 __cdecl IntCos(u32 angle) { return entry(angle + 0x80); }

void __cdecl IntSin_BuildTable() {
    u32 *const table = reinterpret_cast<u32 *>(0x00877358);
    for (s32 i = 0; i < 0x200; ++i) {
        const X87Reg angle = X87_MulIntDouble(2 * i, 0.00613591796875);  // fild; fmul qword [0x700ec8]
        const X87Reg scaled = X87Reg(X87_Sin(angle.v)) * X87Reg(256.0);  // fsin; fmul qword [0x700ec0]
        table[i] = (u32)(long long)scaled.v;                              // __ftol, low half
    }
}
}
