// The game's globals as named objects (.docs/09-globals.md). A module header declares each once, by its original address and size:
//     GAME_DATA(s32[8], gImaAdpcmIndexMoves, 0x0077dd00, 0x20);
// and the code uses the name, never the address. tools/gamedata.py defines the names for each target at build time: the hook DLL, the
// tests and the comparison driver bind each name to its original address (the game's own data, as before); tpw.exe gives each its own
// object with the original's initial bytes, read from the user's tp.exe. One declaration a line; the type has no commas (a typedef).
#pragma once

template <typename T> using GameData = T;

#define GAME_DATA(type, name, address, size) \
    extern GameData<type> &name;             \
    static_assert(sizeof(GameData<type>) == (size), #name ": its size differs from the original's")
