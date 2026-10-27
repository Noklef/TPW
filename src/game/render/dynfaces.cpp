#include "dynfaces.h"

#include <cstdint>
#include <cstring>
#include <xmmintrin.h>

#include "../core/gameheap.h"
#include "../math/sintable.h"
#include "../math/x87.h"
#include "../model/holder.h"
#include "../core/gameclock.h"
#include "../core/thunks.h"
#include "../world/world.h"
#include "vertexblock.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

constexpr u32 kFaces = 0x00820b00, kCount = 0x00871ee0, kTags = 0x00872168;

// fld m32; fistp m32: rounded to nearest even, 0x80000000 when out of range (as cvtss2si does).
s32 Fistp(float f) { return _mm_cvtss_si32(_mm_set_ss(f)); }

// __ftol (0x67a830): truncated through a qword, its low dword.
s32 Ftol(X87Reg x) { return (s32)(long long)x.v; }

float of(u32 b) {
    float f;
    std::memcpy(&f, &b, 4);
    return f;
}

void Finish(u32 n, u32 tag) { 
    global<u32>(kTags + 4 * n) = (n << 5) | (tag & 0x1f);
    global<u32>(kCount) = n + 1;
}

// The memory forms (fadd / fsub / fsubr m32) with m read through a reference as bits, so that a signalling NaN m reaches the x87's
// rule intact (a float passed or returned by value can be quieted on the way): a signalling NaN m gives way to a NaN st.
bool GivesWay(X87Reg st, const float &m) {
    u32 b;
    std::memcpy(&b, &m, 4);
    return st.v != st.v && (b & 0x7f800000) == 0x7f800000 && (b & 0x7fffff) && !(b & 0x400000);
}
X87Reg AddM(X87Reg st, const float &m) { return GivesWay(st, m) ? st : st + X87Reg(m); }
X87Reg SubM(X87Reg st, const float &m) { return GivesWay(st, m) ? st : st - X87Reg(m); }
X87Reg SubRM(X87Reg st, const float &m) { return GivesWay(st, m) ? st : X87Reg(m) - st; }

// a + m on the x87 (fld a; fadd m: m the memory operand), stored as a float.
float Sum(float a, const float &m) { return static_cast<float>(AddM(a, m)); }

// A dword of a vertex array as the float memory operand it is.
const float &F(const u32 &bits) { return *reinterpret_cast<const float *>(&bits); }

float AsFloat(u32 bits) {
    float f;
    std::memcpy(&f, &bits, 4);
    return f;
}
u32 Bits(float f) {
    u32 b;
    std::memcpy(&b, &f, 4);
    return b;
}

// The four vertices' x and z (12 bytes apart, y kept) turned by sine s and cosine c: z' = z c + s x, x' = c x - z s. The products' operand
// order differs between 0x53cda0's loops: 0 (the first face's corners) z c + s x, c x - z s; 1 (the second's) z c + x s, x c - z s; 2 (the
// colours) z c + x s, x c - s z.
void Turn(u32 *v, float s, float c, int order) {
    for (int k = 0; k < 4; ++k) {
        const X87Reg x = AsFloat(v[3 * k]), z = AsFloat(v[3 * k + 2]);
        const X87Reg zc = z * X87Reg(c), sx = order == 0 ? X87Reg(s) * x : x * X87Reg(s);
        v[3 * k + 2] = Bits(static_cast<float>(zc + sx));
        const X87Reg cx = order == 0 ? X87Reg(c) * x : x * X87Reg(c), zs = order == 2 ? X87Reg(s) * z : z * X87Reg(s);
        v[3 * k] = Bits(static_cast<float>(cx - zs));
    }
}

// The sine table's entries for angle and for a (angle + pi / 2, as a float): scale * angle stored as a float, then fistp'd.
void SinCos(float angle, float a, float &s, float &c) {
    const SinTable sines = SinTable_Game();
    s = SinTable_At(sines, static_cast<float>(X87Reg(sines.scale) * angle));
    c = SinTable_At(sines, static_cast<float>(X87Reg(sines.scale) * a));
}

// dx, dy, dz added to the four vertices (dx + x, on the x87).
void Move(u32 *v, float dx, float dy, float dz) {
    for (int k = 0; k < 4; ++k) {
        v[3 * k] = Bits(Sum(dx, F(v[3 * k])));
        v[3 * k + 1] = Bits(Sum(dy, F(v[3 * k + 1])));
        v[3 * k + 2] = Bits(Sum(dz, F(v[3 * k + 2])));
    }
}

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

