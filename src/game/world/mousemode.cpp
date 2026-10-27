#include "mousemode.h"

#include <cstdint>
#include <cstring>

#include "../core/accessors.h"
#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/options.h"
#include "../core/vcall.h"
#include "../math/x87.h"
#include "../model/highlight.h"
#include "../render/dynfaces.h"
#include "../sprite/script.h"
#include "../ui/cursor.h"
#include "../ui/focus.h"
#include "mapcell.h"
#include "thingscan.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u32 bits(float f) {
    u32 u;
    std::memcpy(&u, &f, 4);
    return u;
}

const MouseModeEnv kGameEnv = {reinterpret_cast<void(__cdecl *)(void *)>(0x0067acb0),
                                reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>(0x0067a7d0)};
const MouseModeEnv *g_env = &kGameEnv;

}  // namespace

void MouseModes_SetEnv(const MouseModeEnv *env) { g_env = env ? env : &kGameEnv; }

void MouseModes_EnsureHolder() {
    u8 *&holder = *reinterpret_cast<u8 **>(0x007b05e8);
    if (holder) return;
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(1));
    if (block) MouseModeHolder_Ctor(block);
    holder = block;
}

extern "C" {

u8 *__fastcall MouseModeHolder_Ctor(u8 *holder) {
    u8 *const mode = static_cast<u8 *>(GameHeap_Alloc(8));
    MouseMode_Set(mode ? MouseModeOnlineBase_Ctor(mode) : nullptr);
    *reinterpret_cast<u32 *>(0x007b05e0) = 0xffffffffu;
    *reinterpret_cast<u32 *>(0x007b05d4) = 0xffffffffu;
    return holder;
}

void __cdecl MouseModes_InitUnusedOne() { *reinterpret_cast<float *>(0x007b05a0) = 1.0f; }

void __cdecl MouseModes_InitUnusedEpsilon() { *reinterpret_cast<float *>(0x007b05a4) = 1.0f / 8388608.0f; }

void __cdecl MouseModes_InitTile() {
    *reinterpret_cast<u16 *>(0x007b05cc) = 1;
    *reinterpret_cast<u32 *>(0x007b05d0) = 0;
}

void __cdecl MouseState_SetTileFromPosition(const float *position) {
    u8 *const map = *reinterpret_cast<u8 **>(0x007a0854);
    if (!map) return;
    u8 *const grid = at<u8 *>(map, 0x6c);
    if (!grid) return;
    s32 x = (s32)(long long)((X87Reg(position[0]) - at<float>(grid, 8)) / at<float>(grid, 0x10)).v;  // __ftol
    s32 z = (s32)(long long)((X87Reg(position[2]) - at<float>(grid, 0xc)) / at<float>(grid, 0x14)).v;
    if (x < 0) x = 0;
    if (z < 0) z = 0;
    if ((u32)x >= at<u32>(grid, 0x18)) x = at<u32>(grid, 0x18) - 1;
    if ((u32)z >= at<u32>(grid, 0x1c)) z = at<u32>(grid, 0x1c) - 1;
    if (x < 0) x = 0;
    if (z < 0) z = 0;
    const s32 width = *reinterpret_cast<const s32 *>(0x007a0314), depth = *reinterpret_cast<const s32 *>(0x007a0374);
    if (x >= width) x = width - 1;
    if (z >= depth) z = depth - 1;
    *reinterpret_cast<u16 *>(0x007b05cc) = Map_InBounds(x, z) ? (u16)((z << 7) + x + 1) : 0;
}

void __fastcall MouseState_NewFrame(u8 *self) {
    const u32 count = at<u32>(self, 0xc);
    at<u32>(self, 0) = 0;
    at<u32>(self, 0xc) = count + 1;
}

u32 __fastcall MouseState_ShowFps(u8 *self, void *, float fps) {
    char *const text = reinterpret_cast<char *>(self + 0x10);
    Crt().sprintf(text, reinterpret_cast<const char *>(0x0074d85c), (double)fps);  // "FPS: %4g\n"
    return Obj_Set14_18_10(reinterpret_cast<u8 *>(0x008bba50), nullptr, 0, 0, (u32)(uintptr_t)text);
}

void __fastcall MouseState_ResolvePick(u8 *self) {
    u8 *const picked = at<u8 *>(self, 0);
    at<u32>(self, 4) = 0;
    at<u32>(self, 8) = 0;
    if (picked) {
        const u8 kind = picked[2];
        if (kind == 1 || kind == 4 || kind == 5 || kind == 6 || kind == 8 || kind == 7 || kind == 0x12) at<u8 *>(self, 4) = picked;
    }
    const u32 tile = at<u16>(self, 0x24);
    u16 handle = (u16)tile;
    Map_ObjectOrEntranceAt(&handle, (s32)((tile - 1) & 0x7f), (s32)((tile - 1) >> 7));
    if (handle) at<u32>(self, 8) = *reinterpret_cast<u32 *>(0x007cfb90 + 20 * (u32)handle);  // (an assert on the thing table left out)
    if (!at<u32>(self, 0)) at<u32>(self, 0) = at<u32>(self, 8);
}

void __fastcall MouseMode_Nop4(void *, void *, u32, u32, u32, u32) {}

void __fastcall MMPlaceStaff_Hover(u8 *self, void *, u32, u32, u32, u32) {
    if (!at<s32>(self, 8)) return;
    const float *const point = Highlight_Point();
    const float x = point[0], y = point[1], z = point[2];
    u8 *const cell = *reinterpret_cast<u8 **>(0x007cf83c) + 0x294 + *reinterpret_cast<const u16 *>(0x007b05cc) * 0x44;
    SpriteScript_SetPosition(at<s32>(self, 8), bits(x), bits(y - -5.0f), bits(z), 4);
    const u16 *const handle = reinterpret_cast<const u16 *>(cell);
    if (!MapHandle_IsTypeBDOr10Or12Or19(handle) && !MapCell_HasFlags(cell, nullptr, 0x40) && !MapHandle_ParentIsType19(handle) &&
        (MapCell_IsType0(cell) || MapCell_IsType1(cell) || MapCell_IsType3Or9(cell)))
        return;
    DynFaces_AddSquare(bits(x - 5.0f), 0x3f800000, bits(z - 5.0f), 0, 1, 0, 0x3f800000, 0, 0, 0, 0);
}

void __fastcall MouseMode_Nop2(void *, void *, u32, u32) {}

void __fastcall MouseMode_Dtor(u8 *self) { at<u32>(self, 0) = 0x006fe9b0; }

u8 *__fastcall MouseMode_DeletingDtor(u8 *self, void *, u32 flags) {
    at<u32>(self, 0) = 0x006fe9b0;
    if (flags & 1) g_env->free(self);
    return self;
}

u8 *__fastcall MouseMode_DerivedDeletingDtor(u8 *self, void *, u32 flags) {
    MouseMode_Dtor(self);
    if (flags & 1) g_env->free(self);
    return self;
}

u8 *__fastcall MouseModeTile_Ctor(u8 *self, void *, u32 arg) {
    at<u32>(self, 0) = 0x006fe9e0;
    at<u32>(self, 4) = 1;
    at<u32>(self, 8) = arg;
    at<u32>(self, 0xc) = 0;
    return self;
}

u8 *__fastcall MouseModeTile_CtorKind(u8 *self, void *, u32 arg, u16 kind) {
    at<u32>(self, 4) = 1;
    at<u32>(self, 0xc) = 1;
    at<u32>(self, 0) = 0x006fe9e0;
    at<u32>(self, 8) = arg;
    at<u16>(self, 0x10) = kind;
    return self;
}

u8 *__fastcall MouseModeOnlineBase_Ctor(u8 *self) {
    at<u32>(self, 0) = 0x006fea10;
    at<u32>(self, 4) = 1;
    return self;
}

u8 *__fastcall MMPlaceStaff_Ctor(u8 *self, void *, u32 a, u8 b, u8 c, u32 d) {
    at<u32>(self, 0xc) = a;
    at<u8>(self, 0x10) = b;
    at<u8>(self, 0x11) = c;
    at<u32>(self, 4) = 0;
    at<u32>(self, 8) = 0;
    at<u32>(self, 0x14) = d;
    at<u8>(self, 0x18) = 0;
    at<u32>(self, 0) = 0x006fea40;
    return self;
}

u8 *__fastcall MMStaffDrop_Ctor(u8 *self, void *, u32 a, u8 b) {
    at<u32>(self, 4) = 0;
    at<u32>(self, 8) = 0;
    at<u32>(self, 0) = 0x006fea70;
    at<u32>(self, 0xc) = a;
    at<u8>(self, 0x10) = b;
    return self;
}

u8 *__fastcall MouseMode6feaa0_Ctor(u8 *self) {
    at<u32>(self, 4) = 0;
    at<u32>(self, 0) = 0x006feaa0;
    return self;
}

u8 *__fastcall MouseMode6fead0_Ctor(u8 *self) {
    at<u32>(self, 4) = 0;
    at<u32>(self, 0) = 0x006fead0;
    return self;
}

u8 *__fastcall MMOnline_Ctor(u8 *self) {
    at<u32>(self, 4) = 0;
    at<u32>(self, 0) = 0x006fe980;
    Obj7b51f0_SetCursor(reinterpret_cast<void *>(0x007b51f0), nullptr, 0);
    return self;
}

void __stdcall MouseMode_Set(u8 *mode) {
    u8 *&current = *reinterpret_cast<u8 **>(0x007b05d8);
    if (current) {
        vcall<void>(current, 0x2c);
        if (current) vcall<void>(current, 0, 1u);
    }
    u8 &guard = *reinterpret_cast<u8 *>(0x00fb3518);
    const u8 bits = guard;
    if (!(bits & 1)) {
        guard = bits | 1;
        Setting_ReadMode(reinterpret_cast<u8 *>(0x00fb3b7c));
        g_env->atExit(reinterpret_cast<void(__cdecl *)()>(0x005d1750));
    }
    if (*reinterpret_cast<u32 *>(0x00fb3b7c) == 1 && mode && vcall<u32>(mode, 0x24) == 1) {
        vcall<void>(mode, 0, 1u);
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(8));
        mode = block ? MMOnline_Ctor(block) : nullptr;
    }
    current = mode;
    if (mode) vcall<void>(mode, 0x28);
}

