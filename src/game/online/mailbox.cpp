#include "mailbox.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/base64.h"
#include "../core/buffers.h"
#include "../core/crtopen.h"
#include "../core/crtstream.h"
#include "../core/vdel.h"
#include "../core/crt.h"
#include "../core/crtoutput.h"
#include "../core/initializers.h"
#include "../core/gameheap.h"
#include "../core/ime.h"
#include "../core/iosbase.h"
#include "../core/language.h"
#include "../core/string.h"
#include "../core/narrowfilebuf.h"
#include "../core/nstream.h"
#include "../core/ptrlist.h"
#include "../core/stdlist.h"
#include "../core/stdstring.h"
#include "../core/stdwstring.h"
#include "chatlists.h"
#include "postcardmail.h"
#include "session.h"
#include "chatevents.h"
#include "../render/enginedebug.h"
#include "../core/swearfilter.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const MailboxEnv kGameEnv = {&Init_5cfd80, &NOstream_Ctor, &NIstream_Ctor, &NFileBuf_Ctor, &NFileBuf_Open, &BasicIosW_SetState,
                             &NOfstream_DtorOnIos, &NIfstream_DtorOnIos, &NIos_Widen, &NIstream_Getline,
                             &NOstream_PutText, &CRT_fclose, &NFileBuf_Init, &NFileBuf_Close, &NFileBuf_Dtor, &IosBase_Dtor,
                             reinterpret_cast<s32(__cdecl *)(const char *)>((uintptr_t)0x0067a520), &List_EraseNodeCounted, &StdListString_Insert,
                             &NumberList_Erase, reinterpret_cast<u32 *(__fastcall *)(u8 *, void *, u32 *, u32, const s32 *)>((uintptr_t)0x00634c60), &BasicIosW_Clear,
                             &Language_Id, &StdListString_PushBack, &StdListString_Sort, &StdListString_Unique, &StdListString_AddSorted, &ByteString_Compare, reinterpret_cast<void(__fastcall *)(void *)>((uintptr_t)0x00634bf0),
                             reinterpret_cast<u32(__cdecl *)(u32 *)>((uintptr_t)0x0067d3f0), reinterpret_cast<u8 *(__cdecl *)(const u32 *)>((uintptr_t)0x0067d8e0)};
const MailboxEnv *g_env = &kGameEnv;

const MailboxDropEnv kGameDropEnv = {&NumberList_Erase, reinterpret_cast<s32(__cdecl *)(const char *)>((uintptr_t)0x0067e560), &Mailbox_SaveOutbox, &Mailbox_LoadMessage,
                                     &Mailbox_DeleteMessage, &Online_Instance, &Online_UserName, &Get34_41ce80, &Get38_41ce90, &Online_String14, &Online_String28, &TokenList_PushChars,
                                     reinterpret_cast<void *(__fastcall *)(void *, void *, const u16 *, u32)>(&StdWString_AssignChars), &Mailbox_NewAttachment,
                                     reinterpret_cast<u8 *(__fastcall *)(void *, void *, const char *)>((uintptr_t)0x005c9720)};
const MailboxDropEnv *g_drop = &kGameDropEnv;

const MailClientEnv kGameClientEnv = {reinterpret_cast<void *(__cdecl *)(const char *, const char *)>(&CRT_fopen), reinterpret_cast<s32(__cdecl *)(void *, s32, s32)>(&CRT_fseek), reinterpret_cast<s32(__cdecl *)(void *)>(&CRT_ftell),
                                      reinterpret_cast<u32(__cdecl *)(char *, u32, u32, void *)>(&CRT_fread), &Base64_Encode, reinterpret_cast<u8 *(__fastcall *)(void *)>(&MailClient_Ctor), &MailClient_Connect,
                                      &PostcardMail_Build, &Mailbox_DropFirst, &Mailbox_SaveMessage};
const MailClientEnv *g_client = &kGameClientEnv;

// The ios of a stream, by the offset in its vbtable.
u8 *IosOf(u8 *stream) { return stream + at<u32>(at<u8 *>(stream, 0), 4); }

u32 npos() { return at<u32>(nullptr, 0x006fe5ac); }

const char *TextOf(const StdString &s) { return s.ptr ? s.ptr : reinterpret_cast<const char *>((uintptr_t)0x006fe5b0); }

void WriteNumbers(u8 *self, u32 listAt, u8 *stream) {
    char line[0x100];
    for (u8 *node = at<u8 *>(at<u8 *>(self, listAt), 0); node != at<u8 *>(self, listAt); node = at<u8 *>(node, 0)) {
        CRT_sprintf(line, gMailbox_7740c8, at<u32>(node, 8));
        g_env->write(stream, line);
    }
}

u8 *ReadLine(u8 *stream, char *line, u32 size = 0x100) { return g_env->getline(stream, nullptr, line, size, g_env->widen(IosOf(stream), nullptr, 10)); }

// A read's end, by the state of the ios of the stream it answered (a null stream: the state read at 4, a fault, as in the original).
bool ReadFailed(u8 *got) {
    const u8 *const state = got ? IosOf(got) : nullptr;
    return (at<u8>(state, 4) & 6) != 0;
}

// All nodes of a list of numbers erased, by a cursor up to the head as it was.
void ClearNumbers(u8 *list) {
    const u32 head = at<u32>(list, 4);
    u32 cursor = at<u32>(reinterpret_cast<void *>((uintptr_t)head), 0);
    while (cursor != head) {
        u32 old, out;
        g_env->eraseNumber(list, nullptr, &out, *ListCursor_Next(&cursor, nullptr, &old, 0));
    }
}

// Lines of numbers into a list, until a read fails or a line is empty.
void ReadNumbers(u8 *stream, u8 *list) {
    char line[0x80];
    for (;;) {
        u8 *const got = ReadLine(stream, line, 0x80);
        if (ReadFailed(got) || !line[0]) break;
        const s32 value = g_env->atoi(line);
        u32 out;
        g_env->insertNumber(list, nullptr, &out, at<u32>(list, 4), &value);
    }
}

// The default address pushed, the list sorted and made unique, and walked until it is found.
void AddDefault(u8 *list, const StdString *key) {
    g_env->pushBack(list, nullptr, key);
    g_env->sort(list);
    g_env->unique(list);
    u32 cursor = at<u32>(at<u8 *>(list, 4), 0);
    while (cursor != at<u32>(list, 4)) {
        if (g_env->compare(reinterpret_cast<const u8 *>((uintptr_t)cursor + 8), nullptr, reinterpret_cast<const u8 *>(key)) == 0) break;
        u32 old;
        ListCursor_Next(&cursor, nullptr, &old, 0);
    }
}

