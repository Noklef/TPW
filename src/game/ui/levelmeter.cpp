#include "levelmeter.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/options.h"
#include "../core/selectors.h"
#include "../core/soundplay.h"
#include "../core/soundset.h"
#include "../core/volumes.h"
#include "../core/textarg.h"
#include "../core/vcall.h"
#include "../gfx/barmesh.h"
#include "../render/textparts.h"
#include "../world/advisor.h"
#include "../world/object.h"
#include "../world/objectdefs.h"
#include "../world/research.h"
#include "../world/staff.h"
#include "../world/thingscan.h"
#include "../world/world.h"
#include "dialog7ca098.h"
#include "dialogbase.h"
#include "focus.h"
#include "guimanager.h"
#include "hoverlabel.h"
#include "window.h"
#include "window705b20.h"
#include "windowscript.h"
#include "windowtimers.h"
#include "dialog74fb20.h"
#include "panel7b9940.h"

namespace {

constexpr u32 kLight = 0xd2d06;

u32 get(u8 *window, u32 index) { return Window_DwordA0(window, nullptr, index); }

void set(u8 *window, u32 index, u32 value) { Window_SetDwordA0(window, nullptr, index, value); }

void tell(u8 *window, u32 value) { Window_Notify1(Window_FindChild(window, nullptr, kLight), nullptr, value); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>(static_cast<u8 *>(p) + offset); }

// A research group's level: 0 without a current project (ResearchLab_CurrentObjId's id 0), else the lab's Table_ScaledBySum of it; the
// lab the game [0x7cf83c]'s (GameTable7cfb90_Get1da722_519550), found again for each.
u32 researchLevel(s32 group) {
    u8 *const lab = reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da722_519550(gWorldPtr));
    u16 id;
    if (*ResearchLab_CurrentObjId(lab, nullptr, &id, (u32)group) == 0) return 0;
    return Table_ScaledBySum(lab, nullptr, group);
}

constexpr u32 kMeter[5] = {0xd2d01, 0xd2d08, 0xd2d09, 0xd2d0a, 0xd2d07};   // (by group)

void showLevel(s32 group, u32 level) {
    LevelMeter_SetLevel(Window_FindChild(gLevelmeter_7cc1d8, nullptr, kMeter[group]), level);
}

}  // namespace

extern "C" {

void __cdecl ResearchMeters_ShowProgress(u32 group) {
    const u8 *const lab = reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da722_519550(gWorldPtr));
    const s32 done = 100 - (s32)ResearchLab_LeftPercent(lab, nullptr, group);
    if (group > 4) return;
    u8 *const meter = Window_FindChild(gLevelmeter_7cc1d8, nullptr, kMeter[group]);
    u8 *const gauge = Window_FindChild(meter, nullptr, 0xd2d05);
    vcall<u32>(gauge, 0x1c, (u32)((s32)((u32)done << 10) / 100));
}

void __cdecl ResearchMeters_Update(s32 group) {
    static const s32 kOrder[5] = {0, 4, 1, 2, 3};
    if (group == -1) {
        for (const s32 g : kOrder) {
            const u32 level = researchLevel(g);
            showLevel(g, level);
        }
    } else {
        const u32 level = researchLevel(group);
        if ((u32)group <= 4) showLevel(group, level);
    }
    u32 sum = 0;
    for (const s32 g : kOrder) sum += researchLevel(g);
    Sound_SetParam(gLevelmeter_7cc1d0, 0x12, (u32)((s32)sum / 5));
}

void __cdecl ResearchMeters_ShowName(u32 group) {
    u8 *const defs = reinterpret_cast<u8 *>((uintptr_t)0x007890a0);
    u8 *const lab = reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da722_519550(gWorldPtr));
    u32 id = 0;   // (the original's dword: its high word not set, and not read)
    ResearchLab_CurrentObjId(lab, nullptr, reinterpret_cast<u16 *>(&id), group);
    u8 *gauge = nullptr;
    if (group <= 4) gauge = Window_FindChild(Window_FindChild(gLevelmeter_7cc1d8, nullptr, kMeter[group]), nullptr, 0xd2d05);
    if (!ObjectDefs_Find(defs, nullptr, (u16)id)) {
        Window_NotifyImpl100(gauge, nullptr, (u32)(uintptr_t)Language_WideText(0xb9));
        ResearchMeters_Update((s32)group);
        return;
    }
    u16 name[0x44];
    ObjectDefs_GetFullName(defs, nullptr, id, name, 0x42);
    u8 count[0x14], title[0x10];   // (TextArgInt_Ctor (0xd, the value, unsigned) and TextArgString_Ctor (0xe, the name), inline)
    at<u32>(count, 0xc) = Slots_GetValue(lab, nullptr, group);
    at<u32>(count, 4) = at<u32>(title, 4) = 0x5641524d;
    at<u32>(title, 0xc) = (u32)(uintptr_t)name;
    at<u32>(count, 8) = 0xd;
    at<u32>(count, 0) = 0x006ff170;
    at<u32>(count, 0x10) = 0;
    at<u32>(title, 8) = 0xe;
    at<u32>(title, 0) = 0x006ff174;
    u16 text[0x80];
    TextFormat().format(global<void *>(0x0078ba94), text, 0x7f, 0x1be, title, count);
    Window_NotifyImpl100(gauge, nullptr, (u32)(uintptr_t)text);
}

