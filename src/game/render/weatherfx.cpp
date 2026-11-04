#include "weatherfx.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../files/handles.h"
#include "../files/memsource.h"
#include "../files/quickload.h"
#include "../gfx/texture.h"
#include "../gfx/texman.h"
#include "../math/x87.h"
#include "../terrain/landscape.h"
#include "../gfx/palette.h"
#include "batch.h"
#include "drawbuffer.h"
#include "ribbon.h"
#include "spritequeue.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const float kPi = 3.14159265f;           // 0x701c10
const float kPerDegree = 1.0f / 180.0f;  // 0x701c98

u32 Bits(float f) { return *reinterpret_cast<u32 *>(&f); }

// +0x331c = (x, -1, z) made unit length, unless its length is 0 or a NaN (fcom equal or unordered).
void SetDirection(u8 *fx, X87Reg x, X87Reg z) {
    const float xf = (float)x;
    const X87Reg xx = x * xf;
    const float zf = (float)z;
    const X87Reg zz = z * zf;
    const X87Reg length = X87_Sqrt(xx + zz - X87Reg(-1.0f));   // 0x701c9c
    u32 out[3] = {Bits(xf), 0xbf800000, Bits(zf)};
    if (length.v != 0.0 && length.v == length.v) {   // 0x701ca0
        const X87Reg scale = X87Reg(1.0f) / length;   // 0x701c68
        out[0] = Bits((float)(scale * xf));
        out[1] = Bits((float)(scale * X87Reg(-1.0f)));
        out[2] = Bits((float)(scale * zf));
    }
    at<u32>(fx, 0x331c) = out[0];
    at<u32>(fx, 0x3320) = out[1];
    at<u32>(fx, 0x3324) = out[2];
}

// The weather's random numbers (inline in the original): the renderer's seed at +0xa198 stepped.
u32 NextRandom(u8 *fx) {
    const u32 seed = (u32)_rotl(at<u32>(fx, 0xa198) * 0x13d573u, 3) + 0xe0423u;
    at<u32>(fx, 0xa198) = (u32)_rotl(seed, 4);
    return at<u32>(fx, 0xa198);
}

// Not 0 and not a NaN (fcom against 0: neither equal nor unordered).
bool NonZero(float x) { return x < 0.0f || x > 0.0f; }

// The landscape's point i at (sin((angle - a) degrees) * strength, -1, cos((angle - b) degrees) * strength); its x and z.
void SetPoint(s32 i, float angle, float a, float b, float strength, float &x, float &z) {
    const X87Reg sine = X87_Sin(((X87Reg(angle) - a) * kPi * kPerDegree).v);
    const X87Reg cosine = X87_Cos(((X87Reg(angle) - b) * kPi * kPerDegree).v);
    x = (float)(sine * strength);
    z = (float)(cosine * strength);
    Landscape_SetPoint(reinterpret_cast<u8 *>((uintptr_t)0x008c1828), nullptr, i, Bits(x), 0xbf800000, Bits(z));
}

}  // namespace

