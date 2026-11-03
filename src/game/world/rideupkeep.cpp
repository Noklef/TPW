// What the park's opened rides cost to keep up: see rideupkeep.h.
#include "rideupkeep.h"

#include <cstdint>

#include "../core/filetime.h"
#include "object.h"
#include "objectdefs.h"
#include "objectmsg.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const RideUpkeepEnv kGameEnv = {ObjectDefs_Find, Object_QueueOpen, ObjectControls_FindUncached, ObjectControls_Find, FunnyTime_SpanSince, Object_AgeDays};
const RideUpkeepEnv *g_env = &kGameEnv;

u8 *defOf(const u8 *thing) { return g_env->def(reinterpret_cast<u8 *>((uintptr_t)0x007890a0), nullptr, at<u16>(thing, 0xe)); }
u8 *game() { return global<u8 *>(0x0080239c); }
u8 *controlsTable() { return game() + 0x1d943c; }
u8 *uncached(const u8 *thing) { return g_env->uncached(controlsTable(), nullptr, at<u16>(thing, 0xe), 0); }

}  // namespace

void RideUpkeep_SetEnv(const RideUpkeepEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" s32 __fastcall RideUpkeep_Total(void *) {
    u32 total = 0;
    for (u16 index = at<u16>(game(), 0x1da746); index;) {
        u8 *const thing = global<u8 *>(0x007cfb90 + (u32)index * 20);
        const s32 kind = at<s32>(defOf(thing), 0x4ac);
        if ((kind == 0 || kind == 2) && g_env->open(thing) && at<s32>(uncached(thing), 0x18) > 0) {
            s32 share, days;
            if (!at<u32>(defOf(thing), 0x9c)) {
                share = at<s32>(uncached(thing), 0x18);
                const u32 stamp = at<u32>(g_env->controls(controlsTable(), nullptr, at<u16>(thing, 0xe), 0), 0x1c);
                FileTime out;
                const FileTime *const span = g_env->since(game() + 0x2a0, nullptr, &out, stamp);
                days = (s32)((long long)((unsigned long long)span->high << 32 | span->low) / 0xc92a69c000LL);
            } else {
                share = 1;
                days = g_env->age(thing);
            }
            const s32 step = days / at<s32>(defOf(thing), 0x5c);
            const u32 extra = step < 3 && step >= 0 ? at<u32>(defOf(thing), 0x268 + (u32)step * 4) : 0;
            total += (u32)((s32)(at<u32>(defOf(thing), 0x58) + extra) / share);
        }
        index = at<u16>(thing, 0xc);
    }
    return (s32)total;
}
