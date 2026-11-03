#include "weatherfx.h"

#include <cstdint>
#include <cstdlib>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../files/quickload.h"
#include "../gfx/texture.h"
#include "../gfx/texman.h"
#include "../math/x87.h"
#include "../terrain/landscape.h"
#include "batch.h"
#include "drawbuffer.h"
#include "ribbon.h"

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
