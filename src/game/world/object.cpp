#include "object.h"

#include <cstdint>
#include <cstring>

#include "../coaster/registry.h"
#include "../core/accessors.h"
#include "../core/circbuf.h"
#include "../core/filetime.h"
#include "../core/funnytime.h"
#include "../core/gameheap.h"
#include "../core/initializers.h"
#include "../core/options.h"
#include "../core/savegame.h"
#include "../core/vcall.h"
#include "../math/x87.h"
#include "../model/stripanim.h"
#include "objectdefs.h"
#include "person.h"
#include "mapcell.h"
#include "ridelist.h"
#include "money.h"
#include "things.h"
#include "thingscan.h"
#include "trackrides.h"
#include "world.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const ObjectEnv kGameEnv = {reinterpret_cast<void(__fastcall *)(u8 *, void *, u8 *)>((uintptr_t)0x0040fb10),
                            reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)0x00500870),
                            reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)0x005012f0)};
const ObjectEnv *g_env = &kGameEnv;

}  // namespace

extern "C" {

void Object_SetEnv(const ObjectEnv *env) { g_env = env ? env : &kGameEnv; }

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

void __fastcall Slots_OnEvent(u8 *object, void *, u8 *event) {
    // (the game [[0x7cf6ec]]'s GameTable7cfb90_Get1da720_519510, only looked up: the averages ignore it)
    const auto lookUp = [] { GameTable7cfb90_Get1da720_519510(global<u8 *>(global<u32>(0x007cf6ec))); };
    const u32 type = vcall<u32>(event, 0);
    if (type == 0x1a) {   // (the event's id +8, amount +0xc and kind word +0x10; the original parks the kind in its own argument's slot, sic)
        Slots_AddIfMatch(object, nullptr, at<u32>(event, 8), at<u32>(event, 0xc), at<u16>(event, 0x10), 0);
        return;
    }
    if (type == 0x11) {
        if (!at<u32>(event, 0xc)) Slots_AddIfMatch(object, nullptr, 0x10, 1, 0, 0);
        return;
    }
    if (type != 0xb) return;   // (any other: the original's debug message)
    const s32 index = at<s32>(object, 0x3cc);
    if (index < 0 || index >= 20) return;
    const u8 *const slot = object + 0xc + 48 * (u32)index;
    switch (at<u32>(slot, 0)) {
    case 0x18: {
        lookUp();
        const s32 average = Things_AverageKind3Flag1Of100Minus44();
        Slots_AddIfMatch(object, nullptr, 0x18, (u32)(100 - average), 0, 0);
        return;
    }
    case 0x19: {
        lookUp();
        Slots_AddIfMatch(object, nullptr, 0x19, (u32)Things_AverageStaffStat1f8Int(), 0, 0);
        return;
    }
    case 0x1b: {
        static const u32 kKinds[5] = {5, 4, 6, 7, 8};
        const u32 which = at<u32>(slot, 0x10);
        const u32 kind = which <= 4 ? kKinds[which] : (u32)(uintptr_t)event;   // (else the event's address, sic, after a debug message)
        lookUp();
        Slots_AddIfMatch(object, nullptr, 0x1b, (u32)Things_AverageStat1f8OfKind(kind), 0, 0);
        return;
    }
    default:
        return;
    }
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

s32 __cdecl Objects_CountKind0InState1() {
    s32 count = 0;
    u16 handle = global<u16>(global<u32>(0x0080239c) + 0x1da746);
    while (handle) {
        const u8 *object = global<u8 *>(0x007cfb90 + 20 * (u32)handle);
        const u8 *def = ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(object, 0xe));
        if (at<u32>(def, 0x4ac) == 0 && Is19c_Eq1_4e0370(object)) ++count;
        handle = at<u16>(object, 0xc);
    }
    return count;
}

s32 __stdcall Objects_CountKind0Below44OrState1(s32 least) {
    s32 count = 0;
    u16 handle = global<u16>(global<u32>(0x0080239c) + 0x1da746);
    while (handle) {
        const u8 *object = global<u8 *>(0x007cfb90 + 20 * (u32)handle);
        const u8 *def = ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(object, 0xe));
        if (at<u32>(def, 0x4ac) == 0 && ((s32)((u32)(long long)at<float>(object, 0x44) & 0xff) < least || Is19c_Eq1_4e0370(object))) ++count;
        handle = at<u16>(object, 0xc);
    }
    return count;
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

u32 __fastcall Object_FillRate(const u8 *object) {
    if (!(at<u8>(object, 0x32) & 8)) return 100;
    u8 *const defs = reinterpret_cast<u8 *>(0x007890a0);
    const u16 id = at<u16>(object, 0xe);
    const u32 n = at<u32>(object, 0x58);
    if (n != 0) {
        const u32 row = (u32)at<u8>(object, 0x50) << 6;
        global<float>(0x007cdc54) = (float)(X87Reg((double)n) / X87Reg((double)at<s32>(ObjectDefs_Find(defs, nullptr, id), row + 0x1a8)));
    } else {
        global<float>(0x007cdc54) = 1.0f;
    }
    const float per = at<float>(ObjectDefs_Find(defs, nullptr, id), ((u32)at<u8>(object, 0x50) << 6) + 0x1b4);  // (its debug check of above 0 is left out)
    X87Reg rate = X87Reg((double)at<u8>(object, 0x5d)) * X87Reg(per) * X87Reg(global<float>(0x007cdc54)) / X87Reg((double)at<u8>(object, 0x5c));
    if (!(rate.v > 4.0)) rate = 4.0;  // (a NaN too)
    return (u32)(long long)rate.v;  // __ftol, the low half
}

void __fastcall Object_PayRunningCost(u8 *object, void *, s32 amount) {
    at<s32>(object, 0xf8) += amount;
    at<s32>(object, 0x184) += amount;
    Money_Debit(reinterpret_cast<u8 *>(GameTable7cfb90_Get1da726_5195d0(global<u8 *>(0x0080239c))), nullptr, amount);
    u8 *const defs = reinterpret_cast<u8 *>(0x007890a0);
    u32 id;
    if (at<s32>(ObjectDefs_Find(defs, nullptr, at<u16>(object, 0xe)), 0x4ac) == 1)
        id = 0xc;
    else if (at<s32>(ObjectDefs_Find(defs, nullptr, at<u16>(object, 0xe)), 0x4ac) == 2)
        id = 0xd;
    else
        return;
    // (the extra's upper word is a stale stack word in the original; only its low word, 0, is read)
    Slots_AddIfMatch(reinterpret_cast<u8 *>(GameTable7cfb90_Get1da736_519610(reinterpret_cast<u8 *>(Global80239c_Get()))), nullptr, id, (u32)-amount, 0, 0);
}

namespace {

// 0x4e1a30 / 0x4e1b40's common tail: the two settings as stored (floats), each held to [-0.5, 0.5] (a NaN to -0.5), the second negated for a definition of kind
// (+0x158) 1 or 3; (first + second + 1) times the dword at +0x188.
u32 Object_SettingsTail(const u8 *object, float first, float second) {
    if (!(first >= -0.5f)) first = -0.5f;
    else if (first > 0.5f) first = 0.5f;
    if (!(second >= -0.5f)) second = -0.5f;
    else if (second > 0.5f) second = 0.5f;
    const s32 kind = at<s32>(ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(object, 0xe)), 0x158);
    const X87Reg turned = kind == 1 || kind == 3 ? X87Reg(second) * X87Reg(-1.0f) : X87Reg(second);
    const X87Reg sum = X87Reg(first) + turned;
    const X87Reg value = (sum - X87Reg(-1.0f)) * X87Reg((double)at<u32>(object, 0x188));
    return (u32)(long long)value.v;  // __ftol, the low half
}

}  // namespace

