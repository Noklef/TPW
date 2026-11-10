// The dialog [0x7cc984] opened: see dialog7cc984open.h.
#include "dialog7cc984open.h"

#include <cstdint>

#include "../core/buffers.h"
#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/globalsave.h"
#include "../core/language.h"
#include "../core/namedobjects.h"
#include "../core/textarg.h"
#include "../core/vcall.h"
#include "../core/wtraits.h"
#include "../frontend/scenequeue.h"
#include "../online/itemlist.h"
#include "../online/items.h"
#include "../online/session.h"
#include "../render/textparts.h"
#include "guidepth.h"
#include "hoverlabel.h"
#include "window.h"
#include "windowscript.h"
#include "../online/chatcommand.h"
#include "../coaster/arreplayaction.h"
#include "dialog7cc984.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T game(u32 address) { return reinterpret_cast<T>((uintptr_t)address); }
template <typename T> T at(const u8 *object, u32 offset) { return *reinterpret_cast<const T *>(object + offset); }

const Dialog7cc984OpenEnv kGameEnv = {Class70293c_Assign, Gui_TopDepth, WindowScript_Build, Window_SetDepth, Window_Offset, game<u8 *(__fastcall *)(u8 *)>(0x005b50b0), Owner2c_FindByKey, Owner2c_ValueByKey, NamedObjects_SetThemeLabelAnsi,
                                      Language_WideText, TextPart_Ctor, Window_SetImpl, Language_Font, Window_Set4c, Window_NotifyImpl100, Window_SetShown, GlobalSave_Construct, GlobalSave_Byte25,
                                      game<void(__fastcall *)(u8 *, void *, StdWString *, u32)>(0x005d3230), Window_Notify3000, Window_NotifyC00, TextPart6ff530_Ctor, HoverLabel_SetYellow, Window_SetFocus, Class702da0_Push};
const Dialog7cc984OpenEnv *g_env = &kGameEnv;

const u16 *wide(const wchar_t *text) { return reinterpret_cast<const u16 *>(text); }

u8 *dialog() { return gDialog7cc984_7cc984; }

// The session's holder, made when none (inlined each time).
u8 *holder() {
    u8 *&made = gChatcommand_f7d88c;
    if (!made) {
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x404));
        made = block ? g_env->holderCtor(block) : nullptr;
    }
    return made;
}

// The global save info, made when none (inlined each time).
u8 *save() {
    u8 *&made = gArreplayaction_f7c58c;
    if (!made) {
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x50));
        made = block ? g_env->saveCtor(block) : nullptr;
    }
    return made;
}

// A text part of its kind on the window (null when out of memory), the font and the text.
void dress(u8 *row, u32 text) {
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    g_env->setImpl(row, nullptr, block ? g_env->partCtor(block, nullptr, 0) : nullptr);
    g_env->set4c(row, nullptr, g_env->font(6));
    g_env->notify100(row, nullptr, text);
}

void dressText(u8 *row, const WString &text) { dress(row, (u32)(uintptr_t)text.data); }

// The theme's label of the key [0x7cc9d4] into label.
void themeLabel(WString *label) {
    const u32 key = global<u32>(0x007cc9d4);
    String theme;
    String_CtorStr(&theme, nullptr, reinterpret_cast<const char *>((uintptr_t)g_env->valueByKey(at<u8 *>(holder(), 0xf8), nullptr, key)));
    g_env->setTheme(&global<u8>(0x00786b68), nullptr, &theme, reinterpret_cast<u8 *>(label));
    String_Dtor(&theme);
}

// The string assigned from the wide text.
void assign(StdWString *s, const u16 *text) {
    const u32 length = Crt().wcslen(text);
    if (StdWString_Grow(s, nullptr, length, 1)) {
        WTraits_Copy(s->ptr, text, length);
        ImeWString_SetLength(reinterpret_cast<u8 *>(s), nullptr, length);
    }
}

