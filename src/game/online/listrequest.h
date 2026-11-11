// The online session's list requests (the city, park, theme, server and ride lists at the session's +0xe0 to +0x100): started when logged in or after
// the login dialog, answered, finished. Names invented, unconfirmed. A list: +4 the request or -1, +8 the login its own, +0xc asked, +0x10, +0x14 answered,
// +0x18 an error to post, +0x1c, +0x20 the callback and its user, +0x24, +0x28 the answer's number and done, +0x2c the owner of its items, +0x30 the native
// answer (see Shared_Release66). Its vtable: +4 busy, +8 ask (city, +0x24), +0xc take (+0x30).
#pragma once

#include "../../../re/types.h"

struct ListRequestEnv {
    u32(__cdecl *post)(u32 code, u32 param);   // CommandQueue_Post
    void(__cdecl *open)(const u8 *record, u8 *parent, u32 callback, u32 a, u32 b);   // Dialog7cc984_Open
};
void ListRequest_SetEnv(const ListRequestEnv *env);   // nullptr: the game's

extern "C" {

// 0x5b9b00, 0x5b9d60, 0x5ba030, 0x5ba2f0, 0x5ba560 (thiscall (failed), pops 4; the native answer's elements of 0x30 / 0x66 / 0x10 / 0x26 / 0x34 bytes, each with
// its import's destructor): the error +0x18 posted and cleared. Not failed and no error, answered with a native answer: the owner's items (+0x2c) deleted
// (vtable 0, with 1) with their nodes, +0x24 and +0x28 cleared, the list's take (+0xc) called, +0x24 = [+0x30] + 8 (a null then faults: sic), the native answer
// released and +0x28 = 1. Result 0; but failed, an error or +0x28 still 0: result 1, and Online_ResetAll with the login its own (+8). The callback, when
// set, called cdecl (result, user, +0x2c).
void __fastcall ListRequest_Done30(u8 *list, void *, u32 failed);
void __fastcall ListRequest_Done66(u8 *list, void *, u32 failed);
void __fastcall ListRequest_Done10(u8 *list, void *, u32 failed);
void __fastcall ListRequest_Done26(u8 *list, void *, u32 failed);
void __fastcall ListRequest_Done34(u8 *list, void *, u32 failed);

// 0x5bc7d0 (thiscall (callback, user), pops 8), 0x5bcd10 (thiscall (value, callback, user), pops 0xc; value to +0x34): +4 = -1, +8 to +0x18 cleared, the callback and user kept, the native answer released. Logged in: asked
// (a busy list +0x18 = 0x4e, Done(1); no city from Online_OpenCity +0x18 = 0x4f, Done(1); else +0xc = 1 and +4 = ask's). Else +8 = 1 and the login dialog opened with
// 0x5bc710 / 0x5bca70 and the list.
void __fastcall ListRequest_Start30(u8 *list, void *, u32 callback, u32 user);
void __fastcall ListRequest_Start66(u8 *list, void *, u32 value, u32 callback, u32 user);
// 0x5bcbc0 (thiscall (text, callback, user), pops 0xc): the text assigned to +0x34 (vtable +0x14), then as 0x5bcd10.
void __fastcall ListRequest_StartText66(u8 *list, void *, u32 text, u32 callback, u32 user);
// 0x5bd070, 0x5bd4b0, 0x5bd8c0 (thiscall): as those with no callback, on this list; but asks the session's +0xf8, +0xfc, +0x100 (sic: this list when it is that one).
// Login dialog: 0x5bcfb0, 0x5bd3f0, 0x5bd800 with 0.
void __fastcall ListRequest_Start10(u8 *list);
void __fastcall ListRequest_Start26(u8 *list);
void __fastcall ListRequest_Start34(u8 *list);

// 0x5bc710, 0x5bca70 (cdecl (flag, list)), 0x5bcfb0, 0x5bd3f0, 0x5bd800 (cdecl (flag); the session's +0xf8, +0xfc, +0x100): the login dialog's answer. Flag set: the
// error posted and cleared, Online_ResetAll with the login its own, the callback called with (1, user, +0x2c). Flag 0: asked, as above.
void __cdecl ListRequest_Login30(u32 flag, u8 *list);
void __cdecl ListRequest_Login66(u32 flag, u8 *list);
void __cdecl ListRequest_Login10(u32 flag);
void __cdecl ListRequest_Login26(u32 flag);
void __cdecl ListRequest_Login34(u32 flag);

// 0x5b7910 (thiscall (index), pops 4; name invented, unconfirmed): an index within the owner of the session's list +0xf4: its item (Owner2c_At), [[0xf85614] + 0xc]'s vtable +0x48 called, and the
// item's dialog opened (Dialog7cc984_Open(item, 0, 0x4c4fa0, 0x4c0830, 0)).
void __fastcall Online_OpenListItem(u8 *session, void *, s32 index);

// 0x4c4fa0 (cdecl; name invented, unconfirmed): the callback Online_OpenListItem and the view buttons give their dialogs: [[0xf85614] + 0xc]'s
// vtable +0x44 called (a tail jump; the dialogs call it with no arguments).
void __cdecl Online_ListItemClosed();

// 0x5bc8e0 (cdecl): the session's +0x108 set and its user flags without 0x10: 1. Else 0.
u32 __cdecl Online_Has108NoFlag10();

}
