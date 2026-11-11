#include "sceneleave.h"

#include <cstdint>

#include "../core/soundplay.h"
#include "../core/soundset.h"
#include "../core/vcall.h"
#include "../core/vregs.h"
#include "../core/vobj2.h"
#include "../model/holder.h"
#include "../online/session.h"
#include "../ui/dialogs.h"
#include "frame.h"
#include "../core/staticobjects.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const SceneLeaveEnv kPorts = {&Sound_Stop, &Sound_PlayAt, &Sound_ApplySettings, &Obj_DrainList0c, &Obj_DrainList04, &HolderUser_Release, &Dialog7cdb0c_Message4, &Dialog7cc4c4_Message4, &Dialog7cc4b4_Message4, &Dialog7cc47c_Message4, &Obj_SetNameAndWords};
const SceneLeaveEnv *g_env = &kPorts;

bool Active(u8 *self) { return vcall<u32>(self, 0x24) != 0; }

void StopSounds(u8 *self) {
    u8 *const item = at<u8 *>(self, 0xc);
    if (!item) return;
    g_env->stop(at<u32>(item, 0x1c));
    g_env->stop(at<u32>(item, 0x20));
    at<u32>(item, 0x20) = 0;
    at<u32>(item, 0x1c) = 0;
}

void PlaySounds(u8 *item) {
    at<u32>(item, 0x1c) = g_env->playAt(at<u32>(item, 0x1c), at<u32>(gSoundCategories.lobbyPairs, at<u32>(item, 0x18) * 8 + 4), 1, 0, 0, 0);
    at<u32>(item, 0x20) = g_env->playAt(at<u32>(item, 0x20), at<u32>(gSoundCategories.lobbyPairs, at<u32>(item, 0x18) * 8), 2, 0, 0, 0);
    g_env->apply();
}

void LeaveView(u8 *self) {
    at<u32>(self, 4) = 0;
    g_env->message7cdb0c();
    ObjList_Drain(self, 8);
    ObjList_Drain(self, 0x6c);
    g_env->drain0c(gFrame_f83774);
    global<u32>((u32)(uintptr_t)&gView_8bcbc8) &= 0xfdffffff;
    g_env->stop(at<u32>(self, 0x74));
    g_env->stop(at<u32>(self, 0x78));
    at<u32>(self, 0x78) = 0;
    at<u32>(self, 0x74) = 0;
}

void LeaveCurrent(u8 *self) {
    at<u32>(self, 4) = 0;
    StopSounds(self);
}

void LeaveScene(u8 *self, s32(__cdecl *message)()) {
    if (!Active(self)) return;
    if (Active(self)) LeaveCurrent(self);
    ObjList_Drain(self, 8);
    g_env->release(gFrame_f83774);
    g_env->drain04(gFrame_f83784);
    g_env->drain0c(gFrame_f83774);
    vcall<u32>(self, 0x2c);
    message();
}

// The node holding the item (0 for none).
u8 *FindNode(u8 *list, u8 *item) {
    u8 *node = at<u8 *>(list, 4);
    while (node && at<u8 *>(node, 0) != item) node = at<u8 *>(node, 4);
    return node;
}

void Choose(u8 *self, u8 *item) {
    vcall<u32>(self, 0x4c, item);
    u8 *const current = at<u8 *>(self, 0xc);
    if (current) PlaySounds(current);
}

}  // namespace

void SceneLeave_SetEnv(const SceneLeaveEnv *env) { g_env = env ? env : &kPorts; }

extern "C" {

void __fastcall ViewCtl702bc8_Leave(u8 *self) {
    if (Active(self)) LeaveView(self);
}

void __fastcall ViewCtl702e70_Leave(u8 *self) {
    if (!Active(self)) return;
    if (Active(self)) LeaveView(self);
    g_env->message7cc4c4();
}

void __fastcall ViewCtl702e20_Leave(u8 *self) {
    if (!Active(self)) return;
    if (Active(self)) LeaveView(self);
    g_env->message7cc4c4();
    g_env->setNameWords(Online_Instance(), nullptr, 0, nullptr, nullptr);
}

void __fastcall Class702cb0_Leave(u8 *self) {
    if (Active(self)) LeaveCurrent(self);
}

void __fastcall Scene702ec0_Leave(u8 *self) { LeaveScene(self, g_env->message7cc4b4); }

void __fastcall Scene702dd0_Leave(u8 *self) { LeaveScene(self, g_env->message7cc47c); }

void __fastcall Class702cb0_PlayFirst(u8 *self) {
    StopSounds(self);
    u8 *const list = at<u8 *>(self, 8);
    if (!list || at<u32>(list, 0xc) == 0) return;
    u8 *const first = at<u8 *>(at<u8 *>(list, 4), 0);
    if (first) PlaySounds(first);
}

void __fastcall Class702cb0_NextItem(u8 *self) {
    if (at<u32>(self, 0x14) != 0 || at<u32>(at<u8 *>(self, 8), 0xc) == 0) return;
    StopSounds(self);
    u8 *const list = at<u8 *>(self, 8);
    u8 *const current = at<u8 *>(self, 0xc);
    // (None or the last: the list's own +4, the first node.)
    u8 *const node = !current || current == at<u8 *>(at<u8 *>(list, 8), 0) ? list : FindNode(list, current);
    Choose(self, at<u8 *>(at<u8 *>(node, 4), 0));
}

void __fastcall Class702cb0_PreviousItem(u8 *self) {
    if (at<u32>(self, 0x14) != 0 || at<u32>(at<u8 *>(self, 8), 0xc) == 0) return;
    StopSounds(self);
    u8 *const list = at<u8 *>(self, 8);
    u8 *const current = at<u8 *>(self, 0xc);
    u8 *const before = !current || current == at<u8 *>(at<u8 *>(list, 4), 0) ? at<u8 *>(list, 8) : at<u8 *>(FindNode(list, current), 8);
    Choose(self, at<u8 *>(before, 0));
}
}
