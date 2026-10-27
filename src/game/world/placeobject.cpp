#include "placeobject.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/thunks.h"
#include "../math/intpair.h"
#include "../math/x87.h"
#include "objectdefs.h"
#include "things.h"
#include "trackrides.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const PlaceObjectEnv kGameEnv = {reinterpret_cast<u8 *(__fastcall *)(u8 *, void *, u32, u32, s32, u32)>((uintptr_t)0x004db090)};
const PlaceObjectEnv *g_env = &kGameEnv;

// fild n; fmul [0x700cd0]; fmul [0x700cd8]; __ftol.
s32 scaled(s32 n) {
    const double v = (X87Reg(X87_MulIntDouble(n, global<float>(0x00700cd0))) * global<float>(0x00700cd8)).v;
    return (s32)(long long)v;
}

}  // namespace

extern "C" {

void PlaceObject_SetEnv(const PlaceObjectEnv *env) { g_env = env ? env : &kGameEnv; }

u8 *__cdecl PlaceObject_Create(s32 angle) {
    const u8 *const def = ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, global<u16>(0x00818c38));
    u32 ride = 0;
    const s32 kind = at<s32>(def, 0xa0);
    if (kind) {
        const u32 cell = (u32)global<u16>(0x00818c3c) - 1;
        s32 x = (s32)(cell & 0x7f), y = (s32)(cell >> 7);
        u32 code;
        switch (angle) {
        case 0: x += at<s32>(def, 0xa4); y += at<s32>(def, 0xa8); code = 0; break;
        case 0x5a: x += at<s32>(def, 0xac); y += at<s32>(def, 0xb0); code = 5; break;
        case 0xb4: x += at<s32>(def, 0xb4); y += at<s32>(def, 0xb8); code = 6; break;
        case 0x10e: x += at<s32>(def, 0xbc); y += at<s32>(def, 0xc0); code = 1; break;
        default: code = (u32)angle; break;
        }
        switch (angle) {   // (Position_StepA's steps, inlined)
        case 0: x += 4; y += 1; break;
        case 0x5a: x += 1; y -= 5; break;
        case 0xb4: x -= 5; y -= 2; break;
        case 0x10e: x -= 2; y += 4; break;
        default: break;
        }
        const s32 ty = scaled(y);
        const s32 tx = scaled(x);
        ride = TrakRide_Create(kind, tx, ty, code, 0, global<u16>(0x00818c38));
    }
    u32 pair[2];
    IntPair_Rotated(reinterpret_cast<const u32 *>(def + 0x4b0), nullptr, pair, angle);
    const u16 cell = (u16)((u16)(pair[1] << 7) + global<u16>(0x00818c3c) + (u16)pair[0]);
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x450));
    u8 *const object = block ? g_env->construct(block, nullptr, cell, global<u16>(0x00818c38), angle, ride) : nullptr;
    if (!object) return nullptr;
    u16 handle = 0;
    if (global<u32>(0x008187f8) && at<u32>(def, 0x288) != 2) Thing_GetHandle(reinterpret_cast<const u8 *>((uintptr_t)Global8186ac_Get()), nullptr, &handle);
    Fn_4e0de0(object, nullptr, handle);
    Wrap_Global7c2654_Set_4815b0((u32)(uintptr_t)object);
    return object;
}
}
