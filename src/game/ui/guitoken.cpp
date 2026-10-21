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

}  // namespace

extern "C" {

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
