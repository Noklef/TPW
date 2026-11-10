#include "regthunks.h"

#include "../game/gfx/movieidct.h"
#include "../game/gfx/tgqvideo.h"
#include "../game/gfx/tqivideo.h"
#include "../game/math/ssemath.h"
#include "../game/render/rasterspans.h"

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
__declspec(naked) void RegThunk_SseMath_ExpandMatrix() {
    __asm {
        push edi
        push edx
        call SseMath_ExpandMatrix
        add esp, 8
        ret
    }
}

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

// ECX and EDX kept (pushed apart from the arguments, which the callee may change); the position back in EDI.
__declspec(naked) void RegThunk_TqiVideo_PixelRowYuy2() {
    __asm {
        push ebp
        push edi
        push esi
        call TqiVideo_PixelRowYuy2
        add esp, 12
        ret
    }
}

__declspec(naked) void RegThunk_TqiVideo_PixelRowRgb16() {
    __asm {
        push ebp
        push edi
        push esi
        call TqiVideo_PixelRowRgb16
        add esp, 12
        ret
    }
}

__declspec(naked) void RegThunk_TqiVideo_PixelRowRgb32() {
    __asm {
        push ebp
        push edi
        push esi
        call TqiVideo_PixelRowRgb32
        add esp, 12
        ret
    }
}

__declspec(naked) void RegThunk_TqiVideo_PixelRowBytes32() {
    __asm {
        push ebp
        push edi
        push esi
        call TqiVideo_PixelRowBytes32
        add esp, 12
        ret
    }
}

__declspec(naked) void RegThunk_TqiVideo_DecodeBlock() {
    __asm {
        push ecx
        push edx
        push ecx
        push edx
        push edi
        push ebp
        push esi
        push eax
        call TqiVideo_DecodeBlock
        add esp, 24
        mov edi, eax
        pop edx
        pop ecx
        ret
    }
}

__declspec(naked) void RegThunk_TqiVideo_ReadBlocks() {
    __asm {
        lea eax, [esp + 4]
        push eax
        push edi
        push ebp
        push esi
        call TqiVideo_ReadBlocks
        add esp, 16
        mov edi, eax
        ret
    }
}

// Jumped to with the row pointer in EBP; pops the six registers its set-up pushed, then returns to the set-up's caller.
__declspec(naked) void RegThunk_RasterSpan_TexShadeZ() {
    __asm {
        push ebp
        call RasterSpan_TexShadeZ
        add esp, 4
        pop ecx
        pop ebp
        pop ebx
        pop edx
        pop esi
        pop edi
        ret
    }
}
}
