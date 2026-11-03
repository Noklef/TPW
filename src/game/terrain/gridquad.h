// What the landscape's grid drawings share (0x56f670, 0x570d90): a quad's side tests, which read their vertices from globals and leave them there, and a fan's indices.
#pragma once

#include <cstdint>

#include "../../../re/types.h"
#include "../math/x87.h"

namespace gridquad {

template <typename T> T &Global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
inline u32 Dword(const u8 *p, u32 offset) { return *reinterpret_cast<const u32 *>(p + offset); }

inline void KeepTriangle(const u8 *a, const u8 *b, const u8 *c) {
    Global<u32>(0x008c63bc) = Dword(a, 0);
    Global<u32>(0x008c63c8) = Dword(a, 4);
    Global<u32>(0x008c63b0) = Dword(a, 0xc);
    Global<u32>(0x008c63c0) = Dword(b, 0);
    Global<u32>(0x008c63cc) = Dword(b, 4);
    Global<u32>(0x008c63b4) = Dword(b, 0xc);
    Global<u32>(0x008c63c4) = Dword(c, 0);
    Global<u32>(0x008c63d0) = Dword(c, 4);
    Global<u32>(0x008c63b8) = Dword(c, 0xc);
}

inline const float &X0() { return Global<float>(0x008c63bc); }
inline const float &Y0() { return Global<float>(0x008c63c8); }
inline const float &W0() { return Global<float>(0x008c63b0); }
inline const float &X1() { return Global<float>(0x008c63c0); }
inline const float &Y1() { return Global<float>(0x008c63cc); }
inline const float &W1() { return Global<float>(0x008c63b4); }
inline const float &X2() { return Global<float>(0x008c63c4); }
inline const float &Y2() { return Global<float>(0x008c63d0); }
inline const float &W2() { return Global<float>(0x008c63b8); }

// A quad bound for the clipper: FCOM's less or unordered.
inline bool FacingClipped() {
    const X87Reg left = (X87Reg(W0()) * Y2() - X87Reg(Y0()) * W2()) * X1() - X87Reg(X0()) * W1() * Y2();
    const X87Reg right = (X87Reg(W0()) * Y1() - X87Reg(Y0()) * W1()) * X2() - X87Reg(X0()) * W2() * Y1();
    return !(right.v >= left.v);
}

// A quad with no code set: the same with its last products in another order.
inline bool FacingOpen() {
    const X87Reg left = (X87Reg(W0()) * Y2() - X87Reg(W2()) * Y0()) * X1() - X87Reg(W1()) * Y2() * X0();
    const X87Reg right = (X87Reg(W0()) * Y1() - X87Reg(W1()) * Y0()) * X2() - X87Reg(W2()) * Y1() * X0();
    return !(right.v >= left.v);
}

// A quad with no code set, a batch's vertices in place: the first test's third vertex comes through FLD and FSTP.
inline bool FacingOpenFirst(const u8 *a, const u8 *b, const u8 *c) {
    KeepTriangle(a, b, c);
    Global<u32>(0x008c63c4) = X87_FloatThrough(Dword(c, 0));
    Global<u32>(0x008c63d0) = X87_FloatThrough(Dword(c, 4));
    Global<u32>(0x008c63b8) = X87_FloatThrough(Dword(c, 0xc));
    return FacingOpen();
}

// A fan of count vertices from base, its last triangle first.
inline u16 *Fan(u16 *out, s32 base, s32 count) {
    for (s32 k = count - 3; k >= 0; --k) {
        *out++ = (u16)base;
        *out++ = (u16)(k + base + 1);
        *out++ = (u16)(k + base + 2);
    }
    return out;
}

}  // namespace gridquad
