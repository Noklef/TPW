#include "staffcandidate.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/indexed.h"
#include "../sprite/sprite.h"
#include "random.h"
#include "staff.h"
#include "things.h"

namespace {

template <typename T> T &at(u8 *p, u32 offset) { return *reinterpret_cast<T *>(p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const StaffCandidateEnv kGame = {reinterpret_cast<void(__fastcall *)(void *, void *, u32)>((uintptr_t)0x00481550)};
const StaffCandidateEnv *g_env = &kGame;

u16 *const kTakenText = reinterpret_cast<u16 *>((uintptr_t)0x007cee18);
u16 *const kPickedText = reinterpret_cast<u16 *>((uintptr_t)0x007cef20);

// The grade of the kinds 0-4: a World_Rand wasted, then World_Rand % 3 + the average - 1, in a byte.
u8 random_grade(u32 averageAddress) {
    u8 *const world = global<u8 *>(0x0080239c);
    World_Rand(world);
    const u32 draw = (u32)World_Rand(world);
    return (u8)((u8)(draw % 3) + global<u8>(averageAddress) - 1);
}

}  // namespace

void StaffCandidate_SetEnv(const StaffCandidateEnv *env) { g_env = env ? env : &kGame; }

u8 __fastcall StaffPool_NameTaken(u8 *pool, void *, const u16 *name) {
    u8 *entry = pool;
    for (u32 i = 0; i < 0x20; ++i, entry += 0x14) {
        if (at<u8>(entry, 0xa) == 0) continue;
        StaffPoolEntry_FormatName(entry, nullptr, kTakenText, 0x80);
        if (Crt().wcscmp(name, kTakenText) == 0) return 1;
    }
    u16 handle;
    World_FirstThing(nullptr, nullptr, &handle);
    while (handle != 0) {
        u8 *const thing = global<u8 *>(0x007cfb90 + handle * 20u);
        if (Thing_IsStaff(thing) && Crt().wcscmp(name, Staff_GetName(thing)) == 0) return 1;
        u16 next;
        World_NextThing(nullptr, nullptr, &next, thing);
        handle = next;
    }
    return 0;
}

void __fastcall StaffPoolEntry_PickName(u8 *entry) {
    u32 tries = 0;
    do {
        if (at<u32>(entry, 0) <= 4) at<u32>(entry, 4) = (u32)World_Rand(global<u8 *>(0x0080239c)) % 0x23;
        StaffPoolEntry_FormatName(entry, nullptr, kPickedText, 0x80);
        ++tries;
    } while (StaffPool_NameTaken(global<u8 *>(0x0080239c), nullptr, kPickedText) != 0 && tries < 0xf);
}

void __fastcall StaffPoolEntry_Generate(u8 *entry, void *, u32 kind, u32 slot) {
    static const u8 kFolders[5] = {5, 6, 4, 7, 8};
    at<u32>(entry, 0) = kind;
    u8 grade = (u8)kind;
    if (kind <= 4) {
        at<u8>(entry, 9) = (u8)SpriteFolder_RandomSprite(kFolders[kind]);
        grade = random_grade(0x00785298 + kind * 4);
    }
    at<u8>(entry, 8) = grade > 4 ? 4 : grade;
    StaffPoolEntry_PickName(entry);
    at<u8>(entry, 0xa) = 1;
    g_env->tellDialog(reinterpret_cast<void *>((uintptr_t)0x007b51f0), nullptr, slot);
    Out_SetFromGame1da70c(&at<u32>(entry, 0xc));
    const u32 draw = (u32)World_Rand(global<u8 *>(0x0080239c));
    const s32 timeout = global<s32>(0x0078530c);
    at<u32>(entry, 0x10) = draw % (u32)(timeout >> 1) + (u32)timeout;
}
