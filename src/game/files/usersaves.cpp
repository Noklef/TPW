#include "usersaves.h"

#include <cstdint>
#include <cstring>

#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "dirsource.h"
#include "direntry.h"
#include "file.h"
#include "handles.h"
#include "userslots.h"

namespace {

AddrCalls g_calls;

constexpr u32 kHandleCtor = 0x005f8240, kHandleOpen = 0x005f8510, kHandleFirst = 0x005f8370, kHandleNext = 0x005f83a0, kHandleDtor = 0x005f82a0;
constexpr u32 kThemeFolder = 0x005c8890, kBeforeSave = 0x005c8a10, kLoad = 0x00414d40, kSave = 0x00414920, kSaveObject = 0x0078a450;

template <typename T> T at(u32 address) { return reinterpret_cast<T>((uintptr_t)address); }
u32 word(const void *p) { return static_cast<u32>(reinterpret_cast<uintptr_t>(p)); }

struct Node {
    u8 *item;
    Node *next;
    Node *prev;
};

struct List {
    void *vtable;
    Node *head;
    Node *tail;
    u32 count;
};

struct Entry {
    alignas(4) u8 bytes[0x128];   // a DirEntry with its 0x104 chars of path, as in the original's frame
    DirEntry *operator->() { return reinterpret_cast<DirEntry *>(bytes); }
};

void themeFolder(WString *out) {
    u8 *const slots = UserSlots_Instance();
    g_calls.Th(kThemeFolder, word(slots), out);
}

// The path of a save, narrow in text; the wide one gone. Then SaveGame_Load or SaveGame_Save.
u32 pathCall(u32 call, const WString *name, const WString *extension) {
    WString path;
    WString_Ctor(&path);
    WString folder;
    u8 *const slots = UserSlots_Instance();
    const u32 made = g_calls.Th(kThemeFolder, word(slots), &folder);
    vcall<void>(&path, 0x18, at<const WString *>(made));
    WString_Dtor(&folder);
    vcall<void>(&path, 0x28, at<const WString *>(0x00f7b888));
    vcall<void>(&path, 0x28, name);
    vcall<void>(&path, 0x28, extension);
    String narrow;
    String_Ctor(&narrow);
    String_AssignFromWide(&narrow, &path);
    const u32 answer = g_calls.Th(call, kSaveObject, narrow.data, 0u, 2u);
    String_Dtor(&narrow);
    WString_Dtor(&path);
    return answer;
}

s32 status(u32 result) { return *at<const s32 *>(result); }

Node *newNode(u8 *item) {
    Node *const node = static_cast<Node *>(GameHeap_Alloc(sizeof(Node)));
    if (node) {
        node->item = item;
        node->next = nullptr;
        node->prev = nullptr;
    }
    return node;
}

void append(List *list, u8 *item) {
    if (list->count == 0) {
        Node *const node = newNode(item);
        node->next = list->head;   // (no block: a fault, sic)
        if (list->head)
            list->head->prev = node;
        else
            list->tail = node;
        list->head = node;
    } else {
        for (const Node *n = list->head; n; n = n->next)
            if (n->item == item) return;
        Node *const node = newNode(item);
        node->prev = list->tail;
        if (list->tail)
            list->tail->next = node;
        else
            list->head = node;
        list->tail = node;
    }
    ++list->count;
}

void addEntry(List *list, Entry &entry) {
    String name;
    String_CtorStr(&name, nullptr, entry->name);
    StringView view;
    const StringView *const stem = String_Left(&name, nullptr, &view, name.length - at<const String *>(0x00f7b548)->length);
    vcall<void>(&name, 0x1c, stem);
    void *const block = GameHeap_Alloc(0x1c);
    WString *const item = block ? WString_Ctor(static_cast<WString *>(block)) : nullptr;
    WString_AssignFromAnsi(item, &name);
    u8 *const bytes = reinterpret_cast<u8 *>(item);
    *reinterpret_cast<u32 *>(bytes + 0x14) = entry->writeTime[0];
    *reinterpret_cast<u32 *>(bytes + 0x18) = entry->writeTime[1];
    append(list, bytes);
    String_Dtor(&name);
}

void emptyList(List *list) {
    while (list->count != 0) {
        Node *const node = list->head;
        WString *item = nullptr;
        if (node) {
            list->head = node->next;
            if (node->next)
                node->next->prev = nullptr;
            else
                list->tail = nullptr;
            --list->count;
            item = reinterpret_cast<WString *>(node->item);
            GameHeap_Free(node);
        }
        if (item) {
            WString_Dtor(item);
            GameHeap_Free(item);
        }
    }
    Node *node = list->head;
    list->tail = nullptr;
    list->head = nullptr;
    while (node) {
        Node *const next = node->next;
        GameHeap_Free(node);
        node = next;
    }
    list->count = 0;
}

// The entry made ready for a search: an empty text, the name on it.
void clearEntry(Entry &entry) {
    entry->path[0] = 0;
    entry->name = entry->path;
}

// All but the name's place: the eight dwords, the text, and the name as far into the copy.
void copyEntry(Entry &to, Entry &from) {
    std::memcpy(to.bytes, from.bytes, 0x20);
    const char *const text = reinterpret_cast<const char *>(from.bytes + 0x24);
    std::memcpy(to.bytes + 0x24, text, std::strlen(text) + 1);
    to->name = reinterpret_cast<char *>(to.bytes + 0x24) + (from->name - text);
}

}  // namespace