void __cdecl ResearchMeters_Refresh(u32 group) {
    u8 *const lab = reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da722_519550(gWorldPtr));
    if (!ResearchLab_CurrentNotDone(lab, nullptr, group)) ResearchLab_NextProject(lab, nullptr, group);
    if (!gLevelmeter_7cc1d8) return;
    ResearchMeters_ShowName(group);
    ResearchMeters_ShowProgress(group);
}

void __fastcall Obj7b51f0_RefreshResearch(void *, void *, u32 group) { ResearchMeters_Refresh(group); }

// The five project arrows' handlers, one for each group (the original has five copies).
static s32 arrowProc(u32 group, u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x100) {
        u8 *const lab = reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da722_519550(gWorldPtr));
        if (a == 0xd2d03) ResearchLab_PrevProject(lab, nullptr, group);
        else ResearchLab_NextProject(lab, nullptr, group);
        ResearchMeters_ShowName(group);
        ResearchMeters_ShowProgress(group);
    }
    return LevelMeter_OnMessage(window, message, a, b);
}

s32 __cdecl ResearchMeters_ArrowProc(u8 *window, u32 message, u32 a, u32 b) { return arrowProc(0, window, message, a, b); }
s32 __cdecl ResearchMeters_ArrowProc1(u8 *window, u32 message, u32 a, u32 b) { return arrowProc(1, window, message, a, b); }
s32 __cdecl ResearchMeters_ArrowProc2(u8 *window, u32 message, u32 a, u32 b) { return arrowProc(2, window, message, a, b); }
s32 __cdecl ResearchMeters_ArrowProc3(u8 *window, u32 message, u32 a, u32 b) { return arrowProc(3, window, message, a, b); }
s32 __cdecl ResearchMeters_ArrowProc4(u8 *window, u32 message, u32 a, u32 b) { return arrowProc(4, window, message, a, b); }

void __cdecl LevelMeter_SetLevel(u8 *window, u32 level) {
    if (level == 0) {
        const u32 phase = get(window, 3);
        if (phase == 3 || phase == 1) {
            set(window, 3, 2);
            tell(window, 0x20);
            set(window, 0, 0x200000);
        }
        return;
    }
    const s32 speed = (s32)((level + 0x14) << 16) / 100;
    const u32 phase = get(window, 3);
    if (phase == 0 || phase == 2) {
        set(window, 1, GuiClock_Now());
        set(window, 3, 3);
        set(window, 0, 0x300000);
        tell(window, 0x30);
    }
    set(window, 2, (u32)speed);
}

s32 __cdecl LevelMeter_OnMessage(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x1e) {
        u32 pos = get(window, 0);
        get(window, 1);   // sic: read, not used
        const u32 speed = get(window, 2);
        const u32 phase = get(window, 3);
        if (phase != 0) {
            u32 light = message;
            if (phase == 1) {
                pos += speed * 2;
                light = (u32)((s32)pos >> 16);
                if ((s32)light > 0x1f) light = pos = 0;
            } else if (phase == 2) {
                pos += speed * 2;
                light = (u32)((s32)pos >> 16);
                if ((s32)light > 0x2f) {
                    set(window, 3, 0);
                    light = 0x30;
                    pos = 0x300000;
                }
            } else if (phase == 3) {
                pos += speed * 2;
                light = (u32)((s32)pos >> 16);
                if ((s32)light > 0x3f) {
                    set(window, 3, 1);
                    light = pos = 0;
                }
            }
            tell(window, light);
            set(window, 0, pos);
            set(window, 1, GuiClock_Now());
        }
    }
    return Window_CallDefault(window, nullptr, message, a, b);
}
}

