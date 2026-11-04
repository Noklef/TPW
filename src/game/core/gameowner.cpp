// The game's owner object: see gameowner.h.
#include "gameowner.h"

#include <cstdint>

namespace {

AddrCalls g_calls;

const u32 kFrontEnd = 0x007b51f0, kClocks = 0x00785970;

template <typename T>
T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}
template <typename T>
T &global(u32 address) {
    return *reinterpret_cast<T *>((uintptr_t)address);
}

void SoundsHeld(u8 kept) {
    if (kept) {
        g_calls.Cd(0x0051bcf0);   // Sound_Post700b64
    } else {
        g_calls.Cd(0x00598960);   // Advisor_PauseSpeech
        g_calls.Cd(0x0051c1c0, 1u);   // Sound_Set803ad2
    }
}

void SoundsLetGo(u8 kept) {
    if (kept) {
        g_calls.Cd(0x0051bd30);   // Sound_Post700b68
    } else {
        g_calls.Cd(0x00598990);   // Advisor_ResumeSpeech
        g_calls.Cd(0x0051c1c0, 0u);   // Sound_Set803ad2
    }
}

}  // namespace

void GameOwner_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" void __fastcall GameOwner_ShutdownAll(u8 *self) {
    g_calls.Th(0x00409180, (u32)(uintptr_t)self);   // GameOwner_Shutdown
    const u32 mode = *reinterpret_cast<u32 *>((uintptr_t)0x007b05e8);
    if (mode) {
        g_calls.Th(0x0046c350, mode, 0u);   // MouseMode_Set
        g_calls.Cd(0x0067acb0, mode);
    }
    *reinterpret_cast<u32 *>((uintptr_t)0x007b05e8) = 0;
}

extern "C" {

void __fastcall GameOwner_Pause(u8 *self, void *, s32 mode, u32 flag) {
    if (at<u32>(self, 0x3c) != 1) return;
    at<u32>(self, 0x1c) = 1;
    if (mode == 1) g_calls.Th(0x00481440, kFrontEnd);   // FrontEnd_OpenOnGui
    g_calls.Th(0x00402d90, kClocks);   // Clocks_Pause
    at<u8>(self, 0x20) = (u8)flag;
    SoundsHeld((u8)flag);
}

void __fastcall GameOwner_Resume(u8 *self) {
    if (at<u32>(self, 0x3c) != 1) return;
    at<u32>(self, 0x1c) = 0;
    g_calls.Th(0x00481450, kFrontEnd);   // FrontEnd_Pause
    g_calls.Th(0x00402db0, kClocks);   // Clocks_Resume
    SoundsLetGo(at<u8>(self, 0x20));
}

void __fastcall GameOwner_TogglePause(u8 *self, void *, u32 flag) {
    if (at<u32>(self, 0x3c) != 1) return;
    g_calls.Th(0x00481460, kFrontEnd);   // FrontEnd_Resume
    g_calls.Th(0x00402dd0, kClocks);   // Clocks_TogglePause
    if (at<u32>(self, 0x1c) == 1) {
        const u8 kept = at<u8>(self, 0x20);
        at<u32>(self, 0x1c) = 0;
        SoundsLetGo(kept);
    } else {
        at<u32>(self, 0x1c) = 1;
        at<u8>(self, 0x20) = (u8)flag;
        SoundsHeld((u8)flag);
    }
}

u32 __fastcall GameOwner_Shutdown(u8 *self) {
    if (global<u32>(0x007cf83c)) {
        g_calls.Th(0x00515dd0, global<u32>(0x007cf83c));   // World_ClearLevel
        const u32 level = global<u32>(0x007cf83c);
        if (level) {
            g_calls.Th(0x00409240, level);   // its destructor
            g_calls.Cd(0x0067acb0, level);   // Mem_Free
        }
        global<u32>(0x007cf83c) = 0;
    }
    g_calls.Th(0x00412d50, 0x007890a0u);   // ObjectDefs_Free
    g_calls.Th(0x00585e20, 0x008c1828u);   // Landscape_FreeTextures
    if (global<u32>(0x008bcbc8) & 0x400000) {
        if (at<u32>(self, 0x38)) {
            g_calls.Th(0x00418d70, at<u32>(self, 0x38));   // Landscape_RemoveTextureList
            const u32 list = at<u32>(self, 0x38);
            if (list) g_calls.Cd(0x0067acb0, list);
            at<u32>(self, 0x38) = 0;
        }
    }
    const u32 centre = global<u32>(0x00788d3c);
    if (centre) {
        g_calls.Th(0x00409250, centre);   // MessageCentre_DtorSlots
        g_calls.Cd(0x0067acb0, centre);
    }
    global<u32>(0x00788d3c) = 0;
    g_calls.Th(0x00408f00, (u32)(uintptr_t)self);   // NamedObjects_RemoveGlobals
    return 1;
}
}
