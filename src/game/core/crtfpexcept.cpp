#include "crtfpexcept.h"

#include "crtfloat.h"
#include "trivial.h"

#include <cstdint>
#include <cstring>

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename F> F import_slot(u32 slot) { return *reinterpret_cast<F *>((uintptr_t)slot); }

u32 *words(double *v) { return reinterpret_cast<u32 *>(v); }
const u32 *words(const double *v) { return reinterpret_cast<const u32 *>(v); }

// The global double's two dwords, read live (writable data), or negated through the x87.
void put_global(double *to, u32 address) {
    words(to)[0] = global<u32>(address);
    words(to)[1] = global<u32>(address + 4);
}

void put_negated(double *to, u32 address) {
    double v;
    __asm {
        mov eax, address
        fld qword ptr [eax]
        fchs
        fstp v
    }
    std::memcpy(to, &v, 8);
}

}  // namespace

extern "C" {

s32 __cdecl CRT_statfp() {
    u16 sw;
    __asm fstsw sw
    return (s16)sw;
}

s32 __cdecl CRT_clrfp() {
    u16 sw;
    __asm {
        fnstsw sw
        fnclex
    }
    return (s16)sw;
}

void __cdecl CRT_set_statfp(u32 flags) {
    const u8 f = (u8)flags;
    s32 sink;
    double sink2;
    if (f & 1) __asm {   // (invalid: the huge extended at 0x7811c0 to an integer)
        fld tbyte ptr ds:[0x007811c0]
        fistp sink
        fwait
    }
    if (f & 8) __asm {   // (overflow: it to a double)
        fld tbyte ptr ds:[0x007811c0]
        fstp sink2
        fwait
    }
    if (f & 0x10) __asm {   // (underflow: the tiny one at 0x7811d0)
        fld tbyte ptr ds:[0x007811d0]
        fstp sink2
        fwait
    }
    if (f & 4) __asm {   // (zero divide: 1 / 0)
        fldz
        fld1
        fdivrp st(1), st
        fstp st(0)
        fwait
    }
    if (f & 0x20) __asm {   // (precision: pi as a double)
        fldpi
        fstp sink2
        fwait
    }
}

s32 __cdecl CRT_handle_exc(u32 flags, double *value, u32 cw) {
    const u32 all = flags & 0x1f;
    u32 left = all;
    if ((flags & 8) && (cw & 1)) {
        CRT_set_statfp(1);
        left &= ~8u;
    } else if ((flags & 4) && (cw & 4)) {
        CRT_set_statfp(4);
        left &= ~4u;
    } else if ((flags & 1) && (cw & 8)) {
        CRT_set_statfp(8);
        const bool positive = *value > 0.0;   // (FCOMP 0: below, equal and unordered read as not)
        switch (cw & 0xc00) {
        case 0:
            positive ? put_global(value, 0x00781198) : put_negated(value, 0x00781198);
            break;
        case 0x400:
            positive ? put_global(value, 0x007811a8) : put_negated(value, 0x00781198);
            break;
        case 0x800:
            positive ? put_global(value, 0x00781198) : put_negated(value, 0x007811a8);
            break;
        case 0xc00:
            positive ? put_global(value, 0x007811a8) : put_negated(value, 0x007811a8);
            break;
        }
        left &= ~1u;
    } else if ((flags & 2) && (cw & 0x10)) {
        u32 inexact = (flags & 0x10) ? 1 : 0;
        const double v = *value;
        if (!(v < 0.0) && !(v > 0.0)) {   // (FCOMP's C3: zero, or unordered)
            inexact = 1;
        } else {
            s32 exp;
            u32 w[2];
            std::memcpy(w, &v, 8);
            const double m = CRT_frexp(w[0], w[1], &exp);
            const s32 e = exp - 0x600;
            if (e < -1074) {
                words(value)[0] = words(value)[1] = 0;
                inexact = 1;
            } else {
                const bool negative = !(m >= 0.0);   // (C0: below, or unordered)
                std::memcpy(w, &m, 8);
                w[1] = (w[1] & 0xffff) | ((w[1] >> 16 & 0xf) | 0x10) << 16;   // (the sign and exponent as a denormal's next: 1)
                for (s32 n = -1021 - e; n > 0; --n) {
                    if (w[0] & 1) inexact = 1;
                    w[0] = w[0] >> 1 | w[1] << 31;
                    w[1] >>= 1;
                }
                double out;
                std::memcpy(&out, w, 8);
                if (negative) out = -out;
                std::memcpy(value, &out, 8);
            }
        }
        if (inexact) CRT_set_statfp(0x10);
        left = all & ~2u;
    }
    if ((flags & 0x10) && (cw & 0x20)) {
        CRT_set_statfp(0x20);
        left &= ~0x10u;
    }
    return left == 0;
}

void __cdecl CRT_raise_exc(u32 *rec, u32 *cw, u32 flags, u32 opcode, const double *arg1, double *result) {
    rec[1] = 0;
    rec[2] = 0;
    rec[3] = 0;
    u32 code = (u32)(uintptr_t)rec;   // (sic: no flag leaves the record's address as the code)
    const u8 f = (u8)flags;
    if (f & 0x10) code = 0xc000008f, rec[1] |= 1;
    if (f & 2) code = 0xc0000093, rec[1] |= 2;
    if (f & 1) code = 0xc0000091, rec[1] |= 4;
    if (f & 4) code = 0xc000008e, rec[1] |= 8;
    if (f & 8) code = 0xc0000090, rec[1] |= 0x10;
    rec[2] = (rec[2] & ~0x10u) | (~*cw & 1) << 4;
    rec[2] = (rec[2] & ~8u) | (~*cw & 4) << 1;
    rec[2] = (rec[2] & ~4u) | (~*cw >> 1 & 4);
    rec[2] = (rec[2] & ~2u) | (~*cw >> 3 & 2);
    rec[2] = (rec[2] & ~1u) | (~*cw >> 5 & 1);
    const u8 status = (u8)CRT_statfp();
    if (status & 1) rec[3] |= 0x10;
    if (status & 4) rec[3] |= 8;
    if (status & 8) rec[3] |= 4;
    if (status & 0x10) rec[3] |= 2;
    if (status & 0x20) rec[3] |= 1;
    switch (*cw & 0xc00) {
    case 0: rec[0] &= ~3u; break;
    case 0x400: rec[0] = (rec[0] & ~2u) | 1; break;
    case 0x800: rec[0] = (rec[0] & ~1u) | 2; break;
    case 0xc00: rec[0] |= 3; break;
    }
    switch (*cw & 0x300) {
    case 0: rec[0] = (rec[0] & ~0x14u) | 8; break;
    case 0x200: rec[0] = (rec[0] & ~0x18u) | 4; break;
    case 0x300: rec[0] &= ~0x1cu; break;
    }
    rec[0] = (rec[0] & 0xfffe001f) | (opcode & 0xfff) << 5;
    rec[8] |= 1;
    rec[8] = (rec[8] & ~0x1cu) | 2;
    rec[4] = words(arg1)[0];
    rec[5] = words(arg1)[1];
    rec[0x14] |= 1;
    rec[0x14] = (rec[0x14] & ~0x1cu) | 2;
    rec[0x10] = words(result)[0];
    rec[0x11] = words(result)[1];
    CRT_clrfp();
    // RaiseException(code, 0, 1, the address of the argument holding rec)
    import_slot<void(__stdcall *)(u32, u32, u32, u32 **)>(0x006fd218)(code, 0, 1, &rec);
    if (rec[2] & 0x10) *cw &= ~1u;
    if (rec[2] & 8) *cw &= ~4u;
    if (rec[2] & 4) *cw &= ~8u;
    if (rec[2] & 2) *cw &= ~0x10u;
    if (rec[2] & 1) *cw &= ~0x20u;
    switch (rec[0] & 3) {
    case 0: *cw &= ~0xc00u; break;
    case 1: *cw = (*cw & ~0x800u) | 0x400; break;
    case 2: *cw = (*cw & ~0x400u) | 0x800; break;
    case 3: *cw |= 0xc00; break;
    }
    switch (rec[0] >> 2 & 7) {   // (sic: the precision clears the rounding bits, 0 and 1 then set the precision's)
    case 0: *cw = (*cw & ~0xc00u) | 0x300; break;
    case 1: *cw = (*cw & ~0xc00u) | 0x200; break;
    case 2: *cw &= ~0xc00u; break;
    }
    words(result)[0] = rec[0x10];
    words(result)[1] = rec[0x11];
}

void __cdecl CRT_87except(s32 opcode, s32 *exc, const u16 *cw) {
    u32 rec[0x16];   // (the _FPIEEE_RECORD, uninitialised as the original's frame)
    u32 word = *cw;
    u32 flags = 0;
    switch (exc[0]) {
    case 1: case 5: flags = 8; break;
    case 2: flags = 4; break;
    case 3: flags = 0x11; break;
    case 4: flags = 0x12; break;
    case 8: flags = 0x10; break;
    case 7: exc[0] = 1; break;
    }
    if (flags && !CRT_handle_exc(flags, reinterpret_cast<double *>(exc + 6), word)) {
        if (opcode == 0x10 || opcode == 0x16 || opcode == 0x1d) {
            rec[0xe] = (rec[0xe] & ~0x1cu) | 3;
            rec[0xa] = (u32)exc[4];
            rec[0xb] = (u32)exc[5];
        } else {
            rec[0xe] &= ~1u;
        }
        CRT_raise_exc(rec, &word, flags, (u32)opcode, reinterpret_cast<const double *>(exc + 2), reinterpret_cast<double *>(exc + 6));
    }
    CRT_ctrlfp(word, 0xffff);
    u32 handled = 0;
    if (exc[0] != 8 && global<u32>(0x00781edc) == 0) handled = Zero_5d60c0();   // (_matherr)
    if (!handled) CRT_set_errno_from_matherr(exc[0]);
}

}