extern "C" {

void __fastcall WeatherFx_SetDropCount(u8 *fx, void *, u32 count) { at<u32>(fx, 0x3318) = count; }

void __fastcall WeatherFx_SetFallSpeed(u8 *fx, void *, u32 speed) {
    u32 kinds = at<u32>(fx, 0);
    if (kinds & 1)
        at<u32>(fx, 0x3338) = speed;
    if (kinds & 2)
        at<u32>(fx, 0x333c) = speed;
}

void __fastcall WeatherFx_RequestReload(u8 *fx) { at<u32>(fx, 4) = 1; }

void __fastcall WeatherFx_SetWind(u8 *fx, void *, float angle, float strength) {
    const X87Reg t = X87Reg(angle) * kPi * kPerDegree;
    SetDirection(fx, X87Reg(X87_Cos(t.v)) * strength, X87Reg(X87_Sin(t.v)) * strength);
    const X87Reg turned = X87Reg(angle) - X87Reg(-270.0f);   // 0x701ca8 (kept in the register for point 0, stored for the rest)
    const float from = (float)turned;
    const float s = (float)(X87Reg(strength) * 0.0017f);   // 0x701ca4
    const X87Reg a = turned * kPi * kPerDegree;
    const float x0 = (float)(X87Reg(X87_Sin(a.v)) * s);
    const float z0 = (float)(X87Reg(X87_Cos(a.v)) * s);
    Landscape_SetPoint(reinterpret_cast<u8 *>((uintptr_t)0x008c1828), nullptr, 0, Bits(x0), 0xbf800000, Bits(z0));
    float x, z;
    SetPoint(1, from, 1.9f, 2.3f, s, x, z);     // 0x701cac, 0x701cb0
    SetPoint(2, from, 1.2f, -1.7f, s, x, z);    // 0x701cb4, 0x701cb8
    SetPoint(3, from, -1.8f, -2.9f, s, x, z);   // 0x701cbc, 0x701cc0
    SetDirection(fx, X87Reg(x) * 0.2f, X87Reg(z) * 0.2f);   // 0x701cc4
}

float *__cdecl WeatherFx_BoltMidpoint(float *out, const float *a, const float *b, float scale) {
    float m[3], d[3];
    for (u32 i = 0; i < 3; ++i)
        m[i] = (float)((X87Reg(a[i]) + b[i]) * 0.5f);   // 0x701c84
    for (u32 i = 0; i < 3; ++i)
        d[i] = (float)(X87Reg(b[i]) - a[i]);
    const float q = (float)((X87Reg(m[0]) * m[0] + X87Reg(m[1]) * m[1]) + X87Reg(m[2]) * m[2]);
    u8 *const fx = *reinterpret_cast<u8 **>((uintptr_t)0x008bcbac);
    const float step = 1.0f / 8192;   // 0x701cc8
    const float r1 = (float)(X87Reg((s32)NextRandom(fx) % 0x1fff) * step);
    float r = (float)((X87Reg(r1) * r1) * (X87Reg((s32)NextRandom(fx) % 0x1fff) * step));
    if (NextRandom(fx) & 1)
        r = -r;
    NextRandom(fx);
    float p[3] = {m[0], m[1], m[2]};
    static const u8 kOrder[3][3] = {{0, 1, 2}, {1, 2, 0}, {2, 0, 1}};
    const s32 pick = (s32)NextRandom(fx) % 3;
    const u8 *const order = kOrder[pick == 0 ? 0 : pick == 1 ? 1 : 2];
    const u32 axis = NonZero(d[order[0]]) ? order[0] : NonZero(d[order[1]]) ? order[1] : order[2];
    p[axis] = (float)X87_SubR(X87Reg(q) / d[axis], m[axis]);
    const X87Reg n0 = X87Reg(p[2]) * d[1] - X87Reg(p[1]) * d[2];
    const float n1 = (float)(X87Reg(p[0]) * d[2] - X87Reg(p[2]) * d[0]);
    const float n2 = (float)(X87Reg(p[1]) * d[0] - X87Reg(p[0]) * d[1]);
    const X87Reg along = X87_Sqrt((X87Reg(d[0]) * d[0] + X87Reg(d[1]) * d[1]) + X87Reg(d[2]) * d[2]) * scale * r;
    const X87Reg f = along / X87_Sqrt((X87Reg(p[0]) * p[0] + X87Reg(p[1]) * p[1]) + X87Reg(p[2]) * p[2]);
    for (u32 i = 0; i < 3; ++i)
        p[i] = (float)(f * p[i]);
    const X87Reg g = X87_DivR(X87_Sqrt((n0 * n0 + X87Reg(n1) * n1) + X87Reg(n2) * n2), X87Reg(0.0f));   // 0x701ca0 (sic)
    out[0] = (float)((g * n0 + p[0]) + m[0]);
    const float gf = (float)g;
    out[1] = (float)((X87Reg(gf) * n1 + p[1]) + m[1]);
    out[2] = (float)((X87Reg(gf) * n2 + p[2]) + m[2]);
    return out;
}

u32 __cdecl WeatherFx_Delete() {
    u8 *&slot = *reinterpret_cast<u8 **>((uintptr_t)0x008bcbac);
    if (!slot)
        return 0;
    WeatherFx_FreeTextures(slot);
    GameHeap_Free(slot);
    slot = nullptr;
    return 1;
}

u32 __fastcall WeatherFx_FreeTextures(u8 *fx) {
    for (u32 offset = 0x3348; offset <= 0x3350; offset += 4) {
        if (!at<u8 *>(fx, offset))
            continue;
        Texture_Release(at<u8 *>(fx, offset));
        if (u8 *const array = at<u8 *>(fx, offset))
            Texture_DestroyArray(array);
        at<u32>(fx, offset) = 0;
    }
    return 1;
}

u32 __fastcall WeatherFx_LoadTexture(u8 *fx, void *, u32 which, const char *name) {
    u8 **slot;
    if (which == 1)
        slot = reinterpret_cast<u8 **>(fx + 0x3350);
    else if (which == 2)
        slot = reinterpret_cast<u8 **>(fx + 0x3348);
    else if (which == 3)
        slot = reinterpret_cast<u8 **>(fx + 0x334c);
    else
        slot = reinterpret_cast<u8 **>((uintptr_t)which);
    if (*slot == nullptr) {
        *slot = TexManager_Load(*reinterpret_cast<u8 *const *>((uintptr_t)0x0087a268), nullptr, name);
        if (*slot != nullptr)
            *reinterpret_cast<u32 *>(*slot + 8) |= 0x80200;
    }
    return *slot != nullptr;
}

u32 __cdecl WeatherFx_LoadTextures() {
    u8 *const *const fx = reinterpret_cast<u8 *const *>((uintptr_t)0x008bcbac);
    if (!*fx) return 0;
    const auto name = [](u32 address) { return Quickload_Locate(reinterpret_cast<const char *>((uintptr_t)address), 1, nullptr); };
    WeatherFx_LoadTexture(*fx, nullptr, 2, name(0x00765218));          // "Data\Generic\Weather\snow.tga"
    WeatherFx_LoadTexture(*fx, nullptr, 1, name(0x007651f4));          // "Data\Generic\Weather\raindrop.tga"
    return WeatherFx_LoadTexture(*fx, nullptr, 3, name(0x007651d0));   // "Data\Generic\Weather\lightning.tga"
}

namespace {

// Whether the midpoint lies within the wobbled limit of an end point (squared lengths: a NaN is not near).
bool BoltNear(const u8 *fx, const float *m, const float *p, float wobble) {
    const X87Reg dx = X87Reg(m[0]) - p[0], dy = X87Reg(m[1]) - p[1], dz = X87Reg(m[2]) - p[2];
    const X87Reg q = (dx * dx + dy * dy) + dz * dz;
    const X87Reg limit = X87Reg(wobble) + at<float>(fx, 0xa180);
    return limit.v > q.v;
}

}  // namespace

u32 __fastcall WeatherFx_BoltSubdivide(u8 *fx, void *, const float *a, const float *b) {
    u8 *const rng = *reinterpret_cast<u8 **>((uintptr_t)0x008bcbac);
    float m[3];
    WeatherFx_BoltMidpoint(m, b, a, at<float>(fx, 0x3310));
    if ((NextRandom(rng) & 0x1ff) < 9 && at<s32>(fx, 0xa188) < at<s32>(fx, 0xa1a0)) {
        const s32 ribbon = ++at<s32>(fx, 0xa188);
        const float per = (float)(X87Reg(1.0f) / X87Reg((double)(s32)((u32)ribbon * 0x60)));
        float x = (float)(X87Reg(b[0]) - a[0]), y = (float)(X87Reg(b[1]) - a[1]), z = (float)(X87Reg(b[2]) - a[2]);
        const X87Reg length = X87_Sqrt((X87Reg(x) * x + X87Reg(y) * y) + X87Reg(z) * z);
        if (length.v != 0.0 && length.v == length.v) {
            const X87Reg inverse = X87Reg(1.0f) / length;
            x = (float)(inverse * x);
            y = (float)(inverse * y);
            z = (float)(inverse * z);
        }
        const X87Reg s = X87Reg((double)(s32)((NextRandom(rng) & 0x3ff) + 0x200)) * per * 2.5f;   // 0x701ccc
        x = (float)(s * x);
        y = (float)(s * y);
        z = (float)(s * z);
        const float t = (float)(X87Reg((double)(s32)((NextRandom(rng) & 0x3ff) + 0x200)) * per);
        const float *const n = reinterpret_cast<const float *>(fx + at<s32>(fx, 0x9da0) * 0x30 + 0x3020);
        x = (float)(X87Reg(t) * n[0] + x);
        y = (float)(X87Reg(t) * n[1] + y);
        X87Reg z2 = X87Reg(t) * n[2] + z;
        z = (float)z2;
        const X87Reg dot = (X87Reg(n[0]) * x + X87Reg(n[1]) * y) + X87Reg(n[2]) * z2;
        if (!(dot.v >= 0.0)) {   // (a NaN too)
            const X87Reg half = X87Reg(t) * 0.5f;   // 0x701c84
            x = (float)((half * n[0]) - (X87Reg(x) * -0.5f));   // 0x701cd0
            y = (float)((half * n[1]) - (X87Reg(y) * -0.5f));
            z2 = (half * n[2]) - (z2 * -0.5f);
        }
        const float side[3] = {(float)(X87Reg(a[0]) + x), (float)(X87Reg(y) + a[1]), (float)(z2 + a[2])};
        at<u32>(fx, 0x9da4 + at<u32>(fx, 0xa188) * 4) = 0;
        if (!WeatherFx_BoltSubdivide(fx, nullptr, side, a))
            return 0;
        if (!Ribbon_AddPoint(fx, nullptr, nullptr, side))
            return 0;
        --at<s32>(fx, 0xa188);
    }
    const float wobble = (float)(X87Reg((double)((s32)(NextRandom(rng) & 0xfff) - 0x800)) * at<float>(fx, 0xa184) * (1.0f / 2048.0f));   // 0x701c64
    if (BoltNear(fx, m, b, wobble)) {
        if (!Ribbon_AddPoint(fx, nullptr, m, b))
            return 0;
    } else if (!WeatherFx_BoltSubdivide(fx, nullptr, m, b)) {
        return 0;
    }
    if (BoltNear(fx, m, a, wobble)) {
        if (!Ribbon_AddPoint(fx, nullptr, a, m))
            return 0;
    } else if (!WeatherFx_BoltSubdivide(fx, nullptr, a, m)) {
        return 0;
    }
    return 1;
}

u32 __fastcall WeatherFx_ReloadTextures(u8 *fx) {
    const auto name = [](u32 address) { return Quickload_Locate(reinterpret_cast<const char *>((uintptr_t)address), 1, nullptr); };
    WeatherFx_LoadTexture(fx, nullptr, 2, name(0x00765218));
    WeatherFx_LoadTexture(fx, nullptr, 1, name(0x007651f4));
    WeatherFx_LoadTexture(fx, nullptr, 3, name(0x007651d0));
    *reinterpret_cast<u32 *>(fx + 4) = 0;
    return 1;
}

namespace {
const WeatherDrawEnv kDrawPortEnv = {&Batch_Begin, &Batch_TransformVertices, &Batch_Close, &WeatherFx_BoltSubdivide, &Ribbon_AddPoint};
const WeatherDrawEnv *g_drawEnv = &kDrawPortEnv;
}  // namespace

void WeatherFx_SetDrawEnv(const WeatherDrawEnv *env) { g_drawEnv = env ? env : &kDrawPortEnv; }

u32 __fastcall WeatherFx_DrawBolt(u8 *fx, void *, const float *a, const float *b) {
    const u8 *const texture = at<const u8 *>(fx, 0x334c);
    const u32 id = texture && (at<u32>(texture, 8) & 0x1000) ? at<u32>(texture, 0x1c) : 0;
    if (id) {
        at<u32>(fx, 0xa170) = X87_FloatThrough(at<u32>(texture, 0x28));
        at<u32>(fx, 0xa174) = X87_FloatThrough(at<u32>(texture, 0x2c));
        at<float>(fx, 0xa178) = (float)(X87Reg(at<float>(texture, 0x20)) + X87Reg(at<float>(fx, 0xa170)));
        at<float>(fx, 0xa17c) = (float)(X87Reg(at<float>(texture, 0x24)) + at<float>(fx, 0xa174));
    } else {
        at<u32>(fx, 0xa17c) = 0;
        at<u32>(fx, 0xa178) = 0;
        at<u32>(fx, 0xa174) = 0;
        at<u32>(fx, 0xa170) = 0;
    }
    u8 *const object = *reinterpret_cast<u8 **>((uintptr_t)0x00879ab0);
    if (!g_drawEnv->begin(object, nullptr, at<u32>(fx, 0x330c), 4, 0x1c4, 0x200d52, id))
        return 1;
    at<u32>(fx, 0xa188) = 0;
    at<u32>(fx, 0x9da4) = 0;
    if (!g_drawEnv->subdivide(fx, nullptr, a, b) || !g_drawEnv->addPoint(fx, nullptr, nullptr, a))
        return 0;
    g_drawEnv->transform(object, nullptr, 0);
    return g_drawEnv->close(object, nullptr, 0.0f);
}
}

