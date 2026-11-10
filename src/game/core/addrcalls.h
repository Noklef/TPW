// Calls by the game's addresses, each replaceable: for a function with many callees, a table in place of an environment's fields. No table, or an address not in it: the game's (ported or not).
#pragma once

#include <cstdint>

#include "../../../re/types.h"

struct AddrCall {
    u32 address, target;
};

class AddrCalls {
public:
    void Set(const AddrCall *table, u32 count) {
        table_ = table;
        count_ = count;
    }
    template <typename... A>
    u32 Cd(u32 address, A... args) const {   // cdecl
        return reinterpret_cast<u32(__cdecl *)(A...)>((uintptr_t)Target(address))(args...);
    }
    template <typename... A>
    u32 St(u32 address, A... args) const {   // stdcall
        return reinterpret_cast<u32(__stdcall *)(A...)>((uintptr_t)Target(address))(args...);
    }
    template <typename... A>
    u32 Th(u32 address, u32 self, A... args) const {   // thiscall (fastcall with an unused EDX pops the same)
        return reinterpret_cast<u32(__fastcall *)(u32, u32, A...)>((uintptr_t)Target(address))(self, 0, args...);
    }

private:
    u32 Target(u32 address) const {
        for (u32 i = 0; i < count_; ++i)
            if (table_[i].address == address) return table_[i].target;
        return address;
    }
    const AddrCall *table_ = nullptr;
    u32 count_ = 0;
};
