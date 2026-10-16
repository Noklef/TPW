#include "ridesearch.h"

#include <cstdint>

#include "../core/small15.h"
#include "object.h"
#include "objectdefs.h"
#include "things.h"
#include "trackrides.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// Walks the things newest first, calling test(thing) for each object whose definition is not constrained; test returns whether to measure it and sets the measure.
template <typename Test> u32 Best(u16 *out, Test test) {
    bool found = false;
    s32 best = (s32)(intptr_t)out;
    *out = 0;
    u8 *world = global<u8 *>(0x0080239c);
    u16 handle;
    World_FirstThing(world, nullptr, &handle);
    while (handle != 0) {
        u8 *thing = reinterpret_cast<u8 **>(0x007cfb90)[handle * 5];
        if (at<u8>(thing, 2) == 3) {
            s32 measure;
            if (test(thing, measure) && (!found || measure > best)) {
                found = true;
                best = measure;
                *out = handle;
            }
        }
        u16 next;
        handle = *World_NextThing(world, nullptr, &next, thing);
    }
    return found ? (u32)best : 0;
}

bool Unconstrained(const u8 *thing) {
    return at<u32>(ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(thing, 0xe)), 0x4ac) == 0;
}

}  // namespace

extern "C" {

u32 __stdcall Rides_BestKind11Count(u16 *out) {
    return Best(out, [](const u8 *thing, s32 &measure) {
        if (!Unconstrained(thing) || at<u32>(Object_GetDef(thing), 0x9c) != 1)
            return false;
        TrakRide_CountPieces(at<u32>(thing, 0x28), nullptr, nullptr, &measure, nullptr);
        return true;
    });
}

u32 __stdcall Rides_BestPiecesOfKinds(u16 *out, u32 id) {
    return Best(out, [id](const u8 *thing, s32 &measure) {
        if (!Unconstrained(thing) || !Obj_DefKind123(thing))
            return false;
        if ((u16)id != 0 && at<u16>(thing, 0xe) != (u16)id)
            return false;
        measure = TrakRide_CountPieces(at<u32>(thing, 0x28), nullptr, nullptr, nullptr, nullptr);
        return true;
    });
}

u32 __stdcall Rides_BestPiecesOfKind(u16 *out, u32 kind) {
    return Best(out, [kind](const u8 *thing, s32 &measure) {
        if (!Unconstrained(thing) || at<u32>(Object_GetDef(thing), 0x9c) != kind)
            return false;
        measure = TrakRide_CountPieces(at<u32>(thing, 0x28), nullptr, nullptr, nullptr, nullptr);
        return true;
    });
}

}  // extern "C"