void censorIfSet(StdWString *s) {
    if (g_env->byte25(save())) g_env->censor(save(), nullptr, s, 1);
}

const u16 *shown(const StdWString &s) { return s.ptr ? s.ptr : reinterpret_cast<const u16 *>((uintptr_t)0x006fd6c4); }

void plainRows(u8 *box, u8 *row, WString *text) {
    const u16 *const name = global<const u16 *>(0x007cc9c4);
    WString_Format(text, wide(L"%s : %s"), g_env->text(0x113), name);
    const u16 *const value = global<const u16 *>(0x007cc9c4);
    const u16 *const own = reinterpret_cast<const u16 *>(Online_String14(holder()));
    g_env->setShown(Window_FindChild(dialog(), nullptr, 0xe592f), nullptr, Crt().wcsicmp(own, value) == 0 ? 1 : 0);
    if (g_env->byte25(save())) {   // a copy censored and thrown away, sic
        const u16 *const data = text->data;
        StdWString copy;
        CountedBuf_Reset(reinterpret_cast<u8 *>(&copy), nullptr, 0);
        assign(&copy, data);
        g_env->censor(save(), nullptr, &copy, 1);
        CountedBuf_Reset(reinterpret_cast<u8 *>(&copy), nullptr, 1);
    }
    dressText(row, *text);

    row = Window_FindChild(box, nullptr, 0xe5934);
    WString_AssignStr(text, nullptr, g_env->text(0x115));
    WString_AppendStr(text, nullptr, wide(L" : "));
    // (TextArgInt_Ctor (n, the word, unsigned), inline)
    const u32 first[5] = {0x006ff170, 0x5641524d, 0x12, global<u16>(0x007cca34), 0};
    const u32 second[5] = {0x006ff170, 0x5641524d, 0x11, global<u16>(0x007cca32), 0};
    const u32 third[5] = {0x006ff170, 0x5641524d, 0x10, global<u16>(0x007cca30), 0};
    u16 *const buffer = reinterpret_cast<u16 *>((uintptr_t)0x007ba058);
    TextFormat().format(global<void *>(0x0078ba94), buffer, 0x3fff, 0x1c0, third, second, first);
    WString_AppendStr(text, nullptr, buffer);
    dressText(row, *text);

    row = Window_FindChild(box, nullptr, 0xe5936);
    const u16 *address = global<const u16 *>(0x007cca24);
    WString_AssignStr(text, nullptr, g_env->text(0x114));
    if (!Crt().wcslen(address)) address = g_env->text(0x11c);
    WString_AppendStr(text, nullptr, wide(L" : "));
    WString_AppendStr(text, nullptr, address);
    dressText(row, *text);

    row = Window_FindChild(box, nullptr, 0xe5933);
    WString_Format(text, wide(L"%s : %d"), g_env->text(0x117), global<u32>(0x007cc9d8));
    dressText(row, *text);

    row = Window_FindChild(box, nullptr, 0xe5938);
    WString_Format(text, wide(L"%s : %d"), g_env->text(0x118), global<u32>(0x007cc9dc));
    dressText(row, *text);

    row = Window_FindChild(box, nullptr, 0xe5939);
    WString label;
    WString_Ctor(&label);
    themeLabel(&label);
    WString_Format(text, wide(L"%s : %s"), g_env->text(0x110), label.data);
    dressText(row, *text);

    row = Window_FindChild(box, nullptr, 0xe593a);
    if (global<u32>(0x007cca40) != 0x7fffffff) {
        WString_Format(text, wide(L"%s : %d"), g_env->text(0x111), global<u32>(0x007cca40));
    } else {
        WString_Format(text, wide(L"%s : -"), g_env->text(0x111));
    }
    dressText(row, *text);

    row = Window_FindChild(box, nullptr, 0xe5935);
    WString_Format(text, wide(L"%s : "), g_env->text(0x11b));
    WString_AppendStr(text, nullptr, g_env->text(global<u32>(0x007cca44) ? 0x14d : 0x14c));
    dressText(row, *text);
    WString_Dtor(&label);
}

}  // namespace

