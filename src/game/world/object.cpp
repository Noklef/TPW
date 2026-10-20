#include "object.h"

#include <cstdint>
#include <cstring>

#include "../core/accessors.h"
#include "../core/circbuf.h"
#include "../core/filetime.h"
#include "../core/funnytime.h"
#include "../core/gameheap.h"
#include "../core/initializers.h"
#include "../math/x87.h"
#include "../model/stripanim.h"
#include "objectdefs.h"
#include "person.h"
#include "mapcell.h"
#include "ridelist.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u8 *__fastcall Object_GetDef(const u8 *object) {
    return ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(object, 0xe));
}

void __fastcall Object_AddCustomer(u8 *object) {
    ++at<u32>(object, 0x1a0);
    ++at<u32>(object, 0x1a8);
}

void __fastcall Object_AddWalkAway(u8 *object) {
    ++at<u32>(object, 0x1a4);
    ++at<u32>(object, 0x230);
}

void __fastcall Object_AddHappinessChange(u8 *object, void *, s32 change) {
    const s32 average = at<s32>(object, 0x340);
    at<s32>(object, 0x340) = average == 0 ? change : (s32)((u32)average + (u32)change) / 2;
}

void __fastcall Object_SetQualityOfGoods(u8 *object, void *, u32 value) { at<u32>(object, 0x18c) = value & 0xff; }

void __fastcall Object_SetChanceOfWinning(u8 *object, void *, u32 value) { at<u32>(object, 0x190) = value & 0xff; }

u16 *__fastcall Object_DefWord284(const u8 *object, void *, u16 *out) {
    *out = at<u16>(Object_GetDef(object), 0x284);
    return out;
}

u32 __fastcall Object_DefIsKind3(const u8 *object) { return at<u32>(Object_GetDef(object), 0x9c) == 3; }

u32 __fastcall Object_ScrapValue(const u8 *object) {
    FileTime now, age;
    FunnyTime_Now(global<u8 *>(0x0080239c) + 0x2a0, nullptr, &now, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                  nullptr);
    FileTime_Sub(&now, nullptr, &age, reinterpret_cast<const FileTime *>(object + 0x18));
    const long long ticks = (long long)((unsigned long long)age.high << 32 | age.low);
    const u32 years = (u32)(ticks / 315360000000000LL);  // (_alldiv by 365 days; only the quotient's low dword is looked at)
    u32 column;
    if (years == 0) {
        if ((u32)(ticks / 25920000000000LL) == 0 && at<u32>(object, 0x1a0) == 0 && at<s32>(Object_GetDef(object), 0x188) > 0)
            return 100;  // (under 30 days old with no customer yet, and the first level's first-year value positive)
        column = 0x188;
    } else if (years == 1) {
        column = 0x18c;
    } else if (years == 2) {
        column = 0x190;
    } else {
        column = 0x194;
    }
    return at<u32>(Object_GetDef(object), column + at<u8>(object, 0x50) * 0x40);
}

s32 __fastcall Object_AgeDays(const u8 *object) { return FunnyTime_DaysSince(reinterpret_cast<const FileTime *>(object + 0x18)); }

u32 __fastcall Object_ScrapRefund(const u8 *object) {
    const u8 *const def = Object_GetDef(object);
    return Object_ScrapValue(object) * at<u32>(def, 0x1b8) / 100;
}

u32 __fastcall Kinds_Compatible(const u8 *object, void *, u32 kind) {
    if (kind == 0)
        return 1;
    const u32 mine = at<u32>(object, 8);
    if (kind == 4 && mine == 4)
        return 0;
    if (kind == 0x15 && mine == 0x15)
        return 1;
    if (kind == 1 && mine == 3)
        return 1;
    if (kind == mine || mine == 0)
        return 1;
    if (kind == 4 && mine == 1)
        return 1;
    if (kind == 3 && mine == 1)
        return global<u32>(0x00820ab8) == 0 ? 1 : 0;
    return 0;
}

void __fastcall Slots_AddIfMatch(u8 *object, void *, u32 id, u32 amount, u32 kind, u32 extra) {
    const s32 index = at<s32>(object, 0x3cc);
    if (index < 0 || index >= 20)
        return;
    u8 *slot = object + 0xc + 48 * (u32)index;
    if (!at<u8>(object, 0x3d9))
        return;
    if (id != at<u32>(slot, 0))
        return;
    if ((u16)kind != at<u16>(slot, 0xc))
        return;
    const u16 limit = at<u16>(object, 0x3e0);
    if (limit && (u16)extra != limit)
        return;
    at<u32>(object, 0x3dc) += amount;
}

