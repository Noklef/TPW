#include "partstate.h"

#include <cstdint>

#include <cmath>
#include <cstring>

#include "../core/globalmem.h"
#include "../math/matrix.h"
#include "../math/sintable.h"
#include "../math/x87.h"
#include "../motion/normals.h"
#include "../sprite/script.h"
#include "../world/things.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

u8 *model(const u8 *instance) { return at<u8 *>(instance, 4); }
PartStates *&states(u8 *instance) { return at<PartStates *>(instance, 0x28); }
u32 parts(const u8 *m) { return at<u16>(m, 0x48); }

// Face n of the model: the first +0x44 in one array, the rest in the other.
u8 *face(const u8 *m, u32 n) {
    const u32 first = at<u16>(m, 0x44);
    return n < first ? at<u8 *>(m, 0x70) + n * 0xa0 : at<u8 *>(m, 0x74) + (n - first) * 0x58;
}

const float kMinusHalfPi = -1.57079637f;  // 0x6fe2a4

struct Vec {
    float x, y, z;
};

// A vertex of a node's position blocks, through its remap table (as in normals.cpp).
Vec vertex(const u8 *node, u16 index) {
    const u32 slot = at<const u16 *>(node, 0x94)[index];
    const float *lane = reinterpret_cast<const float *>(at<const u8 *>(node, 0x60) + 0x30 * (slot >> 2)) + (slot & 3);
    return {X87_Through(lane[0]), X87_Through(lane[4]), X87_Through(lane[8])};
}

// p by m (stored by columns), in the original's order of operations.
Vec transform(const Vec &p, const float *m) {
    const X87Reg x = p.x, y = p.y, z = p.z;
    return {(float)(z * m[8] + y * m[4] + X87Reg(m[0]) * x + m[12]),
            (float)(y * m[5] + z * m[9] + x * m[1] + m[13]),
            (float)(x * m[2] + z * m[10] + y * m[6] + m[14])};
}

// The sine and cosine of angle from the game's table.
void sincos(const SinTable &sines, float angle, float *s, float *c) {
    *s = SinTable_At(sines, (float)(X87Reg(sines.scale) * angle));
    *c = SinTable_At(sines, (float)((X87Reg(angle) - kMinusHalfPi) * sines.scale));
}

}  // namespace

