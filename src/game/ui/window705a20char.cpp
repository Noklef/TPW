#include "window705a20char.h"

#include <cstdint>

#include "../core/crt.h"
#include "window.h"
#include "window705a20.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const Window705a20CharEnv kGameEnv = {&Window705a20_NotifyText, &Window705a20_SetSelection, &Window_SendMessage, &Window705a20_TypeChar};
const Window705a20CharEnv *g_env = &kGameEnv;

// The selection taken out of the text (signed loops, unlike the typing's); its start.
s32 DeleteSelection(u8 *window, s32 length) {
    u16 *const buffer = at<u16 *>(window, 0x138);
    const s32 cursor = at<s16>(window, 0x14a);
    const s32 anchor = at<s16>(window, 0x14c);
    s32 low, high;
    if (cursor > anchor) {
        low = anchor;
        high = cursor;
    } else {
        high = anchor;
        low = cursor;
    }
    const s32 gap = high - low;
    s32 i = low;
    for (; i < length - gap; ++i) buffer[i] = buffer[i + gap];
    for (; i < at<s32>(window, 0x13c); ++i) buffer[i] = 0;
    return low;
}

void Move(u8 *window, u32 code, u32 mods) {
    if (mods == 3)
        g_env->setSelection(window, nullptr, static_cast<u32>(-1), 1, code);
    else
        g_env->setSelection(window, nullptr, code, 1, static_cast<u32>(-1));
}

void ToParent(u8 *window, s32 key) { g_env->send(at<u8 *>(window, 0x60), 0x805, at<u32>(window, 0x8c), static_cast<u32>(key)); }

}  // namespace

extern "C" {

void Window705a20Char_SetEnv(const Window705a20CharEnv *env) { g_env = env ? env : &kGameEnv; }

const u16 *__cdecl CharList_Find(const u16 *list, u32 ch) {
    const u16 wanted = static_cast<u16>(ch);
    while (*list && *list != wanted) ++list;
    return *list == wanted ? list : nullptr;
}

void __fastcall Window705a20_TypeChar(u8 *window, void *, u32 ch) {
    u16 *const buffer = at<u16 *>(window, 0x138);
    if (!buffer) return;
    const u32 length = Crt().wcslen(buffer);
    s32 place = -1;
    const u16 typed = static_cast<u16>(ch);
    if (typed == 0xd || typed == 9 || typed == 8 || typed == 0x1b) return;
    if (at<const u16 *>(window, 0x140) && !CharList_Find(at<const u16 *>(window, 0x140), typed)) return;
    if (at<const u16 *>(window, 0x144) && CharList_Find(at<const u16 *>(window, 0x144), typed)) return;
    const s32 cursor = at<s16>(window, 0x14a);
    const s32 anchor = at<s16>(window, 0x14c);
    const u32 capacity = at<u32>(window, 0x13c);
    if (anchor != -1) {
        s32 high;
        if (cursor > anchor) {
            place = anchor;
            high = cursor;
        } else {
            high = anchor;
            place = cursor;
        }
        const s32 gap = high - place - 1;
        s32 i = place + 1;
        for (; i < static_cast<s32>(length - gap); ++i) buffer[i] = buffer[i + gap];
        for (; static_cast<u32>(i) < capacity; ++i) buffer[i] = 0;
    } else if (length < capacity) {
        for (u32 i = length; i > static_cast<u32>(cursor); --i) buffer[i] = buffer[i - 1];
        place = cursor;
    }
    if (place < 0) return;
    buffer[place] = typed;
    g_env->notifyText(window, nullptr, 1);
    g_env->setSelection(window, nullptr, place + 1, 1, static_cast<u32>(-1));
}

void __fastcall Window705a20_KeyDown(u8 *window, void *, u32 keyArg, u32 b) {
    const Window705a20CharEnv *const E = g_env;
    u16 *const buffer = at<u16 *>(window, 0x138);
    if (!buffer) return;
    const s32 key = static_cast<s16>(keyArg);
    const u32 mods = (b >> 16) & 0xff;
    const bool multiline = (at<u32>(window, 0x48) & 0x80) != 0;
    const u32 none = static_cast<u32>(-1);
    switch (key) {
        case 0x2700:
            Move(window, static_cast<u32>(*reinterpret_cast<u8 *>((uintptr_t)0x00faa56c) ? -2 : -3), mods);
            return;
        case 0x2500:
            Move(window, static_cast<u32>(*reinterpret_cast<u8 *>((uintptr_t)0x00faa56c) ? -3 : -2), mods);
            return;
        case 0x2300:
            Move(window, static_cast<u32>(-5), mods);
            return;
        case 0x2400:
            Move(window, static_cast<u32>(-4), mods);
            return;
        case 0x2600:
            if (multiline)
                E->setSelection(window, nullptr, static_cast<u32>(-6), 1, none);
            else
                ToParent(window, key);
            return;
        case 0x2800:
            if (multiline)
                E->setSelection(window, nullptr, static_cast<u32>(-7), 1, none);
            else
                ToParent(window, key);
            return;
        case 0x2e00: {
            const s32 length = static_cast<s32>(Crt().wcslen(buffer));
            const s32 cursor = at<s16>(window, 0x14a);
            if (at<s16>(window, 0x14c) != -1) {
                DeleteSelection(window, length);
                E->setSelection(window, nullptr, static_cast<u32>(cursor), 1, none);
            } else if (cursor < length) {
                for (s32 i = cursor; i < length; ++i) buffer[i] = buffer[i + 1];
            }
            E->notifyText(window, nullptr, 1);
            return;
        }
        case 8: {
            const s32 length = static_cast<s32>(Crt().wcslen(buffer));
            const s32 cursor = at<s16>(window, 0x14a);
            if (at<s16>(window, 0x14c) != -1) {
                const s32 low = DeleteSelection(window, length);
                E->notifyText(window, nullptr, 1);
                E->setSelection(window, nullptr, static_cast<u32>(low), 1, none);
            } else if (cursor > 0) {
                for (s32 i = cursor - 1; i < length; ++i) buffer[i] = buffer[i + 1];
                E->notifyText(window, nullptr, 1);
                E->setSelection(window, nullptr, static_cast<u32>(-2), 1, none);
            }
            return;
        }
        case 0xd:
            if (multiline) E->typeChar(window, nullptr, 0xa);
            return;
        default:
            ToParent(window, key);
            return;
    }
}
}
