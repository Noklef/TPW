#include "nodes.h"

#include <cstdint>

#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

// lo <= x <= hi, as fcom lo (below or unordered: out) then fcomp hi (above: out; a NaN hi passes).
bool within(float x, float lo, float hi) { return x >= lo && !(x > hi); }

}  // namespace

extern "C" {

void __cdecl CoasterNode_Lerp(const u8 *coaster, u32 index, float t, float *out) {
    const u8 *nodes = at<const u8 *>(coaster, 0x9c);
    const float *a = &at<const float>(nodes, index * 0x38 + 4);
    const float *b = &at<const float>(nodes, (index + 1) * 0x38 + 4);
    const X87Reg u = X87Reg(1.0f) - t;
    out[0] = (float)(X87Reg(a[0]) * u + X87Reg(b[0]) * t);
    out[1] = (float)(X87Reg(b[1]) * t + X87Reg(a[1]) * u);
    out[2] = (float)(X87Reg(a[2]) * u + X87Reg(b[2]) * t);
}

u32 __cdecl Coaster_IsPastEnd(const u8 *coaster, float x) {
    return x >= at<float>(coaster, 0xa8) ? 1 : 0;
}

u32 __cdecl Coaster_FindZone(const u8 *owner, u32, const float *pos, float t) {
    const u32 count = at<u32>(owner, 0x7c);
    const u8 *boxes = at<const u8 *>(owner, 0x78);
    u32 found = 0;
    for (u32 i = 0; i < count - 1; ++i) {
        const float *box = &at<const float>(boxes, i * 0x2c);
        if (within(t, box[0], box[1]) && within(pos[1], box[2], box[3]) && within(pos[2], box[4], box[5]) &&
            within(pos[3], box[6], box[7])) {
            found = at<u32>(box, 0x20);
            break;
        }
    }
    return found ? found : at<u32>(boxes, count * 0x2c - 0xc);
}

void __cdecl CoasterSearch_Reset(u32 *search) {
    for (int i = 0; i < 7; ++i) search[i] = 0;
    search[7] = 0x7f7fffff;
}
}
