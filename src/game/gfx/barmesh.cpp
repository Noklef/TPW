#include "barmesh.h"

#include <cstdint>
#include <cstring>

#include "../core/gameheap.h"
#include "../math/x87.h"
#include "../core/trivial.h"
#include "../math/matrix.h"
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

float floatOf(u32 b) {
    float f;
    std::memcpy(&f, &b, 4);
    return f;
}

// 0x477870 / 0x477ff0 up to the animator's mesh: the GUI dialog base (with the name), the vtable, the record (+0xbc: one name, the
// mesh's at +0xb0, flag 2), the animator (+0x6c: flags 0x1003, one node at +0x3c with 4 + 8 vertices, 8 normals, 1 texture block, 8
// indices, the matrix at +0xdc) and Animator_Create; false when that fails (neither does).
bool Mesh_ConstructStart(u8 *mesh, const char *name, u32 vtable) {
    GuiDialog_Ctor(mesh, nullptr, (u32)(uintptr_t)name);
    at<const char *>(mesh, 0xb0) = name;
    at<u32>(mesh, 0) = vtable;
    at<u32>(mesh, 0xac) = 2;
    std::memset(mesh + 0xbc, 0, 0x20);
    at<u8 *>(mesh, 0xcc) = mesh + 0xac;
    at<u8 *>(mesh, 0x74) = mesh + 0x3c;
    at<u8 *>(mesh, 0x78) = mesh + 0xdc;
    at<u32>(mesh, 0x3c) = 3;
    at<u32>(mesh, 0xbc) = 0;
    at<u16>(mesh, 0xc0) = 1;
    at<u32>(mesh, 0xc4) = 0x0074f89c;
    at<u32>(mesh, 0x6c) = 0x1003;
    at<u16>(mesh, 0x70) = 1;
    at<u16>(mesh, 0x40) = 4;
    at<u16>(mesh, 0x42) = 8;
    at<u16>(mesh, 0x44) = 8;
    at<u16>(mesh, 0x46) = 1;
    at<u16>(mesh, 0x48) = 8;
    at<u32>(mesh, 4) = 0;
    at<u32>(mesh, 0x4c) = 0;
    at<u32>(mesh, 0x50) = 0;
    at<u32>(mesh, 0x54) = 0;
    at<u32>(mesh, 0x58) = kHundred;
    at<u32>(mesh, 0x5c) = kHundred;
    at<u32>(mesh, 0x60) = 0;
    if (!Const1_424ec0())
        return false;
    if (!Animator_Create(mesh + 0x6c, mesh + 0xbc, name, 1)) {
        Dword_ClearBit0(&at<u32>(mesh, 0xbc));
        return false;
    }
    Matrix_Identity(at<float *>(mesh, 0x64));
    return true;
}

// The eight normals (+0x80, 12 bytes each) to (0, 0, -1) and the texture block (+0x68): the record's table (+0xd4), the words 0, 4,
// 0, 7.
void Mesh_SetNormals(u8 *mesh) {
    for (u32 k = 0; k < 8; ++k) {
        u8 *const n = at<u8 *>(mesh, 0x80) + k * 0xc;
        at<u32>(n, 0) = 0;
        at<u32>(n, 4) = 0;
        at<u32>(n, 8) = 0xbf800000;
    }
}

void Mesh_SetIndices(u8 *mesh, const u16 (&words)[12]) {
    static const u8 kAt[12] = {2, 4, 6, 0xa, 0xc, 0xe, 0x12, 0x14, 0x16, 0x1a, 0x1c, 0x1e};
    for (u32 k = 0; k < 12; ++k)
        at<u16>(at<u8 *>(mesh, 0x88), kAt[k]) = words[k];
    at<u32>(at<u8 *>(mesh, 0x68), 0) = at<u32>(mesh, 0xd4);
    at<u16>(at<u8 *>(mesh, 0x68), 4) = 0;
    at<u16>(at<u8 *>(mesh, 0x68), 6) = 4;
    at<u16>(at<u8 *>(mesh, 0x68), 8) = 0;
    at<u16>(at<u8 *>(mesh, 0x68), 0xa) = 7;
}

u32 levelTimesHundredth(const u8 *mesh) { return bitsOf(static_cast<float>(X87Reg(at<float>(mesh, 0x120)) * X87Reg(0.01f))); }

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