namespace {
const WeatherDropsEnv kDropsPortEnv = {&Batch_Begin, &DrawBatch_AddPolygon, &Batch_TransformVertices, &Batch_Close};
const WeatherDropsEnv *g_dropsEnv = &kDropsPortEnv;

struct Place {
    float x, y, z;
};

const float &K(uintptr_t address) { return *reinterpret_cast<const float *>(address); }
u8 *BatchObject() { return *reinterpret_cast<u8 **>((uintptr_t)0x00879ab0); }

// The curve's row for a phase (+0x8558: 128 rows of 0x30) and the fraction: p less its __ftol.
const u8 *CurveRow(const u8 *fx, X87Reg p, float &f) {
    const s32 n = (s32)(long long)p.v;
    f = (float)(p - X87Reg((double)n));
    return fx + 0x8558 + (u32)(n & 0x7f) * 0x30;
}

// A row's three cubics in the original's order of additions (first the two low terms); r0 its first coefficient's term, f3 a register or (the drops) a float.
X87Reg CurveX(const u8 *row, float f, float f2, X87Reg r0) {
    return X87Reg(at<float>(row, 0x18)) * f + r0 + X87Reg(at<float>(row, 0xc)) * f2 + at<float>(row, 0x24);
}
template <class F3> X87Reg CurveY(const u8 *row, float f, float f2, F3 f3) {
    return X87Reg(at<float>(row, 0x1c)) * f + X87Reg(at<float>(row, 0x10)) * f2 + X87Reg(at<float>(row, 4)) * f3 + at<float>(row, 0x28);
}
template <class F3> X87Reg CurveZ(const u8 *row, float f, float f2, F3 f3) {
    return X87Reg(at<float>(row, 0x20)) * f + X87Reg(at<float>(row, 0x14)) * f2 + X87Reg(at<float>(row, 8)) * f3 + at<float>(row, 0x2c);
}

// A gust: its x from the register, y and z from the stored floats (and they are multiplied by the wind, not added, sic).
Place Gust(const u8 *fx, float phase, float time) {
    float f;
    const u8 *const row = CurveRow(fx, (X87Reg(phase) - time) * K(0x00701d18), f);
    const X87Reg f2r = X87Reg(f) * f;
    const float f2 = (float)f2r;
    const X87Reg f3 = f2r * f;
    const X87Reg x = CurveX(row, f, f2, f3 * at<float>(row, 0));
    const float y = (float)CurveY(row, f, f2, f3), z = (float)CurveZ(row, f, f2, f3);
    const float &size = at<float>(fx, 0xa194);
    return {(float)(x * size + at<float>(fx, 0x331c)), (float)(X87Reg(y) * size * at<float>(fx, 0x3320)), (float)(X87Reg(z) * size * at<float>(fx, 0x3324))};
}

// Past 4 less 8; below -4 or a NaN less -8; above: the first comparison (of x's register, for x).
void Wrap(float &x, bool above) {
    if (above) x = (float)(X87Reg(x) - K(0x00701cf4));
    else if (!(x >= K(0x00701cf8))) x = (float)(X87Reg(x) - K(0x00701cfc));
}

void WrapPlace(Place &p, X87Reg x) {
    Wrap(p.x, x.v > K(0x00701cf0));
    Wrap(p.y, p.y > K(0x00701cf0));
    Wrap(p.z, p.z > K(0x00701cf0));
}

void SwirledPlaces(u8 *fx, Place *places, float x, float y, float z) {
    const float time = (float)(X87Reg(at<float>(fx, 0x9d98)) * K(0x00701d14));
    const Place gusts[2] = {Gust(fx, at<float>(fx, 0x3558), time), Gust(fx, at<float>(fx, 0x355c), time)};
    Place orbits[16];
    for (s32 i = 0; i < 16; i++) {
        float f;
        const u8 *const row = CurveRow(fx, (X87Reg(at<float>(fx, 0x3558 + i * 4)) - time) * K(0x00701d1c), f);
        const X87Reg f2r = X87Reg(f) * f;
        const float f2 = (float)f2r;
        const X87Reg f3 = f2r * f;
        const float px = (float)CurveX(row, f, f2, X87Reg(at<float>(row, 0)) * f3), py = (float)CurveY(row, f, f2, f3), pz = (float)CurveZ(row, f, f2, f3);
        const float &size = at<float>(fx, 0xa190);
        const float sx = (float)(X87Reg(px) * size), sy = (float)(X87Reg(py) * size), sz = (float)(X87Reg(pz) * size);
        const Place &gust = gusts[i & 1];
        orbits[i] = {(float)(X87Reg(gust.x) + sx), (float)(X87Reg(gust.y) + sy), (float)(X87Reg(gust.z) + sz)};
    }
    for (s32 i = 0; i < at<s32>(fx, 0x3318); i++) {
        float &phase = at<float>(fx, 0x3558 + i * 4);
        const X87Reg v = phase - (X87Reg(at<float>(fx, 0x9d98)) * K(0x00701d14));
        phase = (float)v;
        float f;
        const u8 *const row = CurveRow(fx, v, f);
        const X87Reg f2r = X87Reg(f) * f;
        const float f2 = (float)f2r;
        const float f3 = (float)(f2r * f);
        const X87Reg cx = CurveX(row, f, f2, X87Reg(at<float>(row, 0)) * f3);
        const X87Reg cy = CurveY(row, f, f2, f3), cz = CurveZ(row, f, f2, f3);
        const float &size = at<float>(fx, 0xa18c);
        const Place &orbit = orbits[i & 0xf];
        const X87Reg px = cx * size + orbit.x + (X87Reg(at<float>(fx, 8 + i * 0xc)) - x);
        const X87Reg py = cy * size + orbit.y + y + at<float>(fx, 0xc + i * 0xc);
        const X87Reg pz = cz * size + orbit.z + (X87Reg(at<float>(fx, 0x10 + i * 0xc)) - z);
        Place &p = places[i];
        p = {(float)px, (float)py, (float)pz};
        WrapPlace(p, px);
    }
}

float ShakeDraw() { return (float)(X87Reg((double)(s32)((Crt().rand() & 0x1f) - 0x10)) * K(0x00701c64)); }

void ShakenPlaces(const u8 *fx, Place *places, float x, float y, float z) {
    for (s32 i = 0; i < at<s32>(fx, 0x3318); i++) {
        const float a = ShakeDraw();
        const float b = ShakeDraw();
        const float c = ShakeDraw();
        const X87Reg px = X87Reg(at<float>(fx, 8 + i * 0xc)) - x + a;
        const X87Reg py = X87Reg(b) + y + at<float>(fx, 0xc + i * 0xc);
        const X87Reg pz = X87Reg(at<float>(fx, 0x10 + i * 0xc)) - z + c;
        Place &p = places[i];
        p = {(float)px, (float)py, (float)pz};
        WrapPlace(p, px);
    }
}
}  // namespace

