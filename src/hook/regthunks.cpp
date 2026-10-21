#include "regthunks.h"

#include "../game/gfx/movieidct.h"
#include "../game/gfx/tgqvideo.h"

extern "C" {

// Block in ESI.
__declspec(naked) void RegThunk_DctIdct_Column() {
    __asm {
        push esi
        call DctIdct_Column
        add esp, 4
        ret
    }
}

__declspec(naked) void RegThunk_DctIdct_Row() {
    __asm {
        push esi
        call DctIdct_Row
        add esp, 4
        ret
    }
}

// Block in ESI, out in EDI.
__declspec(naked) void RegThunk_MovieIdct_Column() {
    __asm {
        push edi
        push esi
        call MovieIdct_Column
        add esp, 8
        ret
    }
}

// Block in ESI, out the caller's stack argument (cdecl: the caller pops it).
__declspec(naked) void RegThunk_MovieIdct_Row() {
    __asm {
        push dword ptr [esp + 4]
        push esi
        call MovieIdct_Row
        add esp, 8
        ret
    }
}

__declspec(naked) void RegThunk_MovieIdct_RowShift() {
    __asm {
        push edi
        push esi
        call MovieIdct_RowShift
        add esp, 8
        ret
    }
}

// Luma in ESI, out in EDI, chroma in EBX.
__declspec(naked) void RegThunk_TgqVideo_PixelRow16() {
    __asm {
        push ebx
        push edi
        push esi
        call TgqVideo_PixelRow16
        add esp, 12
        ret
    }
}

__declspec(naked) void RegThunk_TgqVideo_PixelRow32() {
    __asm {
        push ebx
        push edi
        push esi
        call TgqVideo_PixelRow32
        add esp, 12
        ret
    }
}

// No arguments; the counter left in EDI (the callers hand it to 0x67673c).
__declspec(naked) void RegThunk_TgqVideo_DecodeCoefficients() {
    __asm {
        call TgqVideo_DecodeCoefficients
        mov edi, eax
        ret
    }
}
}
