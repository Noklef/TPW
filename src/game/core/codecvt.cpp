#include "codecvt.h"

extern "C" {

u32 __stdcall Span_Limited(u32, u32 from, u32 to, u32 limit) {
    const u32 span = to - from;
    return limit < span ? limit : span;
}

}  // extern "C"
