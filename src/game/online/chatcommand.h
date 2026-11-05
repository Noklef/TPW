// The online chat avatar's commands (the WEACHATR library's Chat, its methods imports called __thiscall on the Chat at the avatar's +0
// with the avatar as their context): a typed line's tokens run (0x4ec030), a notice shown in the chat window (0x4ea890) and a token's
// text (0x4e9fa0). Names invented, unconfirmed.
//
// The avatar (the ECX of 0x4ec030; chatavatar.h's): +0 the Chat; +8 the player's own name (a std::wstring); +0x260 a buffer of 0x400
// wide characters the notices are formatted into; +0xe60 / +0xe70 two std::wstrings (whom the player told last, who told the player
// last: the "reply" targets); +0xe80 a set of names (tree D, its head at +0xe84); +0xe90 / +0xea0 the buddy / ignore sets (the second
// wide string copy's, wstringmap.h's; heads at +0xe94 / +0xea4); +0xeb0 the earmuffs byte; +0xeb1 the away byte; +0xec4 the Vote;
// +0xef8 a goto-in-progress byte; +0xefc a std::wstring (the room asked for).
#pragma once

#include "../../../re/types.h"
#include "../core/stdwstring.h"

extern "C" {

// 0x4e9fa0 (thiscall on a token; out, pops 4): the token's text (its std::wstring at +0x10) copied into out (StdWString_CtorCopy: the
// same code as 0x47ae60). Returns out.
StdWString *__fastcall ChatToken_Text(const u8 *token, void *, StdWString *out);

// 0x4ea890 (thiscall, the avatar not read; a std::wstring by value, a colour; pops 0x14): with the global save info's byte +0x25 set
// (GlobalSave_Byte25 of GlobalSave_Instance, made when missing) the text is censored in place (0x5d3230 on the info, (&text, 1): agent
// B's, through the env); then its characters (null: 0x6fd6c4) are added to the chat window (Window7cc8bc_ChatAddStd, the colour as its
// data) and the string released (_Tidy inlined: one count less, or the block freed when owned alone or frozen).
void __fastcall ChatAvatar_Notice(u8 *avatar, void *, StdWString text, u32 color);

// 0x4ec030 (thiscall on the avatar; the tokens, a std::vector of 0x24-byte tokens (+0x10 the text, a std::wstring; +0x20 the command),
// pops 4; SEH frame): nothing for no tokens; unless the first token's command is 0x12 (away) the byte +0xeb1 is cleared and
// Chat::NotAFK told. Then by the first token's command (tokens past the first are its arguments; "a notice" is an entry of the
// charset [0x78bac8] formatted into +0x260 (Charset_FormatWide; some with a text argument), made a std::wstring and its copy shown
// with ChatAvatar_Notice in colour 7):
//   0 Say(the text, 0) when given and not empty; 1 with an argument and its text Tell(the first, the second, 0) and the first kept at
//   +0xe60 as the reply target (std::wstring assign), alone notice 0xb3 (no target) or 0x56 (+0xe60's name); 2 / 3 Tell(+0xe70 / +0xe60,
//   the text, 0) when the target is set (alone or not set: notices 0xb2 / 0x57 and 0xb3 / 0x56); 4 / 7 RoomShout(the text, 0 / 1) unless
//   the earmuffs are on (notice 0x5f); 5 Say(the text, 1); 6 Tell(the first, the second, 1) (alone: notice 0x72); 0xa
//   SetHearingRange(the second token's +0x20, read whatever the count, sic); 0xc the earmuffs toggled (EarmuffsOff / EarmuffsOn); 0xd
//   UnIgnore a name in the ignore set (ChatIgnoreSet_Find), else Ignore it unless it is the player's own (compare without case: notice
//   0xb1 and a debug line); 0xe Blackmark (own: notice 0xb0); 0xf RemoveBuddy a buddy (ChatBuddySet_Find), else AddBuddy (own: notice
//   0x9b; empty: 0x72); 0x11 Page; 0x12 the away toggle (Say of notice 0x7c or 0x7b's text, AFK when going away); 0x13 unless a goto is
//   in progress (a debug line): Goto a name in the +0xe80 set (kept at +0xefc first), else a dialog (0x47f020, agent B's) with notice
//   0x7f whose button goes to the room (0x4e5210) (own: notice 0x9c); 0x14 Locate; 0x15 a vote (the online session's: a Vote made,
//   connected and sent; see the port); 0x16 the global save's byte +0x25 toggled (notices 0x85 / 0x84); 0x17 Say(the text, 0) (alone:
//   notice 0xa1); 0x18 notice 0x9a; 8, 9, 0xb, 0x10 and past 0x18 nothing. The commands taking a name want an argument: alone, notice
//   0x72 (0xe: 0x69). The strings' allocator bytes are uninitialised stack bytes in the original (0 here).
void __fastcall ChatAvatar_RunCommand(u8 *avatar, void *, const u8 *tokens);

// 0x4ea490 (thiscall on the avatar; the typed line by value, pops 0x10; SEH frame; name invented, unconfirmed): an empty line destroyed. Else
// a scanner on the stack (core/wscanner, 0x58 bytes) parses a copy into a vector of tokens and a string; with no token at all, a token of
// kind 0x17 and one of a copy of the string go in. ChatAvatar_RunCommand on the vector, then all of it destroyed. The allocator bytes are
// one uninitialised stack byte in the original (0 here).
void __fastcall ChatAvatar_ParseCommand(u8 *avatar, void *, StdWString text);

// 0x4ead00 / 0x4eae80 / 0x4eb000 / 0x4eb180 / 0x4eb450 (thiscall on the avatar; a name (a const std::wstring *), pops 4; SEH frames):
// the command 0xe (Blackmark) / 0x11 (Page) / 0x14 (Locate) / 0x13 (Goto) / 0xd (UnIgnore / Ignore) run on the name. A vector of tokens
// (countedpair.h's CountedPairs) gets the command's token (both strings empty) and the name's (CountedPair_CtorWide of a copy of the name:
// its allocator byte, emptied, StdWString_AssignSub of all of it), each by CountedPairVector_InsertN at the end and the temporary token
// released after; ChatAvatar_RunCommand on the vector; its tokens cleared (CountedPair_Clear) and its block freed (Mem_Free, null or not).
// The vector's and the command token's strings' allocator bytes are the low byte of the name's address (sic). Names invented, unconfirmed.
void __fastcall ChatAvatar_Blackmark(u8 *avatar, void *, const StdWString *name);
void __fastcall ChatAvatar_Page(u8 *avatar, void *, const StdWString *name);
void __fastcall ChatAvatar_Locate(u8 *avatar, void *, const StdWString *name);
void __fastcall ChatAvatar_Goto(u8 *avatar, void *, const StdWString *name);
void __fastcall ChatAvatar_ToggleIgnore(u8 *avatar, void *, const StdWString *name);

// 0x4eb300 / 0x4eb5d0 (thiscall on the avatar; SEH frames): the command 0x15 (the vote) / 0xc (the earmuffs toggled) run alone, as the
// five above without the name's token (the allocator bytes stack garbage in the original: 0 here; the token's strings released inline,
// the vector's tokens by CountedBuf_Reset and StdString_Tidy). Names invented, unconfirmed.
void __fastcall ChatAvatar_Vote(u8 *avatar);
void __fastcall ChatAvatar_ToggleEarmuffs(u8 *avatar);

// 0x4ee590 (thiscall on the avatar; a text (a const std::wstring *), pops 4): a copy of the text (StdWString_CtorCopy) shown with
// ChatAvatar_Notice in colour 7. Name invented, unconfirmed.
void __fastcall ChatAvatar_NoticeText(u8 *avatar, void *, const StdWString *text);

// 0x4eea50 (thiscall on the avatar; SEH frame): the notice 0x9d (its string made by _Grow and the characters, as Goto's 0x72) and the
// goto-in-progress byte +0xef8 cleared. Name invented, unconfirmed.
void __fastcall ChatAvatar_GotoDone(u8 *avatar);

// 0x4eb720 / 0x4eb910 (thiscall on the avatar (name), pops 4; SEH frame; names invented, unconfirmed): the command 0xf run on the name
// (RemoveBuddy for a buddy, else AddBuddy), the first only when the name is not in the buddy set (+0xe90: ChatBuddySet_LowerBoundOut of a
// copy, WStdString_Compare), the second only when it is. The allocator bytes are the low byte of the lower bound (sic: its slot was the
// argument's).
void __fastcall ChatAvatar_AddBuddy(u8 *avatar, void *, const StdWString *name);
void __fastcall ChatAvatar_RemoveBuddy(u8 *avatar, void *, const StdWString *name);

// 0x4ebb00 / 0x4ebcf0 (the same; names invented, unconfirmed): the command 0xd run on the name, the first only when the name is not in the
// ignore set (+0xea0: ChatIgnoreSet_LowerBound of a copy, WStdString_Compare), the second only when it is. The same allocator bytes.
void __fastcall ChatAvatar_AddIgnore(u8 *avatar, void *, const StdWString *name);
void __fastcall ChatAvatar_RemoveIgnore(u8 *avatar, void *, const StdWString *name);

// 0x4ebee0 / 0x4ebef0 (thiscall on the avatar; names invented, unconfirmed): ChatAvatar_ToggleEarmuffs when the earmuffs byte +0xeb0 is
// clear / set.
void __fastcall ChatAvatar_EarmuffsOn(u8 *avatar);
void __fastcall ChatAvatar_EarmuffsOff(u8 *avatar);

// 0x4ee5c0 (thiscall on the avatar (name), pops 4; name invented, unconfirmed): what the avatar knows of a name: a buddy (+0xe90) 1, with
// its node's byte +0x1c 3, with its byte +0x1d | 4; ignored (+0xea0, ChatIgnoreSet_Find) | 8.
u32 __fastcall ChatAvatar_NameFlags(const u8 *avatar, void *, const StdWString *name);

// 0x4ee8d0 / 0x4ee9e0 (thiscall on the avatar; names invented, unconfirmed): the tree at +0xe80 / the buddy set +0xe90 in order, each name with its
// ChatAvatar_NameFlags (the first of a copy of the name) added to its window list: Window7cc8bc_ListAddStd / Window7cc8bc_ListAdd1Std (null
// characters: the _Nullstr 0x6fd6c4 / 0x700624).
void __fastcall ChatAvatar_FillRoomList(u8 *avatar);
void __fastcall ChatAvatar_FillBuddyList(u8 *avatar);

// std::wstring::assign(chars, n) as 0x4ec030 inlines it (case 0x17) and agent B's 0x4051d0 is (not hooked: the fncmp driver's stand-in
// for 0x4051d0): n above max_size (0x7ffffffd) is std::_Xlen; a shared block (count not 0 or 0xff) with n 0 loses a count and s is
// emptied; else n 0 empties s (the block released); a block of 31 or fewer characters with room for n is kept, else released and a new
// one made (StdWString_Copy(n)); then the n characters, the length and a terminator. Returns s.
StdWString *__fastcall ChatCommand_AssignChars(StdWString *s, void *, const u16 *chars, u32 n);

// What the commands call through an environment (the fncmp driver's recorders and stand-in; the game's by default: the ports, or the
// game's addresses for agent B's unported functions).
struct ChatCommandEnv {
    s32(__cdecl *format)(u8 *charset, u16 *dest, u32 size, u32 entry, ...);              // Charset_FormatWide 0x6acd60
    StdWString *(__fastcall *assign)(StdWString *s, void *, const u16 *chars, u32 n);    // 0x4051d0 (B): assign(chars, n)
    void(__fastcall *censor)(u8 *save, void *, StdWString *text, u32 flag);              // 0x5d3230 (B)
    void(__stdcall *chatAdd)(u16 *text, u32 data);                                       // Window7cc8bc_ChatAddStd 0x481800
    void(__stdcall *listAdd)(const u16 *text, u32 data);                                 // Window7cc8bc_ListAddStd 0x481820
    void(__stdcall *listAdd1)(const u16 *text, u32 data);                                // Window7cc8bc_ListAdd1Std 0x481870
    u8 *(__fastcall *makeSession)(u8 *block);                                            // 0x5b50b0 (B): the online session's constructor
    void(__cdecl *dialog)(u8 *message, u32 kind, u32, u32, u32 callback, u32, u32, u32);  // 0x47f020 (B)
};
void ChatCommand_SetEnv(const ChatCommandEnv *env);   // nullptr: the game's
const ChatCommandEnv *ChatCommand_Env();
}
