// A GUI text token read as a wide string: see guitokenwide.h.
#include "guitokenwide.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/addrcalls.h"

namespace {
const GuiTokenWideEnv kGameEnv = {GameAddress<u32(__cdecl *)(const char *)>(0x0067ff70),
                                  GameAddress<s32(__cdecl *)(u16 *, const u16 *, const char *)>(0x0067af00)};
const GuiTokenWideEnv *g_env = &kGameEnv;
}  // namespace

void GuiTokenWide_SetEnv(const GuiTokenWideEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

char *__cdecl GuiToken_ReadWide(char *text, u16 *out, u32 count) {
    char *end = Crt().strchr(text, ' ');
    if (!end) end = Crt().strchr(text, 0);
    if (!end) return end;
    const char kept = *end;
    *end = 0;
    if (out && g_env->strlen(text) < static_cast<u32>(static_cast<s32>(static_cast<s16>(count)) * 2)) {
        g_env->swprintf(out, reinterpret_cast<const u16 *>((uintptr_t)&gGuiToken_77c690), text);
    }
    *end = kept;
    return end + 1;
}
}