namespace {
const ResearchPanelEnv kGamePanelEnv = {Window_SendMessage, ResearchMeters_ShowProgress, ResearchMeters_Update, Window_SetTimer, Window_StartTimer, Language_WideText, HoverLabel_Attach, Sound_Forward6, Game_CancelAdvisor,
                                        GameTable7cfb90_Get1da720_519510, Things_SumChain, GameTable7cfb90_Get1da722_519550, Window_FindChild, Volumes_SetChannel, Volumes_SetMaster, Window_Notify1, Sound_SetParam, Sound_Stop, SoundUi_Play, Dialog_KeyHandler};
const ResearchPanelEnv *g_panel = &kGamePanelEnv;

u32 &word(u32 at) { return *reinterpret_cast<u32 *>(static_cast<uintptr_t>(at)); }
u8 *panel() { return reinterpret_cast<u8 *>(static_cast<uintptr_t>(reinterpret_cast<u32 &>(gLevelmeter_7cc1d8))); }
u8 *gameRecord() { return reinterpret_cast<u8 *>(static_cast<uintptr_t>(word((u32)&gWorldPtr))); }
u8 *panelChild(u32 id) { return g_panel->child(panel(), nullptr, id); }

void showAllProgress() {
    g_panel->showProgress(0);
    g_panel->showProgress(4);
    g_panel->showProgress(1);
    g_panel->showProgress(2);
    g_panel->showProgress(3);
}

// (The original looks up the game's table for the call's ECX, which Things_SumChain ignores.)
u32 chainSum() {
    g_panel->table(gameRecord());
    return g_panel->sum();
}

void updateUnlessHeld() {
    if (!gLevelmeter_7cc1bc) g_panel->update(-1);
}

void onChainChanged() {
    if (chainSum() == gLevelmeter_7cc1c8) return;
    gLevelmeter_7cc1c8 = chainSum();
    const u8 *const lab = reinterpret_cast<const u8 *>(static_cast<uintptr_t>(g_panel->lab(gameRecord())));
    u8 *const slider = panelChild(0xd2d10);
    const s32 sum = static_cast<s32>(gLevelmeter_7cc1c8);
    const u32 volume = *reinterpret_cast<const u32 *>(lab + 0x1460) - 0x46;
    const s32 limit = static_cast<s32>(static_cast<u32>((static_cast<s32>(100u - static_cast<u32>(sum)) / 3) + sum) << 10) / static_cast<s32>(reinterpret_cast<u32 &>(gLevelmeter_7cc1b4));
    const s32 level = static_cast<s32>(volume * static_cast<u32>(limit)) / 30;
    vcall<void>(g_panel->child(slider, nullptr, 0xd2d12), 0x1c, limit);
    gLevelmeter_7cc1c0 = static_cast<u32>(limit);
    vcall<void>(panelChild(0xd2d10), 0x1c, level);
}

void onSlider(u32 a, u32 b) {
    u32 level = b;
    u8 *const slider = panelChild(a);
    u8 *const lab = reinterpret_cast<u8 *>(static_cast<uintptr_t>(g_panel->lab(gameRecord())));
    static const u8 kChannels[5] = {3, 2, 1, 0, 4};
    const u32 index = a - 0xd2d0b;
    if (index < 5) {
        g_panel->setChannel(lab, nullptr, kChannels[index], b);
        updateUnlessHeld();
    } else if (index == 5) {
        if (static_cast<s32>(b) > static_cast<s32>(gLevelmeter_7cc1c0)) {
            vcall<void>(slider, 0x1c, gLevelmeter_7cc1c0);
            level = gLevelmeter_7cc1c0;
        }
        const u32 volume = static_cast<u32>(static_cast<s32>(level * 30) / static_cast<s32>(gLevelmeter_7cc1c0)) + 0x46;
        g_panel->setMaster(lab, nullptr, volume);
        updateUnlessHeld();
        showAllProgress();
        const s32 note = static_cast<s32>(volume * 2 - 0x8c) / 7;
        g_panel->notify(g_panel->child(panelChild(0xd2d10), nullptr, 0xd2d11), nullptr, static_cast<u32>(note));
        const u32 share = static_cast<u32>(static_cast<s32>(level * 100) / 1024);
        g_panel->setParam(gLevelmeter_7cc1d0, 0xc, share);
        g_panel->setParam(gLevelmeter_7cc1cc, 0xc, share);
        g_panel->setParam(gLevelmeter_7cc1d0, 0xd, share);
        g_panel->setParam(gLevelmeter_7cc1cc, 0xd, share);
    }
    if (a != 0xd2d10) {
        g_panel->setParam(gLevelmeter_7cc1d4, 0xb, level);
        g_panel->setParam(gLevelmeter_7cc1d4, 0xa, level);
    }
}
}  // namespace

