#include "steering.h"

#include <cstring>

namespace {

u32 *Construct(u32 *self, u32 vtable, const char *name) {
    self[0] = vtable;
    std::strcpy(reinterpret_cast<char *>(self + 1), name);  // repne scasb; rep movsd; rep movsb: the name and its 0
    return self;
}

u32 *Global(u32 address) { return reinterpret_cast<u32 *>(address); }

}  // namespace

extern "C" {

u32 *__fastcall SteerSeek_Ctor(u32 *self) { return Construct(self, 0x7009b8, "seek"); }
u32 *__fastcall SteerArrival_Ctor(u32 *self) { return Construct(self, 0x7009c0, "arrival"); }
u32 *__fastcall SteerSeparation_Ctor(u32 *self) { return Construct(self, 0x7009c4, "separation"); }
u32 *__fastcall SteerAvoidObstacle_Ctor(u32 *self) { return Construct(self, 0x7009c8, "avoid_obstacle"); }
u32 *__fastcall SteerRepelObstacle_Ctor(u32 *self) { return Construct(self, 0x7009cc, "repel_obstacle"); }
u32 *__fastcall SteerAvoidWalls_Ctor(u32 *self) { return Construct(self, 0x7009d0, "avoid_walls"); }
u32 *__fastcall SteerFollowPath_Ctor(u32 *self) { return Construct(self, 0x7009d4, "follow_path"); }

void __fastcall Steer_Dtor(u32 *self) { self[0] = 0x7009bc; }

void __cdecl SteerSeek_Destroy() { Steer_Dtor(Global(0x7cf050)); }
void __cdecl SteerArrival_Destroy() { Steer_Dtor(Global(0x7cf0b0)); }
void __cdecl SteerSeparation_Destroy() { Steer_Dtor(Global(0x7cf0d8)); }
void __cdecl SteerAvoidObstacle_Destroy() { Steer_Dtor(Global(0x7cf360)); }
void __cdecl SteerRepelObstacle_Destroy() { Steer_Dtor(Global(0x7cf338)); }
void __cdecl SteerAvoidWalls_Destroy() { Steer_Dtor(Global(0x7cf088)); }
void __cdecl SteerFollowPath_Destroy() { Steer_Dtor(Global(0x7cf308)); }
}