extern "C" {

void __cdecl PartStates_Create(u8 *instance) {
    if (!parts(model(instance))) return;
    PartStates *ps = static_cast<PartStates *>(GlobalMem_Alloc(0x14, 1));
    ps->entries = static_cast<u8 *>(GlobalMem_Alloc(parts(model(instance)) * 0x14, 1));
    states(instance) = ps;
    const u8 *part = at<const u8 *>(model(instance), 0x7c);
    u8 *entry = ps->entries;
    u32 animated = 0;
    for (u32 i = 0; i < parts(model(instance)); ++i, part += 0x14, entry += 0x14) {
        const u8 *m = model(instance);
        u8 *f = face(m, at<u16>(m, 0x46) + i);
        if (at<u8>(part, 0) & 0x30) {
            at<u32>(f, 0) |= 0x20000000;
            const u32 flags = at<u32>(entry, 0) | 1;
            at<s32>(entry, 0xc) = -1;
            at<u32>(entry, 0) = flags;
            if (at<u32>(f, 0) & 0x400) at<u32>(entry, 0) = flags | 0x10;
            if (at<u32>(part, 0) & 0x40040) at<u32>(at<u8 *>(f, 4), 0) |= 0x10000000;
        }
        if (at<u8>(part, 0) & 8) at<u32>(f, 0) |= 0x40;
        if (at<u32>(part, 0) & 0x10000) at<u32>(f, 0) |= 0x20000;
        if (at<u32>(f, 0) & 0x20000000) ++animated;
    }
    if (animated) {
        ps->flags |= 1;
        ps->slots = animated;
        ps->slotData = static_cast<u8 *>(GlobalMem_Alloc(animated << 6, 1));
    }
    PartStates_MarkNodes(instance, at<u8 *>(model(instance), 0x78), model(instance));
    u8 *slot = ps->slotData;
    entry = ps->entries;
    for (u32 n = parts(model(instance)); n; --n, entry += 0x14) {
        if (!(at<u32>(entry, 0) & 1)) continue;
        at<u8 *>(entry, 4) = slot;
        slot += 0x40;
    }
    part = at<const u8 *>(model(instance), 0x7c);
    for (u32 n = parts(model(instance)); n; --n, part += 0x14)
        if (at<u8>(part, 0) & 2) ps->flags |= 8;
}

u32 __cdecl PartStates_MarkNodes(u8 *instance, u8 *node, const u8 *model) {
    const u32 index = at<u16>(node, 0x50), first = at<u16>(model, 0x46);
    const bool part = index >= first && (s32)(index - first) < (s32)parts(model);
    u32 plain = part && (at<u8>(at<const u8 *>(model, 0x7c) + (index - first) * 0x14, 0) & 0x30) ? 0 : 1;
    if (part && !at<u8 *>(node, 0xc)) at<u32>(states(instance)->entries + (index - first) * 0x14, 0) |= 0x20;
    for (u8 *child = at<u8 *>(node, 0xc); child; child = at<u8 *>(child, 8))
        if (!PartStates_MarkNodes(instance, child, model)) plain = 0;
    if (plain) at<u32>(node, 0) |= 0x40000000;
    return plain;
}

void __cdecl PartStates_Free(u8 *instance) {
    if (!states(instance)) return;
    GlobalMem_Free(states(instance)->slotData);
    states(instance)->slotData = nullptr;
    GlobalMem_Free(states(instance)->entries);
    states(instance)->entries = nullptr;
    GlobalMem_Free(states(instance));
    states(instance) = nullptr;
}

void __cdecl Face_AttachPoint(const float *a, const float *b, const float *c, const float *normal,
                              const u8 *attach, float *onFace, float *out) {
    const float t = at<float>(attach, 4);
    const X87Reg k = X87Reg(1.0f) - t;
    out[0] = (float)(X87Reg(a[0]) * k + X87Reg(b[0]) * t);
    out[1] = (float)(X87Reg(b[1]) * t + X87Reg(a[1]) * k);
    out[2] = (float)(X87Reg(a[2]) * k + X87Reg(b[2]) * t);
    const float s = at<float>(attach, 8), m = (float)(X87Reg(1.0f) - s);
    const X87Reg c0 = X87Reg(c[0]) * s, x = X87Reg(out[0]) * m, y = X87Reg(m) * out[1], z = X87Reg(m) * out[2];
    out[0] = (float)(x + c0);
    out[1] = (float)(X87Reg(c[1]) * s + y);
    out[2] = (float)(X87Reg(c[2]) * s + z);
    std::memcpy(onFace, out, 12);
    const float lift = at<float>(attach, 0xc);
    for (int i = 0; i < 3; ++i) out[i] = (float)(X87Reg(normal[i]) * lift + out[i]);
}

void __cdecl Matrix_AttachAim(const float *from, const float *to, const float *up, const u8 *attach,
                              float *m) {
    const u8 *angles = attach - at<u16>(attach, 2);
    const X87Reg dx = X87Reg(to[0]) - from[0];
    float y = (float)(X87Reg(to[1]) - from[1]), z = (float)(X87Reg(to[2]) - from[2]);
    const float length = (float)X87_Round(std::sqrt((dx * dx + X87Reg(y) * y + X87Reg(z) * z).v));
    const float nx = (float)(dx / length);
    y = (float)(X87Reg(y) / length);
    z = (float)(X87Reg(z) / length);
    float x;
    if (std::isnan(nx)) {  // _isnan (0x67b410); sic: up, rotated
        std::memcpy(&y, &up[0], 4);
        std::memcpy(&z, &up[1], 4);
        x = X87_Through(up[2]);
    } else {
        x = nx;
    }
    m[0] = (float)(X87Reg(z) * up[1] - X87Reg(y) * up[2]);
    m[1] = (float)(X87Reg(x) * up[2] - X87Reg(up[0]) * z);
    m[2] = (float)(X87Reg(up[0]) * y - X87Reg(x) * up[1]);
    std::memcpy(&m[4], up, 12);
    m[8] = x;
    std::memcpy(&m[9], &y, 4);
    std::memcpy(&m[10], &z, 4);
    float r[16], s, c;
    Matrix_Identity(r);
    const SinTable sines = SinTable_Game();
    sincos(sines, at<float>(angles, 8), &s, &c);
    Matrix_RotateZ4(r, c, s);
    sincos(sines, at<float>(angles, 4), &s, &c);
    Matrix_RotateY4(r, c, s);
    sincos(sines, at<float>(angles, 0), &s, &c);
    Matrix_RotateX4(r, c, s);
    Matrix_PreMul3(r, m);
}

void __cdecl PartStates_UpdateNode(u8 *node, const u8 *model, Matrix44 m, const float *parent, u8 *entries) {
    if (at<u32>(node, 0) & 0x20000000) {
        const u32 offset = (at<u16>(node, 0x50) - at<u16>(model, 0x46)) * 0x14;
        const u8 *part = at<const u8 *>(model, 0x7c) + offset;
        const u32 entry = at<u32>(entries, offset);
        if ((entry & 0x20) && !(entry & 0xe)) goto children;
        Matrix_PreMul(reinterpret_cast<const float *>(node + 0x10), m.m);
        const u8 *host = at<const u8 *>(node, 4);
        if ((at<u32>(part, 0) & 0x40040) && (at<u32>(host, 0) & 0x200000)) {
            const u8 *attach = at<const u8 *>(part, 0xc);
            const u16 *f = at<const u16 *>(host, 0x70) + 4 * at<u16>(attach, 0);
            const Vec a = transform(vertex(host, f[1]), parent), b = transform(vertex(host, f[2]), parent),
                      c = transform(vertex(host, f[3]), parent);
            const float *n = reinterpret_cast<const float *>(at<const u8 *>(host, 0x64) + 12 * (f[0] & 0x7fff));
            const X87Reg n0 = n[0], n1 = n[1], n2 = n[2];
            const float normal[3] = {(float)(n2 * parent[8] + n1 * parent[4] + X87Reg(parent[0]) * n0),
                                     (float)(n2 * parent[9] + n1 * parent[5] + n0 * parent[1]),
                                     (float)(n0 * parent[2] + n1 * parent[6] + n2 * parent[10])};
            float onFace[3], point[3];
            Face_AttachPoint(&a.x, &b.x, &c.x, normal, attach, onFace, point);
            std::memcpy(&m.m[12], point, 12);
            if (at<u32>(part, 0) & 0x40000) Matrix_AttachAim(&a.x, onFace, normal, attach, m.m);
        }
        std::memcpy(at<u8 *>(entries, offset + 4), m.m, 0x40);
    } else {
        Matrix_PreMul(reinterpret_cast<const float *>(node + 0x10), m.m);
    }
children:
    if ((at<u32>(node, 0) & 0x10000000) && (at<u32>(node, 0) & 0x10000)) Node_UpdateNormals(node);
    for (u8 *child = at<u8 *>(node, 0xc); child; child = at<u8 *>(child, 8))
        if (!(at<u32>(child, 0) & 0x40000000)) PartStates_UpdateNode(child, model, m, m.m, entries);
}

void __cdecl PartStates_Update(u8 *instance) {
    if (!(at<u8>(instance, 0) & 1) || !(at<u8>(states(instance), 0) & 1)) return;
    Matrix44 m;
    Matrix_Identity(m.m);
    PartStates_UpdateNode(at<u8 *>(model(instance), 0x78), model(instance), m, m.m, states(instance)->entries);
}

s32 __cdecl PartStates_Activate(u8 *instance, u32 part, u32 handle) {
    PartStates *ps = states(instance);
    u8 *const entries = ps->entries;
    ps->flags |= 4;
    ++states(instance)->active;
    u8 *entry = entries + part * 0x14;
    at<u32>(entry, 0) |= 2;
    // (the original asserts that the thing table exists: Debug_Printf, left out)
    const u32 choice = handle ? Thing_SpriteChoice(*reinterpret_cast<u8 **>(0x007cfb90 + 20 * (handle & 0xffff))) : 0;
    const s32 slot = choice & 0x80000000 ? SpriteScript_Start(0x0074f558, 3, choice & 0x7fffffff, 0, 0, 0, 0)
                                         : SpriteScript_Start(0x0074f558, 1, choice, 0, 0, 0, 0);
    at<s32>(entry, 0xc) = slot;
    return slot;
}

void __cdecl PartStates_Release(u8 *instance, u32 part) {
    u8 *entry = states(instance)->entries + part * 0x14;
    --states(instance)->active;
    if (!states(instance)->active) states(instance)->flags &= ~4u;
    at<u32>(entry, 0) &= ~2u;
    SpriteScript_Kill(at<s32>(entry, 0xc));
    at<s32>(entry, 0xc) = -1;
}
}
