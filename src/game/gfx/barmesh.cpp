#include "barmesh.h"

#include <cstdint>
#include <cstring>

#include "../core/gameheap.h"
#include "../core/misc.h"
#include "../math/x87.h"
#include "../model/holder.h"
#include "../ui/guidialog.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u32 bitsOf(float f) {
    u32 b;
    std::memcpy(&b, &f, 4);
    return b;
}

constexpr u32 kHundred = 0x42c80000;  // 100.0f
constexpr u32 kOne = 0x3f800000;
constexpr u32 kHalf = 0x3f000000;

void Mesh_Dtor(u8 *mesh, u32 vtable) {
    at<u32>(mesh, 0) = vtable;
    if (at<u32>(mesh, 4)) {
        Dword_ClearBit0(&at<u32>(mesh, 0xbc));
        Animator_Dispose(mesh + 0x6c);
    }
    GuiDialog_Dtor(mesh);
}

u8 *Mesh_ScalarDtor(u8 *mesh, u32 vtable, u32 flags) {
    Mesh_Dtor(mesh, vtable);
    if (flags & 1) GameHeap_Free(mesh);
    return mesh;
}

}  // namespace

extern "C" {

void __cdecl BarMesh_SetVertex(float *verts, u32 i, float x, float y, float z) {
    u8 *v = reinterpret_cast<u8 *>(verts) + (i >> 2) * 0x30 + (i & 3) * 4;
    at<u32>(v, 0) = X87_FloatThrough(bitsOf(x));
    at<u32>(v, 0x10) = X87_FloatThrough(bitsOf(y));
    at<u32>(v, 0x20) = X87_FloatThrough(bitsOf(z));
}

void __cdecl BarMesh_SetTexCoord(float *coords, u32 i, float u, float v) {
    u8 *t = reinterpret_cast<u8 *>(coords) + (i >> 2) * 0x20 + (i & 3) * 4;
    at<u32>(t, 0) = X87_FloatThrough(bitsOf(u));
    at<u32>(t, 0x10) = X87_FloatThrough(bitsOf(v));
}

void __fastcall BarMesh_SetTopVertices(const u8 *bar) {
    u8 *v = at<u8 *>(bar, 0x7c);
    u32 level = X87_FloatThrough(at<u32>(bar, 0x120));
    at<u32>(v, 8) = kHundred;  // vertex 2
    at<u32>(v, 0x18) = level;
    at<u32>(v, 0x28) = 0;
    v = at<u8 *>(bar, 0x7c);
    level = X87_FloatThrough(at<u32>(bar, 0x120));
    at<u32>(v, 0x34) = kHundred;  // vertex 5
    at<u32>(v, 0x44) = level;
    at<u32>(v, 0x54) = 0;
    v = at<u8 *>(bar, 0x7c);
    level = X87_FloatThrough(at<u32>(bar, 0x120));
    at<u32>(v, 0xc) = 0;  // vertex 3
    at<u32>(v, 0x1c) = level;
    at<u32>(v, 0x2c) = 0;
    level = X87_FloatThrough(at<u32>(bar, 0x120));
    v = at<u8 *>(bar, 0x7c);
    at<u32>(v, 0x30) = 0;  // vertex 4
    at<u32>(v, 0x40) = level;
    at<u32>(v, 0x50) = 0;
}

void __fastcall BarMesh_SetTopTexCoords(const u8 *bar) {
    const u32 v = bitsOf(static_cast<float>(X87Reg(at<float>(bar, 0x120)) * X87Reg(0.01f)));
    u8 *t = at<u8 *>(bar, 0x84);
    at<u32>(t, 0xc) = kHalf;  // 3
    at<u32>(t, 0x1c) = v;
    t = at<u8 *>(bar, 0x84);
    at<u32>(t, 8) = kOne;  // 2
    at<u32>(t, 0x18) = v;
    t = at<u8 *>(bar, 0x84);
    at<u32>(t, 0x20) = 0;  // 4
    at<u32>(t, 0x30) = v;
    t = at<u8 *>(bar, 0x84);
    at<u32>(t, 0x24) = kHalf;  // 5
    at<u32>(t, 0x34) = v;
}

void __fastcall BarMesh_Dtor(u8 *bar) { Mesh_Dtor(bar, 0x006fee88); }

u8 *__fastcall BarMesh_ScalarDtor(u8 *bar, void *, u32 flags) { return Mesh_ScalarDtor(bar, 0x006fee88, flags); }

void __fastcall MeshOverlay_Dtor(u8 *mesh) { Mesh_Dtor(mesh, 0x006feeb8); }

u8 *__fastcall MeshOverlay_ScalarDtor(u8 *mesh, void *, u32 flags) { return Mesh_ScalarDtor(mesh, 0x006feeb8, flags); }
}