void WeatherFx_SetDropsEnv(const WeatherDropsEnv *env) { g_dropsEnv = env ? env : &kDropsPortEnv; }

u32 __fastcall WeatherFx_DrawDrops(u8 *fx, void *, float x, float y, float z) {
    const float across = (float)(X87Reg(*reinterpret_cast<const float *>((uintptr_t)0x008bcbcc)) * at<float>(fx, 0x3354));
    const u32 highBits = X87_FloatThrough(at<u32>(fx, 0x3354));
    const float &high = at<float>(&highBits, 0);
    const u32 colour = at<u32>(*reinterpret_cast<u8 **>((uintptr_t)0x0087a248), 0x2c);
    const u8 *const texture = at<const u8 *>(fx, 0x3348);
    const u32 id = texture && (at<u32>(texture, 8) & 0x1000) ? at<u32>(texture, 0x1c) : 0;
    u32 u0 = 0, v0 = 0;
    X87Reg s = 0.0, t = 0.0;
    if (id) {
        u0 = at<u32>(texture, 0x28);
        v0 = at<u32>(texture, 0x2c);
        s = X87Reg(at<float>(texture, 0x20));
        t = X87Reg(at<float>(texture, 0x24));
    }
    float us[3], vs[3];
    at<u32>(us, 0) = u0;
    at<u32>(vs, 0) = v0;
    us[1] = (float)(us[0] - s * K(0x00701cd0));
    vs[1] = (float)(vs[0] - t * K(0x00701cd0));
    us[2] = (float)(X87Reg(us[0]) + s);
    vs[2] = (float)(X87Reg(vs[0]) + t);

    Place places[0x400];
    if (*reinterpret_cast<const u32 *>((uintptr_t)0x008bcbc8) & 0x100) SwirledPlaces(fx, places, x, y, z);
    else ShakenPlaces(fx, places, x, y, z);

    u8 *const transformer = *reinterpret_cast<u8 **>((uintptr_t)0x0087b1d8);
    at<void(__cdecl *)(u8 *, u8 *, Place *, Place *, u32, u32, s32, u32)>(at<const u8 *>(transformer, 0), 4)(transformer, fx + 0x9d58, places, places, 0xc, 0xc, at<s32>(fx, 0x3318), 2);
    if (!g_dropsEnv->begin(BatchObject(), nullptr, at<u32>(fx, 0x330c), 4, 0x1c4, 0x556, id)) return 1;

    for (s32 i = 0; i < at<s32>(fx, 0x3318); i++) {
        Place &p = places[i];
        if (!(p.z > K(0x00701ca0))) continue;
        const X87Reg w = X87Reg(p.z) * K(0x00701d20) - K(0x00701c9c);
        p.x = (float)(X87Reg(p.x) * w);
        p.y = (float)(X87Reg(p.y) * w);
        const u32 uA = X87_FloatThrough(at<u32>(us, (1 + (i & 1)) * 4)), uB = X87_FloatThrough(at<u32>(us, (i & 1) * 4));
        const u32 vA = X87_FloatThrough(at<u32>(vs, (1 + (i >> 1 & 1)) * 4)), vB = X87_FloatThrough(at<u32>(vs, (i >> 1 & 1) * 4));
        const float nearer = (float)(X87Reg(p.z) - K(0x00701d10)), further = (float)(X87Reg(p.z) - K(0x00701d24));
        const float lower = (float)X87Reg(-(X87Reg(p.y) - high).v);
        // (The second dword of each vertex's colour stays unset, sic.)
        u32 vertices[24];
        vertices[0] = Bits(p.x);
        vertices[1] = Bits((float)X87Reg(-(X87Reg(high) + p.y).v));
        vertices[2] = Bits(nearer);
        vertices[3] = Bits(further);
        vertices[4] = colour;
        vertices[6] = Bits((float)((X87Reg(at<float>(&uA, 0)) + at<float>(&uB, 0)) * K(0x00701c84)));
        vertices[7] = vB;
        vertices[8] = Bits((float)(X87Reg(across) + p.x));
        vertices[9] = Bits(lower);
        vertices[10] = Bits(nearer);
        vertices[11] = Bits(further);
        vertices[12] = colour;
        vertices[14] = uA;
        vertices[15] = vA;
        vertices[16] = Bits((float)(X87Reg(p.x) - across));
        vertices[17] = Bits(lower);
        vertices[18] = Bits(nearer);
        vertices[19] = Bits(further);
        vertices[20] = colour;
        vertices[22] = uB;
        vertices[23] = vA;
        if (!g_dropsEnv->addPolygon(BatchObject(), nullptr, reinterpret_cast<u8 *>(vertices), 3, 0x20, nullptr, 0)) return 0;
    }
    g_dropsEnv->transform(BatchObject(), nullptr, 0);
    return g_dropsEnv->close(BatchObject(), nullptr, -1.0f);
}

