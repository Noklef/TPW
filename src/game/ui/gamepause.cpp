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
#include "dialog7cc184handler.h"
#include "../render/d3dsetup.h"
#include "../online/postcardsend.h"
#include "../core/imports.h"
#include "../core/staticobjects.h"
#include "../core/addrcalls.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename F> F byAddress(u32 address) { return GameAddress<F>(address); }
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
    if (!(gGamepause_fb34bf & 1)) {
        gGamepause_fb34bf |= 1;
        g_env->readMode(object(reinterpret_cast<uintptr_t>(&gDialog74fb20_fb3b7c)));
        g_env->atExit(0x005d1750);
    }
    return gDialog74fb20_fb3b7c == 1;
}

}  // namespace

extern "C" {

void GamePause_SetEnv(const GamePauseEnv *env) { g_env = env ? env : &kGameEnv; }

void __cdecl GamePause_RemakePicture() {
    const GamePauseEnv *const E = g_env;
    u8 *const old = gDialog7cc184handler_7cc18c;
    if (old) {
        E->blocksFree(old);
        E->release(old);
    }
    u8 *const block = static_cast<u8 *>(E->alloc(0x70));
    u8 *const made = block ? E->blocksCtor(block) : nullptr;
    gDialog7cc184handler_7cc18c = made;
    E->fn5d47a0(made);
    u8 desc[0x7c];
    E->fn564430(global<u8 *>((u32)(uintptr_t)&gD3dsetup_879a20 + 0x4), nullptr, reinterpret_cast<SurfaceDesc *>(desc));
    E->shrink(gDialog7cc184handler_7cc18c, nullptr, reinterpret_cast<const SurfaceDesc *>(desc));
    E->storeCom(global<u8 *>((u32)(uintptr_t)&gD3dsetup_879a20 + 0x4));
    E->panelReset(gDialog7cc184handler_7cc18c);
}

void __cdecl GamePause_Resume() {
    const GamePauseEnv *const E = g_env;
    gDialog7cc184handler_7cc1a0 = 0;
    if (!SettingOne()) {
        global<u32>((u32)(uintptr_t)&gNamedObjects_786b68 + 0x3c) = 1;
        E->enter(object((u32)(uintptr_t)&gNamedObjects_786b68));
    }
    GamePause_RemakePicture();
    E->setShown(global<u8 *>(0x00faa5d8), nullptr, 1);
    E->cursorSlot2();
    E->cameraLeave();
    if (SettingOne())
        E->selectFirst();
    else if (E->overlayState())
        E->overlayToggle();
    if (!gGamePause_Resume_7cc194) {
        global<u32(__stdcall *)(u32, u32)>(IMPORT(Kernel32_GetTempPathA))(0x200, (u32)(uintptr_t)&gPostcardsend_7cb958);
        gGamePause_Resume_7cc194 = 1;
    }
    E->openDialog();
    gGamepause_7cc190 = 0;
}

void __cdecl GamePause_Hide() {
    const GamePauseEnv *const E = g_env;
    E->play(0x95);
    if (SettingOne()) {
        E->choose();
    } else {
        E->leave(object((u32)(uintptr_t)&gNamedObjects_786b68), nullptr, 0, 0);
        global<u32>((u32)(uintptr_t)&gNamedObjects_786b68 + 0x3c) = 0;
    }
    E->setShown(global<u8 *>(0x00faa5d8), nullptr, 0);
    E->cursorSlot3();
    E->finish();
    E->fn5996d0(1);
    gGamepause_7cc190 = 1;
    reinterpret_cast<u32 &>(gGamepause_7cc150) = 3;
}
}