u32 __fastcall Object_SettingsValue(const u8 *object, void *, float first, float second) {
    const float a = (float)((X87Reg(first) - X87Reg(50.0f)) * X87Reg(0.005f));
    const float b = (float)((X87Reg(second) - X87Reg(50.0f)) * X87Reg(0.005f));
    return Object_SettingsTail(object, a, b);
}

u32 __fastcall Object_CurrentSettingsValue(const u8 *object) {
    const float a = (float)((X87Reg((double)at<u8>(object, 0x18c)) - X87Reg(50.0f)) * X87Reg(0.005f));
    const float b = (float)((X87Reg((double)at<u8>(object, 0x198)) - X87Reg(50.0f)) * X87Reg(0.005f));
    return Object_SettingsTail(object, a, b);
}

u32 __fastcall Object_HistoryDifference(const u8 *object) {
    // (each sum's debug print of whether 30 is within the size is left out; both are CircBufU32_30_Sum inlined, the second in a register)
    double first, second;
    CircBufU32_30_Sum(object + 0x70, nullptr, &first, 0x1e);
    CircBufU32_30_Sum(object + 0xf8, nullptr, &second, 0x1e);
    return (u32)(long long)(X87Reg(first) - X87Reg(second)).v;  // __ftol, the low half
}

u32 __fastcall Object_HistoryAverage(const u8 *object) {
    // (the debug print of whether 30 is within the size is left out)
    const u8 *const ring = object + 0x340;
    const s32 held = at<u8>(ring, 0x84) ? at<s32>(ring, 0x80) : at<s32>(ring, 0x7c) + 1;
    const s32 count = held < 0x1e ? held : 0x1e;
    X87Reg sum = 0.0;
    for (s32 i = 0; i < count; ++i) {
        u32 value;
        if (CircBufU32_30_Get(ring, nullptr, &value, i)) sum = X87_Add(sum.v, (double)(s32)value);  // fiadd: signed
    }
    if (count <= 0) return 0x32;
    return (u32)(long long)(sum / X87Reg((double)count)).v + 0x32;  // __ftol, the low half
}

