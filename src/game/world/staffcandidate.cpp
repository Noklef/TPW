#include "staffcandidate.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/indexed.h"
#include "../core/gameheap.h"
#include "../sprite/sprite.h"
#include "../ui/menuactions.h"
#include "mousemode.h"
#include "random.h"
#include "staff.h"
#include "things.h"
#include "staffcandidate.h"

namespace {

template <typename T> T &at(u8 *p, u32 offset) { return *reinterpret_cast<T *>(p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

void __fastcall tellStaffDialog(void *, void *, u32 slot) { Wrap_Dialog7ca308_StaffChanged_481550(slot); }   // (0x481550 reads no ECX)
const StaffCandidateEnv kGame = {&tellStaffDialog};
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

namespace {

// Inlined at each of StaffPool_FillAll's five kinds.
bool add_candidate(u8 *pool, u32 kind) {
    if (at<u8>(pool, 0x299 + kind)) return false;
    for (u32 i = 0; i != 0x20; ++i) {
        if (at<u8>(pool, i * 0x14 + 0xa) == 0) {
            StaffPoolEntry_Generate(pool + i * 0x14, nullptr, kind, i);
            return true;
        }
    }
    return false;
}

}  // namespace

u8 __fastcall StaffPool_AddCandidate(u8 *pool, void *, u32 kind) { return add_candidate(pool, kind) ? 1 : 0; }

u8 __fastcall StaffPool_FillAll(u8 *pool) {
    if (at<u8>(pool, 0x298)) return 0;   // (with a debug print, left out)
    for (u32 kind = 0; kind != 5; ++kind) {
        for (s32 n = 0; n < global<s32>(0x007852ac + kind * 4); ++n) add_candidate(pool, kind);
    }
    at<u8>(pool, 0x298) = 1;
    return 1;
}

u8 __fastcall StaffPool_TakeCandidate(u8 *pool, void *, u32 slot) {
    if (slot >= 0x20) return 0;
    u8 *const entry = pool + slot * 0x14;
    if (!at<u8>(entry, 0xa)) return 0;
    const u32 type = at<u32>(entry, 0);
    if (type <= 4) {
        static const u32 kModes[5] = {5, 4, 6, 7, 8};
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x1c));
        u8 *const mode = block ? MMPlaceStaff_Ctor(block, nullptr, kModes[type], at<u8>(entry, 8), at<u8>(entry, 9), at<u32>(entry, 4)) : nullptr;
        MouseModes_EnsureHolder();
        MouseMode_Set(mode);
    }
    at<u8>(entry, 0xb) = 1;
    Staff_CountTypes(pool);
    for (u32 i = 0; i < 0x20; i++) {
        u8 *const other = pool + i * 0x14;
        if (at<u8>(other, 0xa) && at<u8>(pool, at<u32>(other, 0) + 0x299) && !at<u8>(other, 0xb)) {
            at<u8>(other, 0xa) = 0;
            g_env->tellDialog(reinterpret_cast<void *>((uintptr_t)0x007b51f0), nullptr, i);
        }
    }
    return 1;
}

void __fastcall StaffPool_Update(u8 *pool) {
    for (u32 i = 0; i < 0x20; i++) {
        u8 *const entry = pool + i * 0x14;
        if (at<u8>(entry, 0xa) && Game1da70c_MinusAt0(reinterpret_cast<const u32 *>(entry + 0xc)) > at<u32>(entry, 0x10) * 4 && !at<u8>(entry, 0xb)) {
            at<u8>(entry, 0xa) = 0;
            g_env->tellDialog(reinterpret_cast<void *>((uintptr_t)0x007b51f0), nullptr, i);
        }
    }
    u32 *const clock = reinterpret_cast<u32 *>(pool + 0x294);
    if (Game1da70c_MinusAt0(clock) <= global<u32>(0x00785304) * 4) return;
    u32 used = 0;
    for (u32 i = 0; i < 0x20; i++) used += at<u8>(pool, i * 0x14 + 0xa) != 0;
    const u32 count = global<u32>(0x00785308);
    s32 left = (s32)(count < 0x20 - used ? count : 0x20 - used);
    Staff_CountTypes(pool);
    s32 wants[5];
    s32 total = 0;
    for (u32 k = 0; k < 5; k++) {
        if (at<u8>(pool, 0x299 + k)) {
            wants[k] = 0;
        } else {
            s32 ofKind = 0;
            for (u32 i = 0; i < 0x20; i++) ofKind += at<u8>(pool, i * 0x14 + 0xa) != 0 && at<u32>(pool, i * 0x14) == k;
            wants[k] = global<s32>(0x007852c0 + k * 4) - ofKind;
        }
        total += wants[k];
    }
    u32 kind = (u32)(uintptr_t)clock;   // (sic: what the original's register holds until a roll lands)
    while (left > 0 && total > 0) {
        s32 roll = (s32)((u32)World_Rand(global<u8 *>(0x0080239c)) % (u32)total);
        for (u32 k = 0; k < 5; k++) {
            if (roll < wants[k]) {
                kind = k;
                break;
            }
            roll -= wants[k];
        }
        if (at<u8>(pool, kind + 0x299)) break;
        u32 slot = 0;
        while (slot < 0x20 && at<u8>(pool, slot * 0x14 + 0xa)) slot++;
        if (slot == 0x20) break;
        StaffPoolEntry_Generate(pool + slot * 0x14, nullptr, kind, slot);
        left--;
        wants[kind]--;
        total--;
    }
    StaffPool_TopUp(pool);
    Out_SetFromGame1da70c(clock);
}

void __fastcall StaffPool_Start(u8 *pool) {
    StaffPool_FillAll(pool);
    Out_SetFromGame1da70c(&at<u32>(pool, 0x294));
}

void __fastcall StaffPool_TopUp(u8 *pool) {
    // The used entries of the kind, plus the kind's dword +0x280, short of the limit [0x7852d4] (signed).
    const auto is_short = [pool](u32 kind) {
        u32 used = 0;
        for (u32 i = 0; i != 0x20; ++i) {
            if (at<u8>(pool, i * 0x14 + 0xa) && at<u32>(pool, i * 0x14) == kind) ++used;
        }
        return (s32)(at<u32>(pool, 0x280 + kind * 4) + used) < global<s32>(0x007852d4 + kind * 4);
    };
    static const u8 kOrder[5] = {1, 0, 2, 3, 4};
    bool again;
    do {
        again = false;
        for (const u8 kind : kOrder) {
            if (!is_short(kind)) continue;
            add_candidate(pool, kind);
            again = true;   // (even when it could not add: so a kind short with its flag set or no entry free never ends, as the original)
        }
    } while (again);
}