// std::list<u32>::sort as 0x5cbf40 has it inlined: 16 lists, each merged up, one node a time; a list {allocator byte, head, count}.
void SortNumbers(u8 *list) {
    u8 carry[0xc];
    at<u8>(carry, 0) = at<u8>(list, 0);
    at<u8 *>(carry, 4) = Node0c_New(0, 0);
    at<u32>(carry, 8) = 0;
    u8 lists[0xc * 0x10];
    u32 count = 0;
    CRT_VectorCtorIterator(lists, 0xc, 0x10, reinterpret_cast<void(__fastcall *)(void *)>(&ListHead_Init0c), reinterpret_cast<void *>(g_env->numbersDtor));
    while (at<u32>(list, 8)) {
        List_SpliceOne(carry, nullptr, at<u8 *>(at<u8 *>(carry, 4), 0), list, at<u8 *>(at<u8 *>(list, 4), 0));
        u32 i = 0;
        for (; i < count && at<u32>(lists + 0xc * i, 8); ++i) {
            StdListDword_Merge(lists + 0xc * i, nullptr, carry);
            Obj_SwapPair(lists + 0xc * i, nullptr, carry);
        }
        if (i == 0xf) {
            StdListDword_Merge(lists + 0xc * 14, nullptr, carry);
        } else {
            Obj_SwapPair(lists + 0xc * i, nullptr, carry);
            if (i == count) ++count;
        }
    }
    for (; count; --count) StdListDword_Merge(list, nullptr, lists + 0xc * (count - 1));
    CRT_VectorDtorIterator(lists, 0xc, 0x10, g_env->numbersDtor);
    g_env->numbersDtor(carry);
}

const u8 *const kLanguage = reinterpret_cast<const u8 *>((uintptr_t)0x0078b530);
const StdString *const kOtherAddress = reinterpret_cast<const StdString *>((uintptr_t)0x007a155c);
const char *const kEnglishAddress = gMailbox_7740f4;
const char *const kAddressBookName = gMailbox_7740e4;

// Every node of a list of numbers that holds the number erased. The end is read once.
// A std::string's assign, as the original has it inlined.
void AssignText(StdString &to, const char *text) {
    const u32 length = CRT_strlen(text);
    if (StdString_Grow(&to, nullptr, length, 1)) {
        for (u32 i = 0; i < length; ++i) to.ptr[i] = text[i];
        to.len = length;
        to.ptr[length] = 0;
    }
}

void AssignChars(StdString &to, const char *text) { StdString_AssignChars(&to, nullptr, text, CRT_strlen(text)); }

void StdString_Zero(u8 *s) {
    at<u32>(s, 4) = 0;
    at<u32>(s, 8) = 0;
    at<u32>(s, 0xc) = 0;
}

// The client's or target's sender, from the session: its text +0 the user name (the low byte of each wide character, in a buffer of 0x80), +0x3c and +0x40, wide +0x44 and +0x54 the session's two strings, +0x64 = 9.
void FillFromSession(u8 *target, u8 *(__cdecl *instance)()) {
    char name[0x80];   // (the original's frame has this much: a longer name overruns it there too)
    const u8 *const wide = reinterpret_cast<const u8 *>(g_drop->userName(instance()));
    for (u32 i = 0;; ++i) {
        name[i] = static_cast<char>(wide[i * 2]);
        if (!name[i]) break;
    }
    AssignChars(at<StdString>(target, 0), name);
    at<u32>(target, 0x3c) = g_drop->get34(instance());
    at<u32>(target, 0x40) = g_drop->get38(instance());
    const u16 *text = reinterpret_cast<const u16 *>(g_drop->string14(instance()));
    g_drop->assignWide(target + 0x44, nullptr, text, CRT_wcslen(text));
    text = reinterpret_cast<const u16 *>(g_drop->string28(instance()));
    g_drop->assignWide(target + 0x54, nullptr, text, CRT_wcslen(text));
    at<u32>(target, 0x64) = 9;
}

// A text to the mail library's object +0x70, by its import read live.
void LibraryText(u8 *self, u32 import, const char *text) { at<void(__fastcall *)(u8 *, void *, const char *)>(nullptr, import)(at<u8 *>(self, 0x70), nullptr, text); }

// The callback +0x90 told, when there is one.
void Notify(u8 *self, u32 a, u32 b) {
    void(__cdecl *const callback)(u32, u32) = at<void(__cdecl *)(u32, u32)>(self, 0x90);
    if (callback) callback(a, b);
}

// The list's string at the cursor +0x88 given to the library; the cursor moved on, and past the last: the next state.
void SendNextString(u8 *self) {
    u8 *const node = at<u8 *>(self, 0x88);
    at<u8 *>(self, 0x88) = at<u8 *>(node, 0);
    StdString line;
    StdString_Tidy(&line, nullptr, 0);
    StdString_AssignSub(&line, nullptr, &at<StdString>(node, 8), 0, npos());
    if (at<u8 *>(self, 0x88) == at<u8 *>(self, 0x14)) ++at<u32>(self, 0x6c);
    LibraryText(self, 0x006fd570, TextOf(line));
    StdString_Tidy(&line, nullptr, 1);
}

// Each node of a list of strings {?, head +4, count +8} erased.
void EraseStrings(u8 *list) {
    u8 *const head = at<u8 *>(list, 4);
    for (u8 *node = at<u8 *>(head, 0); node != head;) {
        u8 *const next = at<u8 *>(node, 0);
        u32 out;
        List_EraseNodeCounted(list, nullptr, &out, node);
        node = next;
    }
}

// A list of numbers destroyed: each node erased, its head freed, head and count cleared.
void FreeNumbers(u8 *list) {
    u8 *const head = at<u8 *>(list, 4);
    u32 cursor = at<u32>(head, 0);
    while (cursor != reinterpret_cast<uintptr_t>(head)) {
        u32 old, out;
        NumberList_Erase(list, nullptr, &out, *ListCursor_Next(&cursor, nullptr, &old, 0));
    }
    GameHeap_Free(at<u8 *>(list, 4));
    at<u32>(list, 4) = 0;
    at<u32>(list, 8) = 0;
}

