// A map cell's model picked from its neighbours: see cellmodel.h.
#include "cellmodel.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/crt.h"
#include "../motion/animator.h"
#include "../terrain/land.h"
#include "mapcell.h"
#include "queuepieces.h"
#include "random.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// The table's model for the bits: the first row with exactly them, else (none, or its model -1: sic) the first with bits all among them;
// -1 for none.
u32 search(u32 table, s32 rows, u8 bits, s32 turn, s32 *angle) {
    for (s32 i = 0; i < rows; ++i) {
        const u32 row = table + i * 12;
        if (global<u8>(row + 8) == bits) {
            *angle = turn + global<s32>(row + 4);
            if (global<u32>(row) != 0xffffffffu) return global<u32>(row);
            break;
        }
    }
    for (s32 i = 0; i < rows; ++i) {
        const u32 row = table + i * 12;
        const u8 need = global<u8>(row + 8);
        if ((bits & need) == need) {
            *angle = global<s32>(row + 4) + turn;
            return global<u32>(row);
        }
    }
    return 0xffffffffu;
}

// A straight cell facing 0x40 or 0x10 turns by 180.
s32 turn_of(const u8 *cell, u8 direction) {
    return MapCell_StraightNeighbours(cell) && (direction == 0x40 || direction == 0x10) ? 0xb4 : 0;
}

// The ports of 0x522b90 / 0x522d50 / 0x522f20 (agent A's queuepieces); their unsigned arguments pass the same dwords.
using PlaceFn = u32(__cdecl *)(s32, s32, s32, s32, s32, u32);
const CellModelEnv kGameEnv = {reinterpret_cast<PlaceFn>(&TrackQueuePiece_Place), reinterpret_cast<PlaceFn>(&TrackQueuePiece_PlaceAnimated),
                               reinterpret_cast<PlaceFn>(&EndPiece_Place)};
const CellModelEnv *g_env = &kGameEnv;

// The cell's ride (+4), if any, removed (QueuePiece_Remove) and the field cleared.
void remove_ride(u8 *cell) {
    if (!at<u32>(cell, 4)) return;
    QueuePiece_Remove(at<u32>(cell, 4));
    at<u32>(cell, 4) = 0;
}

}  // namespace