void __cdecl MouseMode_Enter6fead0() {
    Window7c24c8_Close();
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(8));
    u8 *const mode = block ? MouseMode6fead0_Ctor(block) : nullptr;
    MouseModes_EnsureHolder();
    MouseMode_Set(mode);
}

void __fastcall MouseMode6fead0_Assign(u8 *) { Obj7b51f0_SetCursor(reinterpret_cast<void *>(0x007b51f0), nullptr, 0x13); }

// The sprite script of the staff being placed: started for the folder and sprite, the one at +8 before it killed.
static void start_staff_script(u8 *self, u32 folder, u8 sprite) {
    const s32 slot = SpriteScript_Start(0x0074ed98, folder, sprite, 0, 0, 0, 0);
    if (const s32 old = at<s32>(self, 8))
        SpriteScript_Kill(old);
    at<s32>(self, 8) = slot;
}

void __fastcall MMPlaceStaff_Assign(u8 *self) {
    Obj7b51f0_SetCursor(reinterpret_cast<void *>(0x007b51f0), nullptr, 9);
    static constexpr u32 kFolders[] = {6, 5, 4, 7, 8};   // (kinds 4-8)
    const u32 kind = at<u32>(self, 0xc) - 4;
    if (kind <= 4)
        start_staff_script(self, kFolders[kind], at<u8>(self, 0x11));
}

void __fastcall MMStaffDrop_Assign(u8 *self) {
    Obj7b51f0_SetCursor(reinterpret_cast<void *>(0x007b51f0), nullptr, 9);
    start_staff_script(self, at<u32>(self, 0xc), at<u8>(self, 0x10));
}

void __cdecl MouseMode_Ensure6feaa0() {
    MouseModes_EnsureHolder();
    u8 *const current = *reinterpret_cast<u8 **>(0x007b05d8);
    if ((current ? vcall<s32>(current, 0x24) : 0) == 8) return;
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(8));
    u8 *const mode = block ? MouseMode6feaa0_Ctor(block) : nullptr;
    MouseModes_EnsureHolder();
    MouseMode_Set(mode);
}
}
