// A list item's texts set: see window7059f0texts.h.
#include "window7059f0texts.h"

#include <cstdint>

#include "../core/wtraits.h"
#include "window7059f0.h"
#include "../core/addrcalls.h"

namespace {

template <typename T> T game(u32 address) { return GameAddress<T>(address); }

const Window7059f0TextsEnv kGameEnv = {game<u8 *(__fastcall *)(u8 *, void *, const u16 *, u32)>(0x004051d0), &Window7059f0_NthItem, &Window7059f0_RefreshRows};
const Window7059f0TextsEnv *g_env = &kGameEnv;

}  // namespace

void Window7059f0Texts_SetEnv(const Window7059f0TextsEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u8 *__fastcall WString_AssignText(u8 *string, void *, const u16 *text) { return g_env->assign(string, nullptr, text, WTraits_Length(text)); }

u8 *__fastcall WString_AssignTextOf(u8 *string, void *, const u16 *text) { return WString_AssignText(string, nullptr, text); }

void __fastcall Window7059f0_SetItemTexts(u8 *window, void *, s32 n, const u32 *texts, u32 mask, u32 refresh, u32) {
    u32 *const items = *reinterpret_cast<u32 **>(window + 0x144);
    if (!items || !texts || n >= *reinterpret_cast<s32 *>(window + 0x150)) return;
    bool any = false;
    const s32 item = g_env->nth(window, nullptr, n);
    if (item >= 0) {
        u32 *cell = *reinterpret_cast<u32 **>(window + 0x144) + item + 4;
        for (s32 column = 0; column < *reinterpret_cast<s16 *>(window + 0x164); ++column, ++cell) {
            if (!(mask & (1u << (column & 31)))) continue;
            any = true;
            if (Window7059f0_ColumnBottom(window, nullptr, column) != 0) {
                *cell = texts[column];
            } else if (texts[column] && *cell) {
                WString_AssignTextOf(reinterpret_cast<u8 *>((uintptr_t)*cell), nullptr, reinterpret_cast<const u16 *>((uintptr_t)texts[column]));
            }
        }
    }
    if (any && refresh) g_env->refresh(window);
}
}
