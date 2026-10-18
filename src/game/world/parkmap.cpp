// The park map's squares classified: see parkmap.h.
#include "parkmap.h"

#include <cstdint>
#include <cstring>

#include "../core/accessors.h"
#include "../core/ptrlist.h"
#include "../math/x87.h"
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

void __fastcall ParkMap_PlaceSprite(const u8 *map, void *, u8 *sprite) {
    const X87Reg width = static_cast<double>(global<u32>(0x007a1968)), height = static_cast<double>(global<u32>(0x007a1930));
    const X87Reg toX = X87_DivR(static_cast<double>(at<s32>(map, 0x24)), 480.0);
    const float toY = static_cast<float>(X87_DivR(static_cast<double>(at<s32>(map, 0x28)), 425.0));
    const float widthF = static_cast<float>(width), heightF = static_cast<float>(height);
    const X87Reg x = (X87Reg(at<float>(sprite, 4)) - at<float>(map, 0x54)) * toX * at<float>(map, 0x5c) + width * 0.5;
    at<float>(sprite, 0xc) = static_cast<float>(x);
    const X87Reg y = (X87Reg(at<float>(sprite, 8)) - at<float>(map, 0x58)) * toY * at<float>(map, 0x5c) + height * 0.5;
    const X87Reg left = X87Reg(at<float>(sprite, 0xc)) + at<float>(sprite, 0xc);
    const float u = static_cast<float>(left / widthF - 1.0);
    const float v = static_cast<float>((y + y) / heightF - 1.0);
    at<float>(sprite, 0x10) = static_cast<float>(y);
    const s32 layer = at<s32>(sprite, 0x14);
    const u32 depth = layer >= 0 && layer <= 2 ? 0x3f7d70a4 : 0x3f7ff972;  // 0.99 / 0.9999
    u32 ub, vb;
    std::memcpy(&ub, &u, 4);
    std::memcpy(&vb, &v, 4);
    PtrArray_SetVectorOf(reinterpret_cast<const u8 *>(0x00879ce0), nullptr, at<u8 *>(sprite, 0x28), ub, vb, depth);
    const float scale = static_cast<float>(X87Reg(at<float>(sprite, 0x24)) * at<float>(map, 0x5c) * 0.5);
    u8 *const frame = reinterpret_cast<u8 *>(static_cast<uintptr_t>(Fn_56b620(at<u32>(sprite, 0x28))));
    at<float>(frame, 0x2c) = static_cast<float>(X87Reg(at<u16>(at<u8 *>(frame, 0x28), 0xc)) * scale * 0.0625);
}

}  // extern "C"
