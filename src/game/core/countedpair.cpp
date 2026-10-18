#include "countedpair.h"

#include "gameheap.h"

extern "C" {

CountedPair *__fastcall CountedPair_CtorWide(CountedPair *p, void *, StdWString s) {
    *reinterpret_cast<u8 *>(&p->narrow.allocator) = static_cast<u8>(s.allocator);
    p->narrow.ptr = nullptr;
    p->narrow.len = 0;
    p->narrow.res = 0;
    *reinterpret_cast<u8 *>(&p->wide.allocator) = static_cast<u8>(s.allocator);
    p->wide.ptr = nullptr;
    p->wide.len = 0;
    p->wide.res = 0;
    StdWString_AssignSub(&p->wide, nullptr, &s, 0, 0xffffffff);  // (npos, the read-only 0x6fef00)
    p->value = 0;
    if (s.ptr) {
        u8 *const count = reinterpret_cast<u8 *>(s.ptr) - 1;
        if (*count == 0 || *count == 0xff)
            GameHeap_Free(reinterpret_cast<u8 *>(s.ptr) - 2);
        else
            --*count;
    }
    return p;
}
}