void RemoveNumber(u8 *list, u32 number) {
    const u32 end = at<u32>(list, 4);
    u32 cursor = at<u32>(at<u8 *>(list, 4), 0);
    while (cursor != end) {
        if (at<u32>(reinterpret_cast<void *>((uintptr_t)cursor), 8) == number) {
            u32 old, out;
            g_drop->eraseNumber(list, nullptr, &out, *ListCursor_Next(&cursor, nullptr, &old, 0));
        } else {
            cursor = at<u32>(reinterpret_cast<void *>((uintptr_t)cursor), 0);
        }
    }
}

// The file of a message removed: the folder and the name made by the format.
void RemoveFile(u8 *self, StdString *path, u32 format, u32 number) {
    StdString_AssignSub(path, nullptr, reinterpret_cast<const StdString *>(self), 0, npos());
    char line[0x80];
    CRT_sprintf(line, reinterpret_cast<const char *>((uintptr_t)format), number);
    StdString_AppendChars(path, nullptr, line, CRT_strlen(line));
    g_drop->remove(TextOf(*path));
}

}  // namespace

void Mailbox_SetEnv(const MailboxEnv *env) { g_env = env ? env : &kGameEnv; }

void MailboxDrop_SetEnv(const MailboxDropEnv *env) { g_drop = env ? env : &kGameDropEnv; }

void MailClient_SetEnv(const MailClientEnv *env) { g_client = env ? env : &kGameClientEnv; }

void __fastcall MailClient_OnEvent(u8 *self, void *, s32 kind, s32 code, u32) {
    if (kind == -5) return Notify(self, 1, 0xffffffff);
    if (kind == -4) return Notify(self, at<u8>(self, 0x84) == 0, 0xffffffff);
    if (kind != 0x78) return;
    switch (code) {
    case 0xdd:
        return;
    case 0x1a5:
        return Notify(self, 1, 2);
    case 0x1c2:
    case 0x226:
        return Notify(self, 1, 7);
    case 0x228:
        if (at<u32>(self, 0x6c) == 1) SendNextString(self);
        return Notify(self, 1, 6);
    case 0xfa:
        break;
    default:
        return Notify(self, 1, 5);
    }
    switch (at<u32>(self, 0x6c)) {
    case 0:
        at<u32>(self, 0x6c) = 1;
        LibraryText(self, 0x006fd578, TextOf(at<StdString>(self, 0)));
        at<u8 *>(self, 0x88) = at<u8 *>(at<u8 *>(self, 0x14), 0);
        return;
    case 1:
        return SendNextString(self);
    case 2:
        g_client->build(self);
        ++at<u32>(self, 0x6c);
        return LibraryText(self, 0x006fd56c, TextOf(at<StdString>(self, 0x74)));
    case 3:
        if (!at<u8>(self, 0x85)) {
            AssignText(at<StdString>(self, 0), gPostcardmail_773d80.f40);
            EraseStrings(self + 0x10);
            if (at<u8 *>(self, 0x68)) {
                Counted2_ScalarDtor(at<u8 *>(self, 0x68), nullptr, 1);
                at<u8 *>(self, 0x68) = nullptr;
            }
            AssignText(at<StdString>(self, 0x74), reinterpret_cast<const char *>((uintptr_t)0x00786de4));
            if (g_client->dropFirst(at<u8 *>(self, 0x8c), nullptr, self)) {
                at<u32>(self, 0x6c) = 0;
                at<void(__fastcall *)(u8 *)>(nullptr, 0x006fd564)(at<u8 *>(self, 0x70));
                return;
            }
        }
        at<u8>(self, 0x84) = 1;
        at<void(__fastcall *)(u8 *)>(nullptr, 0x006fd568)(at<u8 *>(self, 0x70));
        return;
    default:
        return;
    }
}

void __stdcall MailClient_Callback(s32 kind, s32 code, u32 unused, u8 *self) { MailClient_OnEvent(self, nullptr, kind, code, unused); }

void __fastcall MailClient_Dtor(u8 *self) {
    AssignChars(at<StdString>(self, 0), gPostcardmail_773d80.f40);
    u32 out;
    List_EraseRangeCounted1(self + 0x10, nullptr, &out, at<u8 *>(at<u8 *>(self, 0x14), 0), at<u8 *>(self, 0x14));
    if (at<u8 *>(self, 0x68)) {
        Counted2_ScalarDtor(at<u8 *>(self, 0x68), nullptr, 1);
        at<u8 *>(self, 0x68) = nullptr;
    }
    AssignChars(at<StdString>(self, 0x74), reinterpret_cast<const char *>((uintptr_t)0x00786de4));
    at<s32(__stdcall *)()>(nullptr, 0x006fd430)();
    StdString_Tidy(&at<StdString>(self, 0x74), nullptr, 1);
    CountedBuf_Reset(self + 0x54, nullptr, 1);
    CountedBuf_Reset(self + 0x44, nullptr, 1);
    StdString_Tidy(&at<StdString>(self, 0x2c), nullptr, 1);
    StdString_Tidy(&at<StdString>(self, 0x1c), nullptr, 1);
    List_EraseRangeCounted1(self + 0x10, nullptr, &out, at<u8 *>(at<u8 *>(self, 0x14), 0), at<u8 *>(self, 0x14));
    GameHeap_Free(at<u8 *>(self, 0x14));
    at<u32>(self, 0x14) = 0;
    at<u32>(self, 0x18) = 0;
    StdString_Tidy(&at<StdString>(self, 0), nullptr, 1);
}

void __fastcall Mailbox_Dtor(u8 *self) {
    if (u8 *const client = at<u8 *>(self, 0x98)) {
        MailClient_Dtor(client);
        GameHeap_Free(client);
        at<u32>(self, 0x98) = 0;
    }
    StdString_Tidy(&at<StdString>(self, 0x80), nullptr, 1);
    FreeNumbers(self + 0x70);
    FreeNumbers(self + 0x64);
    u32 out;
    List_EraseRange(self + 0x58, nullptr, &out, at<u8 *>(at<u8 *>(self, 0x5c), 0), at<u8 *>(self, 0x5c));
    GameHeap_Free(at<u8 *>(self, 0x5c));
    at<u32>(self, 0x5c) = 0;
    at<u32>(self, 0x60) = 0;
    StdString_Tidy(&at<StdString>(self, 0x3c), nullptr, 1);
    StdListString_Clear(self + 0x30);
    StdString_Tidy(&at<StdString>(self, 0x20), nullptr, 1);
    StdString_Tidy(&at<StdString>(self, 0x10), nullptr, 1);
    StdString_Tidy(&at<StdString>(self, 0), nullptr, 1);
}

