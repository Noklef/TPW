#include "scenesteps.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/options.h"
#include "../core/vcall.h"
#include "../files/userslots.h"
#include "../online/session.h"
#include "../ui/dialog7cc47cinfo.h"
#include "../ui/dialog7cc4b4update.h"
#include "../ui/dialog7cc984open.h"
#include "../ui/dialogs.h"
#include "../ui/frontendentries.h"
#include "lobbymake.h"
#include "lobbymodel.h"
#include "lobbyscene.h"
#include "scenequeue.h"
#include "sceneleave.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const SceneStepsEnv kPorts = {&LobbyModel_Play,   &FrontEnd_Dialog7cc4b4Child, &Dialog7cc4b4_ShowTheme, &Setting_ReadMode, reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)0x0067a7d0),
                              &Class702cb0_NextItem, &Class702cb0_PreviousItem, &Online_ResetConnections, &Dialog7cc47c_Message6On, &Dialog7cc47c_Message6Off, &Dialog7cc984_Open, &Setting_SetMode,
                              &Class702cb0_Step, &Dialog7cc47cInfo_Show, &UserSlots_Instance, &Class702da0_Show, &Class702bb8_Ctor, &Class702da0_Push};
const SceneStepsEnv *g_env = &kPorts;

u8 *Setting() {
    u8 &guard = global<u8>(0x00fb2758);
    u8 *const setting = reinterpret_cast<u8 *>((uintptr_t)0x00fb3b7c);
    if (!(guard & 1)) {
        guard |= 1;
        g_env->readMode(setting);
        g_env->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005d1750));
    }
    return setting;
}

u32 EndState(u8 *self) {
    const u32 state = at<u32>(self, 0x14);
    if (state == 0)
        return 0;
    if (state == 2)
        g_env->play(at<u8 *>(at<u8 *>(self, 0xc), 8), nullptr, 1, 0);
    at<u32>(self, 0x14) = 0;
    return 1;
}

void Step(u8 *self, void(__fastcall *step)(u8 *)) {
    if (at<u32>(self, 0x14) != 0 || at<u32>(Setting(), 0) == 2)
        return;
    step(self);
    g_env->showChild();
    g_env->showTheme();
}

}  // namespace

void SceneSteps_SetEnv(const SceneStepsEnv *env) { g_env = env ? env : &kPorts; }

extern "C" {

u32 __fastcall Scene702ec0_EndState(u8 *self) {
    const u32 ended = EndState(self);
    g_env->showChild();
    g_env->showTheme();
    return ended;
}

void __fastcall Scene702ec0_Next(u8 *self) { Step(self, g_env->next); }

void __fastcall Scene702ec0_Previous(u8 *self) { Step(self, g_env->previous); }

u32 __fastcall Scene702dd0_EndState(u8 *self) {
    u32 result = EndState(self);
    if (result || at<u32>(Online_Instance(), 0x384)) {
        g_env->resetConnections(Online_Instance());
        result = 1;
    }
    g_env->dialogOn();
    return result;
}

static void StepAndTell(u8 *self, void(__fastcall *step)(u8 *)) {
    if (at<u32>(self, 0x14) != 0)
        return;
    step(self);
    at<u32>(self, 0xe8) = 1;
    g_env->dialogOn();
}

void __fastcall Scene702dd0_Next(u8 *self) { StepAndTell(self, g_env->next); }

void __fastcall Scene702dd0_Previous(u8 *self) { StepAndTell(self, g_env->previous); }

void __fastcall Scene702dd0_Step(u8 *self) {
    g_env->step(self);
    if (!at<u32>(self, 0xe4) || !at<u32>(self, 0xe8))
        return;
    const u8 *const holder = at<const u8 *>(self, 0xc);
    if (holder && at<u32>(holder, 0x70))
        g_env->showRecord(holder);
}

void __fastcall Scene702ec0_Step(u8 *self) {
    g_env->step(self);
    if (at<s32>(g_env->users(), 0x60) == -1)
        return;
    const u32 id = (Crt().rand() & 1) ? 0x18b : 0x18a;
    g_env->queueShow(at<u8 *>(global<u8 *>(0x00f85614), 0x14), nullptr, id, 0x15f90);
}

u32 __fastcall ViewCtl702e70_Back(u8 *self) {
    if (!at<u32>(Online_Instance(), 0x384))
        return 0;
    g_env->resetConnections(Online_Instance());
    vcall<void>(self, 0x44);
    return 1;
}

u32 __cdecl ViewCtl702bc8_ListDone(u32 failed, u8 *object, u32 list) {
    return failed ? vcall<u32>(object, 0x2c) : vcall<u32>(object, 0x28, list);
}

void __fastcall Scene702dd0_OpenRecord(u8 *self) {
    if (at<u32>(self, 0x14) != 0)
        return;
    const u8 *const holder = at<const u8 *>(self, 0xc);
    if (!holder || !at<u32>(holder, 0x70))
        return;
    at<u32>(self, 0xe8) = 0;
    g_env->dialogOff();
    g_env->open(at<const u8 *>(at<const u8 *>(self, 0xc), 0x70), nullptr, 0x005e2650, 0x005e2660, reinterpret_cast<u32>(self));
}

void __fastcall ViewCtl702bc8_TakeList(u8 *self, void *, const u8 *list) {
    const u8 *node = at<const u8 *>(list, 4);
    while (node) {
        const u8 *const item = at<const u8 *>(node, 0);
        node = at<const u8 *>(node, 4);
        if (vcall<u32>(self, 0x40, item)) {
            u8 *const marker = static_cast<u8 *>(GameHeap_Alloc(0x10));
            if (marker)
                g_env->makeMarker(marker, nullptr, self, item);
        }
    }
    at<u32>(self, 0x68) = 1;
}

void __fastcall ViewCtl702e70_TakeList(u8 *self, void *, const u8 *list) {
    ViewCtl702bc8_TakeList(self, nullptr, list);
    g_env->queuePush(at<u8 *>(global<u8 *>(0x00f85614), 0x14), nullptr, 0x256, 1);
}

void __cdecl Scene_SetOnlineMode() {
    at<u32>(global<u8 *>(0x00f82884), 0x14) = 2;
    g_env->setMode(Setting(), nullptr, 1);
}
}
