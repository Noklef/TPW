// The GUI's activation told to the whole window tree. Reimplementation of 0x659333, 0x659364 and 0x6593c5. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// What they call (the game's by default; the bench's recorder: its own bench has it).
struct WindowBroadcastEnv {
    u32(__cdecl *post)(u8 *window, u32 message, u32 a, u32 b);   // Window_PostMessage
};
void WindowBroadcast_SetEnv(const WindowBroadcastEnv *env);

extern "C" {

// 0x659364 (cdecl (window, on)): message 0x21 (on) or 0x22 (off) posted to the window (Window_PostMessage, 0, 0), then the same for every child in turn (Fn_65d547, Fn_65d56b).
void __cdecl WindowTree_PostActive(u8 *window, u32 on);

// 0x659333 (cdecl): with a root [0xfaa5d8] and [0x77c488] clear: the tree told on, [0x77c488] 1.
void __cdecl Gui_TellActive();

// 0x6593c5 (cdecl): with a root and [0x77c488] set: the tree told off, [0x77c488] 0. In any case the word [0xfaa5bc] 0.
void __cdecl Gui_TellInactive();
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u32, gWindowbroadcast_77c488, 0x0077c488, 0x4);
