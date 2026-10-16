#include "vregs.h"

#include <cstdint>

#include "gameheap.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// The shadow dword: at +0x14 of the object, found through the offset in dword +4 of its vtable.
u32 &Shadow(u8 *object) { return at<u32>(object, at<u32>(at<u8 *>(object, 0), 4) + 0x14); }

void Drain(u8 *object, u32 field) {
    for (u8 *list = at<u8 *>(object, field); at<u32>(list, 0xc) != 0; list = at<u8 *>(object, field)) {
        u8 *node = at<u8 *>(list, 4);
        u8 *held = nullptr;
        if (node) {
            u8 *next = at<u8 *>(node, 4);
            at<u8 *>(list, 4) = next;
            if (next)
                at<u32>(next, 8) = 0;
            else
                at<u32>(list, 8) = 0;
            held = at<u8 *>(node, 0);
            at<u32>(list, 0xc) -= 1;
            GameHeap_Free(node);
        }
        if (held)
            vcall<u32>(held, 0, 1u);
    }
}

}  // namespace

extern "C" {

void __fastcall Shadow_SetByte(u8 *object, void *, s32 value) {
    if ((u8)(Shadow(object) ^ (u32)value) == 0)
        return;
    comcall<u32>(at<u8 *>(object, 0x24), 0x58, 0x13u, (u32)((value >> 4) & 0xf));
    comcall<u32>(at<u8 *>(object, 0x24), 0x58, 0x14u, (u32)(value & 0xf));
    u32 &shadow = Shadow(object);
    shadow = (((u32)value ^ shadow) & 0xff) ^ shadow;
}

void __fastcall Shadow_SetBits2(u8 *object, void *, u32 value, u32 mask, u32 reg, u32 onValue, u32 offValue) {
    if ((mask & (Shadow(object) ^ value)) == 0)
        return;
    const u32 bits = mask & value;
    comcall<u32>(at<u8 *>(object, 0x24), 0x58, reg, bits ? onValue : offValue);
    u32 &shadow = Shadow(object);
    shadow = (~mask & shadow) | bits;
}

void __fastcall Shadow_SetBits3(u8 *object, void *, u32 value, u32 a, u32 mask, u32 b, u32 onValue, u32 offValue) {
    if ((mask & (Shadow(object) ^ value)) == 0)
        return;
    const u32 bits = mask & value;
    comcall<u32>(at<u8 *>(object, 0x24), 0xa0, a, b, bits ? onValue : offValue);
    u32 &shadow = Shadow(object);
    shadow = (~mask & shadow) | bits;
}

u32 __cdecl Stream_SeekFailed(u8 *object, u32 offset, u32 whence) {
    if (whence != 0 && whence != 1 && whence != 2)
        return 1;
    return (s32)vcall<u32>(object, 0x10, offset, whence) < 0 ? 1 : 0;
}

void __fastcall Obj_DrainList0c(u8 *object) { Drain(object, 0xc); }

void __fastcall Obj_DrainList04(u8 *object) { Drain(object, 4); }

}  // extern "C"
