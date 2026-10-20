// The game's virtual file system: paths inside the mounted WAD archives. Reimplementation of 0x41cff0, the path taken relative to a base
// (an archive's folder or the archive itself).
#pragma once

#include "../../../re/types.h"

struct String;

extern "C" {

// 0x41cff0 (cdecl; path, base, out; name unconfirmed): copies a of path and b of base (each String_SkipDotPrefix'd in place first; the
// copy through the vtable's +0x18, AssignWide, as the game's does), with a's leading "\"s (the global at 0x78bee0) dropped one at a time.
// When b's last chars, as many as the String at 0x78bf40 has, equal ".wad*" (the String at 0x78c810, Names_Equal == 1), b loses as many
// chars as that one has and gets a trailing "\" (String_EnsureTrailingBackslash). Then when a is not shorter than b by more than that
// String's length, and a's first b->length chars equal b (case blind), out = the rest of a with its leading "\"s dropped, and 1 is
// returned; otherwise out = the empty String at 0x78be50 (vtable +0x18) and 0. Each assignment goes through the vtable's +0x1c.
u32 __cdecl Path_StripBase(String *path, String *base, String *out);
}