u32 __fastcall Slots_GetValue(const u8 *object, void *, u32 slot) {
    const s32 entry = at<s32>(object, 0x1414 + 16 * slot);
    if (entry < 0)
        return 0;
    if (slot == 4) {
        const u32 index = at<u32>(object, 0x1454);
        const u8 *record = object + 20 * index;
        const u8 *def = ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(record, 0xc));
        return at<u32>(def, 0x40 * at<u8>(record, 0x15) + 0x1b8);
    }
    const u8 *record = object + 20 * (u32)entry + 0xc;
    u8 *controls = global<u8 *>(0x0080239c) + 0x1d943c;
    return at<u32>(ObjectControls_FindUncached(controls, nullptr, at<u16>(record, 0), 0), 4);
}

u32 __fastcall Obj_HasActivity(const u8 *object) {
    if (at<u32>(object, 0) || at<u32>(object, 0x6c))
        return 1;
    for (u32 offset = 0xe0; offset <= 0x100; offset += 4) {
        if (at<u32>(at<u8 *>(object, offset), 0xc))
            return 1;
    }
    if (at<u32>(object, 0x324) || at<u32>(object, 0x38c))
        return 1;
    return 0;
}

u16 *__fastcall Obj_GetTrackedHandle(u8 *object, void *, u16 *out) {
    const u16 handle = at<u16>(object, 0x5e);
    if (handle == 0) {
        *out = 0;
        return out;
    }
    if (global<u32>(global<u32>(0x0080239c) + 0x1da70c) > at<u32>(object, 0x60) + 0x64) {
        const u8 *thing = global<u8 *>(0x007cfb90 + 20 * (u32)handle);
        if (!Person_HasWord(thing, nullptr, at<u16>(object, 0))) {
            at<u16>(object, 0x5e) = 0;
            at<u32>(object, 0x60) = 0;
            *out = 0;
            return out;
        }
    }
    *out = at<u16>(object, 0x5e);
    return out;
}

u32 __fastcall Is19c_Eq1_4e0370(const u8 *o) {
    return at<u32>(o, 0x19c) == 0x1 ? 1 : 0;
}

u32 __fastcall Is19c_Eq4_4e0380(const u8 *o) {
    return at<u32>(o, 0x19c) == 0x4 ? 1 : 0;
}

u32 __fastcall Is19c_Eq2_4e03e0(const u8 *o) {
    return at<u32>(o, 0x19c) == 0x2 ? 1 : 0;
}

u32 __fastcall Is68_Zero_4e0440(const u8 *o) {
    return at<u32>(o, 0x68) == 0 ? 1 : 0;
}

void __fastcall Object_SetCapacity(u8 *object, void *, u32 capacity) {
    u8 *const defs = reinterpret_cast<u8 *>(0x007890a0);
    const u16 id = at<u16>(object, 0xe);
    const u8 *first = ObjectDefs_Find(defs, nullptr, id);
    const u8 *second = ObjectDefs_Find(defs, nullptr, id);
    u8 value = (u8)capacity;
    if (at<s32>(second, 0x128) + at<s32>(first, 0x124) > 0) {
        if ((s32)value < at<s32>(ObjectDefs_Find(defs, nullptr, id), 0x124)) {
            value = at<u8>(ObjectDefs_Find(defs, nullptr, id), 0x124);
        } else if ((s32)value > at<s32>(ObjectDefs_Find(defs, nullptr, id), 0x128)) {
            value = at<u8>(ObjectDefs_Find(defs, nullptr, id), 0x128);
        }
    }
    Rides_SetElement(Rides_FindById(at<s32>(object, 0x24)), 2, value);  // (the debug print of the capacity is left out)
    at<u8>(object, 0x5d) = value;
}

void __fastcall Object_RestartRide(u8 *object) {
    at<u32>(object, 0x68) = 0;
    at<u16>(object, 0x6c) = 0;
    Rides_SetElement(Rides_FindById(at<s32>(object, 0x24)), 6, 1);
    StripAnim_StartGrow(reinterpret_cast<u8 *const *>(0x007a4610)[at<u32>(object, 0x20)], 1);
}

