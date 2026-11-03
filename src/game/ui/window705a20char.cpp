#include "window705a20char.h"

#include <cstdint>

#include "../core/crt.h"
#include "window705a20.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const Window705a20CharEnv kGameEnv = {&Window705a20_NotifyText, &Window705a20_SetSelection};
const Window705a20CharEnv *g_env = &kGameEnv;

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
}
