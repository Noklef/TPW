#include "gamepause.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/options.h"
#include "../core/soundplay.h"
#include "../core/vobj3.h"
#include "../gfx/imageops.h"
#include "../render/particles.h"
#include "dialog74fb20.h"
#include "dialog7cc184.h"
#include "dialogs.h"
#include "focus.h"
#include "guisettings.h"
#include "window.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename F> F byAddress(u32 address) { return reinterpret_cast<F>((uintptr_t)address); }
u8 *object(u32 address) { return reinterpret_cast<u8 *>((uintptr_t)address); }

void *__cdecl HeapAlloc_(u32 size) { return GameHeap_Alloc(size); }
void __cdecl HeapFree_(void *block) { GameHeap_Free(block); }

const GamePauseEnv kGameEnv = {
    &SoundUi_Play,
    &Setting_ReadMode,
    byAddress<u32(__cdecl *)(u32)>(0x0067a7d0),
    byAddress<void(__fastcall *)(u8 *, void *, u32, u32)>(0x004092a0),
    &Global7cc698_Choose,
    &Window_SetShown,
    &Gui_CursorSlot3,
    &ParticlePool2_Finish,
    byAddress<void(__cdecl *)(u32)>(0x005996d0),
    byAddress<void(__fastcall *)(u8 *)>(0x00409300),
    &Gui_CursorSlot2,
    &CameraMode_Leave,
    &Global7cb2e8_Get,
    &Overlay7cb2b4_Toggle,
    &Windows7cc8bc_SelectFirst,
    &Dialog7cc184_Open,
    &HeapAlloc_,
    &HeapFree_,
    &Blocks7_Free,
    &Blocks7_Ctor,
    byAddress<void(__fastcall *)(u8 *)>(0x005d47a0),
    byAddress<void(__fastcall *)(u8 *, void *, SurfaceDesc *)>(0x00564430),
    &Blocks7_ShrinkSurface,
    &Obj_StoreComSlot80,
    &Panel_Reset,
};
const GamePauseEnv *g_env = &kGameEnv;

// The setting's first read, its guard the bit 0xfb34bf here. The answer: is it 1.
bool SettingOne() {
    if (!(global<u8>(0x00fb34bf) & 1)) {
        global<u8>(0x00fb34bf) |= 1;
        g_env->readMode(object(0x00fb3b7c));
        g_env->atExit(0x005d1750);
    }
    return global<u32>(0x00fb3b7c) == 1;
}

}  // namespace

extern "C" {

void GamePause_SetEnv(const GamePauseEnv *env) { g_env = env ? env : &kGameEnv; }

void __cdecl GamePause_RemakePicture() {
    const GamePauseEnv *const E = g_env;
    u8 *const old = global<u8 *>(0x007cc18c);
    if (old) {
        E->blocksFree(old);
        E->release(old);
    }
    u8 *const block = static_cast<u8 *>(E->alloc(0x70));
    u8 *const made = block ? E->blocksCtor(block) : nullptr;
    global<u8 *>(0x007cc18c) = made;
    E->fn5d47a0(made);
    u8 desc[0x7c];
    E->fn564430(global<u8 *>(0x00879a24), nullptr, reinterpret_cast<SurfaceDesc *>(desc));
    E->shrink(global<u8 *>(0x007cc18c), nullptr, reinterpret_cast<const SurfaceDesc *>(desc));
    E->storeCom(global<u8 *>(0x00879a24));
    E->panelReset(global<u8 *>(0x007cc18c));
}

void __cdecl GamePause_Resume() {
    const GamePauseEnv *const E = g_env;
    global<u32>(0x007cc1a0) = 0;
    if (!SettingOne()) {
        global<u32>(0x00786ba4) = 1;
        E->enter(object(0x00786b68));
    }
    GamePause_RemakePicture();
    E->setShown(global<u8 *>(0x00faa5d8), nullptr, 1);
    E->cursorSlot2();
    E->cameraLeave();
    if (SettingOne())
        E->selectFirst();
    else if (E->overlayState())
        E->overlayToggle();
    if (!global<u8>(0x007cc194)) {
        global<u32(__stdcall *)(u32, u32)>(0x006fd14c)(0x200, 0x007cb958);
        global<u8>(0x007cc194) = 1;
    }
    E->openDialog();
    global<u32>(0x007cc190) = 0;
}

void __cdecl GamePause_Hide() {
    const GamePauseEnv *const E = g_env;
    E->play(0x95);
    if (SettingOne()) {
        E->choose();
    } else {
        E->leave(object(0x00786b68), nullptr, 0, 0);
        global<u32>(0x00786ba4) = 0;
    }
    E->setShown(global<u8 *>(0x00faa5d8), nullptr, 0);
    E->cursorSlot3();
    E->finish();
    E->fn5996d0(1);
    global<u32>(0x007cc190) = 1;
    global<u32>(0x007cc150) = 3;
}
}
