#include "meshlist.h"

#include <cstdint>
#include <cstring>
#include <xmmintrin.h>

#include "../../hook/regthunks.h"
#include "../core/gameheap.h"
#include "../math/mat4.h"
#include "../math/x87.h"
#include "engineerrors.h"
#include "../frontend/lobbysky.h"
#include "fixedverts.h"
#include "../core/staticobjects.h"

namespace {

__declspec(naked) double __cdecl GamePow(double, double) {
    __asm {
        fld qword ptr [esp + 4]
        fld qword ptr [esp + 12]
        call RegThunk_CRT_CIpow
        ret
    }
}

double(__cdecl *g_pow)(double, double) = &GamePow;

const float kPi = 3.14159274f, kHalf = 0.5f;   // 0x7016b8, 0x7016cc
const u32 kStepBits = 0xbc010204;              // 0x7016d8

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u32 bits(float f) {
    u32 b;
    std::memcpy(&b, &f, 4);
    return b;
}

float of(u32 b) {
    float f;
    std::memcpy(&f, &b, 4);
    return f;
}

// fld m32; fistp [0xfb34a4]: rounded to nearest even, 0x80000000 when out of range (as cvtss2si does), kept in the dword.
s32 Fistp(float f) {
    const s32 n = _mm_cvtss_si32(_mm_set_ss(f));
    gFixedVerts_fb34a4 = n;
    return n;
}

// The light Meshes_LightAt summed as a colour: 0x879db4-0x879dbc * [0x7016e4] rounded, held to 0-0xff, alpha 0xff, masked.
u32 LitColour(u32 mask) {
    s32 c[3];
    for (u32 k = 0; k < 3; ++k) {
        c[k] = Fistp((float)(X87Reg(global<float>(0x00879db4 + 4 * k)) * global<float>(0x007016e4)));
        if (c[k] < 0)
            c[k] = 0;
        if (c[k] > 0xff)
            c[k] = 0xff;
    }
    return (0xff000000u | (u32)c[0] << 16 | (u32)c[1] << 8 | (u32)c[2]) & mask;
}

// The colour (+4-+0x10) of a light times the factor added into 0x879db0-0x879dbc.
void AddLight(const u8 *light, X87Reg factor) {
    for (u32 k = 0; k < 4; ++k)
        global<float>(0x00879db0 + 4 * k) = (float)(X87Reg(at<float>(light, 4 + 4 * k)) * factor + global<float>(0x00879db0 + 4 * k));
}

}  // namespace

void Meshes_SetPow(double(__cdecl *pow)(double, double)) { g_pow = pow ? pow : &GamePow; }

extern "C" {

u32 __cdecl Meshes_CreateObject() {
    if (reinterpret_cast<u32 &>(gMeshlist_87a248)) return 0;
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x8244));
    if (block) {
        at<u32>(block, 0) = 0;
        at<u32>(block, 0x28) = 0;
        at<u32>(block, 0x2c) = 0xffffffff;
        gLobbysky_879dd8[0] = 0x3f800000;
        gLobbysky_879dd8[1] = 0x3e19999a;
        gLobbysky_879dd8[2] = 0x3e19999a;
        gLobbysky_879dd8[3] = 0x3e19999a;
        float x, kStep;
        const u32 start = 0xb1800000;
        std::memcpy(&x, &start, 4);
        std::memcpy(&kStep, &kStepBits, 4);
        for (u32 i = 0; i < 0x80; ++i) {
            const double sine = X87_Sin((X87Reg(x) * kPi * kHalf).v);
            at<float>(block, 0x40 + 4 * i) = (float)X87Reg(g_pow(sine, 4.0));
            x = (float)(X87Reg(x) - kStep);
        }
        at<u32>(block, 4) = 0;
    }
    gMeshlist_87a248 = block;
    if (block) return 1;
    EngineErrors_Push(gEngineErrors_879ad0, nullptr, 3, 0xb0754000);
    return 0;
}

u32 __fastcall Meshes_Ready(u8 *object, void *, const u32 *list) {
    for (; *list; list++) {
        u8 *mesh = reinterpret_cast<u8 *>(*list);
        if (at<u32>(mesh, 0x50) & 0x20)
            at<u32>(object, 0x30) = (u32)(uintptr_t)mesh + 0x38;
        if (!(at<u32>(mesh, 0x50) & 0x10000000)) {
            at<u32>(mesh, 0x58) = *reinterpret_cast<u32 *>((u32)(uintptr_t)&gMeshlist_879dec);
            *reinterpret_cast<u32 *>((u32)(uintptr_t)&gMeshlist_879dec) = (u32)(uintptr_t)mesh;
        }
    }
    return 1;
}

void __fastcall Meshes_SetDirection(u8 *object, void *, u32 x, u32 y, u32 z) {
    const float fx = *reinterpret_cast<const float *>(&x), fy = *reinterpret_cast<const float *>(&y);
    const float fz = *reinterpret_cast<const float *>(&z);
    const X87Reg len = X87_Sqrt((X87Reg(fx) * fx + X87Reg(fy) * fy) + X87Reg(fz) * fz);
    if (!(len.v == 0.0f || len.v != len.v)) {  // 0x7016dc
        const X87Reg scale = X87_DivR(len, 1.0f);  // 0x7016c0
        at<float>(object, 0x34) = (float)(scale * fx);
        at<float>(object, 0x38) = (float)(scale * fy);
        at<float>(object, 0x3c) = (float)(scale * fz);
        return;
    }
    at<u32>(object, 0x34) = x;
    at<u32>(object, 0x38) = y;
    at<u32>(object, 0x3c) = z;
}

