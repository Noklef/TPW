#include "soundoccluders.h"

#include <cstdint>

#include "../math/vector.h"
#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 __fastcall SoundOccluders_Occlusion(u8 *list, void *, const float *from, const float *to, float *out) {
    float largest = 0.0f;
    if (at<u32>(list, 0xc)) {
        for (u8 *node = at<u8 *>(list, 4); node;) {
            const float *const o = at<const float *>(node, 0);
            node = at<u8 *>(node, 4);
            if (!(o[3] >= at<float>(list, 0x14))) break;
            const float tox = to[0], fromx = from[0], fromz = from[2];
            const float toz = (float)X87Reg(to[2]);
            const float rr = (float)(X87Reg(o[3]) * o[3]);
            X87Reg dx = X87Reg(o[0]) - tox;
            if (!(dx.v >= 0.0)) dx.v = -dx.v;
            X87Reg dz = X87Reg(o[1]) - toz;
            if (!(dz.v >= 0.0)) dz.v = -dz.v;
            if (!((dx * dx + dz * dz).v >= rr)) continue;   // (to inside the occluder)
            // The nearest point of the line from `from` to `to`.
            const X87Reg ez = X87Reg(toz) - fromz, ex = X87Reg(tox) - fromx;
            const X87Reg fz = X87Reg(o[1]) - fromz, fx = X87Reg(o[0]) - fromx;
            const float ezf = (float)ez, exf = (float)ex;
            const X87Reg t = (fz * ezf + fx * exf) / (ex * exf + ez * ez);
            const float tf = (float)t;
            const float cz = (float)(t * ezf + fromz);
            const float x = (float)(X87Reg(tf) * exf + fromx);
            // x clamped to the segment's x span (inline, as Float_ClampBetween; picked by address so that the compiler makes no minss / maxss)
            const float *cxp;
            if (!(fromx >= tox))
                cxp = x > tox ? &tox : x >= fromx ? &x : &fromx;
            else
                cxp = x > fromx ? &fromx : x >= tox ? &x : &tox;
            const float cx = *cxp;
            const float cl = (float)X87Reg(Float_ClampBetween(cz, fromz, toz));
            const X87Reg ddx = X87Reg(o[0]) - cx, ddz = X87Reg(o[1]) - cl;
            const float ddxf = (float)ddx, ddzf = (float)ddz;
            const X87Reg d2 = ddx * ddxf + ddz * ddzf;
            if (d2.v > rr) continue;
            *out = (float)(1.0f - d2 / rr);
            const float v = *out;
            if (v > largest) largest = v;
        }
    }
    *out = largest;
    return largest == 0.0f || largest != largest ? 0 : 1;
}

}  // extern "C"
