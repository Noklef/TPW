#include "servers.h"

#include <cstdint>
#include <cstring>

#include "../core/stdstring.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// std::string(s) assigned to the server string at 0x7a14f0 + offset, as each copy is inlined.
void assign(u32 offset, const char *s) {
    StdString t;
    StdString_Tidy(&t, nullptr, 0);
    StdString_AssignChars(&t, nullptr, s, (u32)std::strlen(s));
    StdString_AssignSub(&global<StdString>(0x007a14f0 + offset), nullptr, &t, 0, 0xffffffff);  // npos
    StdString_Tidy(&t, nullptr, 1);
}


// A server string made from a C string as the constructor inlines it: emptied, then the characters when _Grow gives room, ended inline
// (size and terminator) or through _Eos.
void make(StdString *s, const char *chars, bool inlineEnd) {
    reinterpret_cast<u8 *>(s)[0] = 0;  // (the allocator byte: an uninitialised stack byte in the original, sic)
    StdString_Tidy(s, nullptr, 0);
    const u32 n = (u32)std::strlen(chars);
    if (!StdString_Grow(s, nullptr, n, 1)) return;
    std::memcpy(s->ptr, chars, n);
    if (inlineEnd) {
        s->len = n;
        s->ptr[n] = 0;
    } else {
        StdString_Eos(s, nullptr, n);
    }
}

const char *chars(u32 address) { return reinterpret_cast<const char *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __cdecl Online_ApplyBalance() {
    const BalanceOnline &b = global<BalanceOnline>(0x00785828);  // gBalanceOnline
    assign(0x00, b.mMailServer);
    assign(0x10, b.mChatServer);
    assign(0x20, b.mLoginServer);
    assign(0x30, b.mCityServer);
    assign(0x40, b.mNewsServer);
    assign(0x50, b.mVoteServer);
    assign(0x6c, b.mComplainEmail);
    global<u16>(0x007a1550) = (u16)b.mMailPort;
    global<u16>(0x007a1552) = (u16)b.mChatPort;
    global<u16>(0x007a1554) = (u16)b.mLoginPort;
    global<u16>(0x007a1556) = (u16)b.mCityPort;
    global<u16>(0x007a1558) = (u16)b.mNewsPort;
    global<u16>(0x007a155a) = (u16)b.mVotePort;
}

u8 *__fastcall OnlineServers_Ctor(u8 *servers) {
    StdString *const s = reinterpret_cast<StdString *>(servers);
    make(&s[0], chars(0x0077485c), true);
    make(&s[1], chars(0x00774844), true);
    make(&s[2], chars(0x00774838), true);
    make(&s[3], chars(0x00774838), false);
    make(&s[4], chars(0x00774838), false);
    servers[0x50] = 0;  // (the allocator byte, sic)
    StdString_Tidy(&s[5], nullptr, 0);
    u16 *const ports = reinterpret_cast<u16 *>(servers + 0x60);
    ports[0] = 0x19;
    ports[1] = 0x1772;
    ports[2] = 0x1dae;
    ports[3] = 0x1da7;
    ports[4] = 0x1dad;
    make(reinterpret_cast<StdString *>(servers + 0x6c), chars(0x0077481c), false);
    return servers;
}

void __fastcall OnlineServers_Dtor(u8 *servers) {
    StdString_Tidy(reinterpret_cast<StdString *>(servers + 0x6c), nullptr, 1);
    StdString *const s = reinterpret_cast<StdString *>(servers);
    for (s32 i = 5; i >= 0; --i) StdString_Tidy(&s[i], nullptr, 1);
}
}