u32 __fastcall Meshes_SetView(u8 *object, void *, const u32 *matrix, u32) {
    u8 *mesh = *reinterpret_cast<u8 **>((uintptr_t)&gMeshlist_879dec);
    *reinterpret_cast<const u32 **>((uintptr_t)0x00879dcc) = matrix;
    u32 *const view = reinterpret_cast<u32 *>((uintptr_t)0x00879df0);
    for (int i = 0; i < 16; ++i) view[i] = matrix[i];
    Mat4_Invert(view);
    const float *const m = reinterpret_cast<const float *>(view);
    for (; mesh; mesh = *reinterpret_cast<u8 **>(mesh + 0x58)) {
        if (mesh[0x50] & 2) {
            float *const d = reinterpret_cast<float *>(mesh + 0x38);
            Mat4_Transform(m, nullptr, d, reinterpret_cast<const float *>(mesh + 0x2c), 3);
            const X87Reg x = d[0], y = d[1], z = d[2];
            const float len = static_cast<float>((y * y + z * z) + x * x);
            if (!(len != len || len == 0.0f || len == 1.0f)) {
                const X87Reg inv = X87Reg(1.0f) / X87_Sqrt(X87Reg(len));
                d[0] = static_cast<float>(X87Reg(d[0]) * inv);
                d[1] = static_cast<float>(inv * X87Reg(d[1]));
                d[2] = static_cast<float>(inv * X87Reg(d[2]));
            }
        }
        if (mesh[0x50] & 4)
            Mat4_Transform(m, nullptr, reinterpret_cast<float *>(mesh + 0x20), reinterpret_cast<const float *>(mesh + 0x14), 2);
    }
    Mat4_Transform(m, nullptr, reinterpret_cast<float *>(object + 0x34), reinterpret_cast<const float *>(object + 0x34), 3);
    return 1;
}

void __fastcall Meshes_LightAt(u32 seed, void *, const float *normal, float x, float y, float z) {
    for (u32 k = 0; k < 0x10; k += 4)
        global<u32>(0x00879db0 + k) = gLobbysky_879dd8[k / 4];
    u32 spill = seed;   // (the stack slot of the ECX pushed for room: d below)
    for (const u8 *light = global<u8 *>((u32)(uintptr_t)&gMeshlist_879dec); light; light = at<u8 *>(light, 0x58)) {
        const u32 kind = at<u32>(light, 0x50) & 0xf;
        if (kind == 2) {
            const X87Reg a = X87Reg(at<float>(light, 0x38)) * normal[0];
            const X87Reg b = X87Reg(at<float>(light, 0x3c)) * normal[1];
            const X87Reg c = X87Reg(normal[2]) * at<float>(light, 0x40);
            const float f = (float)((X87Reg(-a.v) - b) - c);
            if (!(bits(f) & 0x80000000))
                AddLight(light, f);
        } else if (kind == 4) {
            const X87Reg dx = X87Reg(at<float>(light, 0x20)) - x;
            const X87Reg dy = X87Reg(at<float>(light, 0x24)) - y;
            const X87Reg dz = X87Reg(at<float>(light, 0x28)) - z;
            const X87Reg dxx = dx * dx;
            const float fy = (float)dy;
            const X87Reg dyy = dy * fy;
            spill = bits((float)dxx);
            const float fz = (float)dz;
            const X87Reg sum = dyy + of(spill);
            const X87Reg dzz = X87Reg(fz) * fz;
            spill = bits((float)sum);
            spill = bits((float)(dzz + of(spill)));
            if ((s32)spill < at<s32>(light, 0x48))
                AddLight(light, (X87Reg(at<float>(light, 0x48)) - of(spill)) * at<float>(light, 0x4c));
        } else if (kind == 6) {
            const X87Reg dy = X87Reg(at<float>(light, 0x24)) - y;
            const X87Reg dz = X87Reg(at<float>(light, 0x28)) - z;
            const X87Reg dx = X87Reg(at<float>(light, 0x20)) - x;
            const float f = (float)((dy * normal[1] + dz * normal[2]) + dx * normal[0]);
            if (!(bits(f) & 0x80000000))
                AddLight(light, (X87Reg(at<float>(light, 0x48)) - of(spill)) * at<float>(light, 0x4c) * f);
        }
    }
}
void __fastcall Meshes_LightPoints(u8 *object, void *, u32 *colours, const float *normals, const float *points, u32 count) {
    if (global<u32>((u32)(uintptr_t)&gView_8bcbc8) & 0x800) {
        for (u32 i = 0; i < count; ++i)
            colours[i] = at<u32>(object, 0x2c);
        return;
    }
    for (u32 i = 0; i < count; ++i) {
        Meshes_LightAt((u32)(uintptr_t)object, nullptr, normals + 3 * i, points[4 * i], points[4 * i + 1], points[4 * i + 2]);
        colours[i] = LitColour(at<u32>(object, 0x2c));
    }
}

void __fastcall Meshes_LightMesh(u8 *object, void *, u32 *colours, const u8 *mesh) {
    const u32 count = at<u16>(mesh, 0x5e);
    if (global<u32>((u32)(uintptr_t)&gView_8bcbc8) & 0x800) {
        for (u32 i = 0; i < count; ++i)
            colours[i] = at<u32>(object, 0x2c);
        return;
    }
    const u8 *const positions = at<u8 *>(mesh, 0x60);
    const float *const normals = at<float *>(mesh, 0x64);
    for (u32 i = 0; i < count; ++i) {
        const u32 v = at<u16>(at<u8 *>(mesh, 0x94), 2 * i);
        const float *const p = reinterpret_cast<const float *>(positions + 0x30 * (v >> 2)) + (v & 3);
        Meshes_LightAt((u32)(uintptr_t)object, nullptr, normals + 3 * i, p[0], p[4], p[8]);
        colours[i] = LitColour(at<u32>(object, 0x2c));
    }
}
}