void UserSaves_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u32 __stdcall UserSaves_Load(const WString *name, const WString *extension) { return pathCall(kLoad, name, extension); }

u32 __stdcall UserSaves_Save(const WString *name, const WString *extension) {
    g_calls.Th(kBeforeSave, word(UserSlots_Instance()));
    return pathCall(kSave, name, extension);
}

u32 __stdcall UserSaves_LoadA(const WString *name) { return UserSaves_Load(name, at<const WString *>(0x00f7ad98)); }
u32 __stdcall UserSaves_LoadB(const WString *name) { return UserSaves_Load(name, at<const WString *>(0x00f7adf8)); }
u32 __stdcall UserSaves_SaveA(const WString *name) { return UserSaves_Save(name, at<const WString *>(0x00f7ad98)); }
u32 __stdcall UserSaves_SaveB(const WString *name) { return UserSaves_Save(name, at<const WString *>(0x00f7adf8)); }

u8 *__fastcall UserSaves_List(u8 *self) {
    List *const list = reinterpret_cast<List *>(self);
    emptyList(list);
    WString wide;
    themeFolder(&wide);
    String folderPath;
    String_Ctor(&folderPath);
    String_AssignFromWide(&folderPath, &wide);
    Handle folder;
    g_calls.Th(kHandleCtor, word(&folder));
    folder.vtable = at<void *const *>(0x006fd9e8);
    s32 result;
    if (status(g_calls.Th(kHandleOpen, word(&folder), &result, folderPath.data)) >= 0) {
        Entry entry;
        clearEntry(entry);
        String filter;
        String_Ctor(&filter);
        vcall<void>(&filter, 0x18, at<const String *>(0x00f7b948));
        vcall<void>(&filter, 0x28, at<const String *>(0x00f7b548));
        s32 found;
        if (status(g_calls.Th(kHandleFirst, word(&folder), &result, filter.data, entry.bytes)) >= 0) {
            do {
                if (!DirEntry_IsRealDir(entry.bytes)) addEntry(list, entry);
            } while (status(g_calls.Th(kHandleNext, word(&folder), &found, entry.bytes)) >= 0);
        }
        String_Dtor(&filter);
    }
    u8 *const first = list->count ? list->head->item : nullptr;
    g_calls.Th(kHandleDtor, word(&folder));
    String_Dtor(&folderPath);
    WString_Dtor(&wide);
    return first;
}

void __cdecl UserSaves_LoadNewest() {
    WString wide;
    themeFolder(&wide);
    String folderPath;
    String_Ctor(&folderPath);
    String_AssignFromWide(&folderPath, &wide);
    Handle folder;
    g_calls.Th(kHandleCtor, word(&folder));
    folder.vtable = at<void *const *>(0x006fd9e8);
    s32 result;
    if (status(g_calls.Th(kHandleOpen, word(&folder), &result, folderPath.data)) >= 0) {
        Entry entry;
        clearEntry(entry);
        String filter;
        String_Ctor(&filter);
        vcall<void>(&filter, 0x18, at<const String *>(0x00f7b948));
        vcall<void>(&filter, 0x28, at<const String *>(0x00f7b398));
        if (status(g_calls.Th(kHandleFirst, word(&folder), &result, filter.data, entry.bytes)) >= 0) {
            Entry newest;
            clearEntry(newest);
            bool any = false;
            do {
                if (!DirEntry_IsRealDir(entry.bytes) && (!any || FileTime_Compare(entry->writeTime, newest->writeTime) > 0)) {
                    copyEntry(newest, entry);
                    any = true;
                }
            } while (status(g_calls.Th(kHandleNext, word(&folder), &result, entry.bytes)) >= 0);
            if (any) {
                String path;
                String_CtorCopy(&path, nullptr, reinterpret_cast<const StringView *>(&folderPath));
                vcall<void>(&path, 0x28, at<const String *>(0x00f7aed0));
                vcall<void>(&path, 0x20, newest->name);
                g_calls.Th(kLoad, kSaveObject, path.data, 0u, 2u);
                String_Dtor(&path);
            }
        }
        String_Dtor(&filter);
    }
    g_calls.Th(kHandleDtor, word(&folder));
    String_Dtor(&folderPath);
    WString_Dtor(&wide);
}
}
