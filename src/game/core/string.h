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

extern "C" {

// 0x4f76b0 (thiscall; other, pops 4): compares two wide strings (data at +4, length at +8; no data
// is the empty string at 0x6fd6c4) the std::basic_string way: the first differing 16-bit unit
// decides (-1 below, 1 above, unsigned), else the shorter is less. -1, 0 or 1.
s32 __fastcall WString_Compare(const u8 *self, void *, const u8 *other);

// 0x4f7870 (thiscall; pos, n, other, count, pops 16): compare(pos, n, other, count): past the length a pos is std::_Xran; at most n
// characters from pos (no data stays a null pointer, sic, not the empty string) against the count at other, as WString_Compare. -1, 0 or 1.
s32 __fastcall WString_CompareSub(const u8 *self, void *, u32 pos, u32 n, const u16 *other, u32 count);

// 0x424cc0 / 0x424cf0 (cdecl; a, b): the texts the dwords at +8 of a and b point to compare, case blind: 1 when the first is after the second / when
// they are equal, otherwise 0.
u32 __cdecl Names_FirstAfter(const u8 *a, const u8 *b);

u32 __cdecl Names_Equal(const u8 *a, const u8 *b);

// 0x424d10 (cdecl; a, b; name unconfirmed): the same for wide texts (_wcsicmp): 1 when the texts the dwords at +8 of a and b point to are
// equal, case blind, otherwise 0.
u32 __cdecl WNames_Equal(const u8 *a, const u8 *b);

// 0x489350 (thiscall on the destination; source, pops 4): the eight dwords at +0..+0x1c and the text at +0x24 (up to and including its NUL) of the source
// are copied; the dword at +0x20 (a pointer into the record) = the source's moved by the distance between the two. Returns the destination.
u8 *__fastcall Record_CopyWithText(u8 *dest, void *, const u8 *source);

// 0x5b9020 (thiscall; out, pops 4): out is constructed as a wide string from the text pointer at +0x54 (WString_CtorStr). Returns out.
u8 *__fastcall Obj_NameToWString(u8 *object, void *, u8 *out);

// 0x5bd230 (thiscall; name, pops 4): over the nodes of the list at +0x2c -> +4 whose object has a name at +0x14: the dword at +8 of the last object whose name equals the
// given one (case-insensitively) - 1 when none does.
u32 __fastcall Names_FindLastValue(u8 *object, void *, const char *name);

// 0x5c4ce0 (thiscall): the five strings at +0, +0x14, +0x28, +0x3c and +0x50 are constructed empty. Returns the object.
u8 *__fastcall Strings5_Ctor(u8 *object);

// 0x5ed490 (stdcall; first, last, dest, pops 12): a copy (String_CtorCopy) of each string of the array from first up to last (0x14 bytes each) is built in the array at dest
// (a null dest only counts). Returns the end of dest. 0x5ed4d0 (stdcall; dest, count, from): the same string copied into count strings from dest. 0x5ed560 (cdecl;
// dest, from): one copy built at dest when it is not null.
String *__stdcall Strings_CopyRange(const String *first, const String *last, String *dest);

void __stdcall Strings_FillCopy(String *dest, u32 count, const String *from);

void __cdecl String_PlacementCopy(String *dest, const String *from);

// 0x5ed130 (thiscall on a vector of strings {?, first, last, end}; position, count, value, pops 12; name unconfirmed): count copies of the value
// inserted at the position. Without room: a new block (the game's heap) of size + the larger of size and count strings, the strings before the
// position, the copies and the rest built in it (String_CtorCopy), the old ones destroyed (slot 0xc with 0) and their block freed. With room, when
// fewer than count strings follow the position: those are built count further up, the copies built past the old end and the followers assigned
// the value (slot 0x18); otherwise the last count are built up past the end, the rest assigned backwards and the count at the position assigned
// the value. Nothing for a count of 0 with room. (The counts are signed divisions by 0x14, compared unsigned, as in the original.)
void __fastcall StringVector_InsertN(u8 *vec, void *, u8 *position, u32 count, const String *value);

// 0x421770 (thiscall on {list, cached}; key, out, pops 8): the first object of the list (+4 head, nodes {object, next}) whose name (Names_Equal on object + 4) equals the key
// (an object too) is stored to *out and to the cached pointer at +4, and 1 is returned; otherwise (or without a list) the cached pointer is cleared and 0 returned.
u32 __fastcall NamedList_Find(u8 **ref, void *, const u8 *key, u8 **out);

// 0x41ede0 (thiscall; key, pops 4): whether an object of the list the dword at +0x4c points to has a name equal to the key's (any non-zero result of Names_Equal).
u32 __fastcall Obj_HasNamedObject(u8 *object, void *, const u8 *key);

// 0x5c4c60 (thiscall): the five strings at +0x50, +0x3c, +0x28, +0x14 and +0 are destroyed in that order (String_Dtor).
void __fastcall Strings5_Dtor(u8 *object);

// 0x486780 (cdecl; wide text): when the dword at 0x78ba80 is set, the text is converted to bytes (Narrow_FromWide, room 0x3fff) at
// 0x7b5378. Returns 0x7b5378.
u32 __cdecl Text_NarrowToGlobalBuffer(const u16 *text);

// 0x5d1330 (thiscall; other, pops 4): the order of the byte strings (pointer +4, length +8; a null pointer is the empty text at 0x6fe5b0) of
// the object and other: the bytes of the common length are compared as unsigned (-1 or 1 at the first difference), then the lengths
// (-1 shorter, 1 longer, 0 equal).
s32 __fastcall ByteString_Compare(const u8 *object, void *, const u8 *other);

// 0x424d30 (cdecl; suffix, s): a copy of the last suffix->length chars of s (String_Right, String_CtorCopy) is compared with the suffix's text case blind
// (_stricmp); when equal, s = its first s->length - suffix->length chars (String_Left, then the vtable's +0x1c Assign). Returns s.
String *__cdecl String_StripSuffix(const String *suffix, String *s);

// 0x41cea0 (cdecl; s): when s's first chars (as many as the global "." string at 0x78c648 has; String_Left, String_CtorCopy) equal "." (Names_Equal, any non-zero),
// the chars from 1 on are stepped over one at a time while a copy of those at that position (String_Mid, as many as the global has) equals the global "\" (0x78bee0)
// or "." (Names_Equal == 1); then s = its chars from there on (String_From, the vtable's +0x1c Assign). Returns s.
String *__cdecl String_SkipDotPrefix(String *s);

// 0x5c5480 (cdecl; s; name unconfirmed): a copy of s's last chars, as many as the global "\" String at 0xf80be0 has (String_Right,
// String_CtorCopy), is compared with that global (Names_Equal); when they differ (a zero result) the global is appended through
// s's vtable +0x28 (Append).
void __cdecl String_EnsureTrailingBackslash(String *s);

// 0x5c5580 (thiscall on the five strings; name, pops 4; name unconfirmed): the string at +0x3c is given the one at +0x28 through
// its vtable +0x18 (String's AssignWide), ends in "\" (String_EnsureTrailingBackslash), and gets name appended (vtable +0x28).
void __fastcall Strings5_JoinPath(u8 *object, void *, const StringView *name);

// 0x424db0 (cdecl; s, wide; name unconfirmed): the wide text (wide->data) converted to the ANSI code page by WideCharToMultiByte (the import 0x6fd1dc;
// the size asked for first, NUL included), into a block of Mem_GlobalAlloc (none for a size of 0 or less, or a failed allocation), which goes to the
// vtable's +0x14 of s (AssignStr) and is freed (Mem_GlobalFree). Returns the second conversion's count, else 0.
s32 __cdecl String_AssignFromWide(String *s, const WString *wide);

// 0x424e10 (cdecl; wide, s; name unconfirmed): the mirror of String_AssignFromWide: the ANSI text (s->data) converted by MultiByteToWideChar (the
// import 0x6fd174; the units asked for first) into a block of twice that many bytes, which goes to the vtable's +0x14 of wide and is freed.
s32 __cdecl WString_AssignFromAnsi(WString *wide, const String *s);

// 0x4bbb80 (cdecl; text, buffer, size; name unconfirmed): text converted to UTF-16 by MultiByteToWideChar (the import 0x6fd174; code page 0, NUL
// included) into buffer (size units), or with no buffer into the static one at 0x7ba058 (0x3fff units). Returns the buffer written.
u16 *__cdecl String_AnsiToWide(const char *text, u16 *buffer, s32 size);

// 0x5e3170 (cdecl; out, cursor, delimiter; name unconfirmed): the chars from *cursor up to the delimiter (its low byte) appended one at a time (the String
// vtable's +0x24, String_AppendChar) to a new string; *cursor is left past the delimiter and out is constructed as a copy of the string (String_CtorCopy)
// and returned.
String *__cdecl String_ReadToken(String *out, const char **cursor, u32 delimiter);

}  // extern "C"
