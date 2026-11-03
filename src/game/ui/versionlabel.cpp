// The window with the version text: see versionlabel.h.
#include "versionlabel.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../gfx/guitext.h"
#include "../render/textparts.h"
#include "window.h"
#include "windowfactory.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const VersionLabelEnv kGameEnv = {&GuiText_Measure, &Window_CreateChild, &TextPart_Ctor, &Window_SetImpl, &Window_Set4c, &Window_NotifyImpl100};
const VersionLabelEnv *g_env = &kGameEnv;

}  // namespace

void VersionLabel_SetEnv(const VersionLabelEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl VersionLabel_Create(u8 front) {
    u8 *const parent = front ? global<u8 *>(0x00faa5d8) : *reinterpret_cast<u8 **>(global<u8 *>(0x00f82884) + 4);
    u16 text[8];
    Crt().swprintf(text, reinterpret_cast<const u16 *>((uintptr_t)0x00750080), 2, 0);
    const s32 rect[4] = {0, 0, 0x7d00, 0x7d00};
    s32 box[4] = {0, 0, 0, 0};
    g_env->measure(text, rect, 6, 0, 0, 0, box);
    const s32 width = static_cast<s32>(static_cast<u32>(box[2] - box[0]) << 0xb) / global<s16>(0x00faa5c4);
    const s32 height = static_cast<s32>(static_cast<u32>(box[3] - box[1]) * 3 << 9) / global<s16>(0x00faa5c0);
    u8 *const window = g_env->create(parent, nullptr, 1, 3, 0x008cfd55, 0x7c0 - width, 0x5c0 - height, 0x7c0, 0x5c0);
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    g_env->setImpl(window, nullptr, memory ? g_env->part(memory, nullptr, 0) : nullptr);
    g_env->set4c(window, nullptr, reinterpret_cast<u8 *>((uintptr_t)(0x0078b5dc + global<u32>(0x0078b534) * 0x138)));
    g_env->notify(window, nullptr, static_cast<u32>(reinterpret_cast<uintptr_t>(text)));
}
}
