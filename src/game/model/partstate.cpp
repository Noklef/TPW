#include "partstate.h"

#include <cstdint>

#include "../core/globalmem.h"
#include "../sprite/script.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

u8 *model(const u8 *instance) { return at<u8 *>(instance, 4); }
PartStates *&states(u8 *instance) { return at<PartStates *>(instance, 0x28); }
u32 parts(const u8 *m) { return at<u16>(m, 0x48); }

// Face n of the model: the first +0x44 in one array, the rest in the other.
u8 *face(const u8 *m, u32 n) {
    const u32 first = at<u16>(m, 0x44);
    return n < first ? at<u8 *>(m, 0x70) + n * 0xa0 : at<u8 *>(m, 0x74) + (n - first) * 0x58;
}

}  // namespace

extern "C" {

void __cdecl PartStates_Create(u8 *instance) {
    if (!parts(model(instance))) return;
    PartStates *ps = static_cast<PartStates *>(GlobalMem_Alloc(0x14, 1));
    ps->entries = static_cast<u8 *>(GlobalMem_Alloc(parts(model(instance)) * 0x14, 1));
    states(instance) = ps;
    const u8 *part = at<const u8 *>(model(instance), 0x7c);
    u8 *entry = ps->entries;
    u32 animated = 0;
    for (u32 i = 0; i < parts(model(instance)); ++i, part += 0x14, entry += 0x14) {
        const u8 *m = model(instance);
        u8 *f = face(m, at<u16>(m, 0x46) + i);
        if (at<u8>(part, 0) & 0x30) {
            at<u32>(f, 0) |= 0x20000000;
            const u32 flags = at<u32>(entry, 0) | 1;
            at<s32>(entry, 0xc) = -1;
            at<u32>(entry, 0) = flags;
            if (at<u32>(f, 0) & 0x400) at<u32>(entry, 0) = flags | 0x10;
            if (at<u32>(part, 0) & 0x40040) at<u32>(at<u8 *>(f, 4), 0) |= 0x10000000;
        }
        if (at<u8>(part, 0) & 8) at<u32>(f, 0) |= 0x40;
        if (at<u32>(part, 0) & 0x10000) at<u32>(f, 0) |= 0x20000;
        if (at<u32>(f, 0) & 0x20000000) ++animated;
    }
    if (animated) {
        ps->flags |= 1;
        ps->slots = animated;
        ps->slotData = static_cast<u8 *>(GlobalMem_Alloc(animated << 6, 1));
    }
    PartStates_MarkNodes(instance, at<u8 *>(model(instance), 0x78), model(instance));
    u8 *slot = ps->slotData;
    entry = ps->entries;
    for (u32 n = parts(model(instance)); n; --n, entry += 0x14) {
        if (!(at<u32>(entry, 0) & 1)) continue;
        at<u8 *>(entry, 4) = slot;
        slot += 0x40;
    }
    part = at<const u8 *>(model(instance), 0x7c);
    for (u32 n = parts(model(instance)); n; --n, part += 0x14)
        if (at<u8>(part, 0) & 2) ps->flags |= 8;
}

u32 __cdecl PartStates_MarkNodes(u8 *instance, u8 *node, const u8 *model) {
    const u32 index = at<u16>(node, 0x50), first = at<u16>(model, 0x46);
    const bool part = index >= first && (s32)(index - first) < (s32)parts(model);
    u32 plain = part && (at<u8>(at<const u8 *>(model, 0x7c) + (index - first) * 0x14, 0) & 0x30) ? 0 : 1;
    if (part && !at<u8 *>(node, 0xc)) at<u32>(states(instance)->entries + (index - first) * 0x14, 0) |= 0x20;
    for (u8 *child = at<u8 *>(node, 0xc); child; child = at<u8 *>(child, 8))
        if (!PartStates_MarkNodes(instance, child, model)) plain = 0;
    if (plain) at<u32>(node, 0) |= 0x40000000;
    return plain;
}

void __cdecl PartStates_Free(u8 *instance) {
    if (!states(instance)) return;
    GlobalMem_Free(states(instance)->slotData);
    states(instance)->slotData = nullptr;
    GlobalMem_Free(states(instance)->entries);
    states(instance)->entries = nullptr;
    GlobalMem_Free(states(instance));
    states(instance) = nullptr;
}

void __cdecl PartStates_Release(u8 *instance, u32 part) {
    u8 *entry = states(instance)->entries + part * 0x14;
    --states(instance)->active;
    if (!states(instance)->active) states(instance)->flags &= ~4u;
    at<u32>(entry, 0) &= ~2u;
    SpriteScript_Kill(at<s32>(entry, 0xc));
    at<s32>(entry, 0xc) = -1;
}
}