namespace {

const ResearchOpenEnv kOpenGameEnv = {reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)0x0067a7d0),
                                      reinterpret_cast<void(__cdecl *)(const u8 *, u32, u32, u32, u32, u32, u32, u32)>((uintptr_t)0x0047f020), &SoundUi_Play, &Global7852f8_Times78548c, &Things_SumChain,
                                      &GameTable7cfb90_Get1da720_519510, &GameTable7cfb90_Get1da722_519550, &ResearchMeters_ShowName, &ResearchMeters_ShowProgress, &ResearchMeters_Update, &ResearchLab_GroupEffort};
const ResearchOpenEnv *g_open = &kOpenGameEnv;

// The record's byte +7 is not set, as in the original.
void refuse(u32 text) {
    u8 record[0x10];
    at<u16 *>(record, 0) = Language_WideText(text);
    record[4] = record[5] = record[6] = 0xff;
    at<u32>(record, 8) = 4;
    at<u32>(record, 0xc) = 0;
    g_open->messageBox(record, 2, 0, 0, 0, 0, 0, 0);
}

}  // namespace

void ResearchPanel_SetOpenEnv(const ResearchOpenEnv *env) { g_open = env ? env : &kOpenGameEnv; }

void __cdecl ResearchPanel_Open() {
    if (Global7c2534_HasSource() || gLevelmeter_7cc1d8) return;
    Window7c24c8_Close();
    u8 &guard = gResearchPanel_Open_fb34be;
    if (!(guard & 1)) {
        guard |= 1;
        Setting_ReadMode(reinterpret_cast<u8 *>(&gDialog74fb20_fb3b7c));
        g_open->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005d1750));
    }
    if (gDialog74fb20_fb3b7c == 2) {
        refuse(0x1d4);
        return;
    }
    if (at<u16>(gWorldPtr, 0x1da742) == 0) {
        refuse(0x1d3);
        return;
    }
    u8 *const panel = WindowScript_Build(gPanel7b9940_7cb2ac, nullptr, reinterpret_cast<u8 *>((uintptr_t)0x00754490), 0x004a96b0);
    gLevelmeter_7cc1d8 = panel;
    if (!panel) return;
    Window7c24c8_Set(panel);
    gLevelmeter_7cc1bc = 1;
    gLevelmeter_7cc1d0 = g_open->play(0x9a);
    gLevelmeter_7cc1cc = g_open->play(0x9b);
    u8 *const master = Window_FindChild(panel, nullptr, 0xd2d10);
    gResearchPanel_Open_7cc1dc = master;
    if (master) {
        Window_SetHandler(master, nullptr, 0x004a9f90);
        Window705b20_SetRange(gResearchPanel_Open_7cc1dc, nullptr, 0, 0x3ff);
        u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x124));
        u8 *const bar = memory ? BarMesh_Construct(memory, nullptr, gResearchPanel_Open_754ce4) : nullptr;
        Window_SetImpl(Window_FindChild(gResearchPanel_Open_7cc1dc, nullptr, 0xd2d12), nullptr, bar);
        vcall<void>(Window_FindChild(gResearchPanel_Open_7cc1dc, nullptr, 0xd2d12), 0x1c, 0x1f4u);
    }
    static const u32 kSliders[5] = {0xd2d0e, 0xd2d0f, 0xd2d0d, 0xd2d0c, 0xd2d0b};
    for (u32 id : kSliders) {
        if (u8 *const slider = Window_FindChild(panel, nullptr, id)) Window705b20_SetRange(slider, nullptr, 0, 100);
    }
    static const struct {
        u32 id, handler;
    } kMeters[5] = {{0xd2d01, 0x004aa0c0}, {0xd2d07, 0x004aa300}, {0xd2d08, 0x004aa360}, {0xd2d09, 0x004aa3c0}, {0xd2d0a, 0x004aa420}};
    for (const auto &meter : kMeters) {
        u8 *const window = Window_FindChild(panel, nullptr, meter.id);
        if (!window) continue;
        Window_SetHandler(window, nullptr, meter.handler);
        Window_SetDwordA0(window, nullptr, 0, 0x30);
        Window_SetDwordA0(window, nullptr, 3, 0);
        Window_Notify1(Window_FindChild(window, nullptr, kLight), nullptr, 0x30);
        u8 *const label = Window_FindChild(window, nullptr, 0xd2d05);
        u8 *memory = static_cast<u8 *>(GameHeap_Alloc(0x124));
        Window_SetObject98(label, nullptr, memory ? MeshOverlay_Construct(memory, nullptr, gResearchPanel_Open_754cd8) : nullptr);
        memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
        Window_SetImpl(label, nullptr, memory ? TextPart_Ctor(memory, nullptr, 0) : nullptr);
        Window_NotifyC00(label, nullptr, 1, 1);
        Window_Notify3000(label, nullptr, 1, 1);
        Window_Set4c(label, nullptr, Language_Font(gResearchPanel_Open_754bc8));
        Window_Notify40000(label, nullptr, 0xe0, 0x56, 0x56, 0xff);
        Window_NotifyImpl100(label, nullptr, 0);
    }
    g_open->table(gWorldPtr);
    gLevelmeter_7cc1b4 = g_open->times();
    g_open->table(gWorldPtr);
    const s32 sum = static_cast<s32>(g_open->sum());
    reinterpret_cast<s32 &>(gLevelmeter_7cc1c8) = sum;
    const u8 *lab = reinterpret_cast<const u8 *>((uintptr_t)g_open->lab(gWorldPtr));
    u8 *child = Window_FindChild(gLevelmeter_7cc1d8, nullptr, 0xd2d10);
    const s32 limit = static_cast<s32>(static_cast<u32>((100 - sum) / 3 + sum) << 10) / gLevelmeter_7cc1b4;
    const s32 position = static_cast<s32>(static_cast<u32>(*reinterpret_cast<const s32 *>(lab + 0x1460) - 0x46) * static_cast<u32>(limit)) / 30;
    vcall<void>(Window_FindChild(child, nullptr, 0xd2d12), 0x1c, limit);
    reinterpret_cast<s32 &>(gLevelmeter_7cc1c0) = limit;
    vcall<void>(Window_FindChild(gLevelmeter_7cc1d8, nullptr, 0xd2d10), 0x1c, position);
    static const u32 kGroups[5] = {0, 4, 1, 2, 3};
    for (u32 group : kGroups) g_open->showName(group);
    for (u32 group : kGroups) g_open->showProgress(group);
    lab = reinterpret_cast<const u8 *>((uintptr_t)g_open->lab(gWorldPtr));
    for (u32 i = 0; i < 5; ++i) {
        if (u8 *const slider = Window_FindChild(gLevelmeter_7cc1d8, nullptr, kSliders[i])) vcall<void>(slider, 0x1c, g_open->effort(lab, nullptr, kGroups[i]));
    }
    gLevelmeter_7cc1bc = 0;
    g_open->update(-1);
    Advisor_PostId(0xc8);
}