u8 __fastcall Mailbox_DeleteSent(u8 *self) {
    for (u8 *node = at<u8 *>(at<u8 *>(self, 0x74), 0); node != at<u8 *>(self, 0x74); node = at<u8 *>(node, 0)) {
        g_drop->deleteMessage(self, nullptr, at<u32>(node, 8));
        RemoveNumber(self + 0x64, at<u32>(node, 8));
        RemoveNumber(self + 0x58, at<u32>(node, 8));
    }
    u8 *const head = at<u8 *>(self, 0x74);
    for (u8 *node = at<u8 *>(head, 0); node != head;) {
        u8 *const next = at<u8 *>(node, 0);
        at<u8 *>(at<u8 *>(node, 4), 0) = next;
        at<u8 *>(next, 4) = at<u8 *>(node, 4);
        GameHeap_Free(node);
        --at<u32>(self, 0x78);
        node = next;
    }
    g_drop->saveOutbox(self);
    u8 *const first = at<u8 *>(at<u8 *>(self, 0x5c), 0);
    at<u8 *>(self, 0x7c) = first;
    return at<u32>(self, 0x60) ? g_drop->loadMessage(self, nullptr, at<u32>(first, 8)) : 0;
}

u8 __fastcall Mailbox_FirstMessage(u8 *self) {
    u8 *const first = at<u8 *>(at<u8 *>(self, 0x5c), 0);
    at<u8 *>(self, 0x7c) = first;
    return at<u32>(self, 0x60) ? g_drop->loadMessage(self, nullptr, at<u32>(first, 8)) : 0;
}

u8 __fastcall Mailbox_NextMessage(u8 *self) {
    if (!at<u32>(self, 0x60)) return 0;
    u8 *const next = at<u8 *>(at<u8 *>(self, 0x7c), 0);
    at<u8 *>(self, 0x7c) = next;
    u8 *const head = at<u8 *>(self, 0x5c);
    if (next == head) {
        at<u8 *>(self, 0x7c) = at<u8 *>(head, 0);
        return 0;
    }
    return g_drop->loadMessage(self, nullptr, at<u32>(next, 8));
}

u8 __fastcall Mailbox_StoreFromClient(u8 *client, void *, u8 *mailbox) {
    if (!mailbox) return 0;
    EraseStrings(mailbox + 0x30);
    AssignText(at<StdString>(mailbox, 0x10), TextOf(at<StdString>(client, 0)));
    AssignText(at<StdString>(mailbox, 0x20), TextOf(at<StdString>(client, 0x1c)));
    for (u8 *node = at<u8 *>(at<u8 *>(client, 0x14), 0); node != at<u8 *>(client, 0x14); node = at<u8 *>(node, 0)) StringList34_PushChars(mailbox, nullptr, TextOf(at<StdString>(node, 8)));
    AssignText(at<StdString>(mailbox, 0x3c), TextOf(at<StdString>(client, 0x2c)));
    return g_client->saveMessage(mailbox);
}

u8 *__fastcall MailAttachment_Ctor(u8 *self, void *, const char *path) {
    StdString_Zero(self);
    StdString_Zero(self + 0x10);
    u8 *data = nullptr;
    s32 size = static_cast<s32>(reinterpret_cast<uintptr_t>(path));   // (no file: the path's address is the length, sic)
    void *const file = g_client->fopen(path, reinterpret_cast<const char *>((uintptr_t)0x0074cfb8));
    if (file) {
        g_client->fseek(file, 0, 2);
        size = g_client->ftell(file);
        g_client->fseek(file, 0, 0);
        data = static_cast<u8 *>(GameHeap_Alloc(static_cast<u32>(size) + 1));
        if (data) {
            g_client->fread(reinterpret_cast<char *>(data), 1, static_cast<u32>(size), file);
            data[size] = 0;
        }
        g_env->fclose(file);
    }
    AssignText(at<StdString>(self, 0), gMailbox_75447c);
    AssignText(at<StdString>(self, 0x10), gPostcardmail_773d80.f54);
    s32 whole = size;
    while (whole % 3) ++whole;
    const s32 encoded = static_cast<s32>(static_cast<u32>(whole) << 2) / 3;
    char *const text = static_cast<char *>(GameHeap_Alloc(static_cast<u32>(encoded + 2 * (encoded / 76) + 1)));
    at<char *>(self, 0x20) = text;
    if (!text) {
        GameHeap_Free(data);
    } else {
        g_client->encode(text, data, size);
        if (data) GameHeap_Free(data);
    }
    return self;
}

u8 *__fastcall MailClient_Ctor(u8 *self) {
    StdString_Tidy(&at<StdString>(self, 0), nullptr, 0);
    u8 *const head = static_cast<u8 *>(GameHeap_Alloc(0x18));
    at<u8 *>(head, 0) = head;
    at<u8 *>(head, 4) = head;
    at<u8 *>(self, 0x14) = head;
    at<u32>(self, 0x18) = 0;
    StdString_Tidy(&at<StdString>(self, 0x1c), nullptr, 0);
    StdString_Tidy(&at<StdString>(self, 0x2c), nullptr, 0);
    at<u32>(self, 0x3c) = 0;
    CountedBuf_Reset(self + 0x44, nullptr, 0);
    at<u32>(self, 0x58) = 0;
    at<u32>(self, 0x5c) = 0;
    at<u32>(self, 0x60) = 0;
    at<u32>(self, 0x64) = 9;
    at<u32>(self, 0x68) = 0;
    at<u32>(self, 0x70) = 0;
    StdString_Tidy(&at<StdString>(self, 0x74), nullptr, 0);
    at<u8>(self, 0x84) = 0;
    at<u32>(self, 0x90) = 0;
    u8 startup[0x190];   // (the original's frame has this much)
    at<s32(__stdcall *)(u32, void *)>(nullptr, 0x006fd42c)(0x101, startup);
    AssignText(at<StdString>(self, 0), gPostcardmail_773d80.f40);
    return self;
}

u32 __fastcall MailClient_Connect(u8 *self, void *, const char *host, u32 port, u32 arg3, u8 *owner) {
    at<u8>(self, 0x85) = owner ? 0 : 1;
    at<u8 *>(self, 0x8c) = owner;
    if (!at<u32>(self, 0x18)) {
        void(__cdecl *const callback)(u32, u32) = at<void(__cdecl *)(u32, u32)>(self, 0x90);
        if (callback) callback(1, 8);
        return 1;
    }
    if (!at<u8 *>(self, 0x70)) {
        void *const block = GameHeap_Alloc(0x10);
        at<u8 *>(self, 0x70) = block ? at<u8 *(__fastcall *)(void *)>(nullptr, 0x006fd57c)(block) : nullptr;
        if (!at<u8 *>(self, 0x70)) return 1;
    }
    at<u32>(self, 0x6c) = 0;
    const u32 got = at<u32(__fastcall *)(u8 *, void *, const char *, u32, u32, u8 *, u32)>(nullptr, 0x006fd574)(at<u8 *>(self, 0x70), nullptr, host, port, 0x005c9a60, self, arg3);
    return got == 0;
}

