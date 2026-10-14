// The game's string classes: reimplementation of 0x5f5fc0-0x5f6c50, 0x5f7120/0x5f7150 and
// 0x5f78e0-0x5f7e60.
//
// String (vtable 0x703090) holds chars and WString (vtable 0x7030c8) UTF-16 units, both
// NUL-terminated in a heap block (Mem_Alloc, 0x67acc0) sized in steps of 8:
//   +0 vtable, +4 length, +8 data, +0xc capacity ((length & ~7) + 8), +0x10 growth (8).
// The block is never shrunk; growing past the capacity reallocates and copies the old text
// up to its first NUL. Both vtables are only used by their own classes (nothing derives
// from them), so their virtual calls (+4 SetLength, +8 Alloc, +0x1c Assign) are direct
// here. A view (vtable 0x7030c0 for chars, 0x7030f8 for UTF-16, two empty slots) is the
// same layout without ownership: Left/Right/Mid return one, and assigning a WString to a
// String goes through a char view of its data (copying length bytes, not converting).
//
// The C library calls (strncpy, strncat, strncmp, strstr, atoi and the wide forms) are
// written out here with the game's CRT semantics; Format's sprintf stays the game's
// (StringEnv), since the old CRT prints floats differently.
#pragma once

#include <cstdarg>

#include "../../../re/types.h"

struct String {
    const void *vtable;  // 0x703090
    u32 length;
    char *data;
    u32 capacity;
    u32 grow;
};
struct WString {
    const void *vtable;  // 0x7030c8
    u32 length;
    u16 *data;
    u32 capacity;
    u32 grow;
};
static_assert(sizeof(String) == 0x14 && sizeof(WString) == 0x14, "strings are 20 bytes");

// What the strings read: the length and the data (a String, WString or view).
struct StringView {
    const void *vtable;  // 0x7030c0
    u32 length;
    const char *data;
    u32 capacity;  // not set by Left/Right/Mid
};

