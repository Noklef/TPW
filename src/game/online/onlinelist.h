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

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[32], gOnlineList_GetCities_773b90, 0x00773b90, 0x20);
GAME_DATA(char[40], gOnlineList_GetParksInCity_773bb0, 0x00773bb0, 0x28);
GAME_DATA(char[48], gOnlineList_GetTop10Parks_773bd8, 0x00773bd8, 0x30);
GAME_DATA(char[52], gOnlineList_GetParkByName_773c08, 0x00773c08, 0x34);
GAME_DATA(char[48], gOnlineList_GetParkById_773c3c, 0x00773c3c, 0x30);
GAME_DATA(char[32], gOnlineList_GetThemes_773c6c, 0x00773c6c, 0x20);
GAME_DATA(char[36], gOnlineList_GetServers_773c8c, 0x00773c8c, 0x24);
GAME_DATA(char[32], gOnlineList_GetRides_773cb0, 0x00773cb0, 0x20);
