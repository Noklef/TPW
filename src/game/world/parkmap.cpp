// The park map's squares classified: see parkmap.h.
#include "parkmap.h"

#include <cstdint>

#include "mapcell.h"
#include "object.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *cell44(u32 id) { return global<u8 *>(0x007cf83c) + 0x2d8 + (id & 0xffff) * 0x44 - 0x44; }
u8 *thing(u32 handle) { return global<u8 *>(0x007cfb90 + (handle & 0xffff) * 20); }

// The owner's things: the first that tells a kind sets the entry; 1 when one did.
void classify_things(u8 *entry, u8 *cell, u16 owner) {
    for (u16 handle = at<u16>(cell44(owner), 0x24); handle != 0; handle = at<u16>(thing(handle), 0xa)) {
        u8 *const t = thing(handle);
        if (t[2] != 3) continue;
        u32 kind;
        switch (at<u32>(Object_GetDef(t), 0x4ac)) {
        case 0:
            kind = !MapCell_IsType4(cell) && (MapHandle_IsTypeBDOr10Or12Or19(reinterpret_cast<const u16 *>(cell)) || MapCell_IsType15(cell)) ? 8 : 1;
            break;
        case 1: kind = 2; break;
        case 2: kind = 3; break;
        case 3: {
            const u8 flags = t[0x32];
            kind = flags & 0x41 ? 5 : flags & 0x10 ? 6 : flags & 2 ? 7 : 4;
            break;
        }
        default: continue;
        }
        at<u32>(entry, 0) = kind;
        at<u16>(entry, 4) = handle;
        return;
    }
}

}  // namespace

extern "C" {

void __fastcall ParkMap_Classify(u8 *map) {
    for (s32 y = 0; y < at<s32>(map, 0x28); ++y) {
        for (s32 x = 0; x < at<s32>(map, 0x24); ++x) {
            const s32 row = y * 0x80 + 0xc;
            u8 *const entry = x >= 0 && x < 0x80 && row >= 0xc && row < 0x400c ? map + (x + row) * 8 : nullptr;
            at<u32>(entry, 0) = 0;
            at<u16>(entry, 4) = 0;
            u8 *const cell = cell44((static_cast<u32>(y) << 7) + 1 + static_cast<u32>(x));
            const u16 *const handle = reinterpret_cast<const u16 *>(cell);
            if (MapHandle_ParentIsType19(handle)) {
                at<u32>(entry, 0) = 9;
            } else if (MapCell_IsType4(cell) || MapCell_IsType9(cell) || MapCell_IsTypeA(cell) || MapHandle_IsTypeBDOr10Or12Or19(handle) ||
                       MapCell_IsType15(cell)) {
                u16 owner;
                if (MapHandle_IsTypeBDOr10Or12Or19(handle)) {
                    const u8 *e = MapHandle_Entry28(handle);
                    if (MapCell_IsTypeCOr11(e)) e = global<u8 *>(0x007cf83c) + (static_cast<u32>(at<u16>(e, 0x10)) + 0x6cde) * 0x28;
                    owner = at<u16>(e, 0x10);
                } else {
                    owner = at<u16>(cell, 0x10);
                }
                classify_things(entry, cell, owner);
            } else if (MapCell_IsType1(cell)) {
                at<u32>(entry, 0) = 0xa;
            } else if (MapCell_IsType3Or9(cell)) {
                at<u32>(entry, 0) = 0xb;
            } else if (MapCell_HasFlags(cell, nullptr, 0x40)) {
                at<u32>(entry, 0) = 0xc;
            }
        }
    }
}

}  // extern "C"