u32 __fastcall WeatherFx_DrawStreaks(u8 *fx, void *, float x, float y, float z) {
    struct Streak {
        Place from, to;
    };
    const u8 *const texture = at<const u8 *>(fx, 0x3350);
    const u32 id = texture && (at<u32>(texture, 8) & 0x1000) ? at<u32>(texture, 0x1c) : 0;
    u32 sizes[4] = {0, 0, 0, 0};  // the texture's +0x20 (u's), +0x24 (v's), +0x28 (u), +0x2c (v)
    if (id) memcpy(sizes, texture + 0x20, sizeof sizes);
    const float &uSize = at<float>(sizes, 0), &vSize = at<float>(sizes, 4), &u = at<float>(sizes, 8), &v = at<float>(sizes, 0xc);

    Streak streaks[0x400];
    const s32 count = at<s32>(fx, 0x3318);
    if (count > 0) {
        const float &scale = at<float>(fx, 0x3314);
        const X87Reg dx = X87Reg(at<float>(fx, 0x331c)) * scale, dy = X87Reg(at<float>(fx, 0x3320)) * scale, dz = X87Reg(at<float>(fx, 0x3324)) * scale;
        for (s32 i = 0; i < count; i++) {
            Streak &s = streaks[i];
            s.from.x = (float)(X87Reg(at<float>(fx, 8 + i * 0xc)) - x);
            s.from.y = (float)(X87Reg(y) + at<float>(fx, 0xc + i * 0xc));
            s.from.z = (float)(X87Reg(at<float>(fx, 0x10 + i * 0xc)) - z);
            WrapPlace(s.from, X87Reg(s.from.x));
            s.to = {(float)(X87Reg(s.from.x) + dx), (float)(X87Reg(s.from.y) + dy), (float)(X87Reg(s.from.z) + dz)};
        }
    }

    u8 *const transformer = *reinterpret_cast<u8 **>((uintptr_t)0x0087b1d8);
    at<void(__cdecl *)(u8 *, u8 *, Streak *, Streak *, u32, u32, s32, u32)>(at<const u8 *>(transformer, 0), 4)(transformer, fx + 0x9d58, streaks, streaks, 0xc, 0xc, count * 2, 2);
    const u32 flags = (*reinterpret_cast<const u32 *>((uintptr_t)0x008bcbc8) & 2) ? 0x200d22 : 0x200d52;
    u8 *const batch = g_dropsEnv->begin(BatchObject(), nullptr, at<u32>(fx, 0x330c), 4, 0x1c4, flags, id);
    if (!batch) return 1;

    Streak *s = streaks;
    for (s32 left = at<s32>(fx, 0x3318); left != 0; left--, s++) {
        const bool front = s->from.z > K(0x00701ca0);
        s->from.y = (float)X87Reg(-X87Reg(s->from.y).v);
        s->to.y = (float)X87Reg(-X87Reg(s->to.y).v);
        if (!front) continue;
        // Across the streak on the screen, its size a tenth of its length; the nearer, the brighter (none past 6).
        const float acrossY = (float)((X87Reg(s->to.x) - s->from.x) * K(0x00701d00));
        const float acrossX = (float)((X87Reg(s->to.y) - s->from.y) * K(0x00701d00));
        s32 bright = (s32)(long long)((X87Reg(K(0x00701d04)) - s->from.z) * K(0x00701d08)).v;
        if (bright < 0) bright = 0;
        const u32 fog = at<u32>(*reinterpret_cast<u8 **>((uintptr_t)0x0087a248), 0x2c);
        const u32 shade = (u32)bright * 0x30303;
        const u32 colourFrom = (shade | 0x80000000) & fog, colourTo = (shade | 0xff000000) & fog;
        const u32 nearer = Bits((float)(X87Reg(s->from.z) - K(0x00701d0c)));
        const u32 depthFrom = Bits((float)(X87Reg(s->from.z) - K(0x00701d10))), depthTo = Bits((float)(X87Reg(s->to.z) - K(0x00701d10)));
        const u32 uFar = Bits((float)(X87Reg(u) + uSize)), vFar = Bits((float)(X87Reg(v) + vSize));
        const u32 uNear = X87_FloatThrough(at<u32>(sizes, 8)), vNear = X87_FloatThrough(at<u32>(sizes, 0xc));
        // (The second dword of each vertex's colour stays unset, sic.)
        u32 vertices[32];
        vertices[0] = Bits((float)(X87Reg(s->from.x) + acrossX));
        vertices[1] = Bits((float)(X87Reg(s->from.y) - acrossY));
        vertices[2] = nearer;
        vertices[3] = depthFrom;
        vertices[4] = colourFrom;
        vertices[6] = uFar;
        vertices[7] = vFar;
        vertices[8] = Bits((float)(X87Reg(s->from.x) - acrossX));
        vertices[9] = Bits((float)(X87Reg(acrossY) + s->from.y));
        vertices[10] = nearer;
        vertices[11] = depthFrom;
        vertices[12] = colourFrom;
        vertices[14] = uNear;
        vertices[15] = vFar;
        vertices[16] = Bits((float)(X87Reg(s->to.x) - acrossX));
        vertices[17] = Bits((float)(X87Reg(acrossY) + s->to.y));
        vertices[18] = nearer;
        vertices[19] = depthTo;
        vertices[20] = colourTo;
        vertices[22] = uNear;
        vertices[23] = vNear;
        vertices[24] = Bits((float)(X87Reg(s->to.x) + acrossX));
        vertices[25] = Bits((float)(X87Reg(s->to.y) - acrossY));
        vertices[26] = nearer;
        vertices[27] = depthTo;
        vertices[28] = colourTo;
        vertices[30] = uFar;
        vertices[31] = vNear;
        if (!g_dropsEnv->addPolygon(BatchObject(), nullptr, reinterpret_cast<u8 *>(vertices), 4, 0x20, nullptr, 0)) return 0;
    }
    g_dropsEnv->transform(BatchObject(), nullptr, 0);
    at<u32>(batch, 0x30) = 0;
    return g_dropsEnv->close(BatchObject(), nullptr, -1.0f);
}