extern "C" {

u32 __fastcall MapCell_PickModel(const u8 *cell, void *, s32 *angle) {
    u8 bits = MapCell_Neighbours(cell);
    const s32 type = at<s32>(cell, 8);
    const u8 direction = MapCell_Direction(cell);
    *angle = 0;
    const u32 size = static_cast<u32>(type < 0 ? -type : type);
    switch (size) {
    case 4: case 5: case 7: case 9: case 0xa: case 0xc: case 0x11: return 8;
    default: break;
    }
    u32 model;
    switch (size) {
    case 3:
    case 0xb: model = search(0x00762f30, global<s32>(0x00700c18), bits, turn_of(cell, direction), angle); break;
    case 0x10: model = search(0x00762ff0, global<s32>(0x00700c1c), bits, turn_of(cell, direction), angle); break;
    case 0x19: {
        const u32 index = static_cast<u32>(at<u16>(cell, 0)) - 1;
        const s32 x = static_cast<s32>(index & 0x7f), y = static_cast<s32>(index >> 7);
        const s32 turn = turn_of(cell, direction);
        if ((x == global<s32>(0x007639f0) && y == global<s32>(0x007639f4)) || (x == global<s32>(0x007639f8) && y == global<s32>(0x007639fc)))
            bits |= static_cast<u8>(Nibble_Scale(bits));
        model = search(0x00762ff0, global<s32>(0x00700c1c), bits, turn, angle);
        break;
    }
    default: model = search(0x00763138, global<s32>(0x00700c24), bits, 0, angle); break;
    }

    if (!MapCell_CornerNeighbours(cell) && !MapCell_JunctionNeighbours(cell)) {
        const u16 flags = at<u16>(cell, 0xe);
        if (flags & 1) model += 9;
        if (flags & 2) model += 0xc;
        if (flags & 8) model += 0xf;
        if (flags & 4) model = (model & 0xffff0000u) + 0x12;
    }
    if (type == 0x10 || type == 0xb) {
        if (MapCell_CornerNeighbours(cell)) {
            const u8 d = MapCell_Direction(cell), b = MapCell_Neighbours(cell);
            if ((d == 0x40 && b == 0x50) || (d == 0x10 && b == 0x14) || (d == 1 && b == 0x41) || (d == 4 && b == 5)) model += 0x10;
        }
    }
    if (type == 0x10 && MapCell_JunctionNeighbours(cell)) {
        const u8 d = MapCell_Direction(cell);
        if (d == 0x14) *angle = 0x5a;
        if (d == 0x41) model += 0x10;
        if (d == 0x50) {
            model += 0x10;
            *angle = 0x10e;
        }
    }
    if (type != 0x19) return model;

    if (MapCell_CornerNeighbours(cell)) {
        const u8 d = MapCell_Direction(cell), b = MapCell_Neighbours(cell);
        if ((d == 1 && b == 0x41) || (d == 0x10 && b == 0x14) || (d == 4 && b == 5) || (d == 0x40 && (b == 0x50 || b == 0x14)) || (d == 1 && b == 0x50) ||
            (d == 4 && b == 0x41) || (d == 0x10 && b == 5)) {
            model += 0x10;
            if (at<u8>(cell, 0xe) & 0xf) model = 0x1b;
        } else if (at<u8>(cell, 0xe) & 0xf) {
            model = 0x1a;
        }
    }
    if (MapCell_StraightNeighbours(cell) && !(at<u8>(cell, 0xe) & 0xf)) {
        const s32 counter = global<s32>(0x00820aec);
        const u32 variant = counter < 0x12 ? 0 : counter < 0x18 ? 0x14 : counter < 0x1e ? 0x17 : static_cast<u32>(reinterpret_cast<uintptr_t>(angle));
        if (global<u32>(0x00820af0) != variant) model += variant;
        global<u32>(0x00820af0) = variant;
        global<u32>(0x00820aec) = static_cast<u32>(World_Rand(global<u8 *>(0x007cf83c))) % 30;
    }
    return (model & 0xffff) | 0x50000;
}

u32 __fastcall MapCell_PickPathModel(const u8 *cell, void *, s32 *angle) {
    const u8 bits = MapCell_Neighbours(cell);
    const s32 type = at<s32>(cell, 8);
    const u8 direction = MapCell_Direction(cell);
    *angle = 0;
    const u32 size = static_cast<u32>(type < 0 ? -type : type);
    switch (size) {
    case 2: case 4: case 7: case 9: case 0xa: case 0x15: case 0x1e: return 8;
    default: break;
    }
    u32 model;
    if (size == 3) {
        const s32 turn = MapCell_StraightNeighbours(cell) && direction != 0x40 && direction != 0x10 ? 0xb4 : 0;
        model = search(0x007630b0, global<s32>(0x00700c20), bits, turn, angle);
    } else {
        model = search(0x00763138, global<s32>(0x00700c24), bits, 0, angle);
    }
    if (type == 3 && MapCell_CornerNeighbours(cell)) {
        const u8 d = MapCell_Direction(cell), b = MapCell_Neighbours(cell);
        if ((d == 0x40 && b == 0x50) || (d == 0x10 && b == 0x14) || (d == 1 && b == 0x41) || (d == 4 && b == 5)) ++model;
    }
    if (global<u8>(0x00820ac0) && (model & 0xffff0000u) == 0x10000) {
        if ((model & 0xffff) == 2) model += 0x11;
        if ((model & 0xffff) == 0xa) model += 0xa;
    }
    global<u8>(0x00820ac0) = static_cast<u8>(Crt().rand() % 2);
    if ((model & 0xffff0000u) != 0x20000) return model;
    const u16 *const handle = reinterpret_cast<const u16 *>(cell);
    u8 side = 1;
    do {
        if (side & MapCell_Neighbours(cell)) {
            const u8 *const next = Map_StepCellInDirection(handle, nullptr, side);
            if (next && at<s32>(next, 8) == 1 && (MapCell_Neighbours(next) & static_cast<u8>(Nibble_Scale(side)))) {
                const u8 *entry = MapHandle_Entry28(handle);
                if (MapCell_IsTypeCOr11(entry))
                    entry = global<u8 *>(0x007cf83c) + (static_cast<u32>(at<u16>(MapHandle_Entry28(handle), 0x10)) + 0x6cde) * 0x28;
                if (!MapCell_HasLowFlags(entry)) model += 3;
            }
        }
        side = static_cast<u8>(Byte_ShiftLog2(side, 4));
    } while (side != 1);
    return model;
}

void __fastcall MapCell_ApplyPathModel(u8 *cell) {
    s32 angle = 0;   // (MapCell_PickPathModel sets it first)
    const s32 kind = at<s32>(cell, 8);
    u32 model = 0x37;
    if (kind > 0 && kind != 5)
        model = MapCell_PickPathModel(cell, nullptr, &angle);
    s32 *const paint = reinterpret_cast<s32 *>(0x00820aa8);
    paint[0] = static_cast<s32>(model) >> 16;
    paint[1] = static_cast<s32>(model & 0xffff);
    paint[2] = angle;
    const u32 index = static_cast<u32>(at<u16>(cell, 0)) - 1;
    const s32 x = static_cast<s32>(index & 0x7f), y = static_cast<s32>(index >> 7);
    at<s32>(cell, 0x14) = paint[0];
    at<s32>(cell, 0x18) = paint[1];
    at<s32>(cell, 0x1c) = paint[2];
    if (at<s32>(cell, 0x14) == 2) {
        if (at<u32>(cell, 4)) {
            QueuePiece_Remove(at<u32>(cell, 4));
            at<u32>(cell, 4) = 0;
        }
        at<u32>(cell, 4) = QueuePiece_Place(x, y, at<u32>(cell, 0x14), at<u32>(cell, 0x18), at<u32>(cell, 0x1c), 0);
        paint[1] = 8;   // (the erasing tile: set 0, tile 8)
        paint[0] = 0;
        paint[2] = 0;
        Terrain_PaintAt(x, y, paint);
        return;
    }
    if (at<u32>(cell, 4)) {
        QueuePiece_Remove(at<u32>(cell, 4));
        at<u32>(cell, 4) = 0;
    }
    Terrain_PaintAt(x, y, reinterpret_cast<const s32 *>(cell + 0x14));
}

void CellModel_SetEnv(const CellModelEnv *env) { g_env = env ? env : &kGameEnv; }

void __fastcall MapCell_ApplyModel(u8 *cell) {
    if (global<u8>(0x0081b4cc)) {
        const s32 kind = at<s32>(cell, 8);
        if (kind == 0xb || kind == 0x10 || kind == 0xc || kind == 0x11) return;
        if (at<u32>(MapHandle_Cell44(reinterpret_cast<const u16 *>(cell)), 8) == 0x15) return;
    }
    s32 angle = 0;
    u32 model = 0x37;
    const s32 kind = at<s32>(cell, 8);
    if (kind > 0 && kind != 5)
        model = MapCell_PickModel(cell, nullptr, &angle);
    s32 *const kept = reinterpret_cast<s32 *>(0x00820ad8);
    kept[0] = static_cast<s32>(model) >> 16;
    kept[1] = static_cast<s32>(model & 0xffff);
    kept[2] = angle;
    at<s32>(cell, 0x14) = kept[0];
    at<s32>(cell, 0x18) = kept[1];
    at<s32>(cell, 0x1c) = kept[2];
    const u32 index = static_cast<u32>(at<u16>(cell, 0)) - 1;
    const s32 x = static_cast<s32>(index & 0x7f), y = static_cast<s32>(index >> 7);
    const s32 group = at<s32>(cell, 0x14);
    if (group == 3 || group == 4) {
        remove_ride(cell);
        if ((at<u8>(cell, 0xe) & 0x10) || !at<u32>(cell, 0x18)) return;
        const auto place = group == 3 ? g_env->place3 : g_env->place4;
        const u32 ride = place(x, y, at<s32>(cell, 0x14), at<s32>(cell, 0x18), at<s32>(cell, 0x1c), 0);
        at<u32>(cell, 4) = ride;
        if (group == 4 && ride) Animator_Play(global<u8 *>(0x007a4610 + 4 * ride), 0xc, 0, 7, 1.0f, 0);
    } else if (group == 5) {
        if (at<u32>(cell, 4) && at<u32>(MapHandle_Cell44(reinterpret_cast<const u16 *>(cell)), 8) != 0x15) remove_ride(cell);
        at<u32>(cell, 4) = g_env->place5(x, y, at<s32>(cell, 0x14), at<s32>(cell, 0x18), at<s32>(cell, 0x1c), 0);
    } else if (group == 0) {
        if (at<u32>(cell, 0x18) == 0x37) remove_ride(cell);
    }
}

}  // extern "C"
