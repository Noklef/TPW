#include "dialog7ca308.h"

#include <cstdint>
#include <cstring>

#include "../math/x87.h"
#include "../sprite/screenmatrix.h"
#include "../sprite/script.h"
#include "../world/staff.h"
#include "guirect.h"
#include "window.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __cdecl Dialog7ca308_ShowStaff(u32 index) {
    u8 rect[0xc];
    GuiRect_Ctor(rect);
    u8 *const child = Window_FindChild(global<u8 *>(0x007ca308), nullptr, 0x2484);
    Window_GetSavedRect(child, nullptr, rect);
    const s32 left = at<s16>(rect, 4), top = at<s16>(rect, 6), right = at<s16>(rect, 8), bottom = at<s16>(rect, 0xa);
    const float x = (float)X87_MulIntDouble(((right - left) >> 1) + left - 0x3f8, global<float>(0x006ff984));
    const float y = (float)X87_MulIntDouble(((bottom - top) >> 2) - bottom + 0x300, global<float>(0x006ff988));
    const float z = (float)Sprite_FlipY(at<s16>(child, 0xe0) + 1);
    const u8 *const entry = StaffPool_ValidEntry(global<u8 *>(0x007cf83c), nullptr, index);
    if (entry) {
        const u32 type = at<u32>(entry, 0), sprite = entry[9];
        if (type != global<u32>(0x007523e8) || sprite != global<u32>(0x007523ec)) {
            global<u32>(0x007523e8) = type;
            global<u32>(0x007523ec) = sprite;
            if (global<s32>(0x007ca310)) SpriteScript_Kill(global<s32>(0x007ca310));
            static const u32 kFolder[5] = {5, 6, 4, 7, 8};   // (by the type)
            const u32 folder = type <= 4 ? kFolder[type] : index;   // sic: any other type, the index
            u32 bits[3];
            std::memcpy(&bits[0], &x, 4);
            std::memcpy(&bits[1], &y, 4);
            std::memcpy(&bits[2], &z, 4);
            global<s32>(0x007ca310) = SpriteScript_Start(0x0074f758, folder, sprite, 1, bits[0], bits[1], bits[2]);
            SpriteScript_SetRegF(global<s32>(0x007ca310), 8, 0x41000000);
        }
    }
    GuiRect_Dtor(rect);
}
}  // extern "C"
