// The front end's entry points: see frontendentries.h.
#include "frontendentries.h"

#include <cstdint>
#include "dialog7cc4b4open.h"

namespace {

template <typename T> T game(u32 address) { return reinterpret_cast<T>((uintptr_t)address); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
FrontEndEntriesEnv::Plain plain(u32 address) { return game<FrontEndEntriesEnv::Plain>(address); }
FrontEndEntriesEnv::One one(u32 address) { return game<FrontEndEntriesEnv::One>(address); }
FrontEndEntriesEnv::Method method(u32 address) { return game<FrontEndEntriesEnv::Method>(address); }

const FrontEndEntriesEnv kGameEnv = {plain(0x0048a580), plain(0x00489ca0), plain(0x00489ec0), plain(0x00489f50), plain(0x0048c980), plain(0x0048a410), plain(0x0048a520), plain(0x0048a6e0),
                                     plain(0x00662420), plain(0x004a27c0), plain(0x004a27f0), plain(0x00662411),
                                     plain(0x0049ac60), plain(0x0049fb30), plain(0x004b2750), plain(0x00498d80), plain(0x004aa480), plain(0x004989d0), plain(0x00497bc0), plain(0x00486510), plain(0x00486630),
                                     plain(0x004b4280), plain(0x004bbbd0),
                                     one(0x00486d90), one(0x0048c830), one(0x004b3f60), one(0x0048ac40), one(0x0048ae70), one(0x004bfc70),
                                     method(0x005f0b40), method(0x0048cf70), method(0x004cf3e0), method(0x00505490),
                                     game<u32(__cdecl *)(u8 *, u32, u32, u32)>(0x00658d9f)};
const FrontEndEntriesEnv *g_env = &kGameEnv;

}  // namespace

void FrontEndEntries_SetEnv(const FrontEndEntriesEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u32 __cdecl FrontEnd_GuiFrame() { return g_env->guiFrame(); }
u32 __cdecl FrontEnd_GuiStartup() { return g_env->guiStartup(); }
u32 __cdecl FrontEnd_GuiTeardown() { return g_env->guiTeardown(); }

u32 __cdecl FrontEnd_OpenPanels() {
    g_env->panelOpen();
    return g_env->clearGlobals();
}

u32 __cdecl FrontEnd_ShowWindow() { return g_env->panelShowWindow(); }
u32 __cdecl FrontEnd_ExitPanel() { return g_env->panelExit(); }
u32 __cdecl FrontEnd_OpenOnGui() { return g_env->openOnGui(); }
u32 __cdecl FrontEnd_Pause() { return FrontEnd_PauseTimers(); }
u32 __cdecl FrontEnd_Resume() { return FrontEnd_ResumeTimers(); }
u32 __stdcall FrontEnd_MouseHelp(u32 forced) { return g_env->mouseHelp(forced); }
u32 __cdecl FrontEnd_Other5f0b40() { return g_env->other5f0b40(reinterpret_cast<u8 *>((uintptr_t)0x00f86db8)); }
u32 __cdecl FrontEnd_PanelMenu() { return g_env->panelMenu(0); }
u32 __cdecl FrontEnd_Dialog7ca2e8() { return g_env->dialog7ca2e8(); }
u32 __cdecl FrontEnd_Dialog7cb25c() { return g_env->dialog7cb25c(); }
u32 __cdecl FrontEnd_Dialog7cc2ec() { return g_env->dialog7cc2ec(); }
u32 __cdecl FrontEnd_Dialog7ca210() { return g_env->dialog7ca210(); }

u32 __cdecl FrontEnd_Browser() {
    if (global<u32>(0x007cc418)) return g_env->browserOpen(global<u32>(0x007c2658));
    return g_env->openShown();
}

u32 __cdecl FrontEnd_Research() { return g_env->research(); }
u32 __cdecl FrontEnd_Hide7ca1c8() { return g_env->hide7ca1c8(); }
u32 __stdcall FrontEnd_Open74fb20(u32 thing) { return g_env->open74fb20(thing); }
u32 __stdcall FrontEnd_Open74faf0(u32 thing) { return g_env->open74faf0(thing); }

u32 __cdecl FrontEnd_PauseTimers() {
    const u32 root = global<u32>(0x00faa5d8);
    if (!root) return root;
    g_env->restartTimers();
    return g_env->closeTimers();
}

u32 __cdecl FrontEnd_ResumeTimers() {
    g_env->toggle();
    return global<u32>(0x007cb2cc) ? g_env->resumeTimer() : g_env->restartTimers();
}

u32 __cdecl FrontEnd_EnsureMode() { return g_env->ensureMode(); }
u32 __cdecl FrontEnd_SignDestroy() { return g_env->signDestroy(); }
u32 __cdecl FrontEnd_PreviewHide() { return g_env->previewHide(); }
u32 __fastcall FrontEnd_Message4(u8 *object) { return g_env->message4(object); }

u32 __cdecl FrontEnd_Dialog7cc4b4Child() {
    g_env->send(gDialog7cc4b4open_7cc4b4, 6, 1, 0);
    return g_env->child7cc4b4();
}

u32 __cdecl FrontEnd_ReplyList(u32 a, u32, u32 reply) { return a ? a : g_env->fillList(reply); }
u32 __fastcall FrontEnd_GatesStep(u8 *gates) { return g_env->gatesStep(gates); }
u32 __fastcall FrontEnd_TickSpeed(u8 *person) { return g_env->tickSpeed(person); }
}
