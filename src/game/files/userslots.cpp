#include "userslots.h"

#include <cstdint>
#include <initializer_list>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/globalsave.h"
#include "../core/stdlist.h"
#include "../core/stdstring.h"
#include "../core/string.h"
#include "../core/thunks.h"
#include "../core/trivial.h"
#include "../core/vcall.h"
#include "direntry.h"
#include "file.h"
#include "gamepaths.h"
#include "handles.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const WString *wide(u32 address) { return reinterpret_cast<const WString *>((uintptr_t)address); }

// Appended through the wide string's vtable (WString_Append).
void append(WString *s, const WString *tail) { vcall<void>(s, 0x28, tail); }

const UserSlotsEnv kGameEnv = {&UserSlots_Ctor, reinterpret_cast<u16 *(__cdecl *)(s32, u16 *, s32)>((uintptr_t)0x0067e010), &GamePaths_RemoveTree};
const UserSlotsEnv *g_env = &kGameEnv;

}  // namespace

void UserSlots_SetEnv(const UserSlotsEnv *env) { g_env = env ? env : &kGameEnv; }

namespace {
const UserSlotsScanEnv kScanGameEnv = {&GamePaths_SaveFolder, &GamePaths_UsersFolder, &GamePaths_OnlineFolder, &Handle_Open28, &Handle_Vfn1, &Handle_Vfn2, &Dir_Create, &DirEntry_IsRealDir,
                                       reinterpret_cast<s32(__cdecl *)(s32)>((uintptr_t)0x0067a560), reinterpret_cast<s32(__cdecl *)(const char *)>((uintptr_t)0x0067a520), &UserSlots_SlotFolder, &GlobalSave_Construct,
                                       reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)0x005af250), reinterpret_cast<void(__fastcall *)(u8 *, void *, const char *)>((uintptr_t)0x005afd70), &GlobalSave_Byte24, &GamePaths_MakeFolders};
const UserSlotsScanEnv *g_scan = &kScanGameEnv;

void DeleteGlobalSave() {
    if (u8 *const info = at<u8 *>(nullptr, 0x00f7c58c)) {
        g_scan->saveDtor(info);
        GameHeap_Free(info);
    }
    at<u8 *>(nullptr, 0x00f7c58c) = nullptr;
}

void NewGlobalSave() {
    void *const block = GameHeap_Alloc(0x50);
    at<u8 *>(nullptr, 0x00f7c58c) = block ? g_scan->saveCtor(static_cast<u8 *>(block)) : nullptr;
}

// A folder of users named "<slot + 1><name>".
void TakeSlot(u8 *self, const char *folderName, s32 slot) {
    String name;
    String_CtorStr(&name, nullptr, folderName);
    StringView rest;
    vcall<void>(&name, 0x1c, String_Right(&name, nullptr, &rest, name.length - 1));
    u8 *const record = self + slot * 24;
    WString_AssignFromAnsi(reinterpret_cast<WString *>(record), &name);
    WString folder;
    g_scan->slotFolder(self, nullptr, &folder, slot);
    WString path;
    WString_CtorCopy(&path, nullptr, &folder);
    append(&path, wide(0x00f82508));
    append(&path, wide(0x00f818a8));
    String pathText;
    String_Ctor(&pathText);
    String_AssignFromWide(&pathText, &path);
    DeleteGlobalSave();
    const char *const text = pathText.data;
    NewGlobalSave();
    g_scan->saveLoad(at<u8 *>(nullptr, 0x00f7c58c), nullptr, text);
    if (!at<u8 *>(nullptr, 0x00f7c58c)) NewGlobalSave();
    at<u32>(record, 0x14) = g_scan->saveByte24(at<u8 *>(nullptr, 0x00f7c58c));
    DeleteGlobalSave();
    g_scan->makeFolders(self, nullptr, &folder);
    String_Dtor(&pathText);
    WString_Dtor(&path);
    WString_Dtor(&folder);
    String_Dtor(&name);
}