void __fastcall Object_ResetCondition(u8 *object) {
    if (at<u8>(object, 0x32) & 1) {
        const u8 level = (u8)(long long)at<float>(object, 0x44);  // __ftol, the low byte
        if ((double)level < 25.0) {
            u8 *const world = global<u8 *>(0x008023a0);
            const u32 id = ((u32)at<u8>(object, 7) << 7) + at<u8>(object, 5) + 1;  // (a word; the dword's upper half is a stale stack word, unread)
            Map_SpreadRemove(world, nullptr, 6, (u16)id);
            Map_SpreadAdd(world, nullptr, 1, (u16)id);
        }
    }
    Rides_SetElement(Rides_FindById(at<s32>(object, 0x24)), 8, 0);
    at<float>(object, 0x44) = 100.0f;
    at<u16>(object, 0x5e) = 0;
    at<u32>(object, 0x64) = 0;
}

u32 __fastcall Object_Serialize(u8 *object, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    if (Thing_Serialize(object, nullptr, stream, save, version, unused) != 0)
        return 1;
    if (version < 0 || version > 1)
        return 0;
    if (!Save_Field(stream, save, object + 0x10, 4, 0x0075b614) || !Save_Field(stream, save, object + 0xe, 2, 0x0075b610))
        return 1;
    // the date (+0x18) as a SYSTEMTIME's eight fields (year, month, day, day of the week, hour, minute, second, ms) in dwords
    u32 date[8] = {};
    s32 result;
    if (save != 0)
        FileTime_ToSystemTime(reinterpret_cast<FileTime *>(object + 0x18), nullptr, &result, &date[0], &date[1], &date[2], &date[3], &date[4], &date[5],
                              &date[6], &date[7]);  // (on a failure the original saves its stack's dwords)
    for (u32 i = 0; i < 8; ++i)
        if (!Save_Field(stream, save, &date[i], 4, 0x0075b608))
            return 1;
    if (save == 0)
        FileTime_FromSystemTime(reinterpret_cast<FileTime *>(object + 0x18), nullptr, &result, date[0], date[1], date[2], date[4], date[5], date[6],
                                date[7]);
    static const SaveSpec kHead[] = {{0x20, 4, 0x0075b5f8}, {0x32, 2, 0x0075a080}};
    if (!Save_Fields(stream, save, object, kHead, 2))
        return 1;
    for (u32 i = 0; i < 0x21; ++i)  // mNameA[i] and mNameB[i], a pair at a time
        if (!Save_Field(stream, save, object + 0x3c8 + i * 2, 2, 0x0075b5ec) || !Save_Field(stream, save, object + 0x40a + i * 2, 2, 0x0075b5e0))
            return 1;
    static const SaveSpec kIds[] = {{0x24, 4, 0x0075b5cc}, {0x28, 4, 0x0075b5b8}, {0x19c, 4, 0x0075b5b0}, {0x34, 2, 0x0075b5a4}, {0x36, 2, 0x0075b598},
                                    {0xc, 2, 0x0075a4f4},  {0x5e, 2, 0x0075b580}, {0x3a, 2, 0x0075b570},  {0x68, 4, 0x0075b564}, {0x38, 2, 0x0075b558},
                                    {0x3c, 2, 0x0075b54c}, {0x2c, 4, 0x0075b538}, {0x30, 2, 0x0075b528}};
    if (!Save_Fields(stream, save, object, kIds, 13))
        return 1;
    // the histories (the save path inlines CircBufU32_30_Serialize) with the customer and walk-away counts among them
    if (CircBufU32_30_Serialize(object + 0xf8, nullptr, stream, save, version, unused) != 0 ||
        CircBufU32_30_Serialize(object + 0x70, nullptr, stream, save, version, unused) != 0 || !Save_Field(stream, save, object + 0x1a0, 4, 0x0075b518) ||
        CircBufU32_30_Serialize(object + 0x1a8, nullptr, stream, save, version, unused) != 0 || !Save_Field(stream, save, object + 0x1a4, 4, 0x0075b508) ||
        CircBufU32_30_Serialize(object + 0x230, nullptr, stream, save, version, unused) != 0 ||
        CircBufU32_30_Serialize(object + 0x2b8, nullptr, stream, save, version, unused) != 0 ||
        CircBufU32_30_Serialize(object + 0x340, nullptr, stream, save, version, unused) != 0)
        return 1;
    static const SaveSpec kGoods[] = {{0x5d, 1, 0x0075b4f4},  {0x5c, 1, 0x0075b4e0},  {0x58, 4, 0x0075b4d0},  {0x6c, 2, 0x0075b4bc},
                                      {0x188, 4, 0x0075b4ac}, {0x18c, 4, 0x0075b49c}, {0x190, 4, 0x0075b488}, {0x194, 4, 0x0075b478}};
    if (!Save_Fields(stream, save, object, kGoods, 8))
        return 1;
    if (save == 0) {  // the price per use loaded is held to 0..500
        s32 &price = at<s32>(object, 0x194);
        price = price < 0 ? 0 : price > 500 ? 500 : price;
    }
    static const SaveSpec kTail[] = {{0x198, 4, 0x0075b45c},     {0x40, 4, 0x0075b448},  {0x4c, 4, kSaveStatName}, {0x48, 4, kSaveStatName},
                                     {0x44, 4, kSaveStatName},   {0x64, 4, 0x0075b430},  {0x60, 4, 0x0075b414},    {0x184, 4, 0x0075b408},
                                     {0x180, 4, 0x0075b3f8},     {0x54, 4, 0x0075b3e0},  {0x50, 1, 0x0075b3d0}};
    return Save_StatFields(stream, save, object, kTail, 11) ? 0 : 1;
}

