// The online game's list's waiting row: see dialog7cc944row.h.
#include "dialog7cc944row.h"

#include <cstdint>
#include "dialog7cc944.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T game(u32 address) { return reinterpret_cast<T>((uintptr_t)address); }

const Dialog7cc944RowEnv kGameEnv = {game<u8 *(__fastcall *)(u8 *, void *, u32)>(0x0065d7d0), game<void(__fastcall *)(u8 *, void *, u32, u32 *)>(0x00664c71), game<s32(__cdecl *)(u16 *, const u16 *, ...)>(0x0067af00),
                                     game<void(__fastcall *)(u8 *, void *, u32, const u32 *, u32, u32, s32)>(0x006644ea)};
const Dialog7cc944RowEnv *g_env = &kGameEnv;

}  // namespace

void Dialog7cc944Row_SetEnv(const Dialog7cc944RowEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" void __cdecl Dialog7cc944_SetWaitingRow(u32 value) {
    u8 *const dialog = gDialog7cc944_7cc944;
    if (!dialog) return;
    u8 *const list = g_env->findChild(dialog, nullptr, 0xe05b);
    u32 data;
    g_env->itemData(list, nullptr, 0, &data);
    if (data != 0xffffffff) return;
    u16 *const text = reinterpret_cast<u16 *>((uintptr_t)0x007ba058);
    g_env->swprintf(text, reinterpret_cast<const u16 *>((uintptr_t)0x00759160), value);
    u32 texts[6];   // (only the second is set)
    texts[1] = static_cast<u32>(reinterpret_cast<uintptr_t>(text));
    g_env->setTexts(list, nullptr, 0, texts, 2, 1, -1);
}
