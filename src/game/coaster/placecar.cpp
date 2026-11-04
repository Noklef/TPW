#include "placecar.h"

#include <cstdint>

#include "../core/addrcalls.h"
#include "../files/wrapsources.h"
#include "../math/x87.h"
#include "../world/money.h"
#include "../world/world.h"
#include "cars.h"
#include "edittools.h"
#include "registry.h"
#include "train.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const CoasterPlaceCarEnv kPorts = {&CoasterLink_AddEntry,
                                   &CoasterLink_AddCar,
                                   &Car_LengthFromNode,
                                   &Pylon_FindBraceLevel,
                                   &Car_SetChannelRates,
                                   &Car_TravelCost,
                                   &Coaster_ResumeRunning,
                                   &CoasterEdit_SetTools,
                                   reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x00498780),
                                   &CoasterEdit_SwitchToTool1,
                                   &CoasterEdit_RefreshTools,
                                   &CoasterEdit_SetBounds,
                                   &GameTable7cfb90_Get1da726_5195d0,
                                   reinterpret_cast<u32(__fastcall *)(u32)>(&Source_Field0c),
                                   reinterpret_cast<void(__fastcall *)(u32, void *, s32)>(&Money_Debit)};
const CoasterPlaceCarEnv *g_env = &kPorts;

float AsFloat(u32 bits) { return *reinterpret_cast<const float *>(&bits); }

u8 *FindNode(s32 square) {
    for (u8 *link = global<u8 *>(0x00790fe0); link; link = at<u8 *>(link, 0x18)) {
        for (u8 *node = at<u8 *>(link, 0x4c); node; node = at<u8 *>(node, 0x28)) {
            if (!(at<u8>(node, 4) & 5) && at<s32>(node, 8) == square)
                return node;
        }
    }
    return nullptr;
}

}  // namespace

void CoasterPlaceCar_SetEnv(const CoasterPlaceCarEnv *env) { g_env = env ? env : &kPorts; }

extern "C" u32 __cdecl CoasterEdit_PlaceCar(s32 x, s32 y, u32 height, u8 **outNode, u32 *outCar, u32 kind, u32 claim) {
    u8 *const coaster = global<u8 *>(0x00790fec);
    u8 *const edit = coaster + 0x3c;
    u8 *const range = at<u8 *>(coaster, 4) + (global<u8>(0x0079c658) & 0x40 ? 0x1c8 : 0xa8);
    const s32 square = at<s32>(at<u8 *>(global<u8 *>(0x007a0854), 0x6c), 0x18) * y + x;
    const u32 given = height;
    u8 *node = FindNode(square);
    if (node) {
        if (at<u8>(edit, 0) & 2) {
            const u32 current = at<u32>(global<u8 *>(0x00790fec), 0x64);
            const u32 count = at<u32>(node, 0x30);
            for (u32 k = 0; k < count; k++) {
                if (at<u32>(node, 0x34 + k * 4) != current)
                    continue;
                g_env->resumeRunning(global<u8 *>(0x00790fec));
                if (outNode)
                    *outNode = node;
                if (outCar)
                    *outCar = current;
                g_env->setTools(0x10);
                g_env->afterResume();
                return 1;
            }
        }
        if (!given && at<u32>(at<u8 *>(node, at<u32>(node, 0x30) * 4 + 0x30), 0) == at<u32>(edit, 8) - 1) {
            if (outNode)
                *outNode = node;
            return 0;
        }
    } else {
        node = g_env->addEntry(x, y, 0, height, static_cast<u32>(reinterpret_cast<uintptr_t>(range)), nullptr);
    }
    if (!given) {
        g_env->lengthFromNode(node, range, at<u8 *>(edit, 0x24), at<const u8 *>(edit, 0x20), global<u32>(0x0079c664) == 2, 0, &height);
        g_env->findBraceLevel(AsFloat(height), node, reinterpret_cast<u32 *>(&y), reinterpret_cast<float *>(&height), 0);
    }
    if ((u8)kind == 0xff)
        kind = (kind & ~0xffu) | (u8)y;
    u8 *const car = g_env->addCar(node, at<u32>(node, 0x1c), at<u32>(node, 0x24), AsFloat(height), given != 0, range, 2 - global<u32>(0x0079c664), kind & 0xff, claim);
    g_env->setChannelRates(at<u32>(global<u8 *>(0x00790fec), 4), car);
    if (!given) {
        u8 *const editing = global<u8 *>(0x00790fec);
        at<u32>(editing, 0) |= 0x1001;
        const float atX = (float)(X87Reg(at<float>(node, 0x1c)) - global<float>(0x006fdea4));
        const float atZ = (float)(X87Reg(at<float>(node, 0x24)) - global<float>(0x006fdea4));
        const float cost = (float)g_env->travelCost(editing, 1, car, &atX, &atZ, nullptr, nullptr);
        const u32 money = g_env->money(global<u8 *>(0x007cf83c));
        g_env->sourceField(money);
        if (!global<u32>(0x00785a2c))
            g_env->debit(money, nullptr, (s32)(long long)(double)cost);
    }
    if (global<u8>(0x0079c658) & 0x40) {
        const u32 left = --global<u32>(0x0079c664);
        if (!left) {
            global<u32>(0x0079c658) &= ~0x40u;
            if (!given && !global<u32>(0x00785a2c))
                g_env->switchToTool1();
        } else if (left == 1 && !given && !global<u32>(0x00785a2c)) {
            g_env->refreshTools();
        }
    }
    g_env->setBounds();
    if (outNode)
        *outNode = node;
    if (outCar)
        *outCar = static_cast<u32>(reinterpret_cast<uintptr_t>(car));
    return 1;
}

namespace {
AddrCalls g_partCalls;
}  // namespace

void CoasterPlaceCar_SetPartCalls(const AddrCall *table, u32 count) { g_partCalls.Set(table, count); }

extern "C" u32 __cdecl CoasterEdit_PlaceCarPart(s32 x, s32 y, u32 height, u32 *out) {
    u32 node = height, car = (u32)y;   // (the original uses its own argument slots for the outs)
    const u32 result = g_partCalls.Cd(0x00435020, x, y, height, &node, &car, 0xffu, 0u);   // CoasterEdit_PlaceCar
    if (out) *out = node ? at<u32>((u8 *)(uintptr_t)car, 0x70) : 0;
    return result;
}
