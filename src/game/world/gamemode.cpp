#include "gamemode.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/actionrec.h"
#include "../core/gameheap.h"
#include "../coaster/edittools.h"
#include "../ui/cursor.h"
#include "advisor.h"
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
