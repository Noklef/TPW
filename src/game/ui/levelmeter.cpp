#include "levelmeter.h"

#include <cstdint>

#include "../core/soundset.h"
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

// A research group's level: 0 without a current project (ResearchLab_CurrentObjId's id 0), else the lab's Table_ScaledBySum of it; the
// lab the game [0x7cf83c]'s (GameTable7cfb90_Get1da722_519550), found again for each.
u32 researchLevel(s32 group) {
    u8 *const lab = reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da722_519550(global<u8 *>(0x007cf83c)));
    u16 id;
    if (*ResearchLab_CurrentObjId(lab, nullptr, &id, (u32)group) == 0) return 0;
    return Table_ScaledBySum(lab, nullptr, group);
}

void showLevel(s32 group, u32 level) {
    static const u32 kMeter[5] = {0xd2d01, 0xd2d08, 0xd2d09, 0xd2d0a, 0xd2d07};   // (by group)
    LevelMeter_SetLevel(Window_FindChild(global<u8 *>(0x007cc1d8), nullptr, kMeter[group]), level);
}

}  // namespace

extern "C" {

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