namespace {

const WeatherFxReloadEnv kReloadGame = {&Quickload_Locate,
                                        &WeatherFx_LoadTexture,
                                        &SpriteQueue_LoadTexture,
                                        reinterpret_cast<u8 *(__fastcall *)(u8 *)>(&MemorySource_GetLock),
                                        &Handle_Ctor,
                                        &Handle_Open28,
                                        &Handle_Vfn1,
                                        &Handle_Vfn8,
                                        &Handle_Dtor,
                                        &ColourRamps_SetFromPalette};
const WeatherFxReloadEnv *g_reload = &kReloadGame;

const char *TextAt(uintptr_t address) { return reinterpret_cast<const char *>(address); }

}  // namespace

void WeatherFx_SetReloadEnv(const WeatherFxReloadEnv *env) { g_reload = env ? env : &kReloadGame; }

extern "C" void __cdecl WeatherFx_ReloadEffects() {
    u32 handle[2];
    Handle *const h = reinterpret_cast<Handle *>(handle);
    g_reload->ctor(h);
    handle[0] = 0x006fd9e8;
    if (*reinterpret_cast<u8 **>((uintptr_t)0x008bcbac)) {
        const char *name = g_reload->locate(TextAt(0x00765218), 1, nullptr);
        g_reload->load(*reinterpret_cast<u8 **>((uintptr_t)0x008bcbac), nullptr, 2, name);
        name = g_reload->locate(TextAt(0x007651f4), 1, nullptr);
        g_reload->load(*reinterpret_cast<u8 **>((uintptr_t)0x008bcbac), nullptr, 1, name);
        name = g_reload->locate(TextAt(0x007651d0), 1, nullptr);
        g_reload->load(*reinterpret_cast<u8 **>((uintptr_t)0x008bcbac), nullptr, 3, name);
    }
    if (u8 *const queue = *reinterpret_cast<u8 **>((uintptr_t)0x008c47a0))
        g_reload->queueLoad(queue, nullptr, TextAt(0x00765238));
    const u8 *const lock = g_reload->getLock(reinterpret_cast<u8 *>((uintptr_t)0x00f816c0));
    s32 status;
    if (*g_reload->open(h, nullptr, &status, at<u32>(lock, 8)) >= 0) {
        u8 entry[0x128];
        at<u8 *>(entry, 0x20) = entry + 0x24;
        entry[0x24] = 0;
        if (*g_reload->first(h, nullptr, &status, *reinterpret_cast<u32 *>((uintptr_t)0x00878ec0), (u32)(uintptr_t)entry) < 0)
            g_reload->close(h, nullptr, &status, *reinterpret_cast<u32 *>((uintptr_t)0x00878ec0));
    }
    g_reload->setRamps();
    g_reload->dtor(h);
}

