#include "chat.h"

#include <cstdint>
#include <cstring>

#include "../core/buffers.h"
#include "../core/crt.h"
#include "../core/initializers.h"
#include "../core/stdstring.h"
#include "../core/stdwstring.h"
#include "../core/stringmapsrecord.h"
#include "../core/sync.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__fastcall OnlineChat_Ctor(u8 *chat) {
    at<u32>(chat, 0) = 0;
    at<u32>(chat, 4) = 0;
    chat[8] = 0;   // (the allocator bytes here and below: the original's uninitialised stack byte, sic)
    CountedBuf_Reset(chat + 8, nullptr, 0);
    chat[0x18] = 0;
    at<u32>(chat, 0x1c) = 0;
    at<u32>(chat, 0x20) = 0;
    at<u32>(chat, 0x24) = 0;
    for (u32 i = 0x28; i <= 0x2c; ++i) chat[i] = 0;
    CritSec_Ctor(chat + 0x30);
    CritSec_Ctor(chat + 0x48);
    StringMapsRecord_Ctor(chat + 0xe60);
    chat[0xec0] = 0;
    at<u32>(chat, 0xec4) = 0;
    chat[0xec8] = 0;
    for (u32 i = 0xecc; i <= 0xed8; i += 4) at<u32>(chat, i) = 0;
    Init_5ec210(chat + 0xedc);
    at<u32>(chat, 0xeec) = 0;
    at<u32>(chat, 0xef0) = 0;
    at<u32>(chat, 0xef4) = 0;
    chat[0xef8] = 0;
    chat[0xefc] = 0;
    CountedBuf_Reset(chat + 0xefc, nullptr, 0);
    u16 text[0x104];
    Crt().swprintf(text, reinterpret_cast<const u16 *>((uintptr_t)0x0075c0f0), Crt().rand());   // L"%08x"
    const u32 n = Crt().wcslen(text);
    StdWString *const name = reinterpret_cast<StdWString *>(chat + 8);
    if (StdWString_Grow(name, nullptr, n, 1)) {
        for (u32 i = 0; i < n; ++i) name->ptr[i] = text[i];
        name->len = n;
        name->ptr[n] = 0;
    }
    // +0x18 = the string at 0x7a1500: assign(x, 0, npos) inlined (no max_size check)
    StdString *const s = reinterpret_cast<StdString *>(chat + 0x18);
    const StdString *const x = reinterpret_cast<const StdString *>((uintptr_t)0x007a1500);
    const u32 len = x->len, npos = *reinterpret_cast<const u32 *>((uintptr_t)0x006fe5ac);
    const u32 count = npos < len ? npos : len;
    const char *const empty = reinterpret_cast<const char *>((uintptr_t)0x006fe5b0);
    if (s == x) {   // (never: the chat is not at 0x7a14e8)
        StdString_Erase(s, nullptr, count, npos);
        StdString_Erase(s, nullptr, 0, 0);
    } else if (count && count == len && *reinterpret_cast<const u8 *>((x->ptr ? x->ptr : empty) - 1) < 0xfe) {
        StdString_Tidy(s, nullptr, 1);
        char *const p = x->ptr ? x->ptr : const_cast<char *>(empty);
        s->ptr = p;
        s->len = x->len;
        s->res = x->res;
        ++*reinterpret_cast<u8 *>(p - 1);
    } else if (StdString_Grow(s, nullptr, count, 1)) {
        std::memcpy(s->ptr, x->ptr ? x->ptr : empty, count);   // (rep movs)
        s->len = count;
        s->ptr[count] = 0;
    }
    return chat;
}

void __fastcall OnlineChat_Set29And2a(u8 *chat) {
    CritSec_Enter(chat + 0x48);
    at<u8>(chat, 0x29) = 1;
    at<u8>(chat, 0x2a) = 1;
    CritSec_Leave(chat + 0x48);
}

u8 __fastcall OnlineChat_Get2a(u8 *chat) {
    CritSec_Enter(chat + 0x48);
    const u8 value = at<u8>(chat, 0x2a);
    CritSec_Leave(chat + 0x48);
    return value;
}
}
