#include "onlinelist.h"

#include <cstdint>

namespace {

// OutputDebugStringA (0x6fd1e0) with the game's text at address text.
void Say(u32 text) {
    using Output = void(__stdcall *)(const char *);
    (*reinterpret_cast<const Output *>((uintptr_t)0x006fd1e0))(reinterpret_cast<const char *>((uintptr_t)text));
}

// A City method (__thiscall: the city in ECX, the callee pops its arguments) through its import slot.
u32 CallCity(u32 slot, void *city, u32 a, u32 b, u32 c) {
    using Method = u32(__fastcall *)(void *, void *, u32, u32, u32);
    return (*reinterpret_cast<const Method *>((uintptr_t)slot))(city, nullptr, a, b, c);
}
u32 CallCity(u32 slot, void *city, u32 a, u32 b, u32 c, u32 d) {
    using Method = u32(__fastcall *)(void *, void *, u32, u32, u32, u32);
    return (*reinterpret_cast<const Method *>((uintptr_t)slot))(city, nullptr, a, b, c, d);
}

}  // namespace

extern "C" {

u32 __stdcall OnlineList_GetCities(void *city, u32 request) {
    Say(0x00773b90);
    return CallCity(0x006fd55c, city, 0, request, 0);
}

u32 __stdcall OnlineList_GetThemes(void *city, u32 request) {
    Say(0x00773c6c);
    return CallCity(0x006fd548, city, 0, request, 0);
}

u32 __stdcall OnlineList_GetRides(void *city, u32 request) {
    Say(0x00773cb0);
    return CallCity(0x006fd544, city, 0, request, 0);
}

u32 __stdcall OnlineList_GetServers(void *city, u32 request) {
    Say(0x00773c8c);
    return CallCity(0x006fd530, city, 0, 1, request, 0);
}
}  // extern "C"
