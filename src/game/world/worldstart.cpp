// The world's start: see worldstart.h.
#include "worldstart.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "objectdefs.h"
#include "thingmsg.h"
#include "world.h"
#include "things.h"
#include "../ui/dialog74fb20.h"
#include "worldstart.h"
#include "../core/staticobjects.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename F> F game(u32 address) { return reinterpret_cast<F>((uintptr_t)address); }
u8 *address(u32 a) { return reinterpret_cast<u8 *>((uintptr_t)a); }

using Ctor = u8 *(__fastcall *)(u8 *);

const WorldStartEnv kGameEnv = {
    game<void(__fastcall *)(u8 *)>(0x004d3de0),
    game<void(__fastcall *)(u8 *)>(0x00403b00),
    game<Ctor>(0x00550ca0),
    game<s32(__cdecl *)(void *)>(0x0067a7d0),
    game<u32(__fastcall *)(u8 *, void *, const char *, u32)>(0x00401650),
    game<u32(__cdecl *)(void *)>(0x0067d3f0),
    game<void(__fastcall *)(u8 *)>(0x004d7bb0),
    game<void(__fastcall *)(u8 *)>(0x004d7af0),
    {game<Ctor>(0x005089c0), game<Ctor>(0x004da920), game<Ctor>(0x00599e70), game<Ctor>(0x00509b40), game<Ctor>(0x004c58c0), game<Ctor>(0x00502d20), game<Ctor>(0x00511f70), game<Ctor>(0x004cf7c0),
     game<Ctor>(0x0050c9b0), game<Ctor>(0x004d1bd0)},
    game<u8 *(__fastcall *)(u8 *, void *, u32, u32, s32, u32)>(0x004db090),
    game<void(__fastcall *)(u8 *)>(0x00502d60),
    game<void(__cdecl *)()>(0x00544360),
    game<void(__cdecl *)()>(0x0050fdf0),
    game<void(__stdcall *)(u8 *, u32, s32, void *, void *)>(0x0067acf0),
    game<u32(__fastcall *)(u8 *, void *, u8 *)>(0x0040fb10),
};
const WorldStartEnv *g_env = &kGameEnv;

// The ten things: StaffHQ, MechanicHQ, Advisor, TagSystem, ParkAnalyser, ThingKindE, Weather, BankAccount, UIMsgReceiver, Challenges.
const struct {
    u32 size, word;
} kThings[10] = {{0x68, 0x1da718}, {0xe, 0x1da71a}, {0x16f0, 0x1da71c}, {0x80e, 0x1da71e}, {0x21c50, 0x1da720}, {0x146c, 0x1da722}, {0x68, 0x1da724}, {0x128, 0x1da726}, {0xc, 0x1da728}, {0x3e8, 0x1da736}};

const u32 kBalanceFile = (u32)(uintptr_t)&gBalanceLevel;

void read_setting_once() {
    u8 &guard = gWorldStart_fb34a9;
    if (guard & 1) return;
    guard |= 1;
    g_env->settingReadMode(address(reinterpret_cast<uintptr_t>(&gDialog74fb20_fb3b7c)));
    g_env->atexit(address(0x005d1750));
}

u32 load_balance(const char *path, u32 keep) { return g_env->balanceLoad(address(kBalanceFile), nullptr, path, keep); }

// The object of that name among the definitions: its handle, 0 when none is there.
u16 make_named_object(u32 name_at) {
    u8 *const defs = address(0x007890a0);
    u8 *def = ObjectDefs_First(defs);
    while (def && CRT_wcsicmp(reinterpret_cast<const u16 *>(def + 0x390), global<const u16 *>(name_at)) != 0) def = ObjectDefs_Next(defs);
    if (!def) return 0;
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x450));
    u8 *const object = block ? g_env->constructObject(block, nullptr, 1, *reinterpret_cast<u16 *>(def), 0, 0) : nullptr;
    u16 handle;
    return *Thing_GetHandle(object, nullptr, &handle);
}

}  // namespace

void WorldStart_SetEnv(const WorldStartEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" u32 __fastcall World_Start(u8 *world, void *, const char *level) {
    at<u32>(world, 0x1da738) = 1;
    u32 ok = 1;
    g_env->objectControlsInit(gWorldPtr + 0x1d943c);
    g_env->actionRestart(address((u32)(uintptr_t)&gActionRec_785a08));
    for (u8 &b : gWorld_802080) b = 0;
    read_setting_once();
    char path[260];
    if (gDialog74fb20_fb3b7c == 1) {
        if (!load_balance(gText_World_Start_76259c, 0)) ok = 0;
        Crt().sprintf(path, gText_Objectdefs_7488ac, level, gText_World_Start_762588);
        if (!load_balance(path, 1)) ok = 0;
    } else {
        if (!load_balance(gText_World_Start_76256c, 0)) ok = 0;
        Crt().sprintf(path, gText_Objectdefs_7488ac, level, gText_World_Start_76255c);
        if (!load_balance(path, 1)) ok = 0;
        read_setting_once();
        if (gDialog74fb20_fb3b7c == 2) {
            Crt().sprintf(path, gText_Objectdefs_7488a4, level, global<const char *>((u32)(uintptr_t)&gString_7cf498 + 0x8), gText_World_Start_76255c);
            load_balance(path, 1);   // (its failure only printed)
        }
    }
    at<u32>(world, 0x1da70c) = 0;
    at<u32>(world, 0x1da708) = g_env->time(nullptr);
    g_env->initBlock(world + 0x2d8);
    g_env->prepare(world);
    at<u32>(world, 0x1da714) = 0;
    for (u32 i = 0; i < 10; ++i) {
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(kThings[i].size));
        u8 *const thing = block ? g_env->construct[i](block) : nullptr;
        u16 handle;
        at<u16>(world, kThings[i].word) = *Thing_GetHandle(thing, nullptr, &handle);
    }
    for (u32 word = 0x1da72a; word <= 0x1da730; word += 2) at<u16>(world, word) = 0;
    at<u16>(world, 0x1da732) = make_named_object((u32)(uintptr_t)&gWString_8021a8 + 0x8);
    at<u16>(world, 0x1da734) = make_named_object((u32)(uintptr_t)&gWString_7cf930 + 0x8);
    g_env->researchInit(global<u8 *>(0x007cfb90 + 20 * at<u16>(world, 0x1da722)));
    g_env->mathTables();
    g_env->setPathBehaviours();
    u8 msg[12];
    ThingMsg13_Ctor(msg, nullptr, 0);
    u8 *&centre = global<u8 *>(0x00788d3c);
    if (!centre) {
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x244));
        if (block) g_env->vectorCtor(block, 0x14, 0x1d, address(0x00405460), address(0x0040f970));
        centre = block;
    }
    g_env->post(centre, nullptr, msg);
    at<u32>(world, 0x1da738) = 0;
    return ok;
}
