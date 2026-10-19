#include "collobj.h"

#include <cstdint>

#include "../math/vec2.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __cdecl CollPool_Reset() {
    u8 *next = nullptr;
    for (s32 offset = 0x4fec; offset >= 0; offset -= 0x14) {
        u8 *const object = global<u8 *>(0x00877b88) + offset;  // the pool reread each time
        at<u8 *>(object, 0x10) = next;
        next = object;
    }
    global<u8 *>(0x00877b8c) = global<u8 *>(0x00877b88);
}

void __cdecl Collider_ReleaseC0(u8 *collider) {
    for (u8 *node = at<u8 *>(collider, 0xc0); node; node = at<u8 *>(node, 0x14)) {
        --global<s32>(0x00877b74);
        at<u32>(node, 0) = 0;
    }
}

void __cdecl Collider_KeepA8(u8 *collider, u32 mask) {
    u8 *node = at<u8 *>(collider, 0xa8);
    if (!node) return;
    u8 *kept = nullptr;
    do {
        if (mask != 0 && (at<u32>(node, 0) & mask)) {
            if (!kept) {
                kept = node;
            } else {
                u8 *tail = kept;
                while (at<u8 *>(tail, 0x14)) tail = at<u8 *>(tail, 0x14);
                at<u8 *>(tail, 0x14) = node;
            }
        } else {
            --at<s32>(collider, 0xac);
            --global<s32>(0x00877b7c);
            at<u32>(node, 0) &= ~1u;
        }
        u8 *const next = at<u8 *>(node, 0x14);
        at<u8 *>(node, 0x14) = nullptr;
        node = next;
    } while (node);
    at<u8 *>(collider, 0xa8) = kept;
}

void __cdecl Pool_FreeAll(u8 *owner) {
    while (u8 *chunk = at<u8 *>(owner, 0xbc)) {
        while (u8 *node = at<u8 *>(chunk, 0x1c)) {
            at<u32>(chunk, 0x1c) = at<u32>(node, 0x14);
            global<u32>(0x00877b74)--;
            at<u32>(node, 0) &= 0xfffffffe;
        }
        at<u32>(owner, 0xbc) = at<u32>(chunk, 0x24);
        at<u32>(chunk, 0) &= 0xfffffffe;
        global<u32>(0x00877b84)--;
    }
}

// The moved point in no blocker (bit 0 and 0x40) of the table.
static bool blocked(s32 x, s32 y) {
    u8 *node = global<u8 *>(0x00877b70);
    if (!node)
        return false;
    u32 left = global<u32>(0x00877b74);
    if (!left)
        return false;
    do {
        const u32 flags = at<u32>(node, 0);
        if (flags & 1) {
            left--;
            if ((flags & 0x40) && IntArea_Contains(x, y, reinterpret_cast<const s32 *>(node)))
                return true;
        }
        node += 0x20;
    } while (left);
    return false;
}

u8 *__cdecl CollArea_FindNearest(const s32 *area, s32 x, s32 y, u32 self) {
    u8 *node = global<u8 *>(0x00877b70);
    if (!node)
        return nullptr;
    u32 left = global<u32>(0x00877b74);
    if (!left)
        return nullptr;
    s32 best = 0x7fffffff;
    u8 *nearest = nullptr;
    for (;; node += 0x20) {
        const u32 flags = at<u32>(node, 0);
        if (flags & 1) {
            left--;
            u32 mx = at<u32>(node, 4), my = at<u32>(node, 8);
            if (!(flags & 8))
                my -= 0x87c;
            else if (!(flags & 0x10))
                my += 0x87c;
            if (!(flags & 2))
                mx += 0x87c;
            else if (!(flags & 4))
                mx -= 0x87c;
            const s32 *const other = reinterpret_cast<const s32 *>(node);
            if (!(flags & 0x40) && (self || other != area) && IntArea_Overlaps(area, other) && !blocked((s32)mx, (s32)my)) {
                const s32 length = IntVec2_Length((s32)((u32)other[1] - (u32)x), (s32)((u32)other[2] - (u32)y));
                if (length < best) {
                    best = length;
                    nearest = node;
                }
            }
        }
        if (!left)
            break;
    }
    return nearest;
}
}
