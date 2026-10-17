#include "messagecentre.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/stdtree.h"
#include "things.h"

extern "C" {

void __fastcall MessageSlot_Dtor(u8 *slot) { StdTree_DtorB(slot + 4); }

void __fastcall MessageCentre_DtorSlots(u8 *centre) { CRT_VectorDtorIterator(centre, 0x14, 0x1d, reinterpret_cast<void(__fastcall *)(void *)>(&MessageSlot_Dtor)); }

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
}  // extern "C"