void Dialog7cc984Open_SetEnv(const Dialog7cc984OpenEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl Dialog7cc984_Open(const u8 *record, u8 *parent, u32 callback, u32 a, u32 b) {
    if (dialog()) return;
    g_env->assign(&global<u8>(0x007cc988), nullptr, record);
    global<u32>(0x007cca48) = callback;
    global<u32>(0x007cca4c) = a;
    global<u32>(0x007cca50) = b;
    u32 down = 0xc8;
    if (!parent) {
        parent = at<u8 *>(global<u8 *>(0x00f82884), 4);
        down = 0;
    }
    const s32 depth = g_env->topDepth(nullptr, nullptr);
    gDialog7cc984_7cc984 = g_env->build(parent, nullptr, game<u8 *>(0x00759170), 0x004c0880);
    g_env->setDepth(dialog(), nullptr, depth + 4);
    g_env->offset(dialog(), nullptr, 0, down);
    u8 *const box = Window_FindChild(dialog(), nullptr, 0xe5932);
    WString text;
    WString_Ctor(&text);
    const u32 type = global<u32>(0x007cc9d0);
    const u8 *const known = g_env->findByKey(at<u8 *>(holder(), 0xe0), nullptr, type);
    const u32 isKnown = at<u32>(known, 0x60);
    u8 *const row = Window_FindChild(box, nullptr, 0xe5937);
    if (isKnown) {
        WString label;
        WString_Ctor(&label);
        themeLabel(&label);
        WString_Format(&text, wide(L"%s : %s"), g_env->text(0x110), label.data);
        dressText(row, text);
        g_env->setShown(Window_FindChild(dialog(), nullptr, 0xe592f), nullptr, 0);
        WString_Dtor(&label);
    } else {
        plainRows(box, row, &text);
    }

    u8 *const title = Window_FindChild(dialog(), nullptr, 0xe5930);
    dress(title, 0);
    g_env->notify3000(title, nullptr, 1, 0);
    g_env->notifyC00(title, nullptr, 0, 0);
    StdWString name;
    name.ptr = nullptr;
    name.len = 0;
    name.res = 0;
    assign(&name, global<const u16 *>(0x007cc9b0));
    censorIfSet(&name);
    vcall<void>(title, 0x24, shown(name));
    assign(&name, global<const u16 *>(0x007cc99c));
    censorIfSet(&name);
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    u8 *const part = block ? g_env->part2Ctor(block, nullptr, 0) : nullptr;
    g_env->setImpl(Window_FindChild(dialog(), nullptr, 0xe5931), nullptr, part);
    u8 *const font = g_env->font(5);
    g_env->set4c(Window_FindChild(dialog(), nullptr, 0xe5931), nullptr, font);
    g_env->notifyC00(Window_FindChild(dialog(), nullptr, 0xe5931), nullptr, 1, 1);
    const u16 *const owner = shown(name);
    vcall<void>(Window_FindChild(dialog(), nullptr, 0xe5931), 0x24, owner);
    g_env->hover(Window_FindChild(dialog(), nullptr, 0xe5931));
    g_env->setFocus(dialog(), nullptr, 0);
    g_env->push(at<u8 *>(global<u8 *>(0x00f85614), 0x14), nullptr, 0x251, 1);
    if (name.ptr) {  // _Tidy(true), inlined
        u8 &refs = *(reinterpret_cast<u8 *>(name.ptr) - 1);
        if (refs != 0 && refs != 0xff) {
            --refs;
        } else {
            GameHeap_Free(reinterpret_cast<u8 *>(name.ptr) - 2);
        }
    }
    WString_Dtor(&text);
}
}