void ResearchPanel_SetEnv(const ResearchPanelEnv *env) { g_panel = env ? env : &kGamePanelEnv; }

extern "C" s32 __cdecl ResearchPanel_Handler(u8 *window, u32 message, u32 a, u32 b) {
    switch (message) {
    case 5:
        g_panel->send(panel(), 4, 0, 0);
        return 1;
    case 0x10:
        showAllProgress();
        break;
    case 0x14: {
        const u32 first = gLevelmeter_7cc1d0;
        reinterpret_cast<u32 &>(gLevelmeter_7cc1d8) = 0;
        g_panel->forward6(first);
        g_panel->forward6(gLevelmeter_7cc1cc);
        if (gLevelmeter_7cc1d4) g_panel->forward6(gLevelmeter_7cc1d4);
        g_panel->cancel(0xc8);
        break;
    }
    case 0x15:
        if (static_cast<u16>(g_panel->setTimer(window, nullptr, 0x80080, 0x3e8))) g_panel->startTimer(window, nullptr, 0x80080);
        g_panel->attach(window, 0xd2d00, static_cast<u32>(reinterpret_cast<uintptr_t>(g_panel->text(0xb8))));
        break;
    case 0x1e:
        onChainChanged();
        break;
    case 0x100:
        if (static_cast<s32>(a) >= -2 && static_cast<s32>(a) < 0) {
            g_panel->send(panel(), 4, 0, 0);
            return 0;
        }
        break;
    case 0x800:
        onSlider(a, b);
        break;
    case 0x801:
        if (a != 0xd2d10) {
            gLevelmeter_7cc1d4 = g_panel->play(0x9c);
            g_panel->setParam(gLevelmeter_7cc1d4, 0xb, b);
            g_panel->setParam(gLevelmeter_7cc1d4, 0xa, b);
        }
        break;
    case 0x802:
        if (a != 0xd2d10) {
            g_panel->stop(gLevelmeter_7cc1d4);
            gLevelmeter_7cc1d4 = 0;
        }
        break;
    }
    return g_panel->keys(window, message, a, b);
}