u8 __fastcall Mailbox_Send(u8 *self, void *, const char *host, u32 port, u32 arg3) {
    if (!at<u32>(self, 0x6c)) return 0;
    u8 *client = at<u8 *>(self, 0x98);
    if (!client) {
        void *const block = GameHeap_Alloc(0x94);
        client = block ? g_client->ctor(block) : nullptr;
        at<u8 *>(self, 0x98) = client;
        if (!client) return 0;
    } else {
        AssignChars(at<StdString>(client, 0), gPostcardmail_773d80.f40);
        u8 *const head = at<u8 *>(client, 0x14);
        u32 out;
        List_EraseRangeCounted1(client + 0x10, nullptr, &out, at<u8 *>(head, 0), head);
        if (at<u8 *>(client, 0x68)) {
            Counted2_ScalarDtor(at<u8 *>(client, 0x68), nullptr, 1);
            at<u8 *>(client, 0x68) = nullptr;
        }
        AssignChars(at<StdString>(client, 0x74), reinterpret_cast<const char *>((uintptr_t)0x00786de4));
    }
    AssignChars(at<StdString>(self, 0x80), host);
    at<u16>(self, 0x90) = static_cast<u16>(port);
    at<u32>(client, 0x90) = at<u32>(self, 0x94);
    if (!g_drop->loadMessage(self, nullptr, at<u32>(at<u8 *>(at<u8 *>(self, 0x68), 0), 8))) return 0;
    FillFromSession(client, &Online_Instance);
    u8 *const lines = at<u8 *>(self, 0x34);
    for (u8 *node = at<u8 *>(lines, 0); node != lines; node = at<u8 *>(node, 0)) {
        StdString line;
        StdString_Tidy(&line, nullptr, 0);
        AssignChars(line, TextOf(at<StdString>(node, 8)));
        g_env->pushBack(client + 0x10, nullptr, &line);
        StdString_Tidy(&line, nullptr, 1);
    }
    AssignText(at<StdString>(client, 0x1c), TextOf(at<StdString>(self, 0x20)));
    if (g_drop->attach(client, nullptr, TextOf(at<StdString>(self, 0x3c)))) return 0;
    return g_client->connect(client, nullptr, TextOf(at<StdString>(self, 0x80)), at<u16>(self, 0x90), arg3, self) == 0;
}

u8 __fastcall Mailbox_Open(u8 *self, void *, const char *folder) {
    StdString path;
    StdString_Tidy(&path, nullptr, 0);
    u8 stream[0x90];   // (+0 the vbtable, +8 the file buffer, its file at +0x58, +0x5c the ios)
    u8 *const buffer = stream + 8;
    u8 *const ios = stream + 0x5c;
    at<u32>(stream, 0) = 0x00702968;
    g_env->iosInit(ios);
    g_env->istreamCtor(stream, nullptr, buffer, 0, 0);
    g_env->fileCtor(buffer, nullptr, nullptr);
    at<u32>(IosOf(stream), 0) = (u32)(uintptr_t)Class702960_vtable;
    ClearNumbers(self + 0x58);
    ClearNumbers(self + 0x64);
    ClearNumbers(self + 0x70);
    StdString *const own = reinterpret_cast<StdString *>(self);
    StdString_AssignChars(own, nullptr, folder, CRT_strlen(folder));
    const char *const outbox = gMailbox_Open_7740c0;
    StdString_AppendChars(own, nullptr, outbox, CRT_strlen(outbox));
    StdString_AssignSub(&path, nullptr, own, 0, npos());
    const char *const name = gMailbox_7740b4;
    StdString_AppendChars(&path, nullptr, name, CRT_strlen(name));
    if (!g_env->open(buffer, nullptr, TextOf(path), 1)) g_env->setState(IosOf(stream), nullptr, 2, 0);
    if (!at<void *>(stream, 0x58)) {
        const s32 made = at<s32(__stdcall *)(const char *, void *)>(nullptr, 0x006fd120)(TextOf(*own), nullptr);
        g_env->ifstreamDtor(ios);
        NIos_Dtor(ios);
        StdString_Tidy(&path, nullptr, 1);
        return made != 0;
    }
    ReadNumbers(stream, self + 0x58);
    ReadNumbers(stream, self + 0x70);
    ReadNumbers(stream, self + 0x64);
    void *const file = at<void *>(stream, 0x58);
    if (file && g_env->fclose(file) == 0)
        g_env->fileInit(buffer, nullptr, nullptr, 2);
    else
        g_env->setState(IosOf(stream), nullptr, 2, 0);
    g_env->ifstreamDtor(ios);
    NIos_Dtor(ios);
    StdString_Tidy(&path, nullptr, 1);
    return 1;
}

