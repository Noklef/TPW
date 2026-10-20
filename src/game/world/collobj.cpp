#include "collobj.h"

#include <cstdint>

#include "../math/vec2.h"
#include "../math/x87.h"

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

s32 __cdecl Collider_SteerTo(u8 *car, s32 x, s32 y) {
    if ((at<u32>(car, 0) & 0x10000) && at<u32>(car, 0x98)) {
        u8 *found = nullptr;
        u8 *node = global<u8 *>(0x00877b70);
        u32 left = global<u32>(0x00877b74);
        if (node && left) {
            for (;;) {
                const u32 flags = at<u32>(node, 0);
                if (flags & 1) {
                    --left;
                    if (!(flags & 0x40) && IntArea_Contains(x, y, reinterpret_cast<const s32 *>(node))) {
                        found = node;
                        break;
                    }
                }
                node += 0x20;
                if (!left)
                    break;
            }
        }
        if (!found) {
            at<s32>(car, 0x70) = y;
            at<s32>(car, 0x6c) = x;
            return -1;
        }
        if (IntArea_Overlaps(at<const s32 *>(car, 0x98), reinterpret_cast<const s32 *>(found))) {
            at<u32>(car, 0x70) = at<u32>(car, 0x78) + (u32)y;
            at<u32>(car, 0x6c) = at<u32>(car, 0x74) + (u32)x;
            at<u32>(car, 0) &= ~0x10000u;
        } else {
            const u8 *const to = CollArea_FindNearest(at<const s32 *>(car, 0x98), x, y, 0);
            if (!to) {   // (the original reports it to the empty Debug_Printf)
                at<u32>(car, 0x40) = 0;
                at<u32>(car, 0x3c) = 0;
                return -1;
            }
            const u32 flags = at<u32>(to, 0);
            u32 tx = at<u32>(to, 4), ty = at<u32>(to, 8);
            if (!(flags & 8))
                ty -= 0x87c;
            else if (!(flags & 0x10))
                ty += 0x87c;
            if (!(flags & 2))
                tx += 0x87c;
            else if (!(flags & 4))
                tx -= 0x87c;
            at<u32>(car, 0x6c) = tx;
            at<u32>(car, 0x70) = ty;
            at<u32>(car, 0) = (at<u32>(car, 0) | 0x20000) & ~0x10000u;
        }
    }
    const s32 dy = (s32)(at<u32>(car, 0x70) - at<u32>(car, 0x38)), dx = (s32)(at<u32>(car, 0x6c) - at<u32>(car, 0x34));
    const double turned = X87_Atan2((double)dx, (double)(s32)(0u - (u32)dy));   // (fpatan: unrounded)
    const s32 bearing = (s32)(long long)X87_Add(X87_MulDouble(X87_MulDouble(turned, global<double>(0x00700eb8)), global<double>(0x00700ec0)),
                                               global<double>(0x00700f18));   // (x / pi * 256 - 256, then __ftol)
    s32 turn = (s32)((0u - at<u32>(car, 0x50) - (u32)bearing) & 0x1ff);
    s32 ax = dx < 0 ? (s32)(0u - (u32)dx) : dx, ay = dy < 0 ? (s32)(0u - (u32)dy) : dy;
    u32 shift = 0;
    while (ax > 0x8000 || ay > 0x7fff) {
        ++shift;
        ax >>= 1;
        ay >>= 1;
    }
    const s32 squares = (s32)((u32)ax * (u32)ax + (u32)ay * (u32)ay);
    const s32 distance = (s32)((u32)(s32)(long long)X87_Sqrt(X87Reg((double)squares)).v << shift);
    const u8 *const kind = at<const u8 *>(car, 0x9c);
    const s32 rate = at<s32>(kind, 0x28);
    s32 step = rate;
    if (turn > 0x100) {
        step = (s32)(0u - (u32)rate);
        turn = 0x200 - turn;
    }
    if (turn > rate / 2 && at<s32>(kind, 0x54) != 2 && (at<u32>(car, 0) & 0x4000))
        at<u32>(car, 0x50) = (at<u32>(car, 0x50) + (u32)step) & 0x1ff;
    const s32 reach = distance / 512, limit = reach < 0x16 ? 0x16 : reach > 0x30 ? 0x30 : reach;
    if (turn < limit && (at<u32>(car, 0) & 0x84000) == 0x4000 && at<s32>(kind, 0x54) != 2) {
        s32 speed = (s32)(at<u32>(kind, 0x20) + at<u32>(car, 0x68));
        if ((at<u32>(at<const u8 *>(car, 0x98), 0) & 0x1e) != 0x1e)
            speed = (s32)((u32)speed * 3) / 4;
        const u32 heading = at<u32>(car, 0x50);
        const s32 *const sine = reinterpret_cast<const s32 *>((uintptr_t)0x00877358);
        const s32 vy = (s32)((u32)sine[(heading + 0x80) & 0x1ff] * (u32)speed << 2) / 1024;
        const s32 vx = (s32)((u32)sine[heading & 0x1ff] * (u32)speed << 2) / 1024;
        at<u32>(car, 0x3c) += (u32)vx;
        at<u32>(car, 0x40) += (u32)vy;
    }
    return distance;
}
}