void __cdecl DynFaces_AddTurnedGrid(float dx, float dy, float dz, float xoff, s32 rows, s32 cols, u32 tag, float angle) {
    const float du = static_cast<float>(X87Reg(1.0f) / X87Reg(static_cast<double>(rows)));
    const float dv = static_cast<float>(X87Reg(0.550000012f) / X87Reg(static_cast<double>(cols)));
    float v = tag == 0x13 ? 0.550000012f : 1.0f;
    for (s32 i = 0; i < cols; ++i) {
        if (rows > 0) {
            const float a = static_cast<float>(X87Reg(angle) - X87Reg(-1.57079637f));
            u32 u = 0;
            u32 offset = static_cast<u32>(i) * 12;
            for (s32 j = rows; j != 0; --j, offset += 0xe4) {
                const u32 *const p = reinterpret_cast<const u32 *>((uintptr_t)(0x00874180 + offset));
                const u32 *const n = reinterpret_cast<const u32 *>((uintptr_t)(0x00870ed8 + offset));
                // The first face: below, here, below-right, right.
                const u32 at1[4] = {57, 0, 60, 3};
                u32 corners[12], colours[12], uvs[12] = {};
                float s, c;
                for (int k = 0; k < 4; ++k) {
                    corners[3 * k] = Bits(Sum(xoff, F(p[at1[k]])));
                    corners[3 * k + 1] = p[at1[k] + 1];
                    corners[3 * k + 2] = p[at1[k] + 2];
                }
                SinCos(angle, a, s, c);
                Turn(corners, s, c, 0);
                for (int k = 0; k < 12; ++k) colours[k] = X87_FloatThrough(n[at1[k / 3] + k % 3]) ^ 0x80000000u;
                SinCos(angle, a, s, c);
                Turn(colours, s, c, 2);
                Move(corners, dx, dy, dz);
                const u32 u1 = Bits(static_cast<float>(X87Reg(AsFloat(u)) + X87Reg(du)));
                const u32 v1 = Bits(v), v2 = Bits(static_cast<float>(X87Reg(v) - X87Reg(dv)));
                const u32 uv1[8] = {u1, v1, u, v1, u1, v2, u, v2};
                for (int k = 0; k < 4; ++k) uvs[3 * k] = uv1[2 * k], uvs[3 * k + 1] = uv1[2 * k + 1];
                DynFaces_AddPoly(corners, tag, uvs, colours, 3);
                // The second: here, below, right, below-right.
                const u32 at2[4] = {0, 57, 3, 60};
                for (int k = 0; k < 4; ++k) {
                    corners[3 * k] = Bits(Sum(AsFloat(p[at2[k]]), xoff));
                    corners[3 * k + 1] = p[at2[k] + 1];
                    corners[3 * k + 2] = p[at2[k] + 2];
                }
                SinCos(angle, a, s, c);
                Turn(corners, s, c, 1);
                for (int k = 0; k < 12; ++k) colours[k] = n[at2[k / 3] + k % 3];
                SinCos(angle, a, s, c);
                Turn(colours, s, c, 2);
                Move(corners, dx, dy, dz);
                const u32 uv2[8] = {u, v1, u1, v1, u, v2, u1, v2};
                for (int k = 0; k < 4; ++k) uvs[3 * k] = uv2[2 * k], uvs[3 * k + 1] = uv2[2 * k + 1];
                DynFaces_AddPoly(corners, tag, uvs, colours, 3);
                u = u1;
            }
        }
        v = static_cast<float>(X87Reg(v) - X87Reg(dv));
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

void __cdecl DynFaces_Frame() {
    const float moveX = (float)SubM(X87_MulDouble((double)global<s32>(0x008186b8), global<float>(0x00700de0)), global<float>(0x00820afc));
    const u32 t = global<u32>(0x00790a38);
    static const u32 kConstants[][2] = {{0x871ee8, 0}, {0x871eec, 0}, {0x871ef4, 0x3f800000}, {0x871ef8, 0}, {0x871f00, 0},
                                        {0x871f04, 0x3f800000}, {0x871f0c, 0x3f800000}, {0x871f10, 0x3f800000}, {0x871f18, 0x3f000000},
                                        {0x871f1c, 0x3f000000}, {0x871f24, 0xbecccccd}, {0x871f28, 0xbecccccd}, {0x871f30, 0x3ecccccd},
                                        {0x871f34, 0xbecccccd}, {0x871f3c, 0xbecccccd}, {0x871f40, 0x3ecccccd}, {0x871f48, 0x3ecccccd},
                                        {0x871f4c, 0x3ecccccd}};
    for (const auto &c : kConstants)
        global<u32>(c[0]) = c[1];
    const float moveZ = (float)SubM(X87_MulDouble((double)global<s32>(0x008186b4), global<float>(0x00700de0)), global<float>(0x00820af8));
    const float scale = global<float>(0x007b095c);
    const u32 mask = global<u32>(0x007b0960);
    const s32 i1 = Fistp((float)(X87Reg(scale) * of(t)));
    const float s = global<float>(0x007b0968 + 4 * (mask & (u32)i1));
    const s32 i2 = Fistp((float)((X87Reg(of(t)) - global<float>(0x00700df0)) * scale));
    const X87Reg c = global<float>(0x007b0968 + 4 * (mask & (u32)i2));
    for (u32 k = 0; k < 4; ++k) {
        float *const p = &global<float>(0x00871f24 + 0xc * k);
        const X87Reg y = p[1];
        p[1] = (float)(y * c + X87Reg(s) * p[0]);
        p[0] = (float)(c * p[0] - X87Reg(s) * y);
    }
    const float wavePhase = global<float>(0x00871f60);
    global<u32>(0x00871f54) = 0x3f000000;
    global<u32>(0x00871f58) = 0x3f000000;
    for (u32 k = 0; k < 4; ++k) {
        float *const p = &global<float>(0x00871f24 + 0xc * k);
        p[0] = (float)(X87Reg(p[0]) - global<float>(0x00700df4));
        p[1] = (float)(X87Reg(p[1]) - global<float>(0x00700df4));
    }
    global<float>(0x00820afc) = (float)AddM(X87Reg(moveX) * global<float>(0x00700de4), global<float>(0x00820afc));
    global<float>(0x00820af8) = (float)AddM(X87Reg(moveZ) * global<float>(0x00700de4), global<float>(0x00820af8));
    DynFaces_WaveGrid(0x10, 0x13, 20.0f, wavePhase);
    DynFaces_AddGrid(global<float>(0x00870ec8), global<float>(0x00870ecc), global<float>(0x00870ed0), 10, 11, 0, 0);
    DynFaces_AddGrid(global<float>(0x00870ec8), global<float>(0x00870ecc), global<float>(0x00870ed0), 4, 6, 10, 0);
    DynFaces_RippleGrid(7, 5, of(0x3fe66666), global<float>(0x00871ed0));   // 1.8
    u32 tag = 0x12;
    for (u32 e = 0x00763cf8; e < 0x00763d78; e += 0x10) {
        const u8 *const record = reinterpret_cast<const u8 *>(GameTable7cfb90_Get1da724_519590(global<u8 *>(0x007cf83c)));
        const float angle = (float)(X87Reg(at<float>(record, 0x5c)) * global<float>(0x00700dd0));
        DynFaces_AddTurnedGrid(global<float>(e - 4), global<float>(e), global<float>(e + 4), 0.5f, 6, 4, tag, angle);
        tag = tag == 0x12 ? 0x13 : 0x12;
    }
    DynFaces_SortTags(&global<u32>(kTags - 4), global<u32>(kCount));
    VertexBlock_TakeFaces(global<u8 *>(0x00871f64));
    VertexBlock_TakeFaces(global<u8 *>(0x00871ee4));
    VertexBlock_TakeFaces(global<u8 *>(0x00871ed8));
    global<u32>(kCount) = 0;
    u8 *const clock = reinterpret_cast<u8 *>(0x00785970);
    if (Wrap_Obj_Get30_402df0(clock))
        return;
    global<float>(0x00874fc0) = (float)(X87Reg(global<float>(0x00874fc0)) - global<float>(0x00700df8));
    global<float>(0x00871f60) = (float)SubRM(X87_MulDouble(GameClock_Double20(clock), global<float>(0x00700de8)), global<float>(0x00871f60));
    global<float>(0x00871ed0) = (float)SubRM(X87_MulDouble(GameClock_Double20(clock), global<float>(0x00700de8)), global<float>(0x00871ed0));
    const s32 count = (s32)(global<u32>(0x00874fc8) + (u32)Ftol(GameClock_Double20(clock)));
    global<s32>(0x00874fc8) = count;
    if (global<u8>(0x00763c98) != 0) {
        if (count > 6) {
            global<u8>(0x00763c98) = 0;
            global<s32>(0x00874fc8) = 0;
        }
    } else if (count > 1) {
        global<u8>(0x00763c98) = 1;
        global<s32>(0x00874fc8) = 0;
    }
}

}  // extern "C"
