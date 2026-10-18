#include "slots.h"

#include <cstdint>

#include "../motion/animator.h"
#include "../render/renderer.h"
#include "../render/state.h"
#include "holder.h"
#include "parts.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

u8 *&holder_of(u32 slot) { return *reinterpret_cast<u8 **>((uintptr_t)(0x00790398 + slot * 0xb0 + 4)); }

// The face of the slot's part with key and flag 0x400, or null.
u32 *part_flags(u32 key, u32 slot) {
    const u32 part = Model_FindPartByKey4(holder_of(slot), 0x400, key);
    if (part == 0xffffffff) return nullptr;
    const u8 *const model = at<const u8 *>(at<const u8 *>(holder_of(slot), 8), 4);
    const u32 index = at<u16>(model, 0x46) + part, first = at<u16>(model, 0x44);
    return reinterpret_cast<u32 *>(index < first ? at<u8 *>(model, 0x70) + index * 0xa0
                                                 : at<u8 *>(model, 0x74) + (index - first) * 0x58);
}

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __cdecl ModelSlot_Animate(u32 slot) {
    if (u8 *const holder = holder_of(slot)) Animator_Update(holder, reinterpret_cast<const u8 *>(1), 0);
}

void __cdecl ModelSlot_KillAdvisor(u32 slot) {
    if (!holder_of(slot) || !Holder_IsAwake(at<u8 *>(holder_of(slot), 8))) return;
    // (the original logs "Kill advisor" here: Debug_Printf, left out)
    Animator_Play(holder_of(slot), 0xc, 0, 2, 1.0f, 0);
    u8 *const animator = holder_of(slot);
    if (at<u32>(animator, 4) & 0x4000)
        Obj_RemoveFromRect(at<u8 *>(animator, 8), at<u32>(animator, 0xc0), at<u32>(animator, 0xc4));
    else
        Holder_Sleep(at<u8 *>(animator, 8));
}

u32 __cdecl ModelSlot_CallAdvisor(u32 entry, u32 slot) {
    // (the original logs "Initial advisor call" first: Debug_Printf, left out)
    if (u8 *const animator = holder_of(slot)) return Animator_Play(animator, 5, entry - 1, 0x10, 1.0f, 0);
    return 1000;
}

void __cdecl ModelSlot_ClearPartFlag(u32 key, u32 slot) {
    if (u32 *const flags = part_flags(key, slot)) *flags &= ~0x10u;
}

void __cdecl ModelSlot_SetPartFlag(u32 key, u32 slot) {
    if (u32 *const flags = part_flags(key, slot)) *flags |= 0x10;
}

u8 *__cdecl ModelSlot_Holder(u32 slot) { return holder_of(slot); }

void __cdecl ModelSlot_Free(u32 slot) {
    u8 *const base = reinterpret_cast<u8 *>((uintptr_t)(0x00790398 + slot * 0xb0));
    if (!holder_of(slot)) return;
    u8 *const part = base + 0x48;  // (through its virtual base: the offset at +4 of its vbtable)
    Renderer_RemoveFromSlot(global<u8 *>(0x0087a6d0), nullptr, (u32)(uintptr_t)(part + at<s32>(at<const u8 *>(part, 0), 4)), 0x10);
    Instance_Dispose(holder_of(slot));
    Instance_Free(at<u8 *>(base, 0), 0);
    holder_of(slot) = nullptr;
    at<u8 *>(base, 0) = nullptr;
}

u8 *__fastcall ModelSlot_Ctor(u8 *slot) {
    u8 *const part = slot + 0x48;
    at<u32>(part, 0) = 0x006fdd00;
    at<u32>(part, 0xc) = 0x006fdcf8;
    at<u32>(part, 8) = 0x006fdce8;
    // The offsets come through the vbtable pointers just stored, read again for each store as the original does.
    at<u32>(part, at<u32>(at<u8 *>(part, 0xc), 4) + 0xc) = 0x006fdcd8;
    const u32 inner = at<u32>(at<u8 *>(part, 0xc), 4);
    at<u32>(part, inner + 8) = inner - 0x60;
    at<u32>(part, 0x5c) = 0;
    at<u32>(part, at<u32>(at<u8 *>(part, 0), 4)) = 0x006fdcc8;
    const u32 outer = at<u32>(at<u8 *>(part, 0), 4);
    at<u32>(part, outer - 4) = outer - 8;
    at<u32>(part, at<u32>(at<u8 *>(part, 0), 8) + 0x50) = 2;
    u32 *const sceneFlags = reinterpret_cast<u32 *>((uintptr_t)0x008bd508);
    u8 *base = part + at<u32>(at<u8 *>(part, 0), 8);
    SceneFlags_Touch(sceneFlags);
    at<u32>(base, 0x2c) = 0;
    at<u32>(base, 0x30) = 0;
    at<u32>(base, 0x34) = 0x3f800000;   // 1.0f
    base = part + at<u32>(at<u8 *>(part, 0), 8);
    SceneFlags_Touch(sceneFlags);
    for (u32 offset = 4; offset <= 0x10; offset += 4)
        at<u32>(base, offset) = 0x3f800000;
    at<u32>(base, 0x54) = 0xffffffff;
    return slot;
}

void __cdecl ModelSlot_ApplySelection(s32 kind) {
    const u32 slot = global<u32>(0x00f79674);
    // the byte table 0x598da4 sends kinds -1..14 to: loop, clear, set (1, 2, 4, 5, 6, 7, 9, 10, 11, 13), set 0x15 (14) or nothing
    switch (kind) {
    case 0:
        ModelSlot_ClearPartFlag(0x13, slot);
        ModelSlot_ClearPartFlag(0x14, slot);
        break;
    case 1: case 2: case 4: case 5: case 6: case 7: case 9: case 10: case 11: case 13:
        ModelSlot_SetPartFlag(0x13, slot);
        ModelSlot_SetPartFlag(0x14, slot);
        break;
    case 14:
        ModelSlot_SetPartFlag(0x15, slot);
        break;
    case -1:
        for (u32 address = 0x00f797b4; address < 0x00f797c4; address += 4) {
            const u32 clear = global<u32>(address - 0x10);
            if (clear && !(global<u32>(0x00f79660) & 0x10000))
                ModelSlot_ClearPartFlag(clear, global<u32>(0x00f79674));
            const u32 set = global<u32>(address);
            if (set)
                ModelSlot_SetPartFlag(set, global<u32>(0x00f79674));
        }
        break;
    default:
        break;
    }
    if (kind != 0 && !(global<u32>(0x00f79660) & 0x10000))
        ModelSlot_ClearPartFlag((u32)kind, global<u32>(0x00f79674));
}
}
