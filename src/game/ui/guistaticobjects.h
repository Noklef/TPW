// The GUI library's two static objects made and destroyed (0xfaa550 of the class at 0x669fa8, the clock 0xfaa580), and the clock's time. Reimplementation of 0x6584a0, 0x6584af, 0x6584be, 0x6584d0, 0x65964a,
// 0x65965b, 0x659665 and 0x659750. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// What they call (the game's by default; the bench's recorders: their own benches have them).
struct GuiStaticObjectsEnv {
    u32(__fastcall *make)(u8 *object);        // Fn_669fa8
    u32(__fastcall *destroy)(u8 *object);     // Fn_669fee
    u32(__cdecl *atExit)(u32 function);       // CRT atexit
};
void GuiStaticObjects_SetEnv(const GuiStaticObjectsEnv *env);

extern "C" {

void __cdecl GuiStatic_Init();         // 0x6584a0: both of the next two
void __cdecl GuiStatic_Make();         // 0x6584af: 0xfaa550 made (Fn_669fa8)
void __cdecl GuiStatic_AtExit();       // 0x6584be: atexit(0x6584d0)
void __cdecl GuiStatic_Destroy();      // 0x6584d0: 0xfaa550 destroyed (Fn_669fee)
u32 __fastcall GuiClock_Time(u8 *clock);   // 0x65964a (thiscall): timeGetTime
void __cdecl GuiClock_Init();          // 0x65965b: as the next
void __cdecl GuiClock_Make();          // 0x659665: GuiClock_Ctor on 0xfaa580
u8 *__fastcall GuiClock_Ctor(u8 *clock);   // 0x659750 (thiscall): Fn_659770, the vtable 0x7058ec; returns the clock
}

#include "../core/gamedata.h"

// The vtables hold only ports: tpw.exe's loader points their slots at them (.docs/09-globals.md).
GAME_DATA(void *[1], GuiClock_vtable, 0x007058ec, 0x4);
