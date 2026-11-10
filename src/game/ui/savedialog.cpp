// The save and load dialogs: see savedialog.h.
#include "savedialog.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/filetime.h"
#include "../core/keymap.h"
#include "../core/string.h"
#include "../core/vcall.h"
#include "../files/charsets.h"
#include "dialogbase.h"
#include "focus.h"
#include "window.h"
#include "window7059f0.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T game(u32 address) { return reinterpret_cast<T>((uintptr_t)address); }

const SaveDialogEnv kGameEnv = {game<u8 *(__fastcall *)(u8 *)>(0x005ac8f0), game<void(__fastcall *)(u8 *, void *, const WString *)>(0x005ac610), game<void(__fastcall *)(u8 *, void *, u8 *)>(0x005ac5d0), game<void(__fastcall *)(u8 *)>(0x00409300), &Window_SendMessage, &Window_PostMessage,
                                &Focus_Restore, &KeyMaps_EnableAllAndClearFlag, &Dialog_BaseHandler, &Window7059f0_NthItemData, &Charset_FormatWide, game<void(__cdecl *)(const u8 *, u32, u32, u32, u32, u32, u32, u32)>(0x0047f020), &Window7059f0_AddItem};
const SaveDialogEnv *g_env = &kGameEnv;

u8 *list() { return reinterpret_cast<u8 *>((uintptr_t)0x00f7b560); }
u16 *typed() { return reinterpret_cast<u16 *>(gSavedialog_7ca720); }
u8 *next(u8 *item) { return WNamedList_FindAfter(list(), nullptr, item); }
void close(u32 dialog) { g_env->send(global<u8 *>(dialog), 4, 0, 0); }

const u32 kLoadDialog = reinterpret_cast<uintptr_t>(&gSavedialog_7cb244);
const u32 kSaveDialog = reinterpret_cast<uintptr_t>(&gSavedialog_7cb248);
const u32 kArgTag = 0x5641524d;

struct FormatArg {
    u32 vtable;
    u32 tag;
    u32 number;
    u32 value;
    u32 high;
};

// The two fillers differ only in where their arguments lie.
void Fill(u8 *window, u32 dayAt, u32 monthAt, u32 yearAt) {
    FileTime time = {0, 0};
    u32 index = 0;
    for (u8 *item = g_env->first(list()); item; item = next(item)) {
        u32 texts[2];
        texts[0] = *reinterpret_cast<u32 *>(item + 8);
        FileTime local;
        (*game<s32(__stdcall *const *)(const FileTime *, FileTime *)>(0x006fd15c))(reinterpret_cast<const FileTime *>(item + 0x14), &local);
        FileTime_Assign(&time, nullptr, &local);
        s32 status;
        u32 year, month, day, hour, minute;
        FileTime_ToSystemTime(&time, nullptr, &status, &year, &month, &day, nullptr, &hour, &minute, nullptr, nullptr);
        FormatArg args[3];
        args[yearAt] = {0x006ff170, kArgTag, 0x12, year, 0};
        args[monthAt] = {0x006ff170, kArgTag, 0x11, month, 0};
        args[dayAt] = {0x006ff170, kArgTag, 0x10, day, 0};
        u16 text[0x20];
        g_env->format(global<u8 *>(0x0078ba94), text, 0x20, 0x1c0, &args[dayAt], &args[monthAt], &args[yearAt]);
        u16 *const row = reinterpret_cast<u16 *>((uintptr_t)0x007ba058);
        Crt().swprintf(row, reinterpret_cast<const u16 *>((uintptr_t)0x00752564), text, hour, minute);
        texts[1] = static_cast<u32>(reinterpret_cast<uintptr_t>(row));
        g_env->addItem(window, nullptr, texts, index++, -1, 1);
    }
}

void AskFirst() {
    struct {
        u32 vtable;
        u32 tag;
        u32 number;
        const u16 *text;
    } arg = {0x006ff174, kArgTag, 2, typed()};
    u16 text[0x80];
    g_env->format(global<u8 *>(0x0078ba94), text, 0x7f, 0xcd, &arg);
    struct {
        u16 *text;
        u8 colour[4];
        u32 kind;
    } record;
    record.text = text;
    record.colour[0] = record.colour[1] = record.colour[2] = 0xff;
    record.kind = 4;
    g_env->box(reinterpret_cast<const u8 *>(&record), 2, 0, 1, 0x0049e9b0, 0, 0, 1);
}

// 0x14's common end.
void EnableControls() {
    global<u32>(0x00786ba4) = 1;
    g_env->enable(reinterpret_cast<u8 *>((uintptr_t)0x00786b68));
}

}  // namespace

void SaveDialog_SetEnv(const SaveDialogEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __cdecl LoadDialog_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 5) {
        close(kLoadDialog);
        return 1;
    }
    if (message == 0x14) {
        global<u32>(kLoadDialog) = 0;
        g_env->restore();
        EnableControls();
    } else if (message == 0x100) {
        if (a == 0xfffffffe) {
            close(kLoadDialog);
            return 0;
        }
    } else if (message == 0x400) {
        u8 *const child = Window_FindChild(window, nullptr, a);
        if (child && b != 0xffffffff) {
            u32 row = message;
            g_env->nth(child, nullptr, b, &row);
            u8 *item = g_env->first(list());
            for (s32 k = 0; k < static_cast<s32>(row); ++k) item = next(item);
            g_env->load(list(), nullptr, item);
            close(kLoadDialog);
            return 0;
        }
    }
    return g_env->base(window, message, a, b);
}

u32 __cdecl SaveDialog_Save(u32) {
    WString name;
    WString_CtorStr(&name, nullptr, typed());
    g_env->save(list(), nullptr, &name);
    close(kSaveDialog);
    WString_Dtor(&name);
    return 1;
}

s32 __cdecl SaveDialog_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 5) {
        close(kSaveDialog);
        return 1;
    }
    if (message == 0x14) {
        global<u32>(kSaveDialog) = 0;
        g_env->keymaps();
        g_env->restore();
        EnableControls();
    } else if (message == 0x100) {
        if (a == 0xfffffffe) {
            close(kSaveDialog);
            return 0;
        }
        if (a == 0xffffffff) {
            u8 *item = g_env->first(list());
            while (item && Crt().wcscmp(*reinterpret_cast<const u16 **>(item + 8), typed()) != 0) item = next(item);
            if (!item) {
                SaveDialog_Save(0);
                return 0;
            }
            AskFirst();
        }
    } else if (message == 0x400) {
        u8 *const child = Window_FindChild(window, nullptr, a);
        if (child && b != 0xffffffff) {
            u32 row;
            g_env->nth(child, nullptr, b, &row);
            u8 *item = g_env->first(list());
            for (; row; --row) item = next(item);
            const u32 name = *reinterpret_cast<u32 *>(item + 8);
            vcall<void>(Window_FindChild(window, nullptr, 0x23bce2), 0x24, name);
        }
    } else if (message == 0x802) {
        g_env->post(window, 0x100, 0xffffffff, 0);
    } else if (message == 0x804) {
        g_env->post(window, 0x100, 0xfffffffe, 0);
    }
    return g_env->base(window, message, a, b);
}

void __cdecl LoadDialog_FillList(u8 *window) { Fill(window, 2, 1, 0); }

void __cdecl SaveDialog_FillList(u8 *window) {
    WString unused;
    WString_Ctor(&unused);
    Fill(window, 1, 0, 2);
    WString_Dtor(&unused);
}
}