u32 __cdecl Object_TrackNeighboursOk(const u8 *object) {
    const s32 track = at<s32>(Object_GetDef(object), 0x9c);
    if (track <= 0 || track > 2)
        return 1;
    s32 x = object[5] & 0x7f, y = object[7] + (object[5] >> 7);
    const s32 angle = at<s32>(object, 0x10);
    // the cell two or so ahead along the track's direction, then the one beside it (Nibble_Scale of the direction in between has no effect)
    auto cellAt = [](s32 column, s32 row) {
        const u32 id = (u32)(((row << 7) + column + 1) & 0xffff);
        return global<u8 *>(0x007cf83c) + (id + 0x6cde) * 0x28;
    };
    s32 dx = 0, dy = 0;
    switch (angle) {
    case 0: dx = 4; dy = 1; break;
    case 0x5a: dx = 1; dy = -5; break;
    case 0xb4: dx = -5; dy = -2; break;
    case 0x10e: dx = -2; dy = 4; break;
    default: break;
    }
    if (MapCell_NeighbourCount(cellAt(x + dx, y + dy)) != 1)
        return 0;
    switch (angle) {
    case 0: dx = -2; dy = 1; break;
    case 0x5a: dx = 1; dy = 1; break;
    case 0xb4: dx = 1; dy = -2; break;
    case 0x10e: dx = -2; dy = -2; break;
    default: dx = 0; dy = 0; break;
    }
    return MapCell_NeighbourCount(cellAt(x + dx, y + dy)) == 1 ? 1 : 0;
}

u32 __fastcall Obj_BlockValuesBelowLimit(const u8 *object) {
    const u32 id = ((u32)at<u8>(object, 7) * 0x80 + at<u8>(object, 5) + 1) & 0xffff;
    u16 back = (u16)id;
    if (MapCell_IsType3Or9(global<u8 *>(0x008023a0) + id * 0x44 - 0x44)) {
        u16 cell = (u16)(((u32)at<u8>(object, 7) << 7) + at<u8>(object, 5) + 1);
        back = cell;
        while (cell != 0) {
            back = cell;
            u16 next;
            cell = *Map_FindFacingNeighbour(&next, cell);
        }
    }
    u32 best = 0;
    for (u32 k = 0; k < 9; ++k) {
        if (best > at<u32>(object, 0x198)) break;
        const u8 *const pair = reinterpret_cast<const u8 *>(0x007ced88 + k * 8);
        const u16 pos = (u16)((u16)(at<u16>(pair, 4) << 7) + back + at<u16>(pair, 0));
        const u32 value = Map_BlockValue(global<u8 *>(0x0080239c) + 0x2d8, nullptr, pos);
        if (!(best > value)) best = value;
    }
    return best < at<u32>(object, 0x198) ? 1 : 0;
}

u32 __fastcall Obj_TrackedPersonInState(u8 *object) {
    u16 tracked;
    if (*Obj_GetTrackedHandle(object, nullptr, &tracked) == 0) return 0;
    const u8 *person = global<u8 *>(0x007cfb90 + 20 * (u32)at<u16>(object, 0x5e));
    return Person_HasWordInState(person, nullptr, at<u16>(object, 0));
}

void __fastcall Objects_ReleaseTracked(void *, void *, u32, u32) {
    u16 handle = global<u16>(global<u32>(0x0080239c) + 0x1da746);
    while (handle) {
        u8 *object = global<u8 *>(0x007cfb90 + 20 * (u32)handle);
        if (Is19c_Eq1_4e0370(object) || Get64_4e03d0(object) || Is19c_Eq2_4e03e0(object)) {
            u16 tracked;
            if (*Obj_GetTrackedHandle(object, nullptr, &tracked)) Fn_4e0210(object);
        }
        handle = at<u16>(object, 0xc);
    }
}

