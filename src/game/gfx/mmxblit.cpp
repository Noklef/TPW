#include "mmxblit.h"

#include <cstdint>

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

uint64_t &qword(u8 *p, u32 offset) { return *reinterpret_cast<uint64_t *>(p + offset); }

}  // namespace

extern "C" {

void __cdecl MmxBlit_Copy16(const u8 *source, u8 *out, u32 width, u32 rows, s32 sourcePitch, s32 outPitch) {
    const u32 groups = width >> 2;
    do {
        const u8 *from = source;
        u8 *to = out;
        u32 left = groups;
        do {
            const uint64_t a = *reinterpret_cast<const uint64_t *>(from), b = *reinterpret_cast<const uint64_t *>(from + 8);
            qword(to, 0) = a;
            qword(to, 8) = b;
            from += 0x10;
            to += 0x10;
        } while (--left);
        source += sourcePitch;
        out += outPitch;
    } while (--rows);
}

void __cdecl MmxBlit_Copy32(u8 *source, u8 *out, u32 width, u32 rows, s32 sourcePitch, s32 outPitch, u8 *depth, u32 depthPitch,
                            u32 clear, u32 fillLow, u32 fillHigh) {
    global<u32>(0x00fab484) = width;
    global<u32>(0x00fab4a0) = rows;
    global<u32>(0x00fab4b0) = depthPitch;
    if ((u8)clear == 0) {
        global<u32>(0x00fac51c) = fillLow;
        global<u32>(0x00fac518) = fillLow;
        u32 left = global<u32>(0x00fab4a0);
        do {
            s32 count = (s32)global<u32>(0x00fab484);
            u8 *from = source, *to = out;
            do {
                const uint64_t q0 = qword(from, 0), q1 = qword(from, 8), q2 = qword(from, 0x10), q3 = qword(from, 0x18);
                qword(to, 0) = q0;
                qword(to, 8) = q1;
                qword(to, 0x10) = q2;
                qword(to, 0x18) = q3;
                from += 0x20;
                to += 0x20;
                count -= 8;
            } while (count > 0);
            source += sourcePitch;
            out += outPitch;
        } while (--left);
        return;
    }
    const uint64_t depthValue = global<uint64_t>(0x0077e580), fill = (uint64_t)fillHigh << 32 | fillLow;
    u32 left = global<u32>(0x00fab4a0);
    do {
        s32 count = (s32)global<u32>(0x00fab484);
        u8 *from = source, *to = out, *z = depth;
        do {
            const uint64_t q0 = qword(from, 0), q1 = qword(from, 8), q2 = qword(from, 0x10), q3 = qword(from, 0x18);
            qword(from, 0) = fill;
            qword(from, 8) = fill;
            qword(from, 0x10) = fill;
            qword(from, 0x18) = fill;
            qword(to, 0) = q0;
            qword(to, 8) = q1;
            qword(to, 0x10) = q2;
            qword(to, 0x18) = q3;
            qword(z, 0) = depthValue;
            qword(z, 8) = depthValue;
            from += 0x20;
            to += 0x20;
            z += 0x10;
            count -= 8;
        } while (count > 0);
        source += sourcePitch;
        out += outPitch;
        depth += global<u32>(0x00fab4b0);
    } while (--left);
}
}
