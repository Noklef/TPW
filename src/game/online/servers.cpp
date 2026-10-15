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
}
