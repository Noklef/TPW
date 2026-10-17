#include "messagecentre.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/stdtree.h"
#include "../files/file.h"
#include "things.h"

extern "C" {

void __fastcall MessageSlot_Dtor(u8 *slot) { StdTree_DtorB(slot + 4); }

void __fastcall MessageCentre_DtorSlots(u8 *centre) { CRT_VectorDtorIterator(centre, 0x14, 0x1d, reinterpret_cast<void(__fastcall *)(void *)>(&MessageSlot_Dtor)); }

u8 __fastcall MessageCentre_AddThing(u8 *centre, void *, const u8 *thing, s32 slot) {
    if (slot >= 0x1d) return 0;
    u16 handle;
    const u16 key = *Thing_GetHandle(thing, nullptr, &handle);
    u8 *result[2];
    StdMapB_Insert(centre + 0x14 * slot + 4, nullptr, result, &key);
    return static_cast<u8>(reinterpret_cast<uintptr_t>(result[1]));
}

u32 __fastcall MessageCentre_RemoveThing(u8 *centre, void *, const u8 *thing) {
    u32 removed = 0;
    for (u32 k = 0; k < 0x1d; ++k) {
        u8 *const map = centre + 0x14 * k + 4;
        u8 *const *const head = reinterpret_cast<u8 *const *>(map + 4);
        u16 handle;
        const u16 key = *Thing_GetHandle(thing, nullptr, &handle);
        u8 *const it = Tree16a_LowerBound(map, nullptr, &key);
        u8 *const found = it == *head || StdTreeB_KeyLess(&key, nullptr, reinterpret_cast<const u16 *>(it + 0xc)) ? *head : it;
        if (found != *head) {
            u8 *out;
            StdTree_EraseB(map, nullptr, &out, found);
            ++removed;
        }
    }
    return removed;
}

u32 __fastcall MessageCentre_Save(u8 *centre, void *, const u8 *chunk) {
    s32 result;
    s32 done;
    s32 count = 0x1d;
    Stream_Write(&result, *reinterpret_cast<Stream *const *>(chunk), &count, 4, &done);
    for (u32 k = 0; k < 0x1d; ++k) {
        const u8 *const map = centre + 0x14 * k + 4;
        s32 size = *reinterpret_cast<const s32 *>(map + 0xc);
        Stream_Write(&result, *reinterpret_cast<Stream *const *>(chunk), &size, 4, &done);
        u8 *node = **reinterpret_cast<u8 *const *const *>(map + 4);   // (the head's left: the first node)
        while (node != *reinterpret_cast<u8 *const *>(map + 4)) {
            Stream_Write(&result, *reinterpret_cast<Stream *const *>(chunk), node + 0xc, 2, &done);
            StdTree_IncB(&node);
        }
    }
    return 0;
}
}  // extern "C"
