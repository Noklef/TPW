#include "dirstep.h"

#include <cstdint>

#include "../core/accessors.h"

extern "C" {

u32 __cdecl Position_StepA(s32 *x, s32 *y, s32 direction) {
    switch (direction) {
    case 0:
        *x += 4;
        *y += 1;
        return 4;
    case 90:
        *x += 1;
        *y -= 5;
        return 1;
    case 180:
        *x -= 5;
        *y -= 2;
        return 0x40;
    case 270:
        *x -= 2;
        *y += 4;
        return 0x10;
    }
    return (u32)(uintptr_t)x;  // sic: the original reloads its first argument here, not the direction
}

u32 __cdecl Position_StepB(s32 *x, s32 *y, s32 direction) {
    u32 code = (u32)(uintptr_t)x;  // sic: as above, the first argument (the pointer) stands in for the direction
    switch (direction) {
    case 0:
        *x -= 2;
        *y += 1;
        code = 4;
        break;
    case 90:
        *x += 1;
        *y += 1;
        code = 1;
        break;
    case 180:
        *x += 1;
        *y -= 2;
        code = 0x40;
        break;
    case 270:
        *x -= 2;
        *y -= 2;
        code = 0x10;
        break;
    }
    return Nibble_Scale(code) & 0xff;
}

}  // extern "C"
