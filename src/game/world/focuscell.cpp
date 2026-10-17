#include "focuscell.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/actionrec.h"
#include "../core/globals.h"
#include "../core/thunks.h"
#include "dirstep.h"
#include "objectdefs.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u32 CellId(const u8 *thing) { return ((u32)thing[7] << 7) + thing[5] + 1; }

}  // namespace

extern "C" {

void __cdecl FocusCell_Update(u32 record) {
    global<u16>(0x008186a0) = global<u16>(0x00818698);
    global<u16>(0x00818698) = 0;
    global<s32>(0x007639a4) = -1;
    global<s32>(0x007639a8) = -1;
    global<s32>(0x007639ac) = -1;
    s32 direction;
    if ((Global81ae2c_Equals(0x19) || Global81ae2c_Equals(0x1a)) && global<s32>(0x007639f0) != -1) {
        const s32 x = global<s32>(0x007639f0), y = global<s32>(0x007639f4);
        global<s32>(0x007639a4) = x;
        global<s32>(0x007639e0) = x;
        global<s32>(0x007639a8) = y;
        global<s32>(0x007639e4) = y;
        global<u16>(0x00818698) = (u16)(((u32)y << 7) + (u32)x + 1);
        global<s32>(0x007639ac) = 0;
        global<s32>(0x007639e8) = x;
        global<s32>(0x007639ec) = y;
        global<u8>(0x00818c50) = 4;
        global<u32>(0x008186c4) = global<u32>(0x007cf83c) + ((u32)global<u16>(0x00818698) + 0x6cde) * 0x28;
        direction = 0;
    } else {
        u8 *const selected = reinterpret_cast<u8 *>((uintptr_t)Wrap_Global7c2654_Get_4815a0());
        global<u8 *>(0x008186ac) = selected;
        if (!selected)
            return;
        u16 handle;
        if (*reinterpret_cast<u16 *>((uintptr_t)Fn_4e0df0(selected, nullptr, (u32)(uintptr_t)&handle)) != 0) {
            Fn_4e0df0(selected, nullptr, (u32)(uintptr_t)&handle);
            u8 *const thing = global<u8 *>(0x007cfb90 + (u32)handle * 20);
            if (at<u32>(ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(selected, 0xe)), 0x288) == 1) {
                global<u8 *>(0x008186ac) = thing;
                Wrap_Global7c2654_Set_4815b0((u32)(uintptr_t)thing);
            }
        }
        const u8 *const focus = global<u8 *>(0x008186ac);
        global<u16>(0x00818698) = (u16)CellId(focus);
        const u32 cell = global<u16>(0x00818698);
        global<u32>(0x008186c4) = global<u32>(0x007cf83c) + (cell + 0x6cde) * 0x28;
        const s32 x = (s32)((cell - 1) & 0x7f), y = (s32)((cell - 1) >> 7);
        global<s32>(0x007639a4) = x;
        global<s32>(0x007639e0) = x;
        global<s32>(0x007639a8) = y;
        global<s32>(0x007639e4) = y;
        global<s32>(0x007639e8) = x;
        global<s32>(0x007639ac) = at<s32>(focus, 0x10);
        global<s32>(0x007639ec) = y;
        global<u8>(0x00818c50) = (u8)Position_StepA(&global<s32>(0x007639e0), &global<s32>(0x007639e4), global<s32>(0x007639ac));
        direction = global<s32>(0x007639ac);
    }
    Position_StepB(&global<s32>(0x007639e8), &global<s32>(0x007639ec), direction);
    const u8 *const focus = global<u8 *>(0x008186ac);
    if (focus && global<u16>(0x00818698) != global<u16>(0x008186a0) && (u8)record)
        ActionRec_WriteW(reinterpret_cast<ActionRec *>(0x00785a08), nullptr, 0x32, CellId(focus));
}
}