u16 *__fastcall Objects_NearestFreeFor(u8 *self, void *, u16 *out, u32 person) {
    u16 here = 0;
    if (at<u16>(self, 0xc) != 0) {
        here = global<u16>(global<u32>(0x0080239c) + 0x1da746);
        while (here != 0 && here != at<u16>(self, 0xc)) here = at<u16>(global<u8 *>(0x007cfb90 + 20 * (u32)here), 0xc);
    }
    if (here != 0)  // the cursor is in the list: on to the one after it
        at<u16>(self, 0xc) = at<u16>(global<u8 *>(0x007cfb90 + 20 * (u32)at<u16>(self, 0xc)), 0xc);
    else
        at<u16>(self, 0xc) = global<u16>(global<u32>(0x0080239c) + 0x1da746);
    const u16 start = at<u16>(self, 0xc);
    const u8 *const who = global<u8 *>(0x007cfb90 + 20 * (person & 0xffff));
    u32 best = 0xffffffff;
    u16 nearest = 0;
    if (at<u16>(self, 0xc) == 0) {
        *out = 0;
        return out;
    }
    do {
        u8 *const object = global<u8 *>(0x007cfb90 + 20 * (u32)at<u16>(self, 0xc));
        const u16 next = at<u16>(object, 0xc);
        at<u16>(self, 0xc) = next;
        if (next == 0) at<u16>(self, 0xc) = global<u16>(global<u32>(0x0080239c) + 0x1da746);
        if (Is19c_Eq1_4e0370(object) || (Get64_4e03d0(object) && !(at<u8>(object, 0x32) & 1)) || Is19c_Eq2_4e03e0(object)) {
            u16 tracked;
            bool free = *Obj_GetTrackedHandle(object, nullptr, &tracked) == 0;
            if (!free) {
                u16 again;
                free = *Obj_GetTrackedHandle(object, nullptr, &again) == (u16)person;
            }
            if (free) {
                const u32 from = ((((u32)at<u8>(who, 7) << 7) + at<u8>(who, 5) + 1) & 0xffff) - 1;
                const u32 to = ((((u32)at<u8>(object, 7) << 7) + at<u8>(object, 5) + 1) & 0xffff) - 1;
                s32 dx = (s32)(from & 0x7f) - (s32)(to & 0x7f);
                s32 dy = (s32)(from >> 7) - (s32)(to >> 7);
                if (dx < 0) dx = -dx;
                if (dy < 0) dy = -dy;
                const u32 distance = (u32)(dx * dx) + (u32)(dy * dy);
                if (distance < best) {
                    best = distance;
                    nearest = at<u16>(object, 0);
                }
            }
        }
    } while (at<u16>(self, 0xc) != start);
    *out = nearest;
    return out;
}

u32 __fastcall Object_ExitOpen(const u8 *object) {
    const u16 exit = at<u16>(object, 0x38);
    const u8 *cell = global<u8 *>(0x008023a0) + (u32)exit * 0x44 - 0x44;
    const u8 way = exit == at<u16>(object, 0x36) ? (u8)Nibble_Scale(MapCell_Direction(cell)) : MapCell_Direction(cell);
    const u16 beyond = *reinterpret_cast<const u16 *>(Map_StepCellInDirection(reinterpret_cast<const u16 *>(cell), nullptr, way));
    static const u8 codes[4] = {1, 4, 0x10, 0x40};
    for (u32 d = 0; d < 4; ++d) {
        if (codes[d] == (u8)Nibble_Scale(way))  // (not back the way it came)
            continue;
        if (!MapCell_StepBlocked((u8)((u8)beyond - 1) & 0x7f, (u16)(beyond - 1) >> 7, d, 0))
            return 1;  // (the debug print is left out)
    }
    return 0;
}

u32 __fastcall Obj_StampAt5e(u8 *object, void *, u32 word) {
    at<u16>(object, 0x5e) = (u16)word;
    const u32 stamp = at<u32>(global<u8 *>(0x0080239c), 0x1da70c);
    at<u32>(object, 0x60) = stamp;
    return stamp;
}

u32 __fastcall Obj_Diff180_184(const u8 *object) { return at<const u32>(object, 0x180) - at<const u32>(object, 0x184); }

