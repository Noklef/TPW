#include "gamemode.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/actionrec.h"
#include "../core/gameheap.h"
#include "../coaster/edittools.h"
#include "../ui/cursor.h"
#include "advisor.h"
#include "cellrun.h"
#include "mousemode.h"
#include "world.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const GameModeEnv kGameEnv = {
    reinterpret_cast<void(__fastcall *)(u8 *, void *, u32)>(&Advisor_Cancel),   // (its AL unread)
    reinterpret_cast<void(__cdecl *)(u32, u32)>(&CoasterEdit_EditRide),
    reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x00446050),
};
const GameModeEnv *g_env = &kGameEnv;

ActionRec *recorder() { return reinterpret_cast<ActionRec *>((uintptr_t)0x00785a08); }

// The common start: the action recorded, the old mode's advisor messages cancelled, the mode stored. Returns the old mode.
u32 store(u32 mode, u32 record, u32 code) {
    if ((u8)record) ActionRec_WriteW(recorder(), nullptr, code, mode);
    if (global<u32>(0x0081ae2c) != mode) {
        static const u32 kIds[] = {0xb8, 0xcb, 0xd6, 0xcd, 0xc9, 0xce, 0xc1, 0x159, 0x15a, 0x151};
        for (const u32 id : kIds) {
            u8 *const advisor = reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da71c_5194d0(global<u8 *>(0x007cf83c)));
            g_env->advisorCancel(advisor, nullptr, id);
        }
    }
    const u32 old = global<u32>(0x0081ae2c);
    global<u32>(0x0081ae2c) = mode;
    global<u32>(0x0081af64) = old;
    return old;
}

// Mode 0's mouse mode: a new CMMOnline base, the holder made when missing, the mode set.
void online_mode() {
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(8));
    u8 *const mode = block ? MouseModeOnlineBase_Ctor(block) : nullptr;
    MouseModes_EnsureHolder();
    MouseMode_Set(mode);
}

// The other modes' set-up (both functions).
void other_mode(u32 mode) {
    if (mode == 3) {
        Advisor_PostId(0xcb);
        GameCursor_Set(4);
    } else if (mode == 0x15 || mode == 0x16) {
        g_env->showRun(global<u32>(0x008186ac), 1);
    } else if (mode == 1) {
        GameCursor_Set(3);
    }
}

}  // namespace

extern "C" {

void GameMode_SetEnv(const GameModeEnv *env) { g_env = env ? env : &kGameEnv; }

void __cdecl GameMode_SetOnly(u32 mode, u32 record) {
    const u32 old = store(mode, record, 0x19);
    if (mode == 0) {
        if (old != 0x15 && old != 0x16) online_mode();
        return;
    }
    other_mode(mode);
}

void __cdecl GameMode_Set(u32 mode, u32 record) {
    const u32 old = store(mode, record, 0);
    global<s32>(0x0081ede4) = -1;
    global<s32>(0x0081ede8) = -1;
    if (mode == 0) {
        global<u16>(0x00818694) = 0;
        global<u32>(0x00763b00) = 0x80;
        global<u32>(0x00763b04) = 0x80;
        global<u16>(0x007639b0) = 0xffff;
        global<u16>(0x00818684) = 0;
        global<u16>(0x007639b4) = 0xffff;
        global<s32>(0x007639a0) = -1;
        global<u32>(0x0081b604) = 0;
        if (global<u32>(0x0081b134)) ActionRec_WriteW(recorder(), nullptr, 0xa, 0);
        global<u32>(0x0081b134) = global<u32>(0x0081d7a4);
        global<u32>(0x0081d7a4) = 0;
        if (old != 0x15 && old != 0x16) online_mode();
        g_env->resetTools();
        Global816d48_Set(0);
        return;
    }
    other_mode(mode);
    if (mode == 0x35) {
        global<u32>(0x00763b00) = 0x80;
        global<u32>(0x00763b04) = 0x80;
    }
}
}

namespace {

const GameModeStepEnv kGameStepEnv = {reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x00444f30),
                                      reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)0x004de1f0),
                                      reinterpret_cast<void(__cdecl *)(s32, s32, u32, u32)>((uintptr_t)0x00524960),  // (hooked: GameMode_MapClick)
                                      &GameMode_Set,
                                      &GameMode_SetOnly,
                                      &TrackCell_Relink,
                                      &TrackRun_UpdateA,
                                      &TrackRun_UpdateB};
const GameModeStepEnv *g_step = &kGameStepEnv;

// Modes 0xb, 0x10 and 0x35: the track cell relinked and, with relink's low byte and a track run, the run updated.
void relink_run(u32 mode, u32 corners, u32 relink) {
    if (mode != 0xb && mode != 0x10 && mode != 0x35) return;
    g_step->relink(corners);
    u8 *const run = global<u8 *>(0x008186ac);
    if ((u8)relink && run && !g_step->updateA(run, 0, 0, 0)) g_step->updateB(global<u8 *>(0x008186ac), 1, 0, 0);
}

}  // namespace

extern "C" {

void GameMode_SetStepEnv(const GameModeStepEnv *env) { g_step = env ? env : &kGameStepEnv; }

u32 __cdecl GameMode_StepBack(u8 *thing, u32 relink) {
    const u32 mode = global<u32>(0x0081ae2c);
    if (mode == 0x15) {
        g_step->endRun();
        return 0;
    }
    if (mode == 0xe || mode == 0x13 || mode == 4 || mode == 0x3b || mode == 0x16) return 0;
    s32 *const point = &global<s32>(0x0081adb0);
    s32 &count = global<s32>(0x00820a8c);
    bool popped = false;
    if (count > 0) {
        point[0] = global<s32>(0x0081b738 + count * 8);
        point[1] = global<s32>(0x0081b73c + count * 8);
        if (--count != 0)
            popped = true;
        else
            count = 1;
    }
    if (!popped) {
        relink_run(mode, 0, relink);
        g_step->setOnly(mode, 1);
        return 0;
    }
    Global816d48_Set(0);
    bool arm = false;
    switch (mode) {
    case 1: case 3: case 0x34:
        g_step->set(0x34, 1);
        arm = true;
        break;
    case 0xb: case 0x10: case 0x19: case 0x35:
        g_step->set(0x35, 1);
        global<u32>(0x00763b00) = 0x80;
        global<u32>(0x00763b04) = 0x80;
        break;
    }
    g_step->mapClick(point[0], point[1], 0, 0);
    const s32 left = count;
    u32 corners = 0;
    if (left > 0) {
        point[0] = global<s32>(0x0081b738 + left * 8);
        point[1] = global<s32>(0x0081b73c + left * 8);
    }
    if (left == 1) corners = 1;
    if (left > 0) {
        const s32 x = point[0], y = point[1];
        const u8 *const cell = global<u8 *>(0x007cf83c) + 0x294 + ((((u32)y << 7) + (u32)x + 1) & 0xffff) * 0x44;
        if (mode != 3 || *reinterpret_cast<const s32 *>(cell + 8) != 1) g_step->mapClick(x, y, 0, 0);
        global<s32>(0x0081ede4) = point[0];
        global<s32>(0x0081ede8) = point[1];
    }
    g_step->setOnly(mode, 1);
    if (arm) {
        u8 *const target = thing ? thing : global<u8 *>(0x008186ac);
        if (target) g_step->thing4de1f0(target);
    }
    relink_run(mode, corners, relink);
    return 1;
}
}
