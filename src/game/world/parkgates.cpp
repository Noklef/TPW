#include "parkgates.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/indexed.h"
#include "../core/options.h"
#include "../core/savegame.h"
#include "../core/timesig.h"
#include "../math/x87.h"
#include "mapcell.h"
#include "ridelist.h"
#include "rideupkeep.h"
#include "thingscan.h"
#include "visitorsteps.h"
#include "world.h"
#include "../ui/dialog7cb25c.h"
#include "../ui/dialog74fb20.h"
#include "parkgates.h"
#include "balancelevel.h"

namespace {

const SaveSpec kRate[] = {{0x0, 4, (u32)&gText_Parkgates_759b98}};
const SaveSpec kRest[] = {{0x8, 4, (u32)&gText_Parkgates_759b80}, {0xc, 4, (u32)&gText_Parkgates_759b70}, {0x10, 1, 0x00759b64}, {0x11, 1, (u32)&gText_Parkgates_759b58}};

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(u8 *p, u32 offset) { return *reinterpret_cast<T *>(p + offset); }

const ParkGatesEnv kGameEnv = {&Game1da70c_MinusAt0Quarters,
                               &Out_SetFromGame1da70c,
                               &GameTable7cfb90_Get1da720_519510,
                               &GameTable7cfb90_Get1da724_519590,
                               &Game_TakeRideElement1,
                               &World_BusStopBlocked,
                               reinterpret_cast<void(__fastcall *)(u8 *, void *, u8 *, u32)>((uintptr_t)0x0051a2f0),
                               &World_ArrivalVehicleNotSize1,
                               &Map_BusStopPos,
                               &Visitor_Ctor,
                               &ParkAnalyser_CountKind1OnPlainCell,
                               &RideUpkeep_Total,
                               &Setting_ReadMode,
                               reinterpret_cast<s32(__cdecl *)(u32)>((uintptr_t)0x0067a7d0)};
const ParkGatesEnv *g_env = &kGameEnv;

u8 *World() { return global<u8 *>(0x0080239c); }

// The mode setting at 0xfb3b7c, a function-local static (the guard 0xfb34c2 & 1).
u32 Mode() {
    u8 &guard = gDialog7cb25c_fb34c2;
    if (!(guard & 1)) {
        guard |= 1;
        g_env->readMode(reinterpret_cast<u8 *>(&gDialog74fb20_fb3b7c));
        g_env->atExit(0x005d1750);
    }
    return gDialog74fb20_fb3b7c;
}

// The arrival told to the world; the answer (the slot given) is not read.
void Arrival(u32 kind) {
    u8 out[4];
    g_env->arrival(World(), nullptr, out, kind);
}

}  // namespace

void ParkGates_SetEnv(const ParkGatesEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u32 __fastcall ParkGates_Serialize(u8 *gates, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    if (version < 0 || version > 1) return 0;
    if (!Save_Fields(stream, save, gates, kRate, 1)) return 1;
    if (TimeSig_Serialize(gates + 4, nullptr, stream, save, version, unused) != 0) return 1;
    if (!Save_Fields(stream, save, gates, kRest, 4)) return 1;
    return 0;
}

s32 __fastcall ParkGates_ArrivalSize(u8 *) {
    if (at<u32>(World(), 0x1da738) == 4) return 0;
    const s32 people = static_cast<s32>(g_env->count(reinterpret_cast<u8 *>((uintptr_t)g_env->table(World())), nullptr, 0x14));
    s32 share = reinterpret_cast<s32 &>(gBalanceLevel.mPointsPerVisitor);
    if (share <= 1) share = 1;
    const s32 upkeep = g_env->upkeep(reinterpret_cast<void *>((uintptr_t)g_env->table(World())));
    const float scale = at<s32>(reinterpret_cast<u8 *>((uintptr_t)g_env->record(World())), 0x30) > 0 ? 0.800000012f : 1.20000005f;  // 0x700368, 0x700364
    const s32 scaled = static_cast<s32>(static_cast<long long>(X87_MulIntDouble(gBalanceLevel.mNewParkBonus + upkeep, scale)));  // __ftol
    s32 size = reinterpret_cast<s32 &>(gBalanceLevel.mMinPeople);
    const s32 wanted = scaled / share;
    if (size <= wanted) size = wanted;
    if (Mode() == 1 && static_cast<u32>(size + people) > 0x1f4) size = 0x1f4 - people;
    if (Mode() != 1 && static_cast<u32>(size + people) > 0x5dc) size = 0x5dc - people;
    return size;
}

u32 __fastcall ParkGates_SpawnVisitor(u8 *) {
    const u32 other = g_env->notSize1(World());
    u16 pos;
    u16 cell = *g_env->busStop(global<u8 *>(0x008023a0), nullptr, &pos, 1);
    if (other) cell = static_cast<u16>(cell - 0x100);
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x22c));
    // (the original pushes the cell as a dword, its high word uninitialised stack; Visitor_Ctor reads the low word)
    if (block) g_env->visitorCtor(block, nullptr, cell);
    return 1;
}

u32 __fastcall ParkGates_Step(u8 *gates) {
    if (!at<u8>(gates, 0x10)) {
        if (g_env->since(reinterpret_cast<const u32 *>(gates + 4)) > gBalanceLevel.mTimeBetweenArrivals) {
            const s32 size = ParkGates_ArrivalSize(gates);
            at<s32>(gates, 0xc) = size;
            at<u8>(gates, 0x10) = 1;
            // The analyser's ring of arrival sizes (a CircBufU32 at +0x216d8), inlined.
            u8 *const ring = reinterpret_cast<u8 *>((uintptr_t)g_env->table(World())) + 0x216d8;
            const s32 next = at<s32>(ring, 0x18) + 1;
            at<s32>(ring, 0x18) = next;
            if (next >= at<s32>(ring, 0x1c)) {
                at<s32>(ring, 0x18) = 0;
                at<u8>(ring, 0x20) = 1;
            }
            at<s32>(ring, at<s32>(ring, 0x18) * 4 + 4) = size;
        }
    }
    if (at<u8>(gates, 0x10)) {
        const s32 state = g_env->take(World());
        if (state == -1 || state == 6) {
            const s32 people = at<s32>(gates, 0xc);
            Arrival(people <= 0x23 ? 1 : people > 0x3c ? 3 : 2);
        } else if (state == 2) {
            if (at<s32>(gates, 0xc) > 0) {
                ParkGates_SpawnVisitor(gates);
                --at<s32>(gates, 0xc);
                ++at<s32>(reinterpret_cast<u8 *>((uintptr_t)g_env->table(World())), 0x20cc0);
            } else {
                g_env->stamp(reinterpret_cast<u32 *>(gates + 4));
                at<u8>(gates, 0x10) = 0;
                Arrival(0);
            }
        }
    }
    if (g_env->blocked(World())) {
        const s32 state = g_env->take(World());
        if (state == -1 || state == 0 || (state == 2 && at<s32>(gates, 0xc) == 0)) Arrival(0);
    } else if (g_env->take(World()) == 4) {
        Arrival(0);
    }
    return 0;
}
}