u8 __fastcall AddressBook_Load(u8 *self, void *, const char *folder) {
    u8 stream[0x90];   // (+0 the vbtable, +8 the file buffer, its file at +0x58, +0x5c the ios)
    u8 *const buffer = stream + 8;
    u8 *const ios = stream + 0x5c;
    at<u32>(stream, 0) = 0x00702968;
    g_env->iosInit(ios);
    g_env->istreamCtor(stream, nullptr, buffer, 0, 0);
    g_env->fileCtor(buffer, nullptr, nullptr);
    at<u32>(IosOf(stream), 0) = (u32)(uintptr_t)Class702960_vtable;
    StdString path;
    StdString_Tidy(&path, nullptr, 0);
    StdString_AssignChars(&path, nullptr, folder, CRT_strlen(folder));
    StdString_AppendChars(&path, nullptr, kAddressBookName, CRT_strlen(kAddressBookName));
    if (!g_env->open(buffer, nullptr, TextOf(path), 1)) g_env->setState(IosOf(stream), nullptr, 2, 0);
    if (!at<void *>(stream, 0x58)) {
        if (g_env->languageId(kLanguage) == 0x409) {
            StdString text;
            StdString_Tidy(&text, nullptr, 0);
            StdString_AssignChars(&text, nullptr, kEnglishAddress, CRT_strlen(kEnglishAddress));
            AddDefault(self, &text);
            StdString_Tidy(&text, nullptr, 1);
        } else {
            AddDefault(self, kOtherAddress);
        }
        StdString_Tidy(&path, nullptr, 1);
        g_env->ifstreamDtor(ios);
        NIos_Dtor(ios);
        return 0;
    }
    char line[0x80];
    while (!ReadFailed(ReadLine(stream, line, 0x80))) {
        StdString text;
        StdString_Tidy(&text, nullptr, 0);
        StdString_AssignChars(&text, nullptr, line, CRT_strlen(line));
        u8 *out;
        g_env->insert(self, nullptr, &out, at<u8 *>(self, 4), &text);
        StdString_Tidy(&text, nullptr, 1);
    }
    if (!g_env->fileClose(buffer)) g_env->setState(IosOf(stream), nullptr, 2, 0);
    if (g_env->languageId(kLanguage) == 0x409) {
        StdString text;
        StdString_Tidy(&text, nullptr, 0);
        StdString_AssignChars(&text, nullptr, kEnglishAddress, CRT_strlen(kEnglishAddress));
        g_env->addSorted(self, nullptr, &text);
        StdString_Tidy(&text, nullptr, 1);
    } else {
        g_env->addSorted(self, nullptr, kOtherAddress);
    }
    StdString_Tidy(&path, nullptr, 1);
    g_env->ifstreamDtor(ios);
    NIos_Dtor(ios);
    return 1;
}

u8 __fastcall AddressBook_Save(u8 *self, void *, const char *folder) {
    u8 stream[0x8c];   // (+0 the vbtable, +4 the file buffer, its file at +0x54, +0x58 the ios)
    u8 *const buffer = stream + 4;
    u8 *const ios = stream + 0x58;
    at<u32>(stream, 0) = 0x00702978;
    g_env->iosInit(ios);
    g_env->ostreamCtor(stream, nullptr, buffer, 0, 1, 0);
    g_env->fileCtor(buffer, nullptr, nullptr);
    at<u32>(IosOf(stream), 0) = (u32)(uintptr_t)Class702970_vtable;
    StdString path;
    StdString_Tidy(&path, nullptr, 0);
    StdString_AssignChars(&path, nullptr, folder, CRT_strlen(folder));
    StdString_AppendChars(&path, nullptr, kAddressBookName, CRT_strlen(kAddressBookName));
    if (!g_env->open(buffer, nullptr, TextOf(path), 0x12)) g_env->setState(IosOf(stream), nullptr, 2, 0);
    if (!at<void *>(stream, 0x54)) {
        StdString_Tidy(&path, nullptr, 1);
        g_env->ofstreamDtor(ios);
        NIos_Dtor(ios);
        return 0;
    }
    char line[0x80];
    for (u8 *node = at<u8 *>(at<u8 *>(self, 4), 0); node != at<u8 *>(self, 4); node = at<u8 *>(node, 0)) {
        CRT_sprintf(line, gText_Enginedebug_764ff0, TextOf(at<StdString>(node, 8)));
        g_env->write(stream, line);
    }
    void *const file = at<void *>(stream, 0x54);
    if (file && g_env->fclose(file) == 0) {
        g_env->fileInit(buffer, nullptr, nullptr, 2);
    } else {
        u8 *const state = IosOf(stream);
        g_env->clear(state, nullptr, at<u32>(state, 4) | 2, 0);
    }
    StdString_Tidy(&path, nullptr, 1);
    at<u32>(IosOf(stream), 0) = (u32)(uintptr_t)Class702970_vtable;
    g_env->fileDtor(buffer);
    at<u32>(IosOf(stream), 0) = (u32)(uintptr_t)Class70995c_vtable;
    NIos_Dtor(ios);
    return 1;
}

u32 *__fastcall NumberList_Erase(u8 *list, void *, u32 *out, u32 node) {
    u8 *const at_node = reinterpret_cast<u8 *>((uintptr_t)node);
    const u32 next = at<u32>(at_node, 0);
    at<u32>(at<u8 *>(at_node, 4), 0) = next;
    at<u32>(at<u8 *>(at_node, 0), 4) = at<u32>(at_node, 4);
    GameHeap_Free(at_node);
    --at<u32>(list, 8);
    *out = next;
    return out;
}

void __fastcall NumberList_Destroy(u8 *list) {
    u8 *const sentinel = at<u8 *>(list, 4);
    for (u8 *node = at<u8 *>(sentinel, 0); node != sentinel;) {
        u8 *const gone = node;
        node = at<u8 *>(gone, 0);
        at<u32>(at<u8 *>(gone, 4), 0) = at<u32>(gone, 0);
        at<u32>(at<u8 *>(gone, 0), 4) = at<u32>(gone, 4);
        GameHeap_Free(gone);
        --at<u32>(list, 8);
    }
    GameHeap_Free(at<u8 *>(list, 4));
    at<u32>(list, 4) = 0;
    at<u32>(list, 8) = 0;
}

u32 *__fastcall NumberList_Insert(u8 *list, void *, u32 *out, u32 *where, const u32 *value) {
    const u32 before = where[1];
    u32 *const node = static_cast<u32 *>(GameHeap_Alloc(0xc));
    node[0] = where ? static_cast<u32>(reinterpret_cast<uintptr_t>(where)) : static_cast<u32>(reinterpret_cast<uintptr_t>(node));
    node[1] = before ? before : static_cast<u32>(reinterpret_cast<uintptr_t>(node));
    where[1] = static_cast<u32>(reinterpret_cast<uintptr_t>(node));
    *reinterpret_cast<u32 *>(static_cast<uintptr_t>(node[1])) = static_cast<u32>(reinterpret_cast<uintptr_t>(node));
    node[2] = *value;
    ++at<u32>(list, 8);
    *out = static_cast<u32>(reinterpret_cast<uintptr_t>(node));
    return out;
}