u8 __fastcall Object_ComputeRating(const u8 *object, void *, s32 a, s32 b, s32 c) {
    const u8 *const def = Object_GetDef(object);
    if (at<s32>(def, 0x4ac) == 2) {  // a sideshow: 20 + 8% of c times the root of a - b (0..100)
        s32 over = (s32)((u32)a - (u32)b);
        over = over < 0 ? 0 : over > 100 ? 100 : over;
        const double root = X87_Sqrt(X87Reg((double)over)).v;
        const double x = X87_MulDouble(X87_MulDouble(X87_MulDouble(root, (double)c), 0.1), (double)-0.8f);
        return (u8)(20 - (u8)(long long)x);
    }
    if (at<s32>(def, 0x9c) == 3) {  // a coaster: 50 + half its +0x104 float
        const u32 coaster = Rides_GetE0ById(at<s32>(object, 0x24));
        if (coaster == 0)
            return 0;
        u32 first, second;
        Coaster_GetPair(coaster, &first, &second);
        float value;
        std::memcpy(&value, &second, 4);
        return (u8)(long long)X87_SubR(X87Reg(value) * X87Reg(-0.5f), X87Reg(50.0f)).v;
    }
    // a ride: the base (60% of it, plus the track's pieces, 0..100, with a track ride) times the speed and duration over the row's limits (each 0.75..1.25)
    s32 base = at<s32>(def, 0x13c);
    const u32 row = (u32)at<u8>(object, 0x50) << 6;
    const s32 speedLimit = at<s32>(def, row + 0x1a8);
    const s32 durationLimit = at<s32>(def, row + 0x1a0);
    const u32 track = at<u32>(object, 0x28);
    if (track != 0) {
        s32 others = 0, longest = 0, kind11 = 0, free = 0;
        TrakRide_CountPieces(track, &others, &longest, &kind11, &free);
        s32 pieces = kind11 * 3 + longest + others * 2;
        pieces = pieces < 0 ? 0 : pieces > 40 ? 40 : pieces;
        base = (s32)((u32)base * 60) / 100 + pieces;
        base = base < 0 ? 0 : base > 100 ? 100 : base;
    }
    const auto clamp = [](X87Reg x) { return !(x.v >= 0.75) ? X87Reg(0.75) : x.v > 1.25 ? X87Reg(1.25) : x; };
    const X87Reg speed = X87Reg((double)a) / X87Reg((double)speedLimit);
    const float duration = (float)(X87Reg((double)b) / X87Reg((double)durationLimit));
    const float speedFactor = (float)clamp(speed);
    const float factor = (float)(clamp(X87Reg(duration)) * X87Reg(speedFactor));
    return (u8)(long long)X87_MulDouble((double)base, factor);
}

