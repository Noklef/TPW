#include "wtext.h"

namespace {

bool separator(u16 c) { return c == 0 || c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

}  // namespace

extern "C" {

u32 __cdecl WText_WordEquals(const u16 *a, const u16 *b) {
    for (;; ++a, ++b) {
        const u16 x = *a, y = *b;
        if (separator(x) && separator(y)) return 1;
        if (x == 0 || y == 0 || x != y) return 0;
    }
}

const u16 *__cdecl WText_NextLine(const u16 *p) {
    for (;;) {
        // To the end of this line.
        u16 c = *p;
        while (c != 0 && c != '\n' && c != '\r') c = *++p;
        if (c == 0) return p;
        p += (c == '\n' && p[1] == '\r') || (c == '\r' && p[1] == '\n') ? 2 : 1;
        // A line of blanks only (to a line end or the terminator) is passed over.
        const u16 *q = p;
        c = *q;
        while (c == ' ' || c == '\t') c = *++q;
        if (c != 0 && c != '\n' && c != '\r') return p;
    }
}
}