namespace {
AddrCalls g_boltCalls;

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// A coordinate of the view's offset: the 14 bits of the fixed-point place, over the wrap's 8 units.
X87Reg fixed_place(X87Reg place) {
    const u32 bits = (u32)(long long)((place - (-4.0f)) * 16384.0f).v & 0x1ffff;   // 0x701cf8, 0x701d2c
    return X87Reg((double)bits) * 6.103515625e-05f - 4.0f;                         // 0x701c60, 0x701cf0
}

// One of the place's three: stored, then wrapped into -4..4; below -4 or a NaN gets 8 more, above 4 gets 8 less. (The original's X compares its stored float above -4: it differs from the register only past the range of a float, with the same outcome.)
void wrap_place(float &out, X87Reg v) {
    out = (float)v;
    if (!(v.v >= -4.0)) out = (float)(v - (-8.0f));   // 0x701cf8, 0x701cfc
    else if (v.v > 4.0) out = (float)(v - 8.0f);      // 0x701cf0, 0x701cf4
}
}  // namespace

void WeatherFx_SetBoltCalls(const AddrCall *table, u32 count) { g_boltCalls.Set(table, count); }

extern "C" void __cdecl KidShadow_LoadTexture() {
    const u32 queue = global<u32>(0x008c47a0);
    if (queue) g_boltCalls.Th(0x0058a5c0, queue, 0x00765238u);
}

