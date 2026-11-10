// An object's script loaded when it is built. Reimplementation of 0x4dcf90; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// What it calls (the ports by default; the bench's recorders and stand-ins: their own benches have them).
struct ObjectScriptEnv {
    u8 *(__fastcall *findAt390)(u8 *defs, void *, u16 id);                // ObjectDefs_FindAt390
    u8 *(__fastcall *find)(u8 *defs, void *, u16 id);                     // ObjectDefs_Find
    s32(__cdecl *sprintf)(char *to, const char *format, ...);             // 0x67a650
    char *(__cdecl *locate)(const char *name, u32 flags, s32 *index);     // Quickload_Locate
    u16 *(__fastcall *handle)(const u8 *thing, void *, u16 *out);         // Thing_GetHandle
    u32(__cdecl *loadScript)(const char *name, u32 handle, u32 quiet);    // RSSE_LoadScriptFile
    void(__cdecl *setRecord)(u32 ride, u32 id);                           // Ride_SetRecordId
};
void ObjectScript_SetEnv(const ObjectScriptEnv *env);

extern "C" {

// 0x4dcf90 (thiscall): the object's script file named from its definition (the id +0xe: the format 0x75b66c with the definition's +0x28c, the name at +0x390 and the extension 0x700540) and
// located (flags 1); not there: the default 0x75b61c instead (the original's Debug_Printf left out, not the lookup for it). Loaded with the object's handle, quiet by [0x785a2c]; the answer in
// +0x24 and on the ride +0x20.
void __fastcall Object_LoadScript(u8 *object);
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(const char[8], gText_Object_LoadScript_700540, 0x00700540, 0x8);
GAME_DATA(char[28], gText_Object_LoadScript_75b61c, 0x0075b61c, 0x1c);
GAME_DATA(char[8], gText_Object_LoadScript_75b66c, 0x0075b66c, 0x8);
