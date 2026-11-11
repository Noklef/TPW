// The online dialog's list of dial-up connections: see dialuplist.h.
#include "dialuplist.h"

#include <cstdint>

#include "../core/language.h"
#include "window.h"
#include "window7059f0.h"
#include "dialog7cc654handler.h"
#include "guistartup.h"
#include "dialog7cc530open.h"
#include "../core/imports.h"
#include "../core/addrcalls.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

void *dialup() { return reinterpret_cast<void *>((uintptr_t)0x007c24c0); }

u32 __fastcall Count(void *object) { return global<u32(__fastcall *)(void *)>(IMPORT(Wearasr_Dialup_GetNoDialUpConnections))(object); }
u32 __fastcall Properties(void *object, void *, u32 index, void *out) { return global<u32(__fastcall *)(void *, void *, u32, void *)>(IMPORT(Wearasr_Dialup_GetDialupProperties))(object, nullptr, index, out); }
void __fastcall SetShown(u8 *window, void *, u32 shown) { Window_SetShown(window, nullptr, static_cast<u16>(shown)); }

const DialupListEnv kGameEnv = {&Count, &Properties, &Window7059f0_Clear, &SetShown, &Window7059f0_AddItem, &Window7059f0_RefreshRows, &Language_WideText, GameAddress<u16 *(__cdecl *)(u16 *, const u16 *)>(0x0067c430)};
const DialupListEnv *g_env = &kGameEnv;

}  // namespace

void DialupList_SetEnv(const DialupListEnv *env) { g_env = env ? env : &kGameEnv; }

void __cdecl DialupList_Fill(u8 shown) {
    u8 *const list = Window_FindChild(gDialog7cc530open_7cc530, nullptr, 0xc);
    g_env->clear(list);
    if (!shown) {
        g_env->setShown(list, nullptr, 0);
        return;
    }
    g_env->setShown(list, nullptr, 1);
    u32 message = 0xffffffff;
    if (!gGuistartup_7c24d4) {
        message = 0xe7;
    } else if (const s32 count = g_env->count(dialup())) {
        for (s32 i = 0; i < count; ++i) {
            u8 properties[0x820];
            g_env->properties(dialup(), nullptr, i, properties);
            if (properties[0xc] & 1) {
                const u32 texts[2] = {(u32)(uintptr_t)(properties + 0x10), *reinterpret_cast<u32 *>(properties)};
                g_env->add(list, nullptr, texts, i, -1, 0);
            }
        }
    } else {
        message = 0xe6;
    }
    if (message != 0xffffffff) {
        g_env->wideText(message);
        const u16 *const separators = reinterpret_cast<const u16 *>(gDialog7cc654handler_7558f0);
        u16 *word = g_env->token(reinterpret_cast<u16 *>((uintptr_t)0x007ba058), separators);
        while (word) {
            const u32 texts[2] = {(u32)(uintptr_t)word, 0};
            g_env->add(list, nullptr, texts, 0xffffffff, -1, 0);
            word = g_env->token(nullptr, separators);
        }
    }
    g_env->refresh(list);
}
