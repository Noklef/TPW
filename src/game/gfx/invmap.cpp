// The inverse colormap: see invmap.h.
#include "invmap.h"

#include <cstdint>

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const InverseMapEnv kGameEnv = {
    at<void(__cdecl *)(void *)>(0x0067acb0),
    at<InverseMapStatics *>(0x00faa500),
};
const InverseMapEnv *g_env = &kGameEnv;

void *const *const kVtable = at<void *const *>(0x00704ac0);

}  // namespace

void InverseMap_SetEnv(const InverseMapEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

InverseMap *__fastcall InverseMap_Ctor(InverseMap *m) {
    m->vtable = kVtable;
    return m;
}

void __fastcall InverseMap_Dtor(InverseMap *m) { m->vtable = kVtable; }

void *__fastcall InverseMap_DeletingDtor(InverseMap *m, void *, u32 flags) {
    InverseMap_Dtor(m);
    if (flags & 1) g_env->free(m);
    return m;
}

void __stdcall InverseMap_MaxFill(u32 *dist, s32 colormax) {
    for (s32 n = static_cast<s32>(static_cast<u32>(colormax) * colormax * colormax); n > 0; --n) *dist++ = ~0u;
}

void __fastcall InverseMap_Build(InverseMap *m, void *, s32 first, s32 count, const u8 *palette, u32 bits,
                                 u32 *dist, u8 *map) {
    // The shifts take their counts mod 32, as SHL/SHR do.
    const u32 shift = (8 - bits) & 31;
    const u32 colormax = 1u << (bits & 31);
    m->colormax = colormax;
    m->greenStride = colormax;
    m->x = 1u << shift;
    m->xsqr = 1u << ((2 * shift) & 31);
    m->redStride = colormax * colormax;
    InverseMap_MaxFill(dist, static_cast<s32>(colormax));
    const s32 end = static_cast<s32>(static_cast<u32>(first) + static_cast<u32>(count));
    for (m->index = first; m->index < end; m->index = static_cast<s32>(static_cast<u32>(m->index) + 1)) {
        const u8 *p = palette + static_cast<u32>(m->index) * 4;
        m->redCenter = p[0] >> shift;
        m->greenCenter = p[1] >> shift;
        m->blueCenter = p[2] >> shift;
        const u32 x = m->x, half = static_cast<u32>(static_cast<s32>(x) / 2);
        const u32 red = m->redCenter, green = m->greenCenter, blue = m->blueCenter;
        m->redDist = p[0] - x * red - half;
        m->greenDist = p[1] - green * x - half;
        const u32 blueDist = p[2] - blue * x - half;
        m->centerDist = m->redDist * m->redDist + m->greenDist * m->greenDist + blueDist * blueDist;
        const u32 xsqr = m->xsqr;
        m->redInc = ((red + 1) * xsqr - x * p[0]) * 2;
        m->greenInc = ((green + 1) * xsqr - p[1] * x) * 2;
        const u32 cell = red * m->redStride + (blue + green * m->greenStride);
        m->centerDp = dist + cell;
        m->blueInc = ((blue + 1) * xsqr - p[2] * x) * 2;
        m->centerRgb = map + cell;
        InverseMap_RedLoop(m);
    }
}

s32 __fastcall InverseMap_RedLoop(InverseMap *m) {
    InverseMapStatics *s = g_env->statics;
    const u32 step = 2 * m->xsqr;
    s32 detect = 0;
    s->redXx = m->redInc;
    m->redDist = m->centerDist;
    m->redRgb = m->centerRgb;
    m->redDp = m->centerDp;
    s32 restart = 1;
    for (s32 r = m->redCenter; r < static_cast<s32>(m->colormax); ++r, restart = 0) {
        if (InverseMap_GreenLoop(m, nullptr, restart)) detect = 1;
        else if (detect) break;
        m->redDp += m->redStride;
        m->redRgb += m->redStride;
        m->redDist += s->redXx;
        s->redXx += step;
    }
    s->redXx = m->redInc - step;
    m->redDist = m->centerDist - s->redXx;
    m->redDp = m->centerDp - m->redStride;
    m->redRgb = m->centerRgb - m->redStride;
    restart = 1;
    for (s32 r = m->redCenter - 1; r >= 0; --r, restart = 0) {
        if (InverseMap_GreenLoop(m, nullptr, restart)) detect = 1;
        else if (detect) return detect;
        m->redDp -= m->redStride;
        m->redRgb -= m->redStride;
        s->redXx -= step;
        m->redDist -= s->redXx;
    }
    return detect;
}

s32 __fastcall InverseMap_GreenLoop(InverseMap *m, void *, s32 restart) {
    InverseMapStatics *s = g_env->statics;
    const u32 step = 2 * m->xsqr;
    s32 detect = 0;
    if (restart) {
        s->greenHere = m->greenCenter;
        s->greenMin = 0;
        s->greenMax = static_cast<s32>(m->colormax - 1);
        s->greenInc = m->greenInc;
    }
    s32 g = s->greenHere;
    m->greenDist = s->greenCenterDist = m->redDist;
    s->greenXx = s->greenInc;
    m->greenDp = s->greenCenterDp = m->redDp;
    m->greenRgb = s->greenCenterRgb = m->redRgb;
    s32 first = 1;
    for (; g <= s->greenMax; ++g, first = 0) {
        if (InverseMap_BlueLoop(m, nullptr, first)) {
            if (!detect) {
                // The first hit past the old start becomes the new start.
                if (s->greenHere < g) {
                    s->greenHere = g;
                    m->redDp = s->greenCenterDp;
                    m->redRgb = s->greenCenterRgb;
                    m->redDist = s->greenCenterDist;
                    s->greenInc = s->greenXx;
                }
                detect = 1;
            }
        } else if (detect) {
            break;
        }
        m->greenDp += m->greenStride;
        s->greenCenterDp += m->greenStride;
        m->greenRgb += m->greenStride;
        s->greenCenterRgb += m->greenStride;
        m->greenDist += s->greenXx;
        s->greenCenterDist += s->greenXx;
        s->greenXx += step;
    }
    g = s->greenHere - 1;
    s->greenXx = s->greenInc - step;
    m->greenDist = s->greenCenterDist = m->redDist - s->greenXx;
    m->greenDp = s->greenCenterDp = m->redDp - m->greenStride;
    m->greenRgb = s->greenCenterRgb = m->redRgb - m->greenStride;
    first = 1;
    for (; s->greenMin <= g; --g, first = 0) {
        if (InverseMap_BlueLoop(m, nullptr, first)) {
            if (!detect) {
                s->greenHere = g;
                m->redDp = s->greenCenterDp;
                m->redRgb = s->greenCenterRgb;
                m->redDist = s->greenCenterDist;
                s->greenInc = s->greenXx;
                detect = 1;
            }
        } else if (detect) {
            return detect;
        }
        m->greenDp -= m->greenStride;
        s->greenCenterDp -= m->greenStride;
        m->greenRgb -= m->greenStride;
        s->greenCenterRgb -= m->greenStride;
        s->greenXx -= step;
        m->greenDist -= s->greenXx;
        s->greenCenterDist -= s->greenXx;
    }
    return detect;
}

s32 __fastcall InverseMap_BlueLoop(InverseMap *m, void *, s32 restart) {
    InverseMapStatics *s = g_env->statics;
    const u32 step = 2 * m->xsqr;
    if (restart) {
        s->blueHere = m->blueCenter;
        s->blueMin = 0;
        s->blueMax = static_cast<s32>(m->colormax - 1);
        s->blueInc = m->blueInc;
    }
    const s32 max = s->blueMax;
    u8 *rgb = m->greenRgb;
    u32 d = m->greenDist;
    u32 *dp = m->greenDp;
    u32 inc = s->blueInc;
    s32 detect = 0;
    s32 b = s->blueHere;
    for (; b <= max; ++b) {
        if (d < *dp) {
            if (s->blueHere < b) {
                s->blueHere = b;
                m->greenDp = dp;
                m->greenRgb = rgb;
                m->greenDist = d;
                s->blueInc = inc;
            }
            detect = 1;
            break;
        }
        ++dp;
        ++rgb;
        d += inc;
        inc += step;
    }
    const u8 index = static_cast<u8>(m->index);
    const s32 min = s->blueMin;
    for (; b <= max && d < *dp; ++b) {
        *dp++ = d;
        *rgb++ = index;
        d += inc;
        inc += step;
    }
    inc = s->blueInc - step;
    b = s->blueHere - 1;
    d = m->greenDist - inc;
    dp = m->greenDp - 1;
    rgb = m->greenRgb - 1;
    if (!detect) {
        for (; min <= b; --b) {
            if (d < *dp) {
                detect = 1;
                s->blueHere = b;
                m->greenDp = dp;
                m->greenRgb = rgb;
                m->greenDist = d;
                s->blueInc = inc;
                break;
            }
            inc -= step;
            --dp;
            --rgb;
            d -= inc;
        }
    }
    for (; min <= b && d < *dp; --b) {
        *dp-- = d;
        *rgb-- = index;
        inc -= step;
        d -= inc;
    }
    return detect;
}

}  // extern "C"