void WalkUsers(u8 *self, Handle *handle) {
    u8 entry[0x118];   // (a DirEntry: +0x20 its name pointer, +0x24 the path buffer; the original's frame has this much)
    at<u8 *>(entry, 0x20) = entry + 0x24;
    entry[0x24] = 0;
    s32 status;
    const u32 address = static_cast<u32>(reinterpret_cast<uintptr_t>(entry));
    if (*g_scan->first(handle, nullptr, &status, at<u32>(nullptr, 0x00f825d0), address) < 0) return;
    do {
        const char *const name = at<const char *>(entry, 0x20);
        if (CRT_strlen(name) > 1 && g_scan->isRealDir(entry) && g_scan->isDigit(name[0])) {
            const char digit[2] = {name[0], 0};
            const s32 number = g_scan->atoi(digit);
            if (number >= 1 && number <= 4) TakeSlot(self, name, number - 1);
        }
    } while (*g_scan->next(handle, nullptr, &status, address) >= 0);
}
}  // namespace

void UserSlots_SetScanEnv(const UserSlotsScanEnv *env) { g_scan = env ? env : &kScanGameEnv; }

namespace {
const UserSlotsChoiceEnv kChoiceGameEnv = {reinterpret_cast<void(__fastcall *)(u8 *, void *, const char *)>((uintptr_t)0x005cb230),
                                           reinterpret_cast<void(__fastcall *)(u8 *, void *, const char *)>((uintptr_t)0x005cd9c0),
                                           reinterpret_cast<void(__fastcall *)(u8 *, void *, const char *)>((uintptr_t)0x005cd710),
                                           reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)0x005cb720),
                                           reinterpret_cast<void(__fastcall *)(u8 *, void *, const char *)>((uintptr_t)0x005afc60),
                                           &GlobalSave_Instance,
                                           &GlobalSave_ThemeGet,
                                           reinterpret_cast<void(__fastcall *)(void *)>((uintptr_t)0x00550ca0),
                                           reinterpret_cast<void(__fastcall *)(void *, void *, u32)>((uintptr_t)0x00550d80),
                                           reinterpret_cast<s32(__cdecl *)(u32)>((uintptr_t)0x0067a7d0),
                                           reinterpret_cast<void(__fastcall *)(void *, void *, const String *)>((uintptr_t)0x00424820),
                                           reinterpret_cast<void(__fastcall *)(u8 *, void *, const WString *, u32)>((uintptr_t)0x005c8190),
                                           reinterpret_cast<void(__fastcall *)(u8 *, void *, u32)>((uintptr_t)0x005afc50),
                                           reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)0x005cb030)};
const UserSlotsChoiceEnv *g_choice = &kChoiceGameEnv;

// The global save info, made when there is none.
u8 *SaveInfo() {
    u8 *&info = at<u8 *>(nullptr, 0x00f7c58c);
    if (!info) {
        void *const block = GameHeap_Alloc(0x50);
        info = block ? g_scan->saveCtor(static_cast<u8 *>(block)) : nullptr;
    }
    return info;
}

// The mode setting's static (0xfb3b7c), read and its destructor registered on the first use.
void *ModeSetting() {
    void *const setting = reinterpret_cast<void *>((uintptr_t)0x00fb3b7c);
    u8 &guard = at<u8>(nullptr, 0x00fb2764);
    if (!(guard & 1)) {
        guard |= 1;
        g_choice->settingRead(setting);
        g_choice->atexit(0x005d1750);
    }
    return setting;
}
}  // namespace

void UserSlots_SetChoiceEnv(const UserSlotsChoiceEnv *env) { g_choice = env ? env : &kChoiceGameEnv; }

void __fastcall UserSlots_Choose(u8 *self, void *, s32 index) {
    at<s32>(self, 0x60) = index;
    WString folder;
    g_scan->slotFolder(self, nullptr, &folder, index);
    String narrow;
    String_Ctor(&narrow);
    String_AssignFromWide(&narrow, &folder);
    g_choice->mailboxOpen(self + 0x64, nullptr, narrow.data);
    g_choice->bookLoad(self + 0x100, nullptr, narrow.data);
    DeleteGlobalSave();
    append(&folder, wide(0x00f82508));
    append(&folder, wide(0x00f818a8));
    String path;
    String_Ctor(&path);
    String_AssignFromWide(&path, &folder);
    g_scan->saveLoad(SaveInfo(), nullptr, path.data);
    ModeSetting();
    if (at<u32>(nullptr, 0x00fb3b7c) != 1) {
        const bool online = g_scan->saveByte24(SaveInfo()) != 0;
        g_choice->settingSet(ModeSetting(), nullptr, online ? 2 : 0);
    }
    u8 *node = at<u8 *>(nullptr, 0x00786b90);
    while (node) {
        const u8 *const name = at<const u8 *>(node, 0);
        node = at<u8 *>(node, 4);
        g_choice->themeGet(SaveInfo(), nullptr, name);
    }
    String_Dtor(&path);
    String_Dtor(&narrow);
    WString_Dtor(&folder);
}