u8 __fastcall Object_Rating(const u8 *object, void *, u32 baseOnly) {
    const u32 rating = at<u32>(Object_GetDef(object), 0x13c);
    if ((u8)baseOnly != 0)
        return (u8)rating;
    if (rating == 0)
        return 0;
    if (at<s32>(Object_GetDef(object), 0x4ac) == 2)
        return Object_ComputeRating(object, nullptr, at<s32>(object, 0x188), at<s32>(object, 0x194), at<u8>(object, 0x190));
    return Object_ComputeRating(object, nullptr, at<s32>(object, 0x58), at<u8>(object, 0x5c), at<u8>(object, 0x5d));
}

u32 __fastcall Object_InPoorCondition(const u8 *object) {
    return at<u8>(object, 0x32) & 1 && (u8)(long long)at<float>(object, 0x44) < 25 ? 1 : 0;
}

void __fastcall Object_Wear(u8 *object, void *, u32 amount) {
    u8 &guard = global<u8>(0x00fb34b8);   // (the mode setting 0xfb3b7c: a function-local static)
    if (!(guard & 1)) {
        guard |= 1;
        Setting_ReadMode(reinterpret_cast<u8 *>(0x00fb3b7c));
        reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>(0x0067a7d0)(reinterpret_cast<void(__cdecl *)()>(0x005d1750));
    }
    if (global<u32>(0x00fb3b7c) == 1) {
        if (Object_InPoorCondition(object))
            Object_ResetCondition(object);   // (the same steps: the removal's cell from Cell_MakeHandle, the spread's from the map id)
        return;
    }
    const u32 wasPoor = Object_InPoorCondition(object);
    const X87Reg level = X87Reg((double)(unsigned long long)amount) * global<float>(0x007005d0) + at<float>(object, 0x44);   // fild qword; fmul; fadd
    if (level.v > 100.0)
        at<u32>(object, 0x44) = 0x42c80000;
    else if (!(level.v >= 0.0))
        at<u32>(object, 0x44) = 0;
    else
        at<float>(object, 0x44) = (float)level.v;
    if (Object_InPoorCondition(object) && !wasPoor) {
        u8 *const world = global<u8 *>(0x008023a0);
        const u16 id = (u16)((at<u8>(object, 7) << 7) + at<u8>(object, 5) + 1);
        Map_SpreadRemove(world, nullptr, 1, id);
        Map_SpreadAdd(global<u8 *>(0x008023a0), nullptr, 6, id);
    }
}
}
