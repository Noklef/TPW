#include "textscan.h"

namespace {

bool alpha(s8 c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }
bool digit(s8 c) { return c >= '0' && c <= '9'; }
bool punct(s8 c) {
    return (c >= '!' && c <= '/') || (c >= ':' && c <= '@') || (c >= '[' && c <= '`') || (c >= '{' && c <= '~');
}
bool mark(s8 c) { return c == '#' || c == ';'; }

}  // namespace

extern "C" {

u32 __cdecl TextScan_Step(TextScan *scan) {
    if (scan->pos >= scan->length) return 1;
    ++scan->next;
    ++scan->pos;
    return 0;
}

u32 __cdecl TextScan_SkipBlanks(TextScan *scan) {
    for (;;) {
        const s8 c = *scan->next;
        if (alpha(c) || digit(c) || punct(c) || mark(c)) return 0;
        if (TextScan_Step(scan)) return 1;
    }
}

u32 __cdecl TextScan_IsAlpha(const char *const *at) { return alpha(**at) ? 1 : 0; }

u32 __cdecl TextScan_IsDigit(const char *const *at) { return digit(**at) ? 1 : 0; }

u32 __cdecl TextScan_IsPunct(const char *const *at) { return punct(**at) ? 1 : 0; }

u32 __cdecl TextScan_IsBlank(const char *const *at) {
    const s8 c = **at;
    return alpha(c) || digit(c) || punct(c) || mark(c) ? 0 : 1;
}

u32 __cdecl TextScan_Read(TextScan *scan, char *out, s32 size, u32 letters, u32 digits, u32 puncts, u32 marks) {
    *out = 0;
    if (scan->pos >= scan->length) return 1;
    const s32 most = static_cast<s32>(static_cast<u32>(size) - 1);
    for (s32 taken = 0; taken < most;) {
        const s8 c = *scan->next;
        if (!((letters && alpha(c)) || (digits && digit(c)) || (puncts && punct(c)) || (marks && mark(c)))) return 0;
        *out++ = c;
        ++taken;
        *out = 0;
        if (scan->pos >= scan->length) return 1;
        ++scan->next;
        ++scan->pos;
    }
    return 0;
}
}