u8 __fastcall Mailbox_SaveMessage(u8 *self) {
    StdString path;
    StdString_Tidy(&path, nullptr, 0);
    StdString picture;
    StdString_Tidy(&picture, nullptr, 0);
    u8 stream[0x8c];   // (+0 the vbtable, +4 the file buffer, its file at +0x54, +0x58 the ios)
    u8 *const buffer = stream + 4;
    u8 *const ios = stream + 0x58;
    at<u32>(stream, 0) = 0x00702978;
    g_env->iosInit(ios);
    g_env->ostreamCtor(stream, nullptr, buffer, 0, 1, 0);
    g_env->fileCtor(buffer, nullptr, nullptr);
    at<u32>(IosOf(stream), 0) = (u32)(uintptr_t)Class702970_vtable;
    u32 number = 0;
    if (at<u32>(self, 0x60)) {
        u8 *const head = at<u8 *>(self, 0x5c);
        number = static_cast<u32>(reinterpret_cast<uintptr_t>(self));
        for (u8 *node = at<u8 *>(head, 0); node != head; node = at<u8 *>(node, 0)) number = at<u32>(node, 8) + 1;
    }
    const StdString *const own = reinterpret_cast<const StdString *>(self);
    StdString_AssignSub(&path, nullptr, own, 0, npos());
    char line[0x80];
    CRT_sprintf(line, gMailbox_7740d8, number);
    StdString_AppendChars(&path, nullptr, line, CRT_strlen(line));
    if (!g_env->open(buffer, nullptr, TextOf(path), 0x12)) g_env->setState(IosOf(stream), nullptr, 2, 0);
    if (!at<void *>(stream, 0x54)) {
        g_env->ofstreamDtor(ios);
        at<u32>(ios, 0) = (u32)(uintptr_t)NIos_vtable;
        g_env->iosDtor(ios);
        StdString_Tidy(&picture, nullptr, 1);
        StdString_Tidy(&path, nullptr, 1);
        return 0;
    }
    const u32 now = g_env->time(nullptr);
    const u8 *const date = g_env->localtime(&now);
    at<u32>(self, 0x4c) = at<u32>(date, 0xc);
    at<u32>(self, 0x50) = at<u32>(date, 0x10) + 1;
    at<u32>(self, 0x54) = at<u32>(date, 0x14) + 0x76c;
    const char *const lineEnd = reinterpret_cast<const char *>((uintptr_t)0x0074da30);
    const char *const decimal = gChatevents_759920;
    for (u32 offset = 0x4c; offset <= 0x54; offset += 4) {
        CRT_sprintf(line, decimal, at<u32>(self, offset));
        g_env->write(g_env->write(stream, line), lineEnd);
    }
    g_env->write(g_env->write(stream, TextOf(at<StdString>(self, 0x10))), lineEnd);
    g_env->write(g_env->write(stream, TextOf(at<StdString>(self, 0x20))), lineEnd);
    for (u8 *node = at<u8 *>(at<u8 *>(self, 0x34), 0); node != at<u8 *>(self, 0x34); node = at<u8 *>(node, 0)) g_env->write(g_env->write(stream, TextOf(at<StdString>(node, 8))), lineEnd);
    void *const file = at<void *>(stream, 0x54);
    if (file && g_env->fclose(file) == 0)
        g_env->fileInit(buffer, nullptr, nullptr, 2);
    else
        g_env->setState(IosOf(stream), nullptr, 2, 0);
    StdString_AssignSub(&picture, nullptr, own, 0, npos());
    CRT_sprintf(line, gMailbox_7740cc, number);
    StdString_AppendChars(&picture, nullptr, line, CRT_strlen(line));
    StdString *const pictureAt = reinterpret_cast<StdString *>(self + 0x3c);
    at<s32(__stdcall *)(const char *, const char *)>(nullptr, 0x006fd11c)(TextOf(*pictureAt), TextOf(picture));
    StdString_AssignSub(pictureAt, nullptr, &picture, 0, npos());
    u32 value = number;
    u32 out;
    g_env->insertNumber(self + 0x58, nullptr, &out, at<u32>(self, 0x5c), reinterpret_cast<const s32 *>(&value));
    u8 *const list = self + 0x64;
    {
        u8 *const head = at<u8 *>(list, 4);
        u8 *const node = Node0c_New(static_cast<u32>(reinterpret_cast<uintptr_t>(head)), at<u32>(head, 4));
        at<u8 *>(head, 4) = node;
        at<u8 *>(at<u8 *>(node, 4), 0) = node;
        ImeVector_Construct(reinterpret_cast<u32 *>(node + 8), &value);
    }
    if (++at<u32>(list, 8) >= 2) SortNumbers(list);
    u8 *const end = at<u8 *>(list, 4);
    u8 *kept = at<u8 *>(end, 0);
    if (kept != end) {
        for (u8 *next = at<u8 *>(kept, 0); next != end; next = at<u8 *>(kept, 0)) {
            if (at<u32>(kept, 8) == at<u32>(next, 8))
                NumberList_Erase(list, nullptr, &out, static_cast<u32>(reinterpret_cast<uintptr_t>(next)));
            else
                kept = next;
        }
    }
    g_drop->saveOutbox(self);
    g_env->ofstreamDtor(ios);
    NIos_Dtor(ios);
    StdString_Tidy(&picture, nullptr, 1);
    StdString_Tidy(&path, nullptr, 1);
    return 1;
}

void __fastcall Mailbox_DeleteMessage(u8 *self, void *, u32 number) {
    StdString path;
    StdString_Tidy(&path, nullptr, 0);
    RemoveFile(self, &path, reinterpret_cast<uintptr_t>(&gMailbox_7740d8), number);
    RemoveFile(self, &path, reinterpret_cast<uintptr_t>(&gMailbox_7740cc), number);
    StdString_Tidy(&path, nullptr, 1);
}

u32 __fastcall Mailbox_NewAttachment(u8 *target, void *, const char *path) {
    void *const block = GameHeap_Alloc(0x24);
    u8 *const made = block ? g_drop->attachmentCtor(block, nullptr, path) : nullptr;
    at<u8 *>(target, 0x68) = made;
    return made == nullptr;
}

u8 __fastcall Mailbox_DropFirst(u8 *self, void *, u8 *target) {
    const u32 number = at<u32>(at<u8 *>(at<u8 *>(self, 0x68), 0), 8);
    RemoveNumber(self + 0x70, number);
    RemoveNumber(self + 0x64, number);
    RemoveNumber(self + 0x58, number);
    g_drop->deleteMessage(self, nullptr, number);
    g_drop->saveOutbox(self);
    if (!at<u32>(self, 0x6c)) return 0;
    if (!g_drop->loadMessage(self, nullptr, at<u32>(at<u8 *>(at<u8 *>(self, 0x68), 0), 8))) return 0;
    FillFromSession(target, g_drop->instance);
    for (u8 *node = at<u8 *>(at<u8 *>(self, 0x34), 0); node != at<u8 *>(self, 0x34); node = at<u8 *>(node, 0)) g_drop->pushChars(target, nullptr, TextOf(at<StdString>(node, 8)));
    AssignText(at<StdString>(target, 0x1c), TextOf(at<StdString>(self, 0x20)));
    return g_drop->attach(target, nullptr, TextOf(at<StdString>(self, 0x3c))) == 0;
}

