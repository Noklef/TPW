#include "small40.h"

#include <cstdint>

#include "vcall.h"

extern "C" {

u32 __fastcall TextMetrics_Width(u8 *object, void *, const u16 *text, u32 start, u32 end) {
    u32 eax = (u32)(s32)(s16)start;
    s32 remaining = (s32)(s16)end - (s32)(s16)start;
    const u16 *p = text + (s32)(s16)start;
    s32 sum = 0;
    s32 i = 0;
    if (remaining > 0) {
        do {
            const u16 c = *p;
            eax = (eax & 0xffff0000u) | c;
            if (c == 0) break;
            const u32 result = vcall<u32>(object, 0xc, eax);
            ++p;
            eax = (u32)(s32)(s16)result;
            sum += (s32)eax;
            ++i;
        } while (i < remaining);
    }
    return (eax & 0xffff0000u) | (u16)sum;
}

s32 __fastcall TextMetrics_FitIndex(u8 *object, void *, const u16 *text, u32 start, u32 width) {
    u32 eax = (u32)(uintptr_t)text;
    s32 index = (s32)(s16)start;
    const u16 *p = text + index;
    u32 left = width;
    if ((s16)left > 0) {
        do {
            const u16 c = *p;
            eax = (eax & 0xffff0000u) | c;
            if (c == 0) break;
            eax = vcall<u32>(object, 0xc, eax);
            left -= eax;
            ++index;
            ++p;
        } while ((s16)left > 0);
    }
    return index - 1;
}

}  // extern "C"
