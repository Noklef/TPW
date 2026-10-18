#include "ridebounds.h"

#include <cstdint>

#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

}  // namespace

extern "C" {

u32 __cdecl Ride_Bounds(s32 index, float *out) {
    const u8 *const ride = reinterpret_cast<const u8 *const *>(0x007a4610)[index];
    if (!ride) return 0;
    const u8 *const block = at<const u8 *>(ride, 0xcc);
    if (!block) return 0;
    const u32 low = at<u32>(ride, 0xc0), high = at<u32>(ride, 0xc4);
    const float base = at<float>(at<const u8 *>(at<const u8 *>(at<const u8 *>(ride, 8), 4), 0x78), 0x44);
    // fild of each 16-bit half, exact; the far corner is the last tile's + 1 (- -1.0)
    out[0] = (float)(X87Reg(low >> 16) * 10.0f);
    out[1] = (float)(X87Reg(at<float>(block, 0x1c)) + base);
    out[2] = (float)(X87Reg(low & 0xffff) * 10.0f);
    out[3] = (float)((X87Reg(high >> 16) - -1.0f) * 10.0f);
    out[4] = (float)(X87Reg(at<float>(block, 0x28)) + base);
    out[5] = (float)((X87Reg(high & 0xffff) - -1.0f) * 10.0f);
    return 1;
}

u32 __cdecl Ride_Centre(const s32 *handle, float *x, float *y, float *z) {
    float box[6];
    if (*handle == 0 || !Ride_Bounds(*handle, box)) {
        *z = 0.0f;
        *y = 0.0f;
        *x = 0.0f;
        return 1;
    }
    const u8 *const ride = reinterpret_cast<const u8 *const *>(0x007a4610)[*handle];
    const float cx = (float)((X87Reg(box[0]) + box[3]) * 0.5f);
    const float cz = (float)((X87Reg(box[2]) + box[5]) * 0.5f);
    *y = X87_Through(at<float>(at<const u8 *>(at<const u8 *>(at<const u8 *>(ride, 8), 4), 0x78), 0x44));
    *x = cx;
    *z = cz;
    return 0;
}
}
