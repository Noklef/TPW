#include "intpair.h"

extern "C" {

u32 *__fastcall IntPair_Copy(u32 *pair, void *, const u32 *from) {
    pair[0] = from[0];
    pair[1] = from[1];
    return pair;
}

u32 *__fastcall IntPair_Scaled(const u32 *pair, void *, u32 *out, u32 scale) {
    const u32 first = pair[0] * scale, second = pair[1] * scale;
    out[0] = first;
    out[1] = second;
    return out;
}
}
