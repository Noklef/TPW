// The UI's one-call entries: see menuactions.h.
#include "menuactions.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../frontend/scenequeue.h"
#include "../world/advisor.h"
#include "../world/focuscell.h"
#include "../world/gamemode.h"
#include "../world/mapclick.h"
#include "../world/object.h"
#include "../world/ridesweep.h"
#include "../world/staff.h"
#include "dialog7ca098.h"
#include "dialog7ca13c.h"
#include "dialog7ca150.h"
#include "dialog7ca168.h"
#include "dialog7ca210.h"
#include "dialog7ca308.h"
#include "dialog7cb398.h"
#include "dialog7cc1f8.h"
#include "dialogbase.h"
#include "dialogs.h"
#include "msgtags.h"
#include "window.h"
#include "dialog7cb2fcopen.h"
#include "dialog7ca1e0.h"
#include "window7cc8bc.h"
#include "panel7cc218.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(u8 *p, u32 offset) { return *reinterpret_cast<T *>(p + offset); }

const MenuActionsEnv kGameEnv = {&Dialog7cc1f8_Open, &Dialog7ca308_Open, &Dialog7ca150_Open, &Dialog7ca168_Open, &Dialog7ca13c_Open, &Dialog7cb398_Create, &Dialog7ca308_StaffChanged, &MsgTags_Save, &Dialog7ca13c_AddRow, &StaffPool_CountOfType, &Staff_SetName, &Staff_GetName, &Window_SendMessage,
    &Dialog7cb2b8_Child21Post, &Advisor_PostId, &Dialog7cb2b8_ResetChild21, &Game_CancelAdvisor, &Object_RestartRide, &Object_Close, &Object_SetFlag64, &Global7c2654_Set, &FocusCell_Update, &GameMode_Set, &GameMode_MapClick, &Class702da0_Clear, &Things_StepGroup, &Things_StepGroupBack, &Dialog_KeyHandler, &Window_DwordA0};
const MenuActionsEnv *g_env = &kGameEnv;

u8 *shownThing() { return gPanel7cc218_7c2658; }

template <typename R>
R slot(u8 *object, u32 offset) {
    using Fn = R(__fastcall *)(u8 *);
    return (*reinterpret_cast<Fn *>(*reinterpret_cast<u8 **>(object) + offset))(object);
}

}  // namespace

void MenuActions_SetEnv(const MenuActionsEnv *env) { g_env = env ? env : &kGameEnv; }

void __cdecl Wrap_Dialog7cc1f8_Open_4816e0() { g_env->open7cc1f8(0xffffffff, 0); }
void __cdecl Wrap_Dialog7ca308_Open_481700() { g_env->open7ca308(0xffffffff); }
void __cdecl Wrap_Dialog7cb398_Create_481710() { g_env->create7cb398(); }
void __cdecl Wrap_Dialog7ca150_Open_481720() { g_env->open7ca150(-1, 0); }
void __cdecl Wrap_Dialog7ca168_Open_481730() { g_env->open7ca168(0xffffffff, 0); }
void __cdecl Wrap_Dialog7ca13c_Open_481740() { g_env->open7ca13c(0); }

void __stdcall Wrap_Dialog7ca308_StaffChanged_481550(u32 index) { g_env->staffChanged(index); }
void __stdcall Wrap_MsgTags_Save_4816a0(Stream *stream) { g_env->saveTags(stream); }

u32 __cdecl Wrap_Dialog7ca13c_AddRow_4937e0(u8 *guest) {
    g_env->addRow(guest);
    return 1;
}

u32 __stdcall World_StaffCountOfType_4c9e40(u32 type) { return g_env->countOfType(global<u8 *>(0x0080239c), nullptr, type); }

void __stdcall ShownThing_SetName_4b60d0(const u16 *name) {
    if (shownThing()) g_env->setName(shownThing(), nullptr, name);
}

u16 *__cdecl ShownThing_GetName_4b60f0() { return shownThing() ? g_env->getName(shownThing()) : nullptr; }

u32 __cdecl Game_Part8Slot28_4b8510() { return slot<u32>(at<u8 *>(global<u8 *>(0x00f85614), 8), 0x28); }
u32 __cdecl Game_Part10Slot2c_4c4470() { return slot<u32>(at<u8 *>(global<u8 *>(0x00f85614), 0x10), 0x2c); }

