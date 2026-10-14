// The WAD directory's finder: reimplementation of 0x637a50/0x637aa0, 0x637e10, 0x638050-
// 0x638640 and the wildcard list 0x641500-0x641820, with the String list node 0x5f8a50.
//
// A search pattern ("folder\name*.ext", '/' or '\', one leading and one trailing separator
// dropped) is lower-cased, its folder part walked down the tree (the path found kept with a
// '\' after each folder), and its last part compiled into tokens: the text before the first
// wildcard, then each '*' or '?' with the text after it. Matching takes the tokens in order:
// a literal must come next, '?' one char then its text, '*' the first place its text occurs.
// A pattern without wildcards is looked up by the records' name hash and compared whole. The
// subfolders are listed before the files.
//
// The originals' quirks are kept: '*' matches the first occurrence of its text only, and a
// pattern ending in '*' then matches whatever follows; the wildcards right after a '*' are
// dropped ("*?" after a '*' becomes '*'); a pattern ending ".*" is also tried without it; an
// exact pattern's hash covers its folder part too and only its last 1-3 chars are taken as
// bytes; a folder pattern (no last part) leaves "\" after the found folder in the path.
#pragma once

#include "../../../re/types.h"
#include "../core/string.h"
#include "dirsource.h"
#include "waddir.h"

extern "C" {
// 0x5f8a50 (text by value, popped): a node holding a copy of text, unlinked; text destroyed.
StringListNode *__fastcall StringListNode_Ctor(StringListNode *n, void *edx, String text);
// 0x641820 (&out): the first String taken off the list into out (empty when there is none).
String *__fastcall StringList_PopHead(StringList *l, void *edx, String *out);
// 0x641500 (&result, pattern): the list emptied, then filled with the pattern's tokens; 0.
s32 *__fastcall Wildcard_Compile(StringList *l, void *edx, s32 *result, const char *pattern);
// 0x641760 (text): non-zero when text matches the list's tokens; 0x641780 (__stdcall; text,
// first node) does the work.
u32 __fastcall Wildcard_Match(const StringList *l, void *edx, const char *text);
u32 __stdcall Wildcard_MatchFrom(const char *text, const StringListNode *n);

// 0x637e10 (path, &out): the folder path names under f (its parts separated by '\' or '/'),
// each part found appended to out with a separator; nullptr when one is missing.
WadFolder *__fastcall WadFolder_Find(WadFolder *f, void *edx, const String *path, String *out);

// 0x638050 (&root, pattern, &record): empty Strings and lists, then Start.
WadFinder *__fastcall WadFinder_Ctor(WadFinder *f, void *edx, WadFolder **root, const String *pattern,
                                     u32 *record);
// 0x6380f0 (&root, pattern, &record): a new search under *root and its first match; record
// (when not null) gets the matching file's record offset, 0 for a folder or none.
void __fastcall WadFinder_Start(WadFinder *f, void *edx, WadFolder **root, const String *pattern, u32 *record);
// 0x638430: the next match (subfolders first, then files), or none.
void __fastcall WadFinder_Next(WadFinder *f);
// 0x638550: AL non-zero when no match is current and the files are being walked.
u32 __fastcall WadFinder_Done(const WadFinder *f);
// 0x638570 (&entry): the current match as a directory entry (attributes 0x10 for a folder,
// 0x81 and the size for a file; its name after the folders' path).
void __fastcall WadFinder_Fill(const WadFinder *f, void *edx, DirEntry *e);

// 0x637a50 (&result, &entry, pattern) / 0x637aa0 (&result, &entry): the directory's finder
// started or moved on, the match filled in; 0, or -1 when there is none.
s32 *__fastcall WadDir_FindFirst(WadDir *d, void *edx, s32 *result, DirEntry *e, const String *pattern);
s32 *__fastcall WadDir_FindNext(WadDir *d, void *edx, s32 *result, DirEntry *e);
}

struct WadFindEnv {
    void *(__cdecl *alloc)(u32 size);  // Mem_Alloc (0x67acc0)
    void(__cdecl *free)(void *block);  // Mem_Free (0x67acb0)
    s32(__cdecl *toupper)(s32 c);      // 0x67f1a0
};
// The finder's calls go through env (nullptr: the game's).
void WadFind_SetEnv(const WadFindEnv *env);