extern "C" {
// 0x5f5fc0 / 0x5f6870 (__thiscall; length; vtable +8): a new block for length units, its
// terminator set; the old block is not freed.
void __fastcall String_Alloc(String *s, void *edx, u32 length);
void __fastcall WString_Alloc(WString *s, void *edx, u32 length);
// 0x5f6000 / 0x5f68b0 (__thiscall; length; vtable +4): the length and terminator, within the
// capacity; past it, Alloc and copy the old text up to its NUL.
void __fastcall String_SetLength(String *s, void *edx, u32 length);
void __fastcall WString_SetLength(WString *s, void *edx, u32 length);

// Constructors (__thiscall, return this): 0x5f61a0 / 0x5f6a20 empty; 0x5f60b0 / 0x5f6980 from
// a C string (nullptr: empty); 0x5f6040 / 0x5f6900 a copy of a string or view; 0x5f6150 one
// char (always length 1, even for NUL).
String *__fastcall String_Ctor(String *s);
String *__fastcall String_CtorStr(String *s, void *edx, const char *text);
String *__fastcall String_CtorCopy(String *s, void *edx, const StringView *from);
String *__fastcall String_CtorChar(String *s, void *edx, const char *c);
WString *__fastcall WString_Ctor(WString *s);
WString *__fastcall WString_CtorStr(WString *s, void *edx, const u16 *text);
WString *__fastcall WString_CtorCopy(WString *s, void *edx, const WString *from);
// 0x5f61e0 / 0x5f6a60: frees the block. 0x5f7120 / 0x5f7150 (flags): the scalar deleting
// destructors; flags bit 0 also frees the object.
void __fastcall String_Dtor(String *s);
void __fastcall WString_Dtor(WString *s);
String *__fastcall String_DeletingDtor(String *s, void *edx, u32 flags);
WString *__fastcall WString_DeletingDtor(WString *s, void *edx, u32 flags);

// Assignment (__thiscall, return this): 0x5f6200 / 0x5f6a80 one char (NUL: empty);
// 0x5f6240 / 0x5f6ac0 a C string (nullptr: empty); 0x5f62d0 / 0x5f6b50 a string or view
// (vtable +0x1c); 0x5f6290 / 0x5f6b10 the other class's string, through a view of its data.
String *__fastcall String_AssignChar(String *s, void *edx, const char *c);
String *__fastcall String_AssignStr(String *s, void *edx, const char *text);
String *__fastcall String_Assign(String *s, void *edx, const StringView *from);
String *__fastcall String_AssignWide(String *s, void *edx, const WString *from);
WString *__fastcall WString_AssignChar(WString *s, void *edx, const u16 *c);
WString *__fastcall WString_AssignStr(WString *s, void *edx, const u16 *text);
WString *__fastcall WString_Assign(WString *s, void *edx, const WString *from);
WString *__fastcall WString_AssignNarrow(WString *s, void *edx, const String *from);
// Appending (__thiscall, return this): 0x5f6320 / 0x5f6ba0 a C string (nullptr: nothing);
// 0x5f6360 / 0x5f6be0 one char (NUL: nothing); 0x5f6390 / 0x5f6c10 a string or view, whose
// length is read after SetLength: a string appended to itself copies its doubled length,
// past the capacity.
String *__fastcall String_AppendStr(String *s, void *edx, const char *text);
String *__fastcall String_AppendChar(String *s, void *edx, const char *c);
String *__fastcall String_Append(String *s, void *edx, const StringView *from);
WString *__fastcall WString_AppendStr(WString *s, void *edx, const u16 *text);
WString *__fastcall WString_AppendChar(WString *s, void *edx, const u16 *c);
WString *__fastcall WString_Append(WString *s, void *edx, const WString *from);
// 0x5f63d0 / 0x5f6c50 (vtable +0, __cdecl): sprintf into the string. The length is first
// estimated as MFC's CString::FormatV does (strings and chars by their length, numbers 32
// and floats 128 at least, nothing for %E), the string set to it, then the game's
// vsprintf writes and sets the real length. A null format empties it.
void __cdecl String_Format(String *s, const char *format, ...);
void __cdecl WString_Format(WString *s, const u16 *format, ...);

// 0x5f78e0 (__thiscall; length): SetLength and return the data to write into.
char *__fastcall String_GetBuffer(String *s, void *edx, u32 length);
// 0x5f7900 (__thiscall; length): SetLength(length) and terminate; 0x7fffffff: the length of
// the text written, at most the capacity.
void __fastcall String_ReleaseBuffer(String *s, void *edx, u32 length);
// Comparisons (__thiscall): 0x5f7950 equal to a string or view; 0x5f7980 / 0x5f79e0 equal /
// unequal to a C string (nullptr: the empty string); 0x5f7a50 unequal to one char (a
// longer string always is); 0x5f7a90 less than (strncmp over the shorter length, then the
// shorter first); 0x5f7e60 a WString unequal to a wide C string.
bool __fastcall String_Equals(const String *s, void *edx, const StringView *other);
bool __fastcall String_EqualsStr(const String *s, void *edx, const char *text);
bool __fastcall String_NotEqualsStr(const String *s, void *edx, const char *text);
bool __fastcall String_NotEqualsChar(const String *s, void *edx, const char *c);
bool __fastcall String_Less(const String *s, void *edx, const StringView *other);
bool __fastcall WString_NotEqualsStr(const WString *s, void *edx, const u16 *text);
// Searches (__thiscall; what, start): the index or -1. 0x5f7ae0 a char from start;
// 0x5f7b40 a substring (strstr) from start; 0x5f7b90 a char backwards from start (at most
// the last); 0x5f7be0 a substring backwards (start -1 or past the end: from the last place
// it fits); 0x5f7c70 / 0x5f7cf0 any char of a set, forwards / backwards.
s32 __fastcall String_FindChar(const String *s, void *edx, const char *c, u32 start);
s32 __fastcall String_Find(const String *s, void *edx, const char *text, u32 start);
s32 __fastcall String_ReverseFindChar(const String *s, void *edx, const char *c, u32 start);
s32 __fastcall String_ReverseFind(const String *s, void *edx, const char *text, u32 start);
s32 __fastcall String_FindOneOf(const String *s, void *edx, const char *set, u32 start);
s32 __fastcall String_ReverseFindOneOf(const String *s, void *edx, const char *set, u32 start);
// Views (__thiscall; &view, ...; return &view), clamped to the string: 0x5f7d60 the first
// count chars, 0x5f7d90 the last count, 0x5f7dd0 from start on, 0x5f7e10 count from start.
StringView *__fastcall String_Left(const String *s, void *edx, StringView *out, u32 count);
StringView *__fastcall String_Right(const String *s, void *edx, StringView *out, u32 count);
StringView *__fastcall String_From(const String *s, void *edx, StringView *out, u32 start);
StringView *__fastcall String_Mid(const String *s, void *edx, StringView *out, u32 start, u32 count);
}

// What the strings call in the game, so that they can run outside it.
struct StringEnv {
    void *(__cdecl *alloc)(u32 size);                                // Mem_Alloc (0x67acc0)
    void(__cdecl *free)(void *block);                                // Mem_Free (0x67acb0)
    s32(__cdecl *vsprintf)(char *out, const char *format, va_list args);  // 0x67c1e0
    s32(__cdecl *vswprintf)(u16 *out, const u16 *format, va_list args);   // 0x67f4d0
};
// The strings' calls go to env (nullptr: the game's).
void Strings_SetEnv(const StringEnv *env);
