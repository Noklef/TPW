#include "sceneenter.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/soundplay.h"
#include "../core/soundset.h"
#include "../core/vcall.h"
#include "../math/x87.h"
#include "../model/holder.h"
#include "../online/listrequest.h"
#include "../online/session.h"
#include "../render/lightsource.h"
#include "../render/state.h"
#include "../render/weatherfx.h"
#include "../ui/dialog7cc47c.h"
#include "lobbymake.h"
#include "scenequeue.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const SceneEnterEnv kPorts = {&WeatherFx_SetDropCount, &Render_FogOff, &Render_SetAmbient, &LightSlot_Set, &Sound_PlayAt, &LobbyScene_MakeModel, &RideUser_Acquire, &Online_ResetConnections,
                              &ListRequest_Start30, &Class702da0_Clear, &Dialog7cc47c_Create, &HolderUser_Acquire, &Render_FogOn, &Render_SetFog, &ListRequest_Start66, &Sound_ApplySettings};
const SceneEnterEnv *g_env = &kPorts;

u8 *View() { return global<u8 *>(0x00f83778); }

u32 Active(u8 *self) { return vcall<u32>(self, 0x24); }

}  // namespace

void SceneEnter_SetEnv(const SceneEnterEnv *env) { g_env = env ? env : &kPorts; }

extern "C" {

void __fastcall ViewCtl702bc8_Enter(u8 *self) {
    if (Active(self))
        return;
    at<u32>(self, 4) = 1;
    at<u32>(self, 0x44) = 0;
    at<u32>(self, 0x48) = 0;
    at<u32>(self, 0x4c) = 0;
    at<u32>(self, 0x50) = 0;
    at<u32>(self, 0x54) = at<u32>(at<u8 *>(global<u8 *>(0x00f82884), 0x10), 0x14);
    at<u32>(View(), 4) = 0x42700000;
    g_env->setDrops(global<u8 *>(0x008bcbac), nullptr, 0);
    g_env->fogOff(View());
    at<u32>(self, 0x58) = 0;
    at<u32>(self, 0x10) = at<u32>(self, 0x1c);
    at<u32>(self, 0x14) = at<u32>(self, 0x20);
    at<u32>(self, 0x18) = at<u32>(self, 0x24);
    global<u32>(0x008bcbc8) |= 0x2000000;
    g_env->ambient(View(), nullptr, 1, 0x202020);
    const u32 white = 0xffffff;
    g_env->light(View(), nullptr, 1, &white, nullptr, 1);
    const u32 extent = X87_FloatThrough(at<u32>(self, 0x38));
    at<u32>(self, 0x34) = extent;
    at<u32>(View(), 0x28) = extent;
    at<u32>(View(), 0x30) = 0;
    at<u32>(View(), 0x2c) = extent ^ 0x80000000;
    at<u32>(at<u8 *>(global<u8 *>(0x00f85614), 0x14), 0x18) = 0;
    at<u32>(View(), 0x50) = 0xf30000;
    at<u32>(View(), 0x54) = 0xcb0000;
    at<u32>(View(), 0x58) = 0xbf0000;
    at<u32>(self, 0x74) = g_env->playAt(0, global<u32>(0x00803a48), 2, 0, 0, 0);
    at<u32>(self, 0x78) = g_env->playAt(0, global<u32>(0x00803a48), 3, 0, 0, 0);
    u8 *const part = static_cast<u8 *>(GameHeap_Alloc(0x18));
    if (part) {
        at<u32>(part, 0) = 0x00702bb4;
        u8 *const model = g_env->makeModel(global<u8 *>(0x00f83774), nullptr, reinterpret_cast<const char *>(0x00774c80), reinterpret_cast<const char *>(0x00774cc0), 1, 1, 0, 0);
        at<u8 *>(part, 4) = model;
        g_env->acquire(model);
        at<u8 *>(part, 8) = self;
        vcall<void>(self, 0x34, part);
    }
    at<u8 *>(self, 0xc) = part;
    at<u32>(self, 0x68) = 0;
    g_env->resetConnections(Online_Instance());
    g_env->start30(at<u8 *>(Online_Instance(), 0xe0), nullptr, 0x005dcfc0, reinterpret_cast<u32>(self));
}

void __fastcall Class702cb0_Enter(u8 *self) {
    if (Active(self))
        return;
    at<u32>(self, 4) = 1;
    g_env->holderAcquire(global<u8 *>(0x00f83774));
    g_env->ambient(View(), nullptr, 1, 0x444444);
    const u32 colour = 0xeeeeee;
    g_env->light(View(), nullptr, 1, &colour, nullptr, 1);
    at<u32>(View(), 0x28) = 0x40000000;
    at<u32>(View(), 0x2c) = 0x41000000;
    at<u32>(View(), 0x30) = 1;
    at<u32>(View(), 4) = X87_FloatThrough(at<u32>(at<u8 *>(global<u8 *>(0x00f82884), 0x10), 4));
    g_env->fogOn(View());
    g_env->setFog(View(), nullptr, 0xff44ddff, 0x42480000, 0x437a0000);
    at<u32>(self, 0x14) = 0;
    at<u32>(self, 0xc) = 0;
    at<u32>(self, 0x10) = 0;
    g_env->resetConnections(Online_Instance());
}

void __fastcall Scene702dd0_Enter(u8 *self) {
    if (Active(self))
        return;
    g_env->clearQueue(at<u8 *>(global<u8 *>(0x00f85614), 0x14));
    g_env->createDialog();
    vcall<void>(self, 0x28);
    Class702cb0_Enter(self);
    at<u32>(self, 0xe4) = 0;
    g_env->start66(at<u8 *>(Online_Instance(), 0xe4), nullptr, at<u32>(global<u8 *>(0x00f82888), 8), 0x005e23c0, reinterpret_cast<u32>(self));
    u8 *const item = at<u8 *>(self, 0xc);
    if (!item)
        return;
    u8 *const table = global<u8 *>(0x00803a4c);
    at<u32>(item, 0x1c) = g_env->playAt(at<u32>(item, 0x1c), at<u32>(table, at<u32>(item, 0x18) * 8 + 4), 1, 0, 0, 0);
    at<u32>(item, 0x20) = g_env->playAt(at<u32>(item, 0x20), at<u32>(table, at<u32>(item, 0x18) * 8), 2, 0, 0, 0);
    g_env->apply();
}
}
