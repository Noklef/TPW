// A part that shows the text its owner's record names: see textrecordpart.h.
#include "textrecordpart.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/language.h"
#include "../core/methods.h"
#include "window.h"
#include "window7cc8c0handler.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const TextRecordPartEnv kGameEnv = {Window_SendMessage, Records_DataIfTerminated, Language_Widen, Window_NotifyC00, Window_NotifyImpl100, Fn_669f80, Window_SetShown};
const TextRecordPartEnv *g_env = &kGameEnv;

}  // namespace

void TextRecordPart_SetEnv(const TextRecordPartEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall TextRecordPart_Refresh(u8 *part) {
    if (!at<u16>(part, 0xc) || !at<u16>(part, 4)) return;
    const s32 answer = g_env->send(at<u8 *>(part, 0x14), 0xc, 0, 0);
    if (answer == 0 || answer == -1) return;
    const u8 *const record = reinterpret_cast<const u8 *>((uintptr_t)(u32)answer);
    u16 *text = at<u16 *>(record, 4);
    if (!text) {
        const u32 index = at<u32>(record, 0);
        text = index == 0xffffffff ? reinterpret_cast<u16 *>(gWindow7cc8c0handler_78a374) : g_env->widen(g_env->record(global<u8 *>(0x0078ba98), nullptr, index));
    }
    bool newline = false;
    for (const u16 *unit = text; *unit; ++unit)
        if (*unit == 0xa) newline = true;
    u8 *const window = at<u8 *>(part, 0x18);
    g_env->notifyC00(window, nullptr, 1, newline ? 0 : 1);
    g_env->notify100(window, nullptr, (u32)(uintptr_t)text);
    g_env->mark(part);
    if (*text) g_env->setShown(window, nullptr, 1);
    else reinterpret_cast<void(__fastcall *)(u8 *)>(at<uintptr_t>(at<u8 *>(part, 0), 4))(part);
}
}