u8 *__fastcall BarMesh_Construct(u8 *bar, void *, const char *name) {
    if (!Mesh_ConstructStart(bar, name, 0x006fee88))
        return bar;
    u8 *v = at<u8 *>(bar, 0x7c);
    at<u32>(v, 0) = 0;  // vertices 0-2 and 6-7
    at<u32>(v, 0x10) = kHundred;
    at<u32>(v, 0x20) = 0;
    v = at<u8 *>(bar, 0x7c);
    at<u32>(v, 4) = kHundred;
    at<u32>(v, 0x14) = kHundred;
    at<u32>(v, 0x24) = 0;
    v = at<u8 *>(bar, 0x7c);
    at<u32>(bar, 0x120) = 0;
    at<u32>(v, 8) = kHundred;
    at<u32>(v, 0x18) = 0;
    at<u32>(v, 0x28) = 0;
    BarMesh_SetVertex(at<float *>(bar, 0x7c), 5, floatOf(kHundred), at<float>(bar, 0x120), 0.0f);
    BarMesh_SetVertex(at<float *>(bar, 0x7c), 3, 0.0f, at<float>(bar, 0x120), 0.0f);
    BarMesh_SetVertex(at<float *>(bar, 0x7c), 4, 0.0f, at<float>(bar, 0x120), 0.0f);
    v = at<u8 *>(bar, 0x7c);
    at<u32>(v, 0x38) = kHundred;
    at<u32>(v, 0x48) = 0;
    at<u32>(v, 0x58) = 0;
    v = at<u8 *>(bar, 0x7c);
    at<u32>(v, 0x3c) = 0;
    at<u32>(v, 0x4c) = 0;
    at<u32>(v, 0x5c) = 0;
    Mesh_SetNormals(bar);
    static const u16 kIndices[12] = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4};
    Mesh_SetIndices(bar, kIndices);
    u8 *t = at<u8 *>(bar, 0x84);   // texture coordinates
    at<u32>(t, 0) = kHalf;
    at<u32>(t, 0x10) = kOne;
    t = at<u8 *>(bar, 0x84);
    at<u32>(t, 4) = kOne;
    at<u32>(t, 0x14) = kOne;
    t = at<u8 *>(bar, 0x84);
    at<u32>(t, 0x2c) = 0;
    at<u32>(t, 0x3c) = 0;
    t = at<u8 *>(bar, 0x84);
    at<u32>(t, 0x28) = kHalf;
    at<u32>(t, 0x38) = 0;
    const u32 level = levelTimesHundredth(bar);
    t = at<u8 *>(bar, 0x84);
    at<u32>(t, 0xc) = kHalf;
    at<u32>(t, 0x1c) = level;
    t = at<u8 *>(bar, 0x84);
    at<u32>(t, 8) = kOne;
    at<u32>(t, 0x18) = level;
    t = at<u8 *>(bar, 0x84);
    at<u32>(t, 0x20) = 0;
    at<u32>(t, 0x30) = level;
    t = at<u8 *>(bar, 0x84);
    at<u32>(t, 0x24) = kHalf;
    at<u32>(t, 0x34) = level;
    at<u32>(bar, 4) = 1;
    return bar;
}

u8 *__fastcall MeshOverlay_Construct(u8 *mesh, void *, const char *name) {
    if (!Mesh_ConstructStart(mesh, name, 0x006feeb8))
        return mesh;
    u8 *v = at<u8 *>(mesh, 0x7c);
    at<u32>(v, 0) = 0;  // vertices 0, 6, 1 and 7, 3
    at<u32>(v, 0x10) = kHundred;
    at<u32>(v, 0x20) = 0;
    v = at<u8 *>(mesh, 0x7c);
    at<u32>(v, 0x38) = kHundred;
    at<u32>(v, 0x48) = kHundred;
    at<u32>(v, 0x58) = 0;
    v = at<u8 *>(mesh, 0x7c);
    at<u32>(mesh, 0x120) = 0;
    at<u32>(v, 4) = 0;
    at<u32>(v, 0x14) = kHundred;
    at<u32>(v, 0x24) = 0;
    BarMesh_SetVertex(at<float *>(mesh, 0x7c), 4, at<float>(mesh, 0x120), floatOf(kHundred), 0.0f);
    BarMesh_SetVertex(at<float *>(mesh, 0x7c), 2, at<float>(mesh, 0x120), 0.0f, 0.0f);
    BarMesh_SetVertex(at<float *>(mesh, 0x7c), 5, at<float>(mesh, 0x120), 0.0f, 0.0f);
    v = at<u8 *>(mesh, 0x7c);
    at<u32>(v, 0x3c) = kHundred;
    at<u32>(v, 0x4c) = 0;
    at<u32>(v, 0x5c) = 0;
    v = at<u8 *>(mesh, 0x7c);
    at<u32>(v, 0xc) = 0;
    at<u32>(v, 0x1c) = 0;
    at<u32>(v, 0x2c) = 0;
    Mesh_SetNormals(mesh);
    static const u16 kIndices[12] = {0, 1, 2, 2, 3, 0, 4, 6, 7, 7, 5, 4};
    Mesh_SetIndices(mesh, kIndices);
    u8 *t = at<u8 *>(mesh, 0x84);   // texture coordinates
    at<u32>(t, 0) = 0;
    at<u32>(t, 0x10) = kOne;
    t = at<u8 *>(mesh, 0x84);
    at<u32>(t, 0x28) = kOne;
    at<u32>(t, 0x38) = kHalf;
    t = at<u8 *>(mesh, 0x84);
    at<u32>(t, 0xc) = 0;
    at<u32>(t, 0x1c) = kHalf;
    t = at<u8 *>(mesh, 0x84);
    at<u32>(t, 0x2c) = kOne;
    at<u32>(t, 0x3c) = 0;
    const u32 level = levelTimesHundredth(mesh);
    t = at<u8 *>(mesh, 0x84);
    at<u32>(t, 4) = level;
    at<u32>(t, 0x14) = kOne;
    t = at<u8 *>(mesh, 0x84);
    at<u32>(t, 0x20) = level;
    at<u32>(t, 0x30) = kHalf;
    t = at<u8 *>(mesh, 0x84);
    at<u32>(t, 8) = level;
    at<u32>(t, 0x18) = kHalf;
    t = at<u8 *>(mesh, 0x84);
    at<u32>(t, 0x24) = level;
    at<u32>(t, 0x34) = 0;
    at<u32>(mesh, 4) = 1;
    return mesh;
}

void __fastcall BarMesh_SetShown(u8 *mesh, void *, u32 shown) {
    const u16 word = (u16)shown;
    if (word == at<u16>(mesh, 8))
        return;
    if (word)
        Animator_Wake(mesh + 0x6c);
    else
        Animator_Sleep(mesh + 0x6c);
    at<u16>(mesh, 8) = word;
}
}
