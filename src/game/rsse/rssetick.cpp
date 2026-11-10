#include "rssetick.h"

#include <cstdint>

#include "../coaster/cars.h"
#include "../coaster/ridebounds.h"
#include "../core/soundset.h"
#include "../math/x87.h"
#include "../motion/animator.h"
#include "../render/particles.h"
#include "../world/flyingrides.h"
#include "../world/trackrides.h"
#include "rsseexec.h"
#include "scriptkill.h"
#include "../world/ridelist.h"
#include "rsse.h"

namespace {

const RsseTickEnv kPortEnv = {&RSSE_ExecuteInstruction, &Animator_SetQueuedSpeed, &FlyingRide_SetCap, &FlyingRide_SetTenth, &Coaster_SetMixAt, &TrakRide_SetPerformance,
                              &Ride_PartAxes, &ParticleSystem_SetVectorA, &ParticleSystem_SetScaled, &Sound_SetPosition, &RSSE_KillScript};
const RsseTickEnv *g_env = &kPortEnv;

template <typename T>
T &global(uintptr_t address) {
    return *reinterpret_cast<T *>(address);
}
template <typename T>
T &at(void *base, u32 offset) {
    return *reinterpret_cast<T *>(static_cast<u8 *>(base) + offset);
}

s32 ftol(X87Reg x) { return (s32)(long long)x.v; }

u8 *ride_of(const RsseContext *ctx) { return global<u8 *>(0x007a4610 + (uintptr_t)(u32)(ctx->owner * 4)); }

// The node's part's position, each __ftol'd (a null part: 0, 0, 0).
struct Position {
    s32 x, y, z;
};
Position part_position(const RsseContext *ctx, u8 *node) {
    u8 *const table = at<u8 *>(at<u8 *>(at<u8 *>(ride_of(ctx), 8), 0x28), 4);
    u8 *const part = at<u8 *>(table, (u32)(at<s32>(node, 0x14) * 0x14) + 4);
    if (part == nullptr) return {0, 0, 0};
    return {ftol(X87Reg(at<float>(part, 0x30))), ftol(X87Reg(at<float>(part, 0x34))), ftol(X87Reg(at<float>(part, 0x38)))};
}

void run(RsseContext *ctx) {
    if (ctx->code != nullptr) {
        RSSE_CritLock = 0;
        ctx->budget_left = ctx->budget;
        if (ctx->budget > 0) {
            while (ctx->pc >= 0) {
                g_env->execute(ctx);
                if (RSSE_CritLock == 0) --ctx->budget_left;
                if (ctx->budget_left <= 0) break;
            }
        }
    }
    if (ctx->owner != 0) {
        const s32 speed = ctx->speed, scale = at<s16>(ctx, 0xe4);
        const float value = (float)(X87_SubR(X87Reg((double)speed) * -0.01f, 0.5f) * X87Reg((double)scale) * gRsseVm_700fd4);
        g_env->setSpeed(ride_of(ctx), value);
    }
    if (ctx->sound != 0) {
        if ((s32)ctx->var_count > 3) g_env->setCap(ctx->sound, ctx->vars[3] * 1000);
        g_env->setTenth(ctx->sound, ctx->speed);
    }
    if (at<u32>(ctx, 0xe0) != 0) g_env->setMix(at<u32>(ctx, 0xe0), (u32)(s32)ctx->speed);
    const u16 thing = at<u16>(ctx, 0xac);
    if (thing != 0) {
        u8 *const object = global<u8 *>(0x007cfb90 + (uintptr_t)thing * 20);
        if (object != nullptr && at<u32>(object, 0x28) != 0) g_env->setPerformance(at<u32>(object, 0x28), ctx->speed);
    }
    if (ctx->child_id != 0) {
        RsseContext *child = nullptr;
        if (gRsseSystem.initialised != 0) {
            child = gRsseSystem.scriptList;
            while (child != nullptr && child->id != ctx->child_id) child = child->next;
        }
        child->speed = ctx->speed;   // sic: none found writes at 0xc0
    }
}

void node_step(RsseContext *ctx, u8 *node) {
    const s32 kind = at<s32>(node, 8);
    if (kind == 1) {
        const Position p = part_position(ctx, node);
        g_env->setVector(at<u32>(node, 0xc), (s32)((u32)p.x << 10), (s32)((u32)p.y << 10), (s32)((u32)p.z << 10));
    } else if (kind == 2) {
        const Position p = part_position(ctx, node);
        float dir[3];
        g_env->partAxes(ctx->owner, &at<u32>(node, 0x14), dir, nullptr);
        g_env->setVector(at<u32>(node, 0xc), (s32)((u32)p.x << 10), (s32)((u32)p.y << 10), (s32)((u32)p.z << 10));
        const s32 jz = ftol(X87Reg(dir[2]) * gRidelist_700fdc);
        const s32 jy = ftol(X87Reg(dir[1]) * gRidelist_700fdc);
        const s32 jx = ftol(X87Reg(dir[0]) * gRidelist_700fdc);
        g_env->setScaled(at<u32>(node, 0xc), jx, jy, jz);
    } else if (kind >= 3 && kind <= 10 && (((u32)ctx->id ^ gRsseSystem.tickCounter) & 7) == 0) {
        const Position p = part_position(ctx, node);
        g_env->setPosition(at<u32>(node, 0xc), p.x, p.y, p.z);
    }
}

}  // namespace

void RsseTick_SetEnv(const RsseTickEnv *env) { g_env = env ? env : &kPortEnv; }

u32 __cdecl RSSE_Tick() {
    if (gRsseSystem.initialised == 0) return reinterpret_cast<u32 &>(gRsse_8791b4);
    ++gRsseSystem.tickCounter;
    RsseContext *ctx = gRsseSystem.scriptList;
    while (ctx != nullptr) {
        if (ctx->turbo != 0 || (((u32)ctx->id ^ gRsseSystem.tickCounter) & 7) == 0) run(ctx);
        for (u8 *node = static_cast<u8 *>(ctx->objects); node != nullptr; node = at<u8 *>(node, 0)) {
            if (at<s32>(node, 0x10) >= 0) node_step(ctx, node);
        }
        if (ctx->pc < 0) {
            RsseContext *const next = ctx->next;
            g_env->kill(ctx->id, 0);
            ctx = next;
        } else {
            ctx = ctx->next;
        }
    }
    return reinterpret_cast<u32 &>(gRsse_8791b4);
}