void __cdecl Window7cb2fc_Close_4a4580() { g_env->send(gDialog7cb2fcopen_7cb2fc, 4, 0, 0); }
void __cdecl Window7cc8bc_Close_4bf8e0() { g_env->send(gWindow7cc8bc_7cc8bc, 4, 0, 0); }
void __cdecl Window7cc4c4_Send6_4b9c40() { g_env->send(gDialogs_7cc4c4, 6, 0, 0); }

u32 __cdecl Globals7cc684_Set_4bc640(u32 a, u32 b) {
    global<u32>(0x007cc684) = a;
    global<u32>(0x007cc65c) = b;
    return a;
}

u32 __stdcall Global7cc504_Set_4b9f60(u32 a, u32) {
    global<u32>(0x007cc504) = a;
    return a;
}

u32 __fastcall Obj_Reset24_4b4650(u8 *object) {
    at<u32>(object, 0x24) = 0;
    at<u32>(object, 0x28) = 0xffffffff;
    at<u32>(object, 0x2c) = 0xffffffff;
    return 0xffffffff;
}

u32 __fastcall Obj_Count28s_4d0690(const u8 *object) {
    u32 count = 0;
    for (u32 i = 0; i < 8; ++i)
        if (*reinterpret_cast<const u32 *>(object + 0x28 + i * 0x20)) ++count;
    return count;
}

void __cdecl Dialog7ca1e0_Post_498bb0() {
    g_env->child21Post(reinterpret_cast<u32 &>(gDialog7ca1e0_7ca1e0));
    g_env->postId(0x131);
}

void __cdecl Dialog7ca1e0_Cancel_498bd0() {
    g_env->resetChild21(0);
    g_env->cancelAdvisor(0x131);
}

void __fastcall ThingPanel_RestartOrClose_48ccf0(u8 *, void *, u32 restart) {
    if (restart) g_env->restartRide(shownThing());
    else g_env->closeObject(shownThing());
}

void __fastcall ThingPanel_SetFlag64_48ce50(u8 *panel, void *, u8 flag) {
    g_env->setFlag64(shownThing(), nullptr, flag == 0);
    slot<void>(panel, 0x14);
}

u32 __cdecl Thing_GoToCell_48cde0(u8 *thing) {
    const u32 cell = (u16)(((u32)thing[7] << 7) + thing[5] + 1);
    g_env->setThing((u32)(uintptr_t)thing);
    g_env->focusCell(1);
    g_env->setMode(0x33, 1);
    g_env->mapClick(static_cast<s32>((cell - 1) & 0x7f), static_cast<s32>((cell - 1) >> 7), 0, 0);
    return 1;
}

s32 __cdecl ThingPanel_Handler_48d200(u8 *window, u32 message, u32 a, u32 b) {
    u8 *const panel = reinterpret_cast<u8 *>((uintptr_t)g_env->dwordA0(window, nullptr, 6));
    if (message == 0x14) {
        if (shownThing()) slot<void>(panel, 0x3c);
        slot<void>(panel, 0x1c);
        slot<void>(panel, 0x20);
        slot<void>(panel, 8);
        at<u32>(panel, 0x10) = 0;
    } else if (message == 0x1000a) {
        if (a == 0x2600) g_env->stepBack(panel);
        else if (a == 0x2800) g_env->stepGroup(panel);
    }
    return g_env->keyHandler(window, message, a, b);
}

void __cdecl Dialog7cc984_Finish_4c07d0(u32 show) {
    g_env->clear702da0(at<u8 *>(global<u8 *>(0x00f85614), 0x14));
    g_env->send(global<u8 *>(0x007cc984), 4, 0, 0);
    reinterpret_cast<void(__cdecl *)(u32, u32)>(global<uintptr_t>(0x007cca4c))(show, global<u32>(0x007cca50));
    if (show) g_env->send(at<u8 *>(global<u8 *>(0x00f82884), 4), 6, 1, 0);
}

u8 *__cdecl ShownThing_NameBuffer() { return Thing_NameBuffer7ba058(gPanel7cc218_7c2658); }
