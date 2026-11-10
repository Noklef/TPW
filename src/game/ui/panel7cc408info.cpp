// The people panel's facts about the shown visitor: see panel7cc408info.h.
#include "panel7cc408info.h"

#include <cstdint>

#include "../core/funnytime.h"
#include "../core/language.h"
#include "../core/methods.h"
#include "../core/vcall.h"
#include "../files/charsets.h"
#include "../world/person.h"
#include "hoverlabel.h"
#include "window.h"
#include "panel7cc218.h"
#include "../world/world.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

s32 __cdecl Format(u8 *charset, u16 *dest, u32 size, u32 entry, const u32 *arg) { return Charset_FormatWide(charset, dest, size, entry, arg); }
s32 __cdecl FormatPlain(u8 *charset, u16 *dest, u32 size, u32 entry) { return Charset_FormatWide(charset, dest, size, entry); }

const Panel7cc408InfoEnv kGameEnv = {&Format, &FormatPlain, &HoverLabel_Attach, &Visitor_TimeInPark, &FunnyTime_SpanOf, reinterpret_cast<void(__fastcall *)(u8 *, void *, u8 *, u32)>((uintptr_t)0x004e2780), &Window_Notify1, &Records_DataIfTerminated, &Language_Widen};
const Panel7cc408InfoEnv *g_env = &kGameEnv;

u16 *buffer() { return reinterpret_cast<u16 *>((uintptr_t)0x007ba058); }

void show(u8 *page, u32 id, u32 value) {
    if (u8 *const child = Window_FindChild(page, nullptr, id)) vcall<void>(child, 0x1c, value);
}

}  // namespace

void Panel7cc408Info_SetEnv(const Panel7cc408InfoEnv *env) { g_env = env ? env : &kGameEnv; }

void __fastcall Panel7cc408_ShowVisitor(u8 *self) {
    const u8 *const visitor = gPanel7cc218_7c2658;
    if (!visitor) {
        vcall<void>(self, 0x2c);
        return;
    }
    if (!at<u32>(self, 0x24)) {
        const u32 id = at<u32>(visitor, 0x1d8);
        at<u32>(self, 0x24) = id;
        if (id) {
            const u32 arg[5] = {0x006ff170, 0x5641524d, 0x18, id, 0};
            g_env->format(global<u8 *>(0x0078ba94), buffer(), 0x3fff, 0x1c8, arg);
            g_env->attach(at<u8 *>(self, 0x10), 0x3736, 0x007ba058);
        }
    }
    if (u8 *const page = at<u8 *>(self, 0x14)) {
        show(page, 0x3724, at<u32>(visitor, 0x1a0));
        if (u8 *const child = Window_FindChild(page, nullptr, 0x372d)) {
            FileTime span;
            const FileTime *const s = g_env->spanOf(gWorldPtr + 0x2a0, nullptr, &span, g_env->timeInPark(visitor) << 2);
            vcall<void>(child, 0x1c, (u32)((long long)(((unsigned long long)s->high << 32) | s->low) / 36000000000LL));   // (_alldiv: an hour of ticks)
        }
        show(page, 0x372e, at<u32>(visitor, 0x1c4));
        show(page, 0x372f, at<u32>(visitor, 0x1c8));
        show(page, 0x3730, at<u32>(visitor, 0x1cc));
        show(page, 0x3731, at<u32>(visitor, 0x1d0));
        if (u8 *const child = Window_FindChild(page, nullptr, 0x3732)) {
            const u32 happy = (u32)(long long)at<float>(visitor, 0x19c) & 0xff;   // (FLD, __ftol's low byte)
            vcall<void>(child, 0x1c, (u32)((s32)(happy << 10) / 100));
        }
        u8 *const text = Window_FindChild(page, nullptr, 0x3733);
        const u32 kind = at<u32>(visitor, 0x220);
        if (kind == 0x10) {
            u8 name[0x40];
            g_env->copyName(*reinterpret_cast<u8 **>((uintptr_t)(0x007cfb90 + 0x14 * at<u16>(visitor, 0x1dc))), nullptr, name, 0x40);
            const u32 arg[4] = {0x006ff174, 0x5641524d, 0, (u32)(uintptr_t)name};
            g_env->format(global<u8 *>(0x0078baa0), buffer(), 0x3fff, kind, arg);
        } else {
            g_env->formatPlain(global<u8 *>(0x0078baa0), buffer(), 0x3fff, kind);
        }
        vcall<void>(text, 0x24, 0x007ba058);
    }
    const u32 left = at<u32>(visitor, 0x30);
    u8 *const window = at<u8 *>(self, 0x10);
    g_env->notify1(Window_FindChild(Window_FindChild(window, nullptr, 0x3734), nullptr, 0x3735), nullptr, left ? left - 1 : 0x3e7);
    u8 *const child = Window_FindChild(window, nullptr, 0x3734);
    u8 *const text = g_env->record(global<u8 *>(0x0078ba9c), nullptr, left);
    vcall<void>(child, 0x24, (u32)(uintptr_t)g_env->widen(text));
}
