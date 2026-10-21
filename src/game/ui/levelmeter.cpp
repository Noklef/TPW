#include "levelmeter.h"

#include "guimanager.h"
#include "window.h"

namespace {

constexpr u32 kLight = 0xd2d06;

u32 get(u8 *window, u32 index) { return Window_DwordA0(window, nullptr, index); }

void set(u8 *window, u32 index, u32 value) { Window_SetDwordA0(window, nullptr, index, value); }

void tell(u8 *window, u32 value) { Window_Notify1(Window_FindChild(window, nullptr, kLight), nullptr, value); }

}  // namespace

extern "C" {

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
