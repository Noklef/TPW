#include "levelmeter.h"

#include <cstdint>

#include "../core/language.h"
#include "../core/soundset.h"
#include "../core/textarg.h"
#include "../core/vcall.h"
#include "../world/object.h"
#include "../world/objectdefs.h"
#include "../world/research.h"
#include "../world/staff.h"
#include "../world/world.h"
#include "guimanager.h"
#include "window.h"

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
    u8 *const lab = reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da722_519550(global<u8 *>(0x007cf83c)));
    u16 id;
    if (*ResearchLab_CurrentObjId(lab, nullptr, &id, (u32)group) == 0) return 0;
    return Table_ScaledBySum(lab, nullptr, group);
}

constexpr u32 kMeter[5] = {0xd2d01, 0xd2d08, 0xd2d09, 0xd2d0a, 0xd2d07};   // (by group)

void showLevel(s32 group, u32 level) {
    LevelMeter_SetLevel(Window_FindChild(global<u8 *>(0x007cc1d8), nullptr, kMeter[group]), level);
}

}  // namespace

extern "C" {

void __cdecl ResearchMeters_ShowProgress(u32 group) {
    const u8 *const lab = reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da722_519550(global<u8 *>(0x007cf83c)));
    const s32 done = 100 - (s32)ResearchLab_LeftPercent(lab, nullptr, group);
    if (group > 4) return;
    u8 *const meter = Window_FindChild(global<u8 *>(0x007cc1d8), nullptr, kMeter[group]);
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
    Sound_SetParam(global<u32>(0x007cc1d0), 0x12, (u32)((s32)sum / 5));
}

void __cdecl ResearchMeters_ShowName(u32 group) {
    u8 *const defs = reinterpret_cast<u8 *>((uintptr_t)0x007890a0);
    u8 *const lab = reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da722_519550(global<u8 *>(0x007cf83c)));
    u32 id = 0;   // (the original's dword: its high word not set, and not read)
    ResearchLab_CurrentObjId(lab, nullptr, reinterpret_cast<u16 *>(&id), group);
    u8 *gauge = nullptr;
    if (group <= 4) gauge = Window_FindChild(Window_FindChild(global<u8 *>(0x007cc1d8), nullptr, kMeter[group]), nullptr, 0xd2d05);
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
    u8 *const lab = reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da722_519550(global<u8 *>(0x007cf83c)));
    if (!ResearchLab_CurrentNotDone(lab, nullptr, group)) ResearchLab_NextProject(lab, nullptr, group);
    if (!global<u8 *>(0x007cc1d8)) return;
    ResearchMeters_ShowName(group);
    ResearchMeters_ShowProgress(group);
}

void __fastcall Obj7b51f0_RefreshResearch(void *, void *, u32 group) { ResearchMeters_Refresh(group); }

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
