#include "typemodel.h"

#include <cstdint>

#include "../math/x87.h"
#include "../model/parts.h"
#include "../motion/animator.h"
#include "../world/rides.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const TypeModelEnv kPortEnv = {&Ride_Create, &Layer_Play, &Animator_Update, &Ride_Destroy};
const TypeModelEnv *g_env = &kPortEnv;

const float kNoHeight = 0.0f;   // 0x6fde74

// The bank and entry of the layers that the flags 0x20 << i play (0x74ca70, 0x74ca88).
const u32 kBanks[5] = {3, 10, 2, 9, 10};
const u32 kEntries[5] = {0, 0, 0, 0, 1};

u8 *Instance(u32 ride) { return global<u8 *>(0x007a4610 + ride * 4); }

// The part's kind in the model.
u32 kind(const u8 *holder, u32 part) { return at<u32>(at<const u8 *>(at<const u8 *>(at<const u8 *>(holder, 8), 4), 0x7c), part * 0x14 + 4); }

// The part's height in the instance: its object's +0x34; none: 0 (fld).
float height(u32 ride, u32 part) {
    const u8 *const object = at<const u8 *>(at<const u8 *>(at<const u8 *>(at<const u8 *>(Instance(ride), 8), 0x28), 4), part * 0x14 + 4);
    return object ? X87_Through(at<float>(object, 0x34)) : kNoHeight;
}

// The parts of the mask: each into one of two places, by a kind (with flag 8).
void places(const u8 *holder, u8 *block, u32 mask, u32 count, u32 wanted, u32 special, u32 plain) {
    for (u32 i = 0; i < count;) {
        const u32 part = Model_NthPart(holder, mask, ++i);
        if ((at<u32>(block, 0) & 8) && kind(holder, part) == wanted)
            at<u32>(block, special) = part;
        else
            at<u32>(block, plain) = part;
    }
}

void parts(const u8 *holder, u8 *block) {
    u32 count = Model_CountParts(holder, 0x1000000);
    if (count) {
        if (count > 1) {
            at<u32>(block, 0) |= 8;
            at<u32>(block, 0x40) = 2;
            at<u32>(block, 0x54) = 1;
        } else {
            at<u32>(block, 0x40) = 1;
        }
        places(holder, block, 0x1000000, count, 1, 0x48, 0x34);
    }
    if (Model_CountParts(holder, 0x800000)) {
        const u32 part = Model_NthPart(holder, 0x800000, 1);
        at<u32>(block, 0x2c) = part;
        if (at<u32>(block, 0) & 8) {
            at<u32>(block, 0x30) = 0;
            at<u32>(block, 0x44) = 1;
        } else {
            at<u32>(block, 0x30) = kind(holder, part) - 1;
        }
    }
    count = Model_CountParts(holder, 0x2000000);
    if (count) {
        places(holder, block, 0x2000000, count, 3, 0x4c, 0x38);
    } else {
        at<u32>(block, 0x3c) = 0;
        at<u32>(block, 0x50) = 0;
    }
    at<s32>(block, 0x18) = Model_CountParts(holder, 0x100000) ? (s32)Model_NthPart(holder, 0x100000, 1) : -1;
    if (Model_CountParts(holder, 0x80000)) {
        for (u32 nth = 1; nth <= 4; ++nth) {
            const u32 part = Model_NthPart(holder, 0x80000, nth);
            if (part == 0xffffffff) continue;
            const u32 k = kind(holder, part) - 1;
            if (k > 3) continue;
            at<u32>(block, 0x1c + k * 4) = part;
            at<u32>(block, 0) |= 0x400u << k;
        }
    }
    if (at<u32>(block, 0) & 8) {
        at<u32>(block, 0x3c) = 1;
        at<u32>(block, 0x50) = 3;
    } else {
        at<u32>(block, 0x3c) = Model_CountParts(holder, 0x400000);
    }
}

// Layer 0's rate by the value, between the block's +0xd0 and +0xd4; the instance updated.
void stance(u32 ride, const u8 *block, X87Reg x) {
    const float low = at<float>(block, 0xd0), high = at<float>(block, 0xd4);
    u8 *const layer = at<u8 *>(Instance(ride), 0x10);
    if (!(x.v >= (double)low)) x = X87Reg(low);
    if (x.v > (double)high) x = X87Reg(high);
    at<float>(layer, 0x20) = (float)(X87Reg(at<float>(layer, 0x1c)) * x);
    g_env->update(Instance(ride), reinterpret_cast<const u8 *>(1), 0);
}

}  // namespace

void TypeModel_SetEnv(const TypeModelEnv *env) { g_env = env ? env : &kPortEnv; }

extern "C" {

void __cdecl CoasterType_ReadModel(u32, u8 *block, u32 type) {
    u8 *holder = global<u8 *>(0x007ae6a8 + type * 4);
    if (!holder) return;
    if (Animator_HasMotion(&holder, 2, 0)) at<u32>(block, 0) |= 0x80;
    if (Animator_HasMotion(&holder, 3, 0)) at<u32>(block, 0) |= 0x20;
    if (Animator_HasMotion(&holder, 9, 0)) at<u32>(block, 0) |= 0x100;
    if (Animator_HasMotion(&holder, 10, 0)) at<u32>(block, 0) |= 0x40;
    if (Animator_HasMotion(&holder, 10, 1)) at<u32>(block, 0) |= 0x200;
    parts(holder, block);

    global<u32>(0x00791054) = at<u32>(block, 0xb8);
    global<u32>(0x00791050) = at<u32>(block, 0xb4);
    global<u32>(0x00791ce8) = 0;
    global<u32>(0x00791cec) = 0;
    global<u32>(0x00791cf0) = 0;
    global<u32>(0x00791cf4) = 0x00791050;
    const u32 ride = g_env->createRide(type, 0x3fa, reinterpret_cast<const u32 *>(0x00791ce8), 0, 0, 0, 0, 0, -1, 0);

    u8 *const instance = Instance(ride);
    u8 *layer = at<u8 *>(instance, 0x10);
    for (u32 i = 0; i < 5; ++i, layer += 0x38) {
        if ((at<u32>(block, 0) & (0x20u << i)) && (at<u8>(instance, (kBanks[i] + 2) * 12) & 3)) {
            g_env->play(instance, kBanks[i], kEntries[i], layer, 0, 1.0f);
        }
    }
    at<u32>(instance, 4) |= 0x1000;

    stance(ride, block, X87Reg(at<float>(block, 0xdc)));
    const float y = height(ride, at<u32>(block, 0x2c));
    at<float>(block, 0xbc) = y;
    const float top = X87_Through(at<float>(block, 0x68));
    at<float>(block, 0x68) = (double)y >= (double)top ? y : top;
    const u32 other = at<u32>(block, 0x18);
    at<float>(block, 0xc4) = other != 0xffffffff ? (float)(X87Reg(height(ride, other)) - X87Reg(y)) : 0.0f;
    stance(ride, block, X87Reg(at<float>(block, 0xd8)) + at<float>(block, 0xdc));
    at<float>(block, 0xc0) = height(ride, at<u32>(block, 0x2c));
    g_env->destroyRide(ride);
}
}