// The global save written to the slot's file.
static void WriteSaveFile(u8 *self, s32 i) {
    WString file;
    g_scan->slotFolder(self, nullptr, &file, i);
    append(&file, wide(0x00f82508));
    append(&file, wide(0x00f818a8));
    String path;
    String_Ctor(&path);
    String_AssignFromWide(&path, &file);
    g_choice->saveWrite(SaveInfo(), nullptr, path.data);
    String_Dtor(&path);
    WString_Dtor(&file);
}

void __fastcall UserSlots_WriteSave(u8 *self, void *, s32 i) { WriteSaveFile(self, i); }

void __fastcall UserSlots_WriteCurrentSave(u8 *self) { WriteSaveFile(self, at<s32>(self, 0x60)); }

void __fastcall UserSlots_Create(u8 *self, void *, s32 i, const WString *name, u32 flag) {
    u8 *const record = self + i * 0x18;
    if (at<u32>(record, 4)) {
        WString folder;
        g_scan->slotFolder(self, nullptr, &folder, i);
        String narrow;
        String_Ctor(&narrow);
        String_AssignFromWide(&narrow, &folder);
        g_env->removeTree(&narrow);
        vcall<void>(record, 0x14, reinterpret_cast<const u16 *>((uintptr_t)0x0078a374));
        at<u32>(record, 0x14) = 0;
        String_Dtor(&narrow);
        WString_Dtor(&folder);
    }
    vcall<void>(record, 0x18, name);
    at<u32>(record, 0x14) = flag;
    {
        WString folder;
        g_scan->slotFolder(self, nullptr, &folder, i);
        g_choice->copyThemes(self, nullptr, &folder, flag);
        DeleteGlobalSave();
        void *const block = GameHeap_Alloc(0x50);
        u8 *const info = block ? g_scan->saveCtor(static_cast<u8 *>(block)) : nullptr;
        at<u8 *>(nullptr, 0x00f7c58c) = info;
        g_choice->setByte24(info, nullptr, at<u32>(record, 0x14) ? 1 : 0);
        WriteSaveFile(self, i);
        WString_Dtor(&folder);
    }
}

void __fastcall UserSlots_Dtor(u8 *self) {
    u8 *const head = at<u8 *>(self, 0x104);
    for (u8 *node = at<u8 *>(head, 0); node != head;) {
        u8 *const next = at<u8 *>(node, 0);
        at<u8 *>(at<u8 *>(node, 4), 0) = next;
        at<u8 *>(next, 4) = at<u8 *>(node, 4);
        StdString_Tidy(reinterpret_cast<StdString *>(node + 8), nullptr, 1);
        GameHeap_Free(node);
        --at<u32>(self, 0x108);
        node = next;
    }
    GameHeap_Free(head);
    at<u32>(self, 0x104) = 0;
    at<u32>(self, 0x108) = 0;
    g_choice->mailboxDtor(self + 0x64);
    CRT_VectorDtorIterator(self, 0x18, 4, reinterpret_cast<void(__fastcall *)(void *)>(&Wrap_WString_Dtor_5c74a0));
}

