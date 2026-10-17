// The online lists' requests to the city server's DLL (WEACITYR, class City): reimplementation of 0x5bc910, 0x5bcb30, 0x5bcb90, 0x5bcce0, 0x5bce30,
// 0x5bd180, 0x5bd5c0 and 0x5bda10. Each says what it asks for on the debugger's output (OutputDebugStringA, "ONLINELIST: Getting ...") and
// returns what the City method (__thiscall on the city, through its import slot) answers. The names are unconfirmed beyond the methods'.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5bc910 / 0x5bd180 / 0x5bda10 (stdcall; city, request): City::GetCityList / GetThemeList / GetRideList (0x6fd55c / 0x6fd548 / 0x6fd544)
// with (0, request, 0).
u32 __stdcall OnlineList_GetCities(void *city, u32 request);
u32 __stdcall OnlineList_GetThemes(void *city, u32 request);
u32 __stdcall OnlineList_GetRides(void *city, u32 request);

// 0x5bd5c0 (stdcall; city, request): City::GetLogicalServerList (0x6fd530) with (0, 1, request, 0).
u32 __stdcall OnlineList_GetServers(void *city, u32 request);

// 0x5bcb30 / 0x5bcb90 / 0x5bce30 (thiscall; city, request; pops 8): City::GetParkInCityList / GetParkInCityListTop10 / GetParkByID (0x6fd558 /
// 0x6fd554 / 0x6fd54c) with (0, the list's +0x34, request, 0).
u32 __fastcall OnlineList_GetParksInCity(const u8 *list, void *, void *city, u32 request);
u32 __fastcall OnlineList_GetTop10Parks(const u8 *list, void *, void *city, u32 request);
u32 __fastcall OnlineList_GetParkById(const u8 *list, void *, void *city, u32 request);

// 0x5bcce0 (thiscall; city, request; pops 8): City::GetParkInWorldByName (0x6fd550) with (0, the list's +0x3c (a wide name), request, 0).
u32 __fastcall OnlineList_GetParkByName(const u8 *list, void *, void *city, u32 request);
}