extern "C" {

u32 __fastcall WeatherFx_TouchTextures(u8 *fx) {
    if (at<u32>(fx, 0x3308) && at<u32>(fx, 0x334c)) g_boltCalls.Th(0x58c9f0, global<u32>(0x008c4b44), at<u32>(fx, 0x334c), 0x3f000000u);
    if (at<u32>(fx, 0x3318)) {
        const u32 kind = at<u32>(fx, 0);
        if (kind == 1) {
            if (at<u32>(fx, 0x3350)) g_boltCalls.Th(0x58c9f0, global<u32>(0x008c4b44), at<u32>(fx, 0x3350), 0x3f000000u);
        } else if (kind == 2) {
            if (at<u32>(fx, 0x3348)) g_boltCalls.Th(0x58c9f0, global<u32>(0x008c4b44), at<u32>(fx, 0x3348), 0x3dcccccdu);
        }
    }
    return 1;
}

u32 __fastcall WeatherFx_DrawBolts(u8 *fx) {
    if (at<u32>(fx, 4) && !g_boltCalls.Cd(0x0057ed50))
        return 0;
    u32 i = at<u32>(fx, 0x3308);
    while (i--) {
        u8 *const bolt = fx + 0x3008 + i * 0x30;
        const s32 left = (s32)(at<u32>(bolt, 0x24) - at<u32>(fx, 0x9d9c));
        at<s32>(bolt, 0x24) = left;
        if (left <= 0) {
            const u32 last = --at<u32>(fx, 0x3308);
            if (i != last)
                std::memcpy(bolt, fx + 0x3008 + last * 0x30, 0x30);
            continue;
        }
        at<u32>(fx, 0x9da0) = i;
        const u32 ticks = at<u32>(bolt, 0x2c);
        if (ticks) {
            at<u32>(fx, 0xa198) = ticks;
            const X87Reg t = X87Reg((double)(left % 500)) * 0.002f;   // 0x701cd8
            s32 shade = t.v < 0.9f ? 0xff - (s32)(long long)((X87Reg(0.9f) - t) * 1.11f * 64.0f).v    // 0x701cdc, 0x701ce0, 0x701ce4
                                   : 0xc0 - (s32)(long long)((X87Reg(1.0f) - t) * 10.0f * -64.0f).v;  // 0x701c68, 0x701ce8, 0x701cec
            shade += (s32)(g_boltCalls.Cd(0x0067b5c0) & 0x3f);
            at<u32>(fx, 0xa1a0) = 4;
            at<u32>(fx, 0xa19c) = (((u32)shade | 0xffffff00u) << 8 | (u32)shade) << 8 | (u32)shade;
        } else {
            at<u32>(fx, 0xa198) = g_boltCalls.Cd(0x00591210);
            at<u32>(fx, 0xa19c) = 0xffffffffu;
            at<u32>(fx, 0xa1a0) = 0;
        }
        if (!g_boltCalls.Th(0x0057ec50, (u32)(uintptr_t)fx, bolt + 0xc, bolt))
            return 0;
    }
    return 1;
}

u32 __fastcall WeatherFx_AddBolt(u8 *fx, void *, u32 ax, u32 ay, u32 az, u32 bx, u32 by, u32 bz, u32 life, u32 flash) {
    const s32 count = at<s32>(fx, 0x3308);
    if (count >= 0x10)
        return 1;
    u8 *const bolt = fx + 0x3008 + count * 0x30;
    at<u32>(bolt, 0) = ax;
    at<u32>(bolt, 4) = ay;
    at<u32>(bolt, 8) = az;
    at<u32>(bolt, 0xc) = bx;
    at<u32>(bolt, 0x10) = by;
    at<u32>(bolt, 0x14) = bz;
    at<u32>(bolt, 0x24) = life;
    at<u32>(bolt, 0x2c) = flash ? g_boltCalls.Cd(0x00591210) | 1 : 0;
    const float x = (float)(X87Reg(at<float>(bolt, 0)) - at<float>(bolt, 0xc));
    const float y = (float)(X87Reg(at<float>(bolt, 4)) - at<float>(bolt, 0x10));
    const float z = (float)(X87Reg(at<float>(bolt, 8)) - at<float>(bolt, 0x14));
    u32 out[3] = {Bits(x), Bits(y), Bits(z)};
    const X87Reg length = X87_Sqrt(X87Reg(x) * x + X87Reg(y) * y + X87Reg(z) * z);
    if (length.v != 0.0 && length.v == length.v) {   // 0x701ca0
        const X87Reg scale = X87Reg(1.0f) / length;   // 0x701c68
        out[0] = Bits((float)(scale * x));
        out[1] = Bits((float)(scale * y));
        out[2] = Bits((float)(scale * z));
    }
    at<u32>(bolt, 0x18) = out[0];
    at<u32>(bolt, 0x1c) = out[1];
    at<u32>(bolt, 0x20) = out[2];
    at<s32>(fx, 0x3308) = count + 1;
    return 1;
}

u32 __fastcall WeatherFx_Frame(u8 *fx) {
    u32 old = at<u32>(fx, 0x3344);
    const u32 now = g_boltCalls.Th(0x00402d70, 0x00785970);
    at<u32>(fx, 0x3344) = now;
    if ((s32)(now - old) > 500) old = now - 500;
    const s32 gap = (s32)(now - old);
    at<s32>(fx, 0x9d9c) = gap;
    at<float>(fx, 0x9d98) = (float)(X87Reg((double)gap) * 0.001f);   // 0x701c88
    if (global<u8>(0x008bd508) & 4) at<u32>(fx, 0x9d98) = 0;
    at<u32>(fx, 0x330c) = 2;
    if (!g_boltCalls.Th(0x0057ee30, (u32)(uintptr_t)fx)) return 0;
    if (!at<u32>(fx, 0x3318)) return 1;

    const float scale = global<u8>(0x00790ab0) & 0x16 ? 0.3f : at<float>(fx, 0x3340);   // 0x3e99999a
    const u8 *const cam = global<u8 *>(0x0087afe8);
    const X87Reg x = fixed_place(X87Reg(at<float>(cam, 0x30)) * scale);
    const X87Reg y = fixed_place(X87Reg(-(X87Reg(at<float>(cam, 0x34)) * scale).v));
    const float z = (float)fixed_place(X87Reg((float)(X87Reg(at<float>(cam, 0x38)) * scale)));
    std::memcpy(fx + 0x9d58, global<u8 *>(0x0087b154), 0x40);
    at<u32>(fx, 0x9d88) = 0;
    at<u32>(fx, 0x9d8c) = 0;
    at<u32>(fx, 0x9d90) = 0;

    const u32 mask = at<u32>(fx, 0);
    const float step = (float)(X87Reg(at<float>(fx, mask & 1 ? 0x3338 : 0x333c)) * at<float>(fx, 0x9d98));
    const X87Reg south = X87Reg(at<float>(fx, 0x3320)) * step;
    const float west = (float)(X87Reg(at<float>(fx, 0x3324)) * step);
    wrap_place(at<float>(fx, 0x3328), X87_SubR(X87Reg(at<float>(fx, 0x331c)) * step, at<float>(fx, 0x3328)));
    wrap_place(at<float>(fx, 0x332c), south + at<float>(fx, 0x332c));
    wrap_place(at<float>(fx, 0x3330), X87Reg(at<float>(fx, 0x3330)) - west);

    at<float>(fx, 0xa1bc) = (float)X87Reg(at<float>(cam, 0x20));
    at<float>(fx, 0xa1c0) = (float)X87Reg(at<float>(cam, 0x24));
    at<float>(fx, 0xa1c4) = (float)X87Reg(at<float>(cam, 0x28));
    at<u32>(fx, 0xa1c8) = 0;   // (the original's negated dot product of the camera's direction and this goes first, overwritten by +0x9d90, 0)

    const u32 ax = Bits((float)(x + at<float>(fx, 0x3328)));
    const u32 ay = Bits((float)(y + at<float>(fx, 0x332c)));
    const u32 az = Bits((float)(X87Reg(z) + at<float>(fx, 0x3330)));
    if (mask & 1) return g_boltCalls.Th(0x0057efd0, (u32)(uintptr_t)fx, ax, ay, az) ? 1 : 0;
    if (!(mask & 2)) return 1;
    return g_boltCalls.Th(0x0057f490, (u32)(uintptr_t)fx, ax, ay, az) ? 1 : 0;
}
}