void __fastcall UserSlots_Close(u8 *self) {
    WString folder;
    g_scan->slotFolder(self, nullptr, &folder, at<s32>(self, 0x60));
    String narrow;
    String_Ctor(&narrow);
    String_AssignFromWide(&narrow, &folder);
    g_choice->bookSave(self + 0x100, nullptr, narrow.data);
    g_choice->outboxSave(self + 0x64);
    {
        WString saveFile;
        g_scan->slotFolder(self, nullptr, &saveFile, at<s32>(self, 0x60));
        append(&saveFile, wide(0x00f82508));
        append(&saveFile, wide(0x00f818a8));
        String path;
        String_Ctor(&path);
        String_AssignFromWide(&path, &saveFile);
        g_choice->saveWrite(g_choice->saveInstance(), nullptr, path.data);
        String_Dtor(&path);
        WString_Dtor(&saveFile);
    }
    DeleteGlobalSave();
    WString optionsFile;
    WString_Ctor(&optionsFile);
    String options;
    String_Ctor(&options);
    WString root;
    vcall<void>(&optionsFile, 0x18, g_scan->saveFolder(&root));
    WString_Dtor(&root);
    append(&optionsFile, wide(0x00f82508));
    append(&optionsFile, wide(0x00f82028));
    String_AssignFromWide(&options, &optionsFile);
    g_choice->optionsSave(reinterpret_cast<void *>((uintptr_t)0x0078d8d8), nullptr, &options);
    at<s32>(self, 0x60) = -1;
    String_Dtor(&options);
    WString_Dtor(&optionsFile);
    String_Dtor(&narrow);
    WString_Dtor(&folder);
}

void __fastcall UserSlots_Scan(u8 *self) {
    for (u32 i = 0; i < 4; i++) {
        vcall<void>(self + 24 * i, 0x14, 0x0078a374u);
        at<u32>(self + 24 * i, 0x14) = 0;
    }
    WString saveWide, usersWide, onlineWide;
    String save, users, online;
    g_scan->saveFolder(&saveWide);
    String_Ctor(&save);
    String_AssignFromWide(&save, &saveWide);
    g_scan->usersFolder(&usersWide);
    String_Ctor(&users);
    String_AssignFromWide(&users, &usersWide);
    g_scan->onlineFolder(&onlineWide);
    String_Ctor(&online);
    String_AssignFromWide(&online, &onlineWide);
    Handle handle;
    Handle_Ctor(&handle);
    handle.vtable = reinterpret_cast<void *const *>(0x006fd9e8);
    s32 status;
    const auto path = [](const String &s) { return static_cast<u32>(reinterpret_cast<uintptr_t>(s.data)); };
    if (*g_scan->open(&handle, nullptr, &status, path(save)) < 0) {
        g_scan->create(&status, save.data);
        g_scan->create(&status, users.data);
        g_scan->create(&status, online.data);
    } else {
        if (*g_scan->open(&handle, nullptr, &status, path(users)) < 0)
            g_scan->create(&status, users.data);
        else
            WalkUsers(self, &handle);
        if (*g_scan->open(&handle, nullptr, &status, path(online)) < 0) g_scan->create(&status, online.data);
    }
    Handle_Dtor(&handle);
    String_Dtor(&online);
    WString_Dtor(&onlineWide);
    String_Dtor(&users);
    WString_Dtor(&usersWide);
    String_Dtor(&save);
    WString_Dtor(&saveWide);
}

