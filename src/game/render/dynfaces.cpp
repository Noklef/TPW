#include "dynfaces.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/misc.h"
#include "../math/sintable.h"
#include "../math/x87.h"
#include "../model/holder.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

constexpr u32 kFaces = 0x00820b00, kCount = 0x00871ee0, kTags = 0x00872168;

void Finish(u32 n, u32 tag) { 
    global<u32>(kTags + 4 * n) = (n << 5) | (tag & 0x1f);
    global<u32>(kCount) = n + 1;
}

// a + b on the x87, stored as a float.
float Sum(float a, float b) { return static_cast<float>(X87Reg(a) + X87Reg(b)); }

}  // namespace

extern "C" {

void __cdecl DynFaces_AddSquare(u32 a, u32 b, u32 c, u8 d, u32 tag, u32 e, u32 f, u32 g, u8 h, u32 i, u32 j) {
    const s32 n = global<s32>(kCount);
    if (n >= 0x806) return;  // (the original prints that there are too many)
    if (tag == 1 && global<u8>(0x00763c98) == 0) return;
    u8 *const face = reinterpret_cast<u8 *>(kFaces + n * 0xa0);
    at<u32>(face, 0) = a;
    at<u32>(face, 4) = b;
    at<u32>(face, 8) = c;
    at<u8>(face, 0xc) = d;
    at<u32>(face, 0x10) = e;
    at<u32>(face, 0x14) = f;
    at<u32>(face, 0x18) = g;
    at<u8>(face, 0x1c) = h;
    at<u8>(face, 0x9c) = (u8)tag;
    at<u32>(face, 0x94) = 1;
    at<u32>(face, 0x98) = global<u32>(0x00874fc4) == 0 ? i : 1;
    at<u32>(face, 0x90) = j;
    Finish((u32)n, tag);
}

void __cdecl DynFaces_AddPoly(const u32 *corners, u32 tag, const u32 *uvs, const u32 *colours, u32 user) {
    const s32 n = global<s32>(kCount);
    if (n >= 0x806) return;
    u8 *const face = reinterpret_cast<u8 *>(kFaces + n * 0xa0);
    for (u32 k = 0; k < 4; ++k) {
        at<u32>(face, 12 * k + 0) = corners[3 * k];
        at<u32>(face, 12 * k + 4) = corners[3 * k + 1];
        at<u32>(face, 12 * k + 8) = corners[3 * k + 2];
        at<u32>(face, 0x30 + 12 * k) = uvs[3 * k];
        at<u32>(face, 0x34 + 12 * k) = uvs[3 * k + 1];
        at<u32>(face, 0x60 + 12 * k) = colours[3 * k];
        at<u32>(face, 0x64 + 12 * k) = colours[3 * k + 1];
        at<u32>(face, 0x68 + 12 * k) = colours[3 * k + 2];
    }
    at<u8>(face, 0x9c) = (u8)tag;
    at<u32>(face, 0x94) = 0;
    at<u32>(face, 0x98) = 0;
    at<u32>(face, 0x90) = user;
    Finish((u32)n, tag);
}

void __cdecl DynFaces_SortTags(u32 *tags, u32 count) {
    u32 stack[50];  // the original's 50 dwords; slot 0 is never used (pushes go to sp + 1, sp + 2)
    u32 sp = 0, depth = 0;
    u32 ir = count, l = 1;
    for (;;) {
        if (ir - l < 7) {  // unsigned, as the original (a count of 0 goes to the partition)
            for (u32 j = l + 1; j <= ir; j++) {
                const u32 a = tags[j];
                u32 i = j - 1;
                while (i >= 1) {  // sic: down to 1, not l
                    if ((tags[i] & 0x1f) <= (a & 0x1f)) break;
                    tags[i + 1] = tags[i];
                    i--;
                }
                tags[i + 1] = a;
            }
            if (depth == 0) return;
            ir = stack[sp];
            l = stack[sp - 1];
            sp -= 2;
            depth -= 2;
            continue;
        }
        const u32 k = (l + ir) >> 1;
        u32 t = tags[k];
        tags[k] = tags[l + 1];
        tags[l + 1] = t;
        if ((tags[l] & 0x1f) > (tags[ir] & 0x1f)) { t = tags[l]; tags[l] = tags[ir]; tags[ir] = t; }
        if ((tags[l + 1] & 0x1f) > (tags[ir] & 0x1f)) { t = tags[l + 1]; tags[l + 1] = tags[ir]; tags[ir] = t; }
        if ((tags[l] & 0x1f) > (tags[l + 1] & 0x1f)) { t = tags[l]; tags[l] = tags[l + 1]; tags[l + 1] = t; }
        u32 i = l + 1, j = ir;
        const u32 a = tags[l + 1], key = a & 0x1f;
        for (;;) {
            do i++; while ((tags[i] & 0x1f) < key);
            do j--; while ((tags[j] & 0x1f) > key);
            if (j < i) break;
            t = tags[i]; tags[i] = tags[j]; tags[j] = t;
        }
        tags[l + 1] = tags[j];
        tags[j] = a;
        depth += 2;
        sp += 2;
        if (ir - i + 1 >= j - l) {
            stack[sp] = ir;
            stack[sp - 1] = i;
            ir = j - 1;
        } else {
            stack[sp] = j - 1;
            stack[sp - 1] = l;
            l = i;
        }
    }
}

void __cdecl DynFaces_AddGrid(float dx, float dy, float dz, s32 rows, s32 cols, s32 row, s32 col) {
    u32 uvs[12] = {0, 0, 0, 0x3f800000, 0, 0, 0, 0x3f800000, 0, 0x3f800000, 0x3f800000, 0};  // (0, 0) (1, 0) (0, 1) (1, 1), stride 12
    u32 face = 0;
    for (s32 i = 0; i < cols; i++) {
        if (rows <= 0) continue;
        u32 offset = static_cast<u32>(i + row * 19 + col) * 12;
        for (s32 j = rows; j != 0; j--, offset += 0xe4) {
            const float *const p0 = reinterpret_cast<const float *>((uintptr_t)(0x00874180 + offset));
            const float *const p1 = p0 + 3, *const p2 = p0 + 57, *const p3 = p0 + 60;
            const float corners[12] = {Sum(p0[0], dx), Sum(p0[1], dy), Sum(dz, p0[2]), Sum(p2[0], dx), Sum(p2[1], dy), Sum(p2[2], dz),
                                       Sum(p1[0], dx), Sum(p1[1], dy), Sum(p1[2], dz), Sum(dx, p3[0]), Sum(p3[1], dy), Sum(p3[2], dz)};
            const u32 *const c0 = reinterpret_cast<const u32 *>((uintptr_t)(0x00870ed8 + offset));
            const u32 *const order[4] = {c0, c0 + 57, c0 + 3, c0 + 60};
            u32 colours[12];
            for (int k = 0; k < 12; k++) colours[k] = X87_FloatThrough(order[k / 3][k % 3]);
            const u32 tag = global<u32>(0x00871d18 + 4 * face++);
            DynFaces_AddPoly(reinterpret_cast<const u32 *>(corners), tag, uvs, colours, 1);
        }
    }
}

void __cdecl DynFaces_WaveGrid(s32 rows, s32 cols, float step, float phase) {
    const SinTable sines = SinTable_Game();
    const float k = static_cast<float>(X87Reg(6.28318501f) * 0.333333343f);
    const float a = static_cast<float>(X87Reg(0.0174532998f) * 10.0f);
    const float sinA = SinTable_At(sines, static_cast<float>(X87Reg(sines.scale) * a));
    const float cosA = SinTable_At(sines, static_cast<float>((X87Reg(a) - -1.57079637f) * sines.scale));
    const float alongX = static_cast<float>(X87Reg(cosA) * k);
    const float alongZ = static_cast<float>(-(X87Reg(k) * sinA).v);
    if (cols <= 0) return;
    float z = 0;
    u32 column = 0;
    for (s32 c = cols; c != 0; c--, column += 0xc) {
        if (rows > 0) {
            float x = 0;
            const float base = static_cast<float>(X87Reg(z) * alongZ);
            const float half = static_cast<float>(X87Reg(phase) * 0.5f);
            u32 offset = column;
            for (s32 r = rows; r != 0; r--, offset += 0xe4) {
                float *const p = reinterpret_cast<float *>((uintptr_t)(0x00874180 + offset));
                float *const n = reinterpret_cast<float *>((uintptr_t)(0x00870ed8 + offset));
                const float h = static_cast<float>(X87Reg(x) * alongX + base - half);
                p[0] = x;
                p[1] = static_cast<float>(X87Reg(SinTable_At(sines, static_cast<float>((X87Reg(h) - -1.57079637f) * sines.scale))) * 1.70000005f);
                p[2] = z;
                const float t = X87_Through(SinTable_At(sines, static_cast<float>(X87Reg(sines.scale) * h)));
                n[1] = 1.0f;
                n[0] = t;
                n[2] = t;
                const X87Reg nx = n[0], ny = n[1], nz = n[2];
                const X87Reg length = X87_Sqrt(nx * nx + ny * ny + nz * nz);
                n[0] = static_cast<float>(X87Reg(n[0]) / length);
                n[1] = static_cast<float>(X87Reg(n[1]) / length);
                n[2] = static_cast<float>(X87Reg(n[2]) / length);
                x = static_cast<float>(X87Reg(x) + step);
            }
        }
        z = static_cast<float>(X87Reg(z) + step);
    }
}

void __cdecl DynFaces_RippleGrid(s32 rows, s32 cols, float step, float phase) {
    const SinTable sines = SinTable_Game();
    const float k = static_cast<float>(X87Reg(6.28318501f) * 0.333333343f);
    float z = 0;
    const float depth = static_cast<float>(X87Reg(X87_MulIntDouble(cols, step)));
    if (cols <= 0) return;
    u32 column = 0;
    for (s32 c = cols; c != 0; c--, column += 0xc) {
        if (rows > 0) {
            float x = 0;
            const X87Reg dz = X87Reg(z) - depth;
            const float dz2 = static_cast<float>(dz * dz);
            const float lift = static_cast<float>(X87Reg(phase) * 0.600000024f);
            const float reach = static_cast<float>(X87Reg(X87_MulIntDouble(rows - 4, step)));
            u32 offset = column;
            for (s32 r = rows; r != 0; r--, offset += 0xe4) {
                float *const p = reinterpret_cast<float *>((uintptr_t)(0x00874180 + offset));
                float *const n = reinterpret_cast<float *>((uintptr_t)(0x00870ed8 + offset));
                const X87Reg d = X87_Sqrt(X87Reg(x) * x + dz2) - lift;
                const float angle = static_cast<float>(X87Reg(k) * d);
                const float quarter = static_cast<float>(X87Reg(6.28318501f) * d * 0.25f);
                const X87Reg q = X87Reg(x) / reach;
                const float fall = q.v >= 1.0 ? 1.0f : static_cast<float>(q);  // (a NaN is kept)
                const float across = static_cast<float>(
                    X87Reg(SinTable_At(sines, static_cast<float>((X87Reg(angle) - -1.57079637f) * sines.scale))) * fall * 0.300000012f);
                const float up = SinTable_At(sines, static_cast<float>((X87Reg(quarter) - -1.57079637f) * sines.scale));
                p[0] = x;
                p[1] = static_cast<float>(X87Reg(up) * (X87Reg(fall) + fall) * 0.300000012f + z - X87Reg(x) * x * 0.0199999996f);
                p[2] = across;
                n[2] = 1.0f;
                const float t = static_cast<float>(X87Reg(SinTable_At(sines, static_cast<float>(X87Reg(sines.scale) * angle))) * fall);
                n[0] = t;
                n[1] = t;
                const X87Reg nx = n[0], ny = n[1], nz = n[2];
                const X87Reg length = X87_Sqrt(nx * nx + ny * ny + nz * nz);
                n[0] = static_cast<float>(X87Reg(n[0]) / length);
                n[1] = static_cast<float>(X87Reg(n[1]) / length);
                n[2] = static_cast<float>(X87Reg(n[2]) / length);
                x = static_cast<float>(X87Reg(x) + step);
            }
        }
        z = static_cast<float>(X87Reg(z) - step);
    }
}

void __cdecl DynFaces_ReleaseModels() {
    u32 &flag = *reinterpret_cast<u32 *>((uintptr_t)0x00874fcc);
    if (flag != 0) {
        static const u32 kModels[3] = {0x00871f64, 0x00871ee4, 0x00871ed8};
        for (u32 address : kModels) {
            u8 *const object = *reinterpret_cast<u8 *const *>((uintptr_t)address);
            if (object == nullptr) continue;
            Animator_Sleep(object + 0x54);
            Animator_Dispose(object + 0x54);
            GameHeap_Free(object);
        }
        Dword_ClearBit0(reinterpret_cast<u32 *>((uintptr_t)0x00763c78));
    }
    flag = 0;
}

}  // extern "C"
