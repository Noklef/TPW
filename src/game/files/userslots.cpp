#include "userslots.h"

#include <cstdint>
#include <initializer_list>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/stdlist.h"
#include "../core/stdstring.h"
#include "../core/string.h"
#include "../core/thunks.h"
#include "../core/trivial.h"
#include "../core/vcall.h"
#include "gamepaths.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const WString *wide(u32 address) { return reinterpret_cast<const WString *>((uintptr_t)address); }

// Appended through the wide string's vtable (WString_Append).
void append(WString *s, const WString *tail) { vcall<void>(s, 0x28, tail); }

const UserSlotsEnv kGameEnv = {&UserSlots_Ctor, reinterpret_cast<u16 *(__cdecl *)(s32, u16 *, s32)>((uintptr_t)0x0067e010)};
const UserSlotsEnv *g_env = &kGameEnv;

}  // namespace

void UserSlots_SetEnv(const UserSlotsEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u8 *__fastcall UserSlots_Ctor(u8 *self) {
    CRT_VectorCtorIterator(self, 0x18, 4, reinterpret_cast<void(__fastcall *)(void *)>(&WString_CtorReturning), reinterpret_cast<void *>(&Wrap_WString_Dtor_5c74a0));
    UserSlotsPart_Ctor(self + 0x64);
    ListHead_Ctor(self + 0x100);
    at<s32>(self, 0x60) = -1;
    return self;
}

WString *__fastcall UserSlots_SlotName(const u8 *self, void *, WString *out, s32 i) {
    u16 digits[4];
    g_env->itow(i + 1, digits, 10);
    WString name;
    WString_CtorStr(&name, nullptr, digits);
    append(&name, reinterpret_cast<const WString *>(Records24_IfUsed(const_cast<u8 *>(self), nullptr, i)));
    WString_CtorCopy(out, nullptr, &name);
    WString_Dtor(&name);
    return out;
}

WString *__fastcall UserSlots_SlotFolder(const u8 *self, void *, WString *out, s32 i) {
    WString path;
    GamePaths_UsersFolder(&path);
    append(&path, wide(0x00f82508));
    WString name;
    UserSlots_SlotName(self, nullptr, &name, i);
    append(&path, &name);
    WString_Dtor(&name);
    WString_CtorCopy(out, nullptr, &path);
    WString_Dtor(&path);
    return out;
}

WString *__fastcall UserSlots_ChosenFolder(const u8 *self, void *, WString *out) { return UserSlots_SlotFolder(self, nullptr, out, at<s32>(self, 0x60)); }

WString *__fastcall UserSlots_ThemeFolder(const u8 *self, void *, WString *out) {
    WString path;
    UserSlots_ChosenFolder(self, nullptr, &path);
    append(&path, wide(0x00f82508));
    String label;
    String_CtorCopy(&label, nullptr, reinterpret_cast<const StringView *>(Obj_Self(reinterpret_cast<u8 *>((uintptr_t)0x00786b68))));
    WString name;
    WString_Ctor(&name);
    WString_AssignFromAnsi(&name, &label);
    append(&path, &name);
    WString_CtorCopy(out, nullptr, &path);
    WString_Dtor(&name);
    String_Dtor(&label);
    WString_Dtor(&path);
    return out;
}

WString *__stdcall UserSlots_ThemePath(WString *out) {
    WString path;
    WString_Ctor(&path);
    u8 *const slots = UserSlots_Instance();
    WString folder;
    UserSlots_ThemeFolder(slots, nullptr, &folder);
    vcall<void>(&path, 0x18, static_cast<const WString *>(&folder));
    WString_Dtor(&folder);
    for (const u32 tail : {0x00f7d740u, 0x00f7d668u, 0x00f7cf30u})
        append(&path, wide(tail));
    WString_CtorCopy(out, nullptr, &path);
    WString_Dtor(&path);
    return out;
}

u8 *__fastcall UserSlotsPart_Ctor(u8 *self) {
    for (const u32 offset : {0u, 0x10u, 0x20u})
        StdString_Tidy(reinterpret_cast<StdString *>(self + offset), nullptr, 0);
    ListHead_CtorFrom(self + 0x30, nullptr, self + 0x30);
    StdString_Tidy(reinterpret_cast<StdString *>(self + 0x3c), nullptr, 0);
    for (const u32 offset : {0x5cu, 0x68u, 0x74u}) {
        u8 *const sentinel = static_cast<u8 *>(GameHeap_Alloc(0xc));
        at<u8 *>(sentinel, 0) = sentinel;
        at<u8 *>(sentinel, 4) = sentinel;
        at<u8 *>(self, offset) = sentinel;
        at<u32>(self, offset + 4) = 0;
    }
    at<u32>(self, 0x84) = 0;
    at<u32>(self, 0x88) = 0;
    at<u32>(self, 0x8c) = 0;
    at<u16>(self, 0x90) = 0x19;
    at<u32>(self, 0x94) = 0;
    at<u32>(self, 0x98) = 0;
    return self;
}

u8 *__cdecl UserSlots_Instance() {
    u8 *&slots = *reinterpret_cast<u8 **>((uintptr_t)0x00f81ebc);
    if (!slots) {
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x110));
        slots = block ? g_env->ctor(block) : nullptr;
    }
    return slots;
}

u32 __fastcall Records24_CountUsed(const u8 *records) {
    u32 count = 0;
    for (u32 i = 0; i < 4; i++)
        count += at<u32>(records, 4 + 24 * i) != 0 ? 1 : 0;
    return count;
}

u8 *__fastcall Records24_IfUsed(u8 *records, void *, u32 i) {
    u8 *record = records + 24 * i;
    return at<u32>(record, 4) != 0 ? record : nullptr;
}

u32 __fastcall Records24_Field14(const u8 *records, void *, u32 i) { return at<u32>(records, 24 * i + 0x14); }

}  // extern "C"