void __fastcall NIos_Dtor(u8 *self) {
    at<u32>(self, 0) = (u32)(uintptr_t)NIos_vtable;
    g_env->iosDtor(self);
}

u8 __fastcall Mailbox_SaveOutbox(u8 *self) {
    StdString path;
    StdString_Tidy(&path, nullptr, 0);
    u8 stream[0x8c];   // (+0 the vbtable, +4 the file buffer, its file at +0x54, +0x58 the ios: the original's frame has this much)
    u8 *const buffer = stream + 4;
    u8 *const ios = stream + 0x58;
    at<u32>(stream, 0) = 0x00702978;
    g_env->iosInit(ios);
    g_env->ostreamCtor(stream, nullptr, buffer, 0, 1, 0);
    g_env->fileCtor(buffer, nullptr, nullptr);
    at<u32>(IosOf(stream), 0) = (u32)(uintptr_t)Class702970_vtable;
    StdString_AssignSub(&path, nullptr, reinterpret_cast<const StdString *>(self), 0, npos());
    const char *const name = gMailbox_7740b4;
    StdString_AppendChars(&path, nullptr, name, CRT_strlen(name));
    if (!g_env->open(buffer, nullptr, TextOf(path), 0x12)) g_env->setState(IosOf(stream), nullptr, 2, 0);
    if (!at<void *>(stream, 0x54)) {
        g_env->ofstreamDtor(ios);
        NIos_Dtor(ios);
        StdString_Tidy(&path, nullptr, 1);
        return 0;
    }
    const char *const lineEnd = reinterpret_cast<const char *>((uintptr_t)0x0074da30);
    WriteNumbers(self, 0x5c, stream);
    g_env->write(stream, lineEnd);
    WriteNumbers(self, 0x74, stream);
    g_env->write(stream, lineEnd);
    WriteNumbers(self, 0x68, stream);
    void *const file = at<void *>(stream, 0x54);
    if (file && g_env->fclose(file) == 0)
        g_env->fileInit(buffer, nullptr, nullptr, 2);
    else
        g_env->setState(IosOf(stream), nullptr, 2, 0);
    at<u32>(IosOf(stream), 0) = (u32)(uintptr_t)Class702970_vtable;
    g_env->fileDtor(buffer);
    at<u32>(IosOf(stream), 0) = (u32)(uintptr_t)Class70995c_vtable;
    at<u32>(ios, 0) = (u32)(uintptr_t)NIos_vtable;
    g_env->iosDtor(ios);
    StdString_Tidy(&path, nullptr, 1);
    return 1;
}

u8 __fastcall Mailbox_LoadMessage(u8 *self, void *, u32 number) {
    StdString path;
    StdString_Tidy(&path, nullptr, 0);
    StdString unused;
    StdString_Tidy(&unused, nullptr, 0);
    u8 stream[0x90];   // (+0 the vbtable, +8 the file buffer, its file at +0x58, +0x5c the ios)
    u8 *const buffer = stream + 8;
    u8 *const ios = stream + 0x5c;
    at<u32>(stream, 0) = 0x00702968;
    g_env->iosInit(ios);
    g_env->istreamCtor(stream, nullptr, buffer, 0, 0);
    g_env->fileCtor(buffer, nullptr, nullptr);
    at<u32>(IosOf(stream), 0) = (u32)(uintptr_t)Class702960_vtable;
    StdString_AssignSub(&path, nullptr, reinterpret_cast<const StdString *>(self), 0, npos());
    char line[0x100];
    CRT_sprintf(line, gMailbox_7740d8, number);
    StdString_AppendChars(&path, nullptr, line, CRT_strlen(line));
    if (!g_env->open(buffer, nullptr, TextOf(path), 1)) g_env->setState(IosOf(stream), nullptr, 2, 0);
    if (!at<void *>(stream, 0x58)) {
        g_env->ifstreamDtor(ios);
        NIos_Dtor(ios);
        StdString_Tidy(&unused, nullptr, 1);
        StdString_Tidy(&path, nullptr, 1);
        return 0;
    }
    ReadLine(stream, line);
    at<s32>(self, 0x4c) = g_env->atoi(line);
    ReadLine(stream, line);
    at<s32>(self, 0x50) = g_env->atoi(line);
    ReadLine(stream, line);
    at<s32>(self, 0x54) = g_env->atoi(line);
    ReadLine(stream, line);
    StdString_AssignChars(reinterpret_cast<StdString *>(self + 0x10), nullptr, line, CRT_strlen(line));
    ReadLine(stream, line);
    StdString_AssignChars(reinterpret_cast<StdString *>(self + 0x20), nullptr, line, CRT_strlen(line));
    u8 *const list = self + 0x30;
    u8 *const head = at<u8 *>(self, 0x34);
    u32 cursor = at<u32>(head, 0);
    while (cursor != static_cast<u32>(reinterpret_cast<uintptr_t>(head))) {
        u32 old, out;
        const u32 node = *ListCursor_Next(&cursor, nullptr, &old, 0);
        g_env->erase(list, nullptr, &out, reinterpret_cast<u8 *>((uintptr_t)node));
    }
    for (;;) {
        u8 *const got = ReadLine(stream, line);
        // (a null stream: the state read at 4, a fault, as in the original)
        const u8 *const state = got ? IosOf(got) : nullptr;
        if ((at<u8>(state, 4) & 6) != 0) break;
        StdString text;
        StdString_Tidy(&text, nullptr, 0);
        StdString_AssignChars(&text, nullptr, line, CRT_strlen(line));
        u8 *out;
        g_env->insert(list, nullptr, &out, at<u8 *>(self, 0x34), &text);
        StdString_Tidy(&text, nullptr, 1);
    }
    if (!g_env->fileClose(buffer)) g_env->setState(IosOf(stream), nullptr, 2, 0);
    StdString_AssignSub(reinterpret_cast<StdString *>(self + 0x3c), nullptr, reinterpret_cast<const StdString *>(self), 0, npos());
    CRT_sprintf(line, gMailbox_7740cc, number);
    StdString_AppendChars(reinterpret_cast<StdString *>(self + 0x3c), nullptr, line, CRT_strlen(line));
    g_env->ifstreamDtor(ios);
    NIos_Dtor(ios);
    StdString_Tidy(&unused, nullptr, 1);
    StdString_Tidy(&path, nullptr, 1);
    return 1;
}