extern "C" {

u8 *__fastcall UserSlots_Ctor(u8 *self) {
    CRT_VectorCtorIterator(self, 0x18, 4, reinterpret_cast<void(__fastcall *)(void *)>(&WString_CtorReturning), reinterpret_cast<void *>(&Wrap_WString_Dtor_5c74a0));
    UserSlotsPart_Ctor(self + 0x64);
    ListHead_Ctor(self + 0x100);
    at<s32>(self, 0x60) = -1;
    return self;
}

WString *__fastcall UserSlots_SlotName(const u8 *self, void *, WString *out, s32 i) {
    u16 digits[4];
    g_env->itow(i + 1, digits, 10);
    WString name;
    WString_CtorStr(&name, nullptr, digits);
    append(&name, reinterpret_cast<const WString *>(Records24_IfUsed(const_cast<u8 *>(self), nullptr, i)));
    WString_CtorCopy(out, nullptr, &name);
    WString_Dtor(&name);
    return out;
}

WString *__fastcall UserSlots_SlotFolder(const u8 *self, void *, WString *out, s32 i) {
    WString path;
    GamePaths_UsersFolder(&path);
    append(&path, wide(0x00f82508));
    WString name;
    UserSlots_SlotName(self, nullptr, &name, i);
    append(&path, &name);
    WString_Dtor(&name);
    WString_CtorCopy(out, nullptr, &path);
    WString_Dtor(&path);
    return out;
}

void __fastcall UserSlots_ClearSlot(u8 *self, void *, s32 i) {
    WString folder;
    UserSlots_SlotFolder(self, nullptr, &folder, i);
    String path;
    String_Ctor(&path);
    String_AssignFromWide(&path, &folder);
    g_env->removeTree(&path);
    u8 *const slot = self + 0x18 * i;
    vcall<void>(slot, 0x14, reinterpret_cast<const void *>((uintptr_t)0x0078a374));
    at<u32>(slot, 0x14) = 0;
    String_Dtor(&path);
    WString_Dtor(&folder);
}

WString *__fastcall UserSlots_ChosenFolder(const u8 *self, void *, WString *out) { return UserSlots_SlotFolder(self, nullptr, out, at<s32>(self, 0x60)); }

WString *__fastcall UserSlots_ThemeFolder(const u8 *self, void *, WString *out) {
    WString path;
    UserSlots_ChosenFolder(self, nullptr, &path);
    append(&path, wide(0x00f82508));
    String label;
    String_CtorCopy(&label, nullptr, reinterpret_cast<const StringView *>(Obj_Self(reinterpret_cast<u8 *>((uintptr_t)0x00786b68))));
    WString name;
    WString_Ctor(&name);
    WString_AssignFromAnsi(&name, &label);
    append(&path, &name);
    WString_CtorCopy(out, nullptr, &path);
    WString_Dtor(&name);
    String_Dtor(&label);
    WString_Dtor(&path);
    return out;
}

WString *__stdcall UserSlots_ThemePath(WString *out) {
    WString path;
    WString_Ctor(&path);
    u8 *const slots = UserSlots_Instance();
    WString folder;
    UserSlots_ThemeFolder(slots, nullptr, &folder);
    vcall<void>(&path, 0x18, static_cast<const WString *>(&folder));
    WString_Dtor(&folder);
    for (const u32 tail : {0x00f7d740u, 0x00f7d668u, 0x00f7cf30u})
        append(&path, wide(tail));
    WString_CtorCopy(out, nullptr, &path);
    WString_Dtor(&path);
    return out;
}

u8 *__fastcall UserSlotsPart_Ctor(u8 *self) {
    for (const u32 offset : {0u, 0x10u, 0x20u})
        StdString_Tidy(reinterpret_cast<StdString *>(self + offset), nullptr, 0);
    ListHead_CtorFrom(self + 0x30, nullptr, self + 0x30);
    StdString_Tidy(reinterpret_cast<StdString *>(self + 0x3c), nullptr, 0);
    for (const u32 offset : {0x5cu, 0x68u, 0x74u}) {
        u8 *const sentinel = static_cast<u8 *>(GameHeap_Alloc(0xc));
        at<u8 *>(sentinel, 0) = sentinel;
        at<u8 *>(sentinel, 4) = sentinel;
        at<u8 *>(self, offset) = sentinel;
        at<u32>(self, offset + 4) = 0;
    }
    at<u32>(self, 0x84) = 0;
    at<u32>(self, 0x88) = 0;
    at<u32>(self, 0x8c) = 0;
    at<u16>(self, 0x90) = 0x19;
    at<u32>(self, 0x94) = 0;
    at<u32>(self, 0x98) = 0;
    return self;
}

u8 *__cdecl UserSlots_Instance() {
    u8 *&slots = *reinterpret_cast<u8 **>((uintptr_t)0x00f81ebc);
    if (!slots) {
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x110));
        slots = block ? g_env->ctor(block) : nullptr;
    }
    return slots;
}

u32 __fastcall Records24_CountUsed(const u8 *records) {
    u32 count = 0;
    for (u32 i = 0; i < 4; i++)
        count += at<u32>(records, 4 + 24 * i) != 0 ? 1 : 0;
    return count;
}

u8 *__fastcall Records24_IfUsed(u8 *records, void *, u32 i) {
    u8 *record = records + 24 * i;
    return at<u32>(record, 4) != 0 ? record : nullptr;
}

u32 __fastcall Records24_Field14(const u8 *records, void *, u32 i) { return at<u32>(records, 24 * i + 0x14); }

}  // extern "C"
