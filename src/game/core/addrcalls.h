// Calls by the game's addresses, each replaceable: for a function with many callees, a table in place of an environment's fields. No table, or an address not in it: the game's (ported or not).
#pragma once

#include <cstdint>
#include <type_traits>

#include "../../../re/types.h"

struct AddrCall {
    u32 address, target;
};

// What a call to a game address reaches when no table names it: unset (the hook DLL, the tests), the address itself, where the hook's jump
// takes it to the port; in tpw.exe the loader's resolver (src/loader), the port, or the address when there is none (the call then traps).
inline u32(__cdecl *g_addressTarget)(u32 address) = nullptr;

// A game function's address as what a call there reaches (above), so in tpw.exe a call through an env table or a stored callback goes
// straight to the port instead of faulting into the loader each time. Env tables are filled after the loader sets the resolver.
inline u32 GameCode(u32 address) { return g_addressTarget ? g_addressTarget(address) : address; }

// A game address as the pointer T: GameCode for a function.
template <typename T>
T GameAddress(u32 address) {
    if constexpr (std::is_function_v<std::remove_pointer_t<T>>) address = GameCode(address);
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}

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
        return g_addressTarget ? g_addressTarget(address) : address;
    }
    const AddrCall *table_ = nullptr;
    u32 count_ = 0;
};
