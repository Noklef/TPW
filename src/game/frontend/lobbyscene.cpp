#include "lobbyscene.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/string.h"
#include "../core/vcall.h"
#include "../math/x87.h"
#include "../model/holder.h"
#include "lobbymodel.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall Class702af0_AcquireModels(u8 *object) {
    if (at<u32>(object, 4) != 0)
        return;
    for (u8 *node = at<u8 *>(at<u8 *>(object, 0x58), 4); node;) {
        u8 *const item = at<u8 *>(node, 0);
        node = at<u8 *>(node, 4);
        if (at<u32>(item, 0xc) == 0) {
            RideUser_Acquire(at<u8 *>(item, 8));
            at<u32>(item, 0xc) = 1;
        }
    }
    at<u32>(object, 4) = 1;
}

void __fastcall Class702ca8_AcquireModels(u8 *object) {
    if (at<u32>(object, 0x10) != 0)
        return;
    RideUser_Acquire(at<u8 *>(object, 4));
    RideUser_Acquire(at<u8 *>(object, 8));
    at<u32>(object, 0x10) = 1;
}

u32 __fastcall Class702cb0_StartPendingPlay(u8 *object) {
    const u32 state = at<u32>(object, 0x14);
    if (state == 0)
        return 0;
    if (state == 2)
        LobbyModel_Play(at<u8 *>(at<u8 *>(object, 0xc), 8), nullptr, 1, 0);
    at<u32>(object, 0x14) = 0;
    return 1;
}

void __fastcall Class702cb0_StepCamera(u8 *object) {
    const float angle = at<float>(object, 0x18);
    const X87Reg s = X87_Sin(angle);
    const u8 *const model = at<const u8 *>(object, 0xc);
    for (u32 i = 0; i < 0xc; i += 4)  // the model's +0x3c to the target (+0x4c), then to the eye (+0x34)
        at<u32>(object, 0x4c + i) = at<u32>(model, 0x3c + i);
    for (u32 i = 0; i < 0xc; i += 4)
        at<u32>(object, 0x34 + i) = at<u32>(model, 0x3c + i);
    const X87Reg c = X87_Cos(angle);  // (also stored to a local nothing reads)
    const float radius = at<float>(object, 0x20);
    const float ex = (float)(s * radius + at<float>(object, 0x34));
    const float ey = (float)(X87Reg(at<float>(object, 0x24)) + at<float>(object, 0x38));
    const float ez = (float)(c * radius + at<float>(object, 0x3c));
    at<float>(object, 0x34) = ex;
    at<float>(object, 0x38) = ey;
    at<float>(object, 0x3c) = ez;

    // the position (+0x28) eased a tenth of the way to the eye, the look-at (+0x40) a fifth to the target, by the frame's scale
    const X87Reg near1 = X87Reg(at<float>(at<const u8 *>(reinterpret_cast<const void *>(0x00f82884), 0), 8)) * 0.100000001f;  // 0x702c7c
    float *const pos = &at<float>(object, 0x28);
    const float px = (float)((X87Reg(pos[3]) - pos[0]) * near1 + pos[0]);
    const float py = (float)((X87Reg(pos[4]) - pos[1]) * near1 + pos[1]);
    const float pz = (float)((X87Reg(pos[5]) - pos[2]) * near1 + pos[2]);
    pos[0] = px;
    pos[1] = py;
    pos[2] = pz;
    const X87Reg near2 = X87Reg(at<float>(at<const u8 *>(reinterpret_cast<const void *>(0x00f82884), 0), 8)) * 0.200000003f;  // 0x702ca4
    float *const look = &at<float>(object, 0x40);
    for (int i = 0; i < 3; i++)
        look[i] = (float)((X87Reg(look[i + 3]) - look[i]) * near2 + look[i]);

    u8 *camera = at<u8 *>(reinterpret_cast<const void *>(0x00f83778), 0);
    for (u32 i = 0; i < 0xc; i += 4)
        at<u32>(camera, 8 + i) = at<u32>(pos, i);
    camera = at<u8 *>(reinterpret_cast<const void *>(0x00f83778), 0);
    for (u32 i = 0; i < 0xc; i += 4)
        at<u32>(camera, 0x14 + i) = at<u32>(look, i);
}

u8 *__fastcall Class702af0_Ctor(u8 *object, void *, s32 count, const char *name, const char *name2, const u32 *first, const u32 *second, u32 a,
                                u32 b) {
    String_Ctor(reinterpret_cast<String *>(object + 0x10));
    String_Ctor(reinterpret_cast<String *>(object + 0x24));
    at<u32>(object, 0) = 0x00702af0;
    u32 *list = static_cast<u32 *>(GameHeap_Alloc(0x10));
    if (list) {
        list[1] = 0;
        list[2] = 0;
        list[3] = 0;
        list[0] = 0x00702aec;
    }
    at<u32>(object, 4) = 0;
    at<u32 *>(object, 0x58) = list;
    s32 share = (s32)((u32)count * *reinterpret_cast<u32 *>((uintptr_t)0x007858dc)) / 100;
    if (share < 1)
        share = 1;
    else if (share > count)
        share = count;
    at<s32>(object, 8) = share;
    at<u32>(object, 0xc) = 0;
    vcall<void>(object + 0x10, 0x14, name);
    vcall<void>(object + 0x24, 0x14, name2);
    at<u32>(object, 0x38) = first[0];
    at<u32>(object, 0x3c) = first[1];
    at<u32>(object, 0x40) = first[2];
    at<u32>(object, 0x44) = second[0];
    at<u32>(object, 0x48) = second[1];
    const u32 last = second[2];
    at<u32>(object, 0x50) = a;
    at<u32>(object, 0x4c) = last;
    at<u32>(object, 0x54) = b;
    vcall<void>(*reinterpret_cast<u8 **>((uintptr_t)0x00f83784), 0x18, object);
    return object;
}
}
