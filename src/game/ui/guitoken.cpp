#include "guitoken.h"

#include <cstdint>

#include "../core/crt.h"

namespace {

// the token's end: the first space, else the text's terminator (strchr(text, 0), which the original checks for none)
char *tokenEnd(char *text) {
    char *end = Crt().strchr(text, ' ');
    if (!end) end = Crt().strchr(text, 0);
    return end;
}

const GuiTokenEnv kGameEnv = {reinterpret_cast<s32(__cdecl *)(const char *, const char *, s32 *, s32 *, s32 *, s32 *)>((uintptr_t)0x0067f620),
                              reinterpret_cast<s32(__cdecl *)(const char *, const char *, s32 *, s32 *)>((uintptr_t)0x0067f620)};
const GuiTokenEnv *g_env = &kGameEnv;

}  // namespace

void GuiToken_SetEnv(const GuiTokenEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

char *__cdecl GuiToken_ReadCommand(char *text, u16 *out) {
    text = GuiToken_SkipBlanks(text);
    char *const end = tokenEnd(text);
    if (!end) return end;
    const char kept = *end;
    *end = 0;
    bool found = false;
    const s16 count = *reinterpret_cast<const s16 *>((uintptr_t)0x0077c598);
    for (s32 i = 0; i < count && !found; ++i) {
        const u8 *const entry = reinterpret_cast<const u8 *>((uintptr_t)(0x0077c4d0 + 8 * i));
        if (Crt().strcmp(text, *reinterpret_cast<const char *const *>(entry)) == 0) {
            if (out) *out = *reinterpret_cast<const u16 *>(entry + 4);
            found = true;
        }
    }
    *end = kept;
    return found ? end + 1 : nullptr;
}

char *__cdecl GuiToken_ReadRect(char *text, s32 *a, s32 *b, s32 *c, s32 *d) {
    char *const end = tokenEnd(text);
    if (!end) return end;
    const char kept = *end;
    *end = 0;
    if (a && b && c && d) g_env->scanFour(text, reinterpret_cast<const char *>((uintptr_t)0x0077c698), a, b, c, d);
    *end = kept;
    return end + 1;
}

char *__cdecl GuiToken_ReadPoint(char *text, s32 *a, s32 *b) {
    char *const end = tokenEnd(text);
    if (!end) return end;
    const char kept = *end;
    *end = 0;
    if (a && b) g_env->scanTwo(text, reinterpret_cast<const char *>((uintptr_t)0x0077c6a4), a, b);
    *end = kept;
    return end + 1;
}

char *__cdecl GuiToken_SkipBlanks(char *text) {
    while (*text == ' ' || *text == '\t') ++text;
    return text;
}

char *__cdecl GuiToken_ReadKind(char *text, u16 *out) {
    char *const end = tokenEnd(text);
    if (!end) return end;
    const char kept = *end;
    *end = 0;
    bool found = false;
    const s16 count = *reinterpret_cast<const s16 *>((uintptr_t)0x0077c59c);
    for (s32 i = 0; i < count && !found; ++i) {
        const u8 *const entry = reinterpret_cast<const u8 *>((uintptr_t)(0x0077c540 + 8 * i));
        if (Crt().strcmp(text, *reinterpret_cast<const char *const *>(entry)) == 0) {
            if (out) *out = *reinterpret_cast<const u16 *>(entry + 4);
            found = true;
        }
    }
    *end = kept;
    return found ? end + 1 : nullptr;
}

char *__cdecl GuiToken_ReadWord(char *text, u16 *out) {
    char *const end = tokenEnd(text);
    if (!end) return end;
    const char kept = *end;
    *end = 0;
    if (out) *out = (u16)Crt().atoi(text);
    *end = kept;
    return end + 1;
}

char *__cdecl GuiToken_ReadDword(char *text, u32 *out) {
    char *const end = tokenEnd(text);
    if (!end) return end;
    const char kept = *end;
    *end = 0;
    if (out) *out = (u32)Crt().atoi(text);
    *end = kept;
    return end + 1;
}

char *__cdecl GuiToken_ReadString(char *text, char *out, u32 count) {
    char *const end = tokenEnd(text);
    if (!end) return end;
    const char kept = *end;
    *end = 0;
    if (out) Crt().strncpy(out, text, (u32)(s32)(s16)count);
    *end = kept;
    return end + 1;
}
}
