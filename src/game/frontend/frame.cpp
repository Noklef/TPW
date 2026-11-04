#include "frame.h"

#include <cstdint>

#include "../core/gameclock.h"
#include "../core/gameheap.h"
#include "../core/soundset.h"
#include "../core/vcall.h"
#include "../core/vdel.h"
#include "../core/vdtor.h"
#include "../files/userslots.h"
#include "../online/session.h"
#include "../ui/dialog7cb3f8.h"
#include "../ui/dialogf82884child.h"
#include "../ui/panelparts.h"
#include "../ui/versionlabel.h"
#include "../ui/window.h"
#include "../ui/windowscript.h"
#include "islandcamera.h"
#include "lobbyscene.h"

namespace {

template <typename T> T &at(u8 *base, u32 offset) { return *reinterpret_cast<T *>(base + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>(static_cast<uintptr_t>(address)); }

void Delete(u8 *&object) {
    if (object) vcall<void>(object, 0, 1u);   // (the deleting destructor)
    object = nullptr;
}

const FrameEnv kGameEnv = {&DialogF82884_ShowChild, &Window_CallDefault};
const FrameEnv *g_env = &kGameEnv;

const FrameStartEnv kGameStartEnv = {&WindowScript_Build, &Window_SetFocus, &Window_FindChild, &Window_SetHandler, &UserSlots_Ctor, &Window_SendMessage, &VersionLabel_Create, &Clocks_Now,
                                     &Sound_RegisterGlobalLobbySfx, &IslandCamera_LoadSettings, &Class702a60_Ctor, &Class702a90_Ctor, &Class702b08_Ctor, &Class702d78_Ctor, &Online_Construct, &Dialog7cb3f8_Open};
const FrameStartEnv *g_start = &kGameStartEnv;

u8 *Make(u32 size, u8 *(__fastcall *ctor)(u8 *)) {
    u8 *const object = static_cast<u8 *>(GameHeap_Alloc(size));
    return object ? ctor(object) : nullptr;
}

u8 *UserSlots() {
    u8 *&slots = global<u8 *>(0x00f81ebc);
    if (!slots) slots = Make(0x110, g_start->userSlots);
    return slots;
}

void ShowPanel(u32 offset) { vcall<void>(at<u8 *>(global<u8 *>(0x00f85614), offset), 0x1c); }

}  // namespace

void Frame_SetEnv(const FrameEnv *env) { g_env = env ? env : &kGameEnv; }
void Frame_SetStartEnv(const FrameStartEnv *env) { g_start = env ? env : &kGameStartEnv; }

extern "C" {

u32 __fastcall Frame_Start(u8 *frame) {
    at<u32>(frame, 0x14) = 1;
    if (u8 *const maker = global<u8 *>(0x00faa5d8)) {
        u8 *const window = g_start->build(maker, nullptr, reinterpret_cast<u8 *>((uintptr_t)0x00774c18), 0x005d58b0);
        at<u8 *>(frame, 4) = window;
        g_start->setFocus(window, nullptr, 0);
        u8 *const child = g_start->findChild(at<u8 *>(frame, 4), nullptr, 0xbf432);
        g_start->setHandler(child, nullptr, 0x005d58e0);
        if (!at<u32>(UserSlots(), 0xc4)) g_start->send(child, 6, 0, 0);
        g_start->label(0);
    }
    const u32 now = g_start->now(reinterpret_cast<u8 *>((uintptr_t)0x00785970));
    at<u32>(frame, 0x2c) = now;
    at<u32>(frame, 0x28) = now;
    at<u32>(frame, 0x1c) = 0;
    g_start->sound();
    u8 *const settings = static_cast<u8 *>(GameHeap_Alloc(0x6c));
    if (settings) at<u32>(settings, 0) = 0x007029fc;
    at<u8 *>(frame, 0x10) = settings;
    g_start->camera(settings);
    global<u8 *>(0x00f83774) = Make(0x10, g_start->make702a60);
    global<u8 *>(0x00f83778) = Make(0x64, g_start->make702a90);
    global<u8 *>(0x00f83784) = Make(8, g_start->make702b08);
    global<u8 *>(0x00f85614) = Make(0x1c, g_start->make702d78);
    u8 *node = at<u8 *>(at<u8 *>(frame, 0x18), 4);
    while (node) {
        u8 *const object = at<u8 *>(node, 0);
        node = at<u8 *>(node, 4);
        vcall<void>(object, 4);
    }
    u8 *&session = global<u8 *>(0x00f7d88c);
    if (!session) session = Make(0x404, g_start->online);
    if (at<u32>(session, 0x108)) {
        ShowPanel(0x10);
    } else if (global<u32>(0x00f8288c)) {
        ShowPanel(0xc);
        global<u32>(0x00f8288c) = 0;
    } else {
        if (at<s32>(UserSlots(), 0x60) == -1) g_start->openDialog();
        ShowPanel(4);
    }
    ShowPanel(0x14);
    return at<u32>(frame, 0x14);
}

s32 __fastcall Frame_HandleMessage(u8 *frame, void *, u8 *window, u32 message, u32 wparam, u32 lparam) {
    const bool has_frame = global<u32>(0x00f82884) != 0;
    switch (message) {
        case 0x11:
            if (wparam == 1) g_env->showChild();
            break;
        case 0x14:
            if (has_frame) at<u32>(frame, 4) = 0;
            break;
        case 0x22:
            at<u32>(frame, 0x1c) = 0;
            break;
        case 0x10003:
            at<u32>(frame, 0x20) = wparam >> 16;
            at<u32>(frame, 0x24) = wparam & 0xffff;
            break;
        case 0x10004:
            if (wparam < 3) at<u8>(frame, 0x1c) &= static_cast<u8>(~(1u << wparam));
            break;
        case 0x10005:
            if (wparam < 3) at<u8>(frame, 0x1c) |= static_cast<u8>(1u << wparam);
            break;
    }
    if (global<u32>(0x00f82884)) {
        for (u8 *node = at<u8 *>(at<u8 *>(frame, 0x18), 4); node;) {
            u8 *const object = at<u8 *>(node, 0);
            node = at<u8 *>(node, 4);
            vcall<void>(object, 0x14, message, wparam, lparam);
        }
    }
    return g_env->callDefault(window, nullptr, message, wparam, lparam);
}

s32 __cdecl Frame_WindowProc(u8 *window, u32 message, u32 wparam, u32 lparam) {
    u8 *const frame = global<u8 *>(0x00f82884);
    return frame ? Frame_HandleMessage(frame, nullptr, window, message, wparam, lparam) : 0;
}

u32 __fastcall Class7029f0_Close(u8 *frame) {
    for (u8 *node = at<u8 *>(at<u8 *>(frame, 0x18), 4); node;) {
        u8 *const object = at<u8 *>(node, 0);
        node = at<u8 *>(node, 4);
        vcall<void>(object, 0x10);
    }
    Delete(global<u8 *>(0x00f83784));
    Delete(global<u8 *>(0x00f83778));
    Delete(global<u8 *>(0x00f83774));
    Delete(global<u8 *>(0x00f85614));
    Delete(at<u8 *>(frame, 0x10));
    Sound_ReleaseLobbySfx();
    Window_SendMessage(at<u8 *>(frame, 4), 4, 0, 0);
    return at<u32>(frame, 0x14);
}
}
