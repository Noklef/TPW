#include "fontloaders.h"

#include "../core/gameheap.h"
#include "../core/methods.h"
#include "../core/refcounted.h"
#include "../core/vcall.h"
#include "resindex.h"

#include <cstdint>

namespace {
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
}

extern "C" {

s32 *__fastcall BitmapFont_Load(u8 *, void *, s32 *result, u8 *stream, u8 **object, u32, u8 *) {
    u8 header[12];
    if (vcall<s32>(stream, 8, header, 8u) != 8 || at<u32>(header, 0) != 0x424d4642) {   // "BFMB"
        vcall<s32>(stream, 4);
        *result = -1;
        return result;
    }
    const u8 lead = (u8)(0xff - header[4]);
    u8 *const font = static_cast<u8 *>(GameHeap_Alloc(0xc));
    if (font) {
        Class70a078_Ctor(font);
        at<u32>(font, 0) = 0x70a140;
        font[8] = lead;
        at<u16>(font, 0xa) = at<u16>(header, 6);
    }
    *object = font;
    *result = 0;
    return result;
}

u8 *__fastcall FontBase_Ctor(u8 *font) {
    Class70a078_Ctor(font);
    at<u32>(font, 0) = 0x70a208;
    return font;
}

u8 *__fastcall Font_Ctor(u8 *font, void *, u8 *data, u8 *index) {
    FontBase_Ctor(font);
    at<u8 *>(font, 8) = data;
    at<u8 *>(font, 0xc) = data;
    at<u8 *>(font, 0x10) = data + 8;
    at<u32>(font, 0) = 0x70a158;
    u8 *const record = ResIndex_Find(index, nullptr, 1);
    if (Record_Word4OrSix(record) == 0) {
        s32 slot;
        ResIndex_GetEntry(index, nullptr, &slot, record, &at<u8 *>(font, 0x14));
    } else {
        at<u32>(font, 0x14) = 0;
    }
    return font;
}

s32 *__fastcall Font_Load(u8 *, void *, s32 *result, u8 *stream, u8 **object, u32, u8 *index) {
    const u32 size = vcall<u32>(stream, 0x14);
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(size + 0x18));
    if (!block) {
        *result = -1;
        return result;
    }
    u8 *const data = block + 0x18;
    if ((u32)vcall<s32>(stream, 8, data, size) != size || at<u32>(data, 0) != 0x42463446) {   // "F4FB"
        GameHeap_Free(block);
        *result = -1;
        return result;
    }
    Font_Ctor(block, nullptr, data, index);
    *object = block;
    *result = 0;
    return result;
}
}