double __fastcall Object_DefRowScore(const u8 *object, void *, float a, float b, u32, u32 useB) {
    u8 *const defs = reinterpret_cast<u8 *>(0x007890a0);
    const u16 id = at<u16>(object, 0xe);
    const u32 row = (u32)at<u8>(object, 0x50) << 6;
    // element 5 of the ride record, through fild and __ftol (exact)
    s32 load = (s32)Rides_GetElement(Rides_FindById(at<s32>(object, 0x24)), 5);
    if (load < 0) {
        load = 0;
    } else if (load > at<s32>(ObjectDefs_Find(defs, nullptr, id), 0x128)) {
        load = at<s32>(ObjectDefs_Find(defs, nullptr, id), 0x128);
    }
    const float share = (float)(X87Reg(a) / X87Reg((double)at<s32>(ObjectDefs_Find(defs, nullptr, id), 0x138)) * X87Reg(0.9f));
    const bool aOver = !((double)at<s32>(ObjectDefs_Find(defs, nullptr, id), row + 0x1ac) > (double)a);
    X87Reg first = X87Reg(share) - X87Reg(-0.1f);
    if (aOver) first = (first - X87Reg(-1.0f)) * X87Reg(0.5f);
    const float firstStored = (float)first;
    X87Reg q = useB & 0xff ? X87Reg(b) / X87Reg((double)at<s32>(ObjectDefs_Find(defs, nullptr, id), 0x128))
                           : X87Reg((double)load) / X87Reg((double)at<s32>(ObjectDefs_Find(defs, nullptr, id), 0x128));
    const float second = (float)(q * X87Reg(0.9f));
    const double tenth = X87_MulIntDouble(load, 0.1f);
    const bool loadOver = !((double)at<s32>(ObjectDefs_Find(defs, nullptr, id), row + 0x19c) > tenth);
    X87Reg next = X87Reg(second) - X87Reg(-0.1f);
    if (loadOver) next = (next - X87Reg(-1.0f)) * X87Reg(0.5f);
    const float secondStored = (float)next;
    const X87Reg sum = X87Reg(secondStored) + X87Reg(firstStored);
    return (X87Reg(X87_MulIntDouble(at<s32>(ObjectDefs_Find(defs, nullptr, id), row + 0x1b0), sum.v)) * X87Reg(0.5f)).v;
}

u8 __fastcall Object_DefRowPercent(const u8 *object, void *, float a, float b, u32 c) {
    const X87Reg score = X87Reg(Object_DefRowScore(object, nullptr, a, b, c, 1)) * X87Reg(9.0f);
    return (u8)(0x64 - (u32)(long long)score.v);  // __ftol, low half
}

void __fastcall Object_CellStandPoint(const u8 *object, void *, u32 exit, u8 *x, u8 *y) {
    u8 *const defs = reinterpret_cast<u8 *>(0x007890a0);
    const u16 id = at<u16>(object, 0xe);
    const u32 cell = at<u16>(object, exit ? 0x38 : 0x36);
    x[1] = (u8)((cell - 1) & 0x7f);
    y[1] = (u8)((cell - 1) >> 7);
    // (the debug prints of a point outside (0, 1) are left out)
    const float px = at<float>(ObjectDefs_Find(defs, nullptr, id), exit ? 0xdc : 0xd4);
    const float py = at<float>(ObjectDefs_Find(defs, nullptr, id), exit ? 0xe0 : 0xd8);
    X87Reg first = 0.0;
    float second = px;
    switch (at<s32>(object, 0x10)) {
    case 0: first = X87Reg(px); second = py; break;
    case 0x5a: first = X87Reg(py); second = (float)(X87Reg(1.0f) - X87Reg(px)); break;
    case 0xb4: first = X87Reg(1.0f) - X87Reg(px); second = (float)(X87Reg(1.0f) - X87Reg(py)); break;
    case 0x10e: first = X87Reg(1.0f) - X87Reg(py); second = px; break;
    default: first = X87Reg(px); second = px; break;  // (sic: both from the x point)
    }
    *x = (u8)(long long)X87_MulDouble(first.v, 255.0);  // __ftol, the low byte
    *y = (u8)(long long)X87_MulFloatDouble(second, 255.0);
}

u32 __fastcall Object_HistoryPercent(const u8 *object) {
    // (each sum's debug print of whether 30 is within the size is left out; the second and third sums are CircBufU32_30_Sum inlined, in a register)
    double first, second, third;
    CircBufU32_30_Sum(object + 0x1a8, nullptr, &first, 0x1e);
    if ((u32)(long long)first == 0) return 0;  // __ftol, the low half
    CircBufU32_30_Sum(object + 0x2b8, nullptr, &second, 0x1e);
    CircBufU32_30_Sum(object + 0x1a8, nullptr, &third, 0x1e);
    return (u32)(long long)second * 100u / (u32)(long long)third;
}
}
