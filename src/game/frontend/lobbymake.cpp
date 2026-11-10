#include "lobbymake.h"

#include "../model/rideresource.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/soundset.h"
#include "../core/string.h"
#include "../core/vcall.h"
#include "../math/x87.h"
#include "../model/holder.h"
#include "../model/release.h"
#include "../world/rides.h"
#include "lobbymodel.h"
#include "frame.h"
#include "lobbyscene.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const LobbyMakeEnv kPorts = {reinterpret_cast<decltype(LobbyMakeEnv::load)>(Ride_LoadResource),
                             &Ride_Create,
                             &Holder_Sleep,
                             &Obj_RemoveFromRect,
                             &LobbyModel_TurnY,
                             &LobbyModel_SetPosition,
                             &Sound_RegisterLobbyCategories,
                             &RideUser_Acquire,
                             &LobbyModel_AlignTo,
                             &LobbyModel_SetScale};
const LobbyMakeEnv *g_env = &kPorts;

u8 *Scene() { return gFrame_f83774; }

String *StringAt(u8 *object, u32 offset) { return reinterpret_cast<String *>(object + offset); }

}  // namespace

void LobbyMake_SetEnv(const LobbyMakeEnv *env) { g_env = env ? env : &kPorts; }

extern "C" {

u8 *__fastcall LobbyResource_Ctor(u8 *resource, void *, const char *name, const char *pack, s32 base, u32 animated, u32 a, u32 b, u32 c) {
    String *const first = StringAt(resource, 4), *const second = StringAt(resource, 0x18);
    String_Ctor(first);
    String_Ctor(second);
    vcall<void>(first, 0x14, name);
    vcall<void>(second, 0x14, pack);
    const u32 flags = animated ? 0x40c0 : 0xc0;
    at<u32>(resource, 0) = g_env->load(at<const char *>(resource, 0xc), at<const char *>(resource, 0x20), 0, 0, flags, Int_Add(0x4650, base), a, b, c, 0, 0);
    vcall<void>(Scene(), 0x1c, resource);
    return resource;
}

u8 *__fastcall LobbyScene_MakeModel(u8 *scene, void *, const char *name, const char *pack, u32 animated, u32 a, u32 b, u32 c) {
    u8 *resource = nullptr;
    for (u8 *node = at<u8 *>(at<u8 *>(scene, 8), 4); node && !resource;) {
        u8 *const held = at<u8 *>(node, 0);
        node = at<u8 *>(node, 4);
        if (!Crt().stricmp(at<const char *>(held, 0xc), name) && !Crt().stricmp(at<const char *>(held, 0x20), pack))
            resource = held;
    }
    if (!resource) {
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x2c));
        if (block)
            resource = LobbyResource_Ctor(block, nullptr, name, pack, at<s32>(at<u8 *>(scene, 8), 0xc), animated, a, b, c);
    }
    u8 *const model = static_cast<u8 *>(GameHeap_Alloc(0x34));
    if (!model)
        return nullptr;
    const u32 type = at<u32>(resource, 0);
    String *const first = StringAt(model, 0xc), *const second = StringAt(model, 0x20);
    String_Ctor(first);
    String_Ctor(second);
    at<u32>(model, 0) = (u32)(uintptr_t)Class702a50_vtable;
    const u32 ride = g_env->create(type, 0x101, nullptr, 0, 0, 0, 0, 0, -1, 0);
    at<u32>(model, 4) = ride;
    u8 *const instance = gRides_7a4610[ride];
    if (at<u32>(instance, 4) & 0x4000)
        g_env->removeRect(at<u8 *>(instance, 8), at<u32>(instance, 0xc0), at<u32>(instance, 0xc4));
    else
        g_env->sleep(at<u8 *>(instance, 8));
    at<u32>(model, 8) = 0;
    vcall<void>(first, 0x14, name);
    vcall<void>(second, 0x14, pack);
    vcall<void>(Scene(), 0x18, model);
    return model;
}

