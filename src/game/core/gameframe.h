// The game's step each frame (0x54e360), a state machine over the whole session: logos, the frontend, a park's load, its frames, its end. x87 in the original's order.
#pragma once

#include "../../../re/types.h"

extern "C" {

// Its calls, about 150 callees, go by the game's addresses. A table of replacements, so that it can run outside the game (tests/fncmp/gameframe.py gives every one a recorder); none: the game's.
struct GameFrameCall {
    u32 address, target;
};
void GameFrame_SetEnv(const GameFrameCall *table, u32 count);

// 0x54e360 (cdecl; name invented, unconfirmed): returns 2 when state 9's load failed, else 0. First the time since the last call (0x875030; with 0x7b05d0 set, the rate from it shown:
// MouseState_ShowFps) and how long the mouse's buttons have been held (0x8785dc, 0x8786dc). Then, with 0x879068 set, by the state 0x87906c (past 15 nothing), the next one set as it says:
// - 4: 5. 5-8: the logos (the two shown with 0x51b300 run until that or a key: 0x6595a8, Escape, Space), 8 to 1.
// - 1: the frontend loaded: the load screen and its text, the advisor, particles, the object at 0xf82884 (0x38 bytes) and the sprites; to 2. 2: its frames while the object's +0x14 is 1,
//   the particles stepped each 31 ms (500 ms at most behind); then to 3. 3: the object closed and freed, all of it released; to 9 when it answered 2, else 0xc.
// - 9: the park's load: its folders and name from the online session (at 0xf7d88c) or none, sounds, particles, the save object at 0xf81ebc, then 0x457a90, NamedObjects_CreateWorld, the
//   sprites, 0x407e00, 0x457c30 (each failing: 2 returned, the state kept), to 0xf. In the online mode (setting 0xfb3b7c 1) a wait of up to 300 s for the chat (at 0x7cddec) to connect.
//   Without 0x788268 the save named by the session loaded, or (other modes) 0x5accf0. In the others, with the global save's flag, its messages posted (setting 2: 10, 0 to 0x788d3c).
// - 0xf: 0x550b30 (out of mode 1), to 0xa. 0xd: to 0xe. 0xe: 0x5996d0, 0x5ac5f0, to 0xa.
// - 0xa: a frame: the game clock frozen by 0x878128, the world's fixed steps of 31 ms (2 s at most behind), each timed into the last eight (0x878088-0x878124): particles and cars, RSSE; every
//   second step the flying rides, sprite scripts; every eighth the world (up to three a frame, in mode 1 the chat's turn instead) and the reverb; every 32nd the music levels from the world's
//   counts. With 0x7a1a14 & 8 or not 0x7a1a8c & 1: the sound's placement, then all the drawing steps, the flash on the camera's flag 0x16 changing, the screen's fade by 0x87502c. A grab's
//   next name by 0x878128. The averages printed to 0x877eb8. 0x879088 1: to 0xd; 2, 3: to 0xb; so too with the chat's flag in mode 1.
// - 0xb: the park's end: to 0xc (0x879088 3 or no 0x200 in 0x7a1a8c), else 9 or 1 by the session's +0x3f0; sounds and scripts released; in mode 1 the chat freed and without +0x3f0 the mode set by the
//   global save's flag (2 or 0). 0xc: all the singletons freed and 0x7a1a14 | 2, the state kept.
u32 __cdecl Game_RunFrame();

// 0x54e2b0 (cdecl; name invented, unconfirmed): the renderer's restore callback (Renderer_SetRestoreCallback, by address): one frame drawn without the world's steps. Rides_AnimateAll(3);
// without the camera's flag 0x16 the pick at 0x7a1c70 (RayPick_World(0x7a1c70, 60.0f, 0x7a1c90, 0), MouseState_SetTileFromPosition, Frame_SetPoint); 0x40f220, DynFaces_Frame, the
// highlight's pulse and whitening, MouseState_NewFrame (0x7b05a8); Frame_Begin timed into 0x87868c and 0x87907c (0x879064 = 0); SpriteScripts_Draw, MouseState_ResolvePick; 0x879080 = 0.
void __cdecl Game_RestoreFrame();
}

#include "gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[0x14], gGameframe_74d0c4, 0x0074d0c4, 0x14);
GAME_DATA(char[0xc], gGameframe_764fe4, 0x00764fe4, 0xc);
GAME_DATA(char[0x28], gGameframe_7650b4, 0x007650b4, 0x28);
GAME_DATA(char[0x40], gGameframe_7650ec, 0x007650ec, 0x40);
GAME_DATA(char[0x18], gGameframe_76512c, 0x0076512c, 0x18);
GAME_DATA(char[0x1c], gGameframe_765190, 0x00765190, 0x1c);
GAME_DATA(char[0x18], gGameframe_7651b8, 0x007651b8, 0x18);
GAME_DATA(u32[40], gGameframe_SampleRings_878088, 0x00878088, 0xa0);   // (5 rings of the last 8 times: world, frame, RSSE, 0x520130, sprite scripts)
GAME_DATA(u32, gGameframe_878128, 0x00878128, 0x4);
GAME_DATA(u32[5], gGameframe_SampleStarts_878688, 0x00878688, 0x14);   // (their steps' starts, then what each took)

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(const float, gGame_RunFrame_700f84, 0x00700f84, 0x4);
GAME_DATA(const float, gGameframe_700f88, 0x00700f88, 0x4);
GAME_DATA(const u8[0x40], gGameframe_700f8c, 0x00700f8c, 0x40);
GAME_DATA(u8[0xc], gGameframe_7651ac, 0x007651ac, 0xc);
GAME_DATA(u8[0x4], gGameframe_879088, 0x00879088, 0x4);
