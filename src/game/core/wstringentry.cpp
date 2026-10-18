#include "wstringentry.h"

#include "gameheap.h"

extern "C" {

WStringEntry *__fastcall WStringEntry_Ctor(WStringEntry *e, void *, StdWString s) {
    *reinterpret_cast<u8 *>(&e->first.allocator) = static_cast<u8>(s.allocator);
    e->first.ptr = nullptr;
    e->first.len = 0;
    e->first.res = 0;
    *reinterpret_cast<u8 *>(&e->second.allocator) = static_cast<u8>(s.allocator);
    e->second.ptr = nullptr;
    e->second.len = 0;
    e->second.res = 0;
    StdWString_AssignSub(&e->second, nullptr, &s, 0, 0xffffffff);  // (npos, the read-only 0x6fef00)
    e->value = 0;
    if (s.ptr) {
        u8 *const count = reinterpret_cast<u8 *>(s.ptr) - 1;
        if (*count == 0 || *count == 0xff)
            GameHeap_Free(reinterpret_cast<u8 *>(s.ptr) - 2);
        else
            --*count;
    }
    return e;
}
}