u8 *__fastcall Class702bb8_Ctor(u8 *self, void *, u8 *owner, const u8 *record) {
    at<u32>(self, 0) = (u32)(uintptr_t)Class702bb8_vtable;
    at<const u8 *>(self, 0xc) = record;
    const char *const pack = Crt().wcsicmp(at<const u16 *>(record, 0x14), reinterpret_cast<const u16 *>(0x00774ca8)) == 0 ? reinterpret_cast<const char *>(0x00774c9c) : reinterpret_cast<const char *>(0x00774c94);
    u8 *const model = LobbyScene_MakeModel(Scene(), nullptr, reinterpret_cast<const char *>(0x00774c80), pack, 0, 1, 0, 0);
    at<u8 *>(self, 4) = model;
    g_env->acquire(model);
    float raw[3];
    for (u32 k = 0; k < 3; ++k)
        at<u32>(raw, 4 * k) = at<u32>(record, 0x34 + 4 * k);
    const X87Reg x = raw[0], y = raw[1], z = raw[2];
    const float length = (float)X87_Sqrt(x * x + y * y + z * z);
    float unit[3];
    for (u32 k = 0; k < 3; ++k)
        unit[k] = (float)(X87Reg(raw[k]) / length);
    g_env->alignTo(at<const u8 *>(self, 4), nullptr, unit);
    g_env->setPosition(at<const u8 *>(self, 4), nullptr, raw);
    const X87Reg scaled = X87Reg((double)at<s32>(record, 0x40)) * global<float>(0x00702b64) - global<float>(0x00702b68);
    float z_scale = (float)scaled;
    if (scaled.v < global<float>(0x00702b6c))
        at<u32>(&z_scale, 0) = 0x3ca3d70a;
    else if (z_scale > global<float>(0x00702b70))
        at<u32>(&z_scale, 0) = 0x3dcccccd;
    const float xy = (float)(global<float>(0x00702b78) - X87Reg(z_scale) * global<float>(0x00702b74));
    g_env->setScale(at<const u8 *>(self, 4), nullptr, xy, xy, z_scale);
    at<u8 *>(self, 8) = owner;
    vcall<void>(owner, 0x30, self);
    return self;
}

u8 *__fastcall Class702ca8_Ctor(u8 *self, void *, u8 *owner, u32 a1, const char *title_, const char *name, const char *pack, const char *pack2, float degrees, const u32 *point, const u16 *text, u32 a9, u32 b, u32 c) {
    String *const title = StringAt(self, 0x28);
    String_Ctor(title);
    WString *const wide = reinterpret_cast<WString *>(self + 0x48);
    WString_Ctor(wide);
    at<u32>(self, 0) = (u32)(uintptr_t)Class702ca8_vtable;
    at<u8 *>(self, 4) = LobbyScene_MakeModel(Scene(), nullptr, name, pack, 0, 2, b, c);
    at<u8 *>(self, 8) = LobbyScene_MakeModel(Scene(), nullptr, name, pack2, 0, 1, b, c);
    at<u32>(self, 0x10) = 0;
    at<u32>(self, 0x24) = a1;
    vcall<void>(title, 0x14, title_);
    at<u32>(self, 0x3c) = point[0];
    at<u32>(self, 0x40) = point[1];
    at<u32>(self, 0x44) = point[2];
    vcall<void>(wide, 0x14, text);
    at<u32>(self, 0x5c) = 0;
    at<u32>(self, 0x60) = 0;
    const u32 ground[3] = {point[0], 0, point[2]};
    at<u32>(self, 0x64) = (Crt().rand() & 0xff) << 16;
    at<u32>(self, 0x68) = (Crt().rand() & 0xff) << 16;
    at<u32>(self, 0x6c) = (Crt().rand() & 0xff) << 16;
    at<u32>(self, 0x70) = a9;
    const float angle = (float)(X87Reg(global<float>(0x00702c18)) * global<float>(0x00702c54) * degrees);
    at<float>(self, 0x14) = angle;
    g_env->turnY(at<const u8 *>(self, 4), nullptr, angle);
    g_env->turnY(at<const u8 *>(self, 8), nullptr, at<float>(self, 0x14));
    g_env->setPosition(at<const u8 *>(self, 4), nullptr, reinterpret_cast<const float *>(ground));
    g_env->setPosition(at<const u8 *>(self, 8), nullptr, reinterpret_cast<const float *>(ground));
    at<u32>(self, 0x18) = (u32)g_env->categories(at<const char *>(self, 0x30));
    at<u32>(self, 0x1c) = 0;
    at<u32>(self, 0x20) = 0;
    at<u8 *>(self, 0xc) = owner;
    vcall<void>(owner, 0x30, self);
    return self;
}
}
