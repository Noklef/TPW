#include "rsseexec.h"

#include <cstdint>
#include <cstring>

#include "../coaster/cars.h"
#include "../coaster/queue.h"
#include "../coaster/registry.h"
#include "../coaster/ridebounds.h"
#include "../core/crt.h"
#include "../core/gameclock.h"
#include "../core/gameheap.h"
#include "../core/soundplay.h"
#include "../core/soundset.h"
#include "../files/file.h"
#include "../math/x87.h"
#include "../model/holder.h"
#include "../model/parts.h"
#include "../model/partstate.h"
#include "../motion/animator.h"
#include "../render/lightsource.h"
#include "../render/particles.h"
#include "../world/cellrun.h"
#include "../world/flyinglaunch.h"
#include "../world/flyingrides.h"
#include "../world/random.h"
#include "../world/ridelist.h"
#include "../world/things.h"
#include "../world/trackrides.h"
#include "../world/trakridecars.h"
#include "rsse.h"
#include "scream.h"
#include "scriptfile.h"
#include "scriptkill.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

s32 __cdecl PortTime(s32 *t) { return Crt().time(t); }
const u8 *__cdecl PortLocalTime(const s32 *t) { return reinterpret_cast<const u8 *(__cdecl *)(const s32 *)>((uintptr_t)0x0067d8e0)(t); }

const RsseExecEnv kPortEnv = {
    &Clocks_Now, &Mem_AllocLogged, &Mem_GlobalFree, &Rsse_Trigger, &Rsse_TriggerNode, &ParticleSystem_SetAge, &Sound_Forward6, &Sound_Stop, &Sound_SetParam, &Layer_ClearQueue, &Animator_Play, &Layer_GetFlags,
    &Ride_EntryPosition, &Model_FindPartByKey4, &PartStates_Activate, &PartStates_Release, &Ride_SlotKey, &MeshRef_ClearFlag10000000, &MeshRef_SetFlag10000000, &LightSource_SetColourScaled, &LightSource_SetColour,
    &Ride_Centre, &Rsse_GetVector, &RSSE_StartScream, &RSSE_SetScreamVolume, &KidSound_Play, &KidSound_PlayGraded, &Sound_PlayAt, &Sound_Set803ad0, &FlyingRide_Create, &FlyingRide_Free, &FlyingRide_AddToSlot,
    &FlyingRide_StepInSlot, &FlyingRide_Set4, &FlyingRide_SetCap, &FlyingRide_SetMode, &FlyingRide_Get3c, &FlyingRide_Take3c, &FlyingRide_Set44, &FlyingRide_SetOn, &FlyingRide_FirstSlotReady, &FlyingRide_Set20,
    &FlyingRide_Set24, &FlyingRide_Busy, &TrakRide_AddNextCar, &TrakRide_PopNode, &TrakRide_StartBump, &TrakRide_AddCar, &TrakRide_Close, &TrakRide_Open, &TrakRide_Break, &TrakRide_Fixed, &TrakRide_MarkWornOut,
    &TrakRide_RemoveCars, &TrakRide_Field5c, &TrakRide_LoadCar, &TrakRide_SetField4, &TrakRide_RemoveCar, &TrackRun_AnimateA, &TrackRun_AnimateB, &CoasterLink_Find, &Coaster_QueuePush, &Coaster_QueueRoom,
    &Coaster_BoardingPop, &Coaster_SetModeAt, &Coaster_SetMixAt, &Coaster_StepStateAt, &Coaster_SetRiders, &RSSE_LoadScriptFile, &RSSE_KillScript, &Rsse_StartWalker, &Rsse_WalkerBack, &Rsse_TakeFinished,
    &RSSE_StartTimer, &Ride_SetEffect, &Rides_FindById, &Ride_PartPlace, &PortTime, &PortLocalTime};
const RsseExecEnv *g_env = &kPortEnv;

const u32 kMask = 0xff000000, kVar = 0x40000000, kLabel = 0x20000000, kString = 0x10000000;

bool is_var(u32 word) { return (word & kMask) == kVar; }
// The variable a word names (index * 4 wrapping, which takes the tag off).
s32 &var(RsseContext *ctx, u32 word) { return at<s32>(ctx->vars, word * 4); }
void set_var(RsseContext *ctx, u32 word, s32 value) {
    if (is_var(word)) var(ctx, word) = value;
}
void set_result(RsseContext *ctx, u32 word, s32 value) {
    ctx->result = value;
    set_var(ctx, word, value);
}

u32 fetch(RsseContext *ctx) { return RSSE_FetchWord(ctx); }
s32 value(RsseContext *ctx) { return RSSE_OperandValue(ctx, RSSE_FetchWord(ctx)); }

u8 *ride(const RsseContext *ctx) { return global<u8 *>(0x007a4610 + (u32)ctx->owner * 4); }
u32 now() { return g_env->clock(reinterpret_cast<u8 *>((uintptr_t)0x00785970)); }
s32 ftol(X87Reg x) { return (s32)(long long)x.v; }
u32 float_bits(float f) {
    u32 b;
    std::memcpy(&b, &f, 4);
    return b;
}
// fild n; fmul 0.01 (0x700fe0); the float stored.
float hundredths(s32 n) { return (float)X87Reg(X87_MulIntDouble(n, (double)0.01f)); }
s32 rand_half() { return (s32)((u32)World_Rand(global<u8 *>(0x007cf83c)) >> 1); }

// Animator_Play with an owner (0 without).
s32 play(RsseContext *ctx, u32 bank, u32 entry, u32 mode, float speed, u32 index) { return ctx->owner != 0 ? (s32)g_env->play(ride(ctx), bank, entry, mode, speed, index) : 0; }
// Its answer less 300, at least 300, as the result and the word's variable.
void played(RsseContext *ctx, s32 answer, u32 &word) {
    ctx->result = (s32)((u32)answer - 300);
    if (ctx->result < 300) ctx->result = 300;
    if (is_var(word)) {
        var(ctx, word) = ctx->result;
        word ^= kVar;
    }
}
u32 end_time(s32 n, float factor) {
    const u32 clock = now();
    return (u32)ftol(X87Reg((double)n) / factor) + clock;
}
void rate_1000(RsseContext *ctx) { at<u16>(ctx, 0xe4) = 1000; }
void end_slice(RsseContext *ctx, s32 back) {
    ctx->pc -= back;
    ctx->budget_left = 0;
}
// A wait in +0xa0 under way: reached clears it, else pc back 3 and the slice ended.
void wait_on(RsseContext *ctx) {
    const u32 clock = now();
    if (ctx->wait_deadline <= clock) ctx->wait_deadline = 0;
    else end_slice(ctx, 3);
}
// The second step of 0x13 and 0x1a.
void two_step_wait(RsseContext *ctx, u32 word, u32 index) {
    if (ctx->owner != 0) g_env->getFlags(ride(ctx), &word, nullptr, index);
    if (word + 1 == at<u32>(ctx, 0xbc)) at<u32>(ctx, 0xbc) = 0;
    else end_slice(ctx, 4);
}
// 0x14, 0x1b.
void anim_state(RsseContext *ctx, u32 word, u32 index) {
    const u32 flags = ctx->owner != 0 ? g_env->getFlags(ride(ctx), reinterpret_cast<u32 *>(&ctx->result), nullptr, index) : 0;
    if (flags & 4) ctx->result = -1;
    set_var(ctx, word, ctx->result);
}

void jump(RsseContext *ctx, u32 word) { ctx->pc = (word & kMask) == kLabel ? (s32)(word ^ kLabel) : RSSE_PC_DEAD; }
void push(RsseContext *ctx, s32 v) {
    if (ctx->stack != nullptr && ctx->sp >= 0 && ctx->sp < (s32)ctx->stack_size) {
        ctx->stack[ctx->sp] = v;
        ctx->sp--;
    } else {
        ctx->pc = RSSE_PC_DEAD;
    }
}
s32 pop(RsseContext *ctx) {
    if (ctx->stack != nullptr && ctx->sp >= -1 && ctx->sp < (s32)(ctx->stack_size - 1)) {
        ctx->sp++;
        return ctx->stack[ctx->sp];
    }
    ctx->pc = RSSE_PC_DEAD;
    return 0;
}

// 0x2c and 0x2d once waiting: the operand skipped.
void wait_operand(RsseContext *ctx) {
    fetch(ctx);
    const u32 clock = now();
    if (ctx->wait_deadline <= clock) ctx->wait_deadline = 0;
    else end_slice(ctx, 2);
}

u8 *new_node(RsseContext *ctx) {
    u8 *const node = reinterpret_cast<u8 *>((uintptr_t)g_env->alloc(0x1c));
    at<void *>(node, 0) = ctx->objects;
    at<u32>(node, 4) = 0;
    if (ctx->objects != nullptr) at<u8 *>(ctx->objects, 4) = node;
    ctx->objects = node;
    global<s32>(0x008791b4)++;
    at<s32>(node, 8) = value(ctx);
    at<s32>(node, 0x10) = value(ctx);
    return node;
}
// 0xa, 0xb: the nodes of a value stopped, unlinked and freed.
void remove_nodes(RsseContext *ctx, s32 key, void(__cdecl *stop)(u32)) {
    u8 *node = static_cast<u8 *>(ctx->objects);
    while (node != nullptr) {
        u8 *const next = at<u8 *>(node, 0);
        if (key == at<s32>(node, 0x18)) {
            const u32 kind = at<u32>(node, 8);
            if (kind == 1 || kind == 2) g_env->setAge(at<u32>(node, 0xc), 0xfffffffe);
            else if (kind >= 3 && kind <= 10) stop(at<u32>(node, 0xc));
            if (at<u8 *>(node, 4) != nullptr) at<u8 *>(at<u8 *>(node, 4), 0) = at<u8 *>(node, 0);
            if (at<u8 *>(node, 0) != nullptr) at<u8 *>(at<u8 *>(node, 0), 4) = at<u8 *>(node, 4);
            if (node == ctx->objects) ctx->objects = at<u8 *>(node, 0);
            g_env->free(node);
            global<s32>(0x008791b4)--;
        }
        node = next;
    }
}

RsseContext *find_script(s32 id) {
    if (global<u32>(0x008791a0) == 0) return nullptr;
    RsseContext *p = global<RsseContext *>(0x008791b0);
    while (p != nullptr && p->id != id) p = p->next;
    return p;
}
// 0x3f, 0x40.
u32 load_child(RsseContext *ctx, u32 word) {
    char name[256];
    std::strcpy(name, at<const char *>(ctx, 0x38));
    std::strcat(name, at<const char *>(ctx, 0x34) + (word ^ kString));
    return g_env->loadScript(name, 0, 0);
}
// 0x42, 0x44.
void write_other(RsseContext *ctx, s32 id, s32 index, s32 v) {
    RsseContext *const p = find_script(id);
    if (index < (s32)p->var_count) {
        p->vars[index] = v;
        ctx->result = v;
    }
}
// 0x43, 0x45.
void read_other(RsseContext *ctx, s32 id, s32 index, u32 word) {
    RsseContext *const p = find_script(id);
    if (index < (s32)p->var_count) {
        const s32 v = p->vars[index];
        ctx->result = v;
        var(ctx, word) = v;
    }
}

// 0x49, 0x4a: the first rider (past its time when timed) on the beat out.
void rider_out(RsseContext *ctx, u32 word, bool timed) {
    const u32 first = now();
    s32 out = 0;
    for (s32 i = 0; i < (s32)ctx->rider_capacity; ++i) {
        RsseRiderSlot &slot = ctx->riders[i];
        if (slot.guest == 0) continue;
        const u32 clock = now();
        if (timed && !((s32)slot.release_time < (s32)first)) continue;
        if ((s32)(clock - slot.start_time) % 1000 / 200 == 0) {
            at<s16>(ctx, 0x6c)--;
            out = slot.guest;
            slot.guest = 0;
            break;
        }
    }
    set_result(ctx, word, out);
}

u32 slot_key(const RsseContext *ctx, s32 part) { return g_env->slotKey((u32)ctx->owner, g_env->findPart(ride(ctx), 0x20000, (u32)part)); }
u8 *const *mesh(u32 key) { return reinterpret_cast<u8 *const *>((uintptr_t)key); }

// 0x5a: a name's match in a script.
bool name_matches(const RsseContext *p, const char *name) {
    const char *const base = at<const char *>(p, 0x34);
    const s32 at_name = (s32)(uintptr_t)p->name;
    return base != nullptr && at_name >= 0 && std::strcmp(name, base + at_name) == 0;
}
void pick_script(RsseContext *ctx, u32 word, u32 out) {
    ctx->result = 0;
    if ((word & kMask) != kString) return;
    const char *const base = at<const char *>(ctx, 0x34);
    if (base == nullptr) return;
    const char *const name = base + (word ^ kString);
    s32 count = 0;
    for (const RsseContext *p = global<RsseContext *>(0x008791b0); p != nullptr; p = p->next) {
        if (name_matches(p, name)) count++;
    }
    if (count == 0) return;
    s32 pick = rand_half() % count;
    if (pick < 0) pick = -pick;
    s32 seen = 0;
    for (const RsseContext *p = global<RsseContext *>(0x008791b0); p != nullptr; p = p->next) {
        if (name_matches(p, name) && ++seen == pick + 1) {
            ctx->result = p->id;
            if ((out & kVar) == kVar) at<s32>(ctx->vars, out * 4) = p->id;
        }
    }
}

void time_field(RsseContext *ctx, u32 offset, s32 add) {
    s32 t;
    g_env->time(&t);
    ctx->result = at<s32>(g_env->localTime(&t), offset) + add;
    set_var(ctx, fetch(ctx), ctx->result);
}

void flying(RsseContext *ctx) {
    const s32 sound = ctx->sound;
    switch (fetch(ctx)) {
    case 1: {
        const s32 quick = value(ctx);
        u32 part = g_env->findPart(ride(ctx), 0x800, 99);
        if (part == 0xffffffff) return;
        float place[3];
        g_env->entryPosition((u32)ctx->owner, &part, place);
        const u32 turn = (u32)((s32)(at<u32>(at<u8 *>(ride(ctx), 8), 0x10) << 12) / 360);
        const u32 z = (u32)ftol(X87Reg(place[2])), y = (u32)ftol(X87Reg(place[1])), x = (u32)ftol(X87Reg(place[0]));
        ctx->sound = g_env->flyCreate((x * 15 & 0xffff0000) | at<u16>(ctx, 0xac), (u32)ctx->id, x * 300, y * 300, z * 300, turn, (u32)quick);
        return;
    }
    case 2: g_env->flyFree(sound); return;
    case 3: {
        const u32 word = fetch(ctx);
        if (is_var(word)) ctx->result = (s32)g_env->flyAddToSlot(sound, (u32)var(ctx, word));
        return;
    }
    case 4: {
        const u32 word = fetch(ctx);
        if (is_var(word)) {
            var(ctx, word) = (s32)g_env->flyStep(sound);
            ctx->result = var(ctx, word);
        }
        return;
    }
    case 5: g_env->flySet4(sound, (u32)value(ctx)); return;
    case 8: g_env->flyCap(sound, (s32)((u32)value(ctx) * 1000)); return;
    case 9: g_env->flyMode(sound, value(ctx)); return;
    case 0xa:
        value(ctx);
        ctx->result = (s32)g_env->flyGet3c(sound);
        return;
    case 0xb:
        value(ctx);
        ctx->result = (s32)g_env->flyTake3c(sound);
        return;
    case 0xc: g_env->flySet44(sound, (u32)value(ctx)); return;
    case 0xe: g_env->flyOn(sound, (u32)value(ctx)); return;
    case 0xf:
        value(ctx);
        ctx->result = (s32)g_env->flyReady(sound);
        return;
    case 0x10: {
        const u32 word = fetch(ctx);
        if (is_var(word)) {
            var(ctx, word) = (s32)g_env->flyBusy(sound);
            ctx->result = var(ctx, word);
        }
        return;
    }
    case 0x11: g_env->flySet20(sound, (u32)value(ctx)); return;
    case 0x12: g_env->flySet24(sound, (u32)value(ctx)); return;
    default: return;
    }
}

void track(RsseContext *ctx) {
    const u8 *const thing = Thing_ByHandle(at<u16>(ctx, 0xac));
    const u32 sub = fetch(ctx);
    if (sub == 0 || sub > 0x11 || sub == 0xf) return;
    if (sub == 5) {
        fetch(ctx);
        ctx->result = at<s32>(thing, 0x2c);
        return;
    }
    if (sub == 0x11) {
        const s32 v = value(ctx);
        if ((s32)at<s16>(ctx, 0xe6) != v) {
            at<s16>(ctx, 0xe6) = (s16)v;
            const u32 start = v != 0 ? 0 : 1;
            g_env->animateA(thing, start);
            g_env->animateB(thing, start);
        }
        return;
    }
    const u32 word = fetch(ctx);
    if (sub <= 2 && !is_var(word)) return;
    const u32 handle = at<u32>(thing, 0x28);
    switch (sub) {
    case 1: ctx->result = (s32)g_env->trakAddNext(handle, (u32)var(ctx, word)); return;
    case 2:
        var(ctx, word) = (s32)g_env->trakPop(handle);
        ctx->result = var(ctx, word);
        return;
    case 3: g_env->trakBump(handle); return;
    case 4: ctx->result = (s32)(uintptr_t)g_env->trakAddCar(handle, 0); return;
    case 6: g_env->trakClose(handle); return;
    case 7: g_env->trakOpen(handle); return;
    case 8:
        if (RSSE_OperandValue(ctx, word) != 0) g_env->trakBreak(handle);
        else g_env->trakFixed(handle);
        return;
    case 9:
        if (RSSE_OperandValue(ctx, word) != 0) g_env->trakWorn(handle);
        else g_env->trakFixed(handle);
        return;
    case 0xa: g_env->trakRemoveCars(handle); return;
    case 0xb: set_result(ctx, word, (s32)g_env->trakField5c(handle)); return;
    case 0xc: ctx->result = (s32)g_env->trakLoadCar(handle); return;
    case 0xd: {
        const s32 v = RSSE_OperandValue(ctx, word);
        g_env->trakSetField4(handle, (u32)v * 0x1e);
        ctx->result = v;
        return;
    }
    case 0xe: {
        const s32 v = RSSE_OperandValue(ctx, word);
        g_env->trakSetField4(handle, 0u - (u32)v);
        ctx->result = v;
        return;
    }
    default: ctx->result = (s32)g_env->trakRemoveCar(handle, 1); return;   // (0x10)
    }
}

void coaster(RsseContext *ctx) {
    const u32 key = at<u32>(ctx, 0xe0);
    switch (fetch(ctx)) {
    case 1: g_env->queuePush(key, (u32)value(ctx)); return;
    case 2: {
        const u32 word = fetch(ctx);
        set_result(ctx, word, g_env->queueRoom(key));
        return;
    }
    case 3: {
        const u32 word = fetch(ctx);
        set_result(ctx, word, (s32)g_env->boardingPop(key));
        return;
    }
    case 4: g_env->setModeAt(key, (u32)value(ctx)); return;
    case 5: g_env->stepStateAt(key, (u32)value(ctx)); return;
    case 6: g_env->setRiders(key, value(ctx)); return;
    case 7: value(ctx); return;
    case 8: {
        fetch(ctx);
        const u32 link = g_env->linkFind((u32)ctx->id);
        at<u32>(ctx, 0xe0) = link;
        if (link != 0) g_env->setMixAt(link, (u32)(s32)ctx->speed);
        return;
    }
    default: return;
    }
}

}  // namespace

void RsseExec_SetEnv(const RsseExecEnv *env) { g_env = env ? env : &kPortEnv; }

extern "C" {

void __cdecl RSSE_ExecuteInstruction(RsseContext *ctx) {
    const float factor = (float)X87_SubR(X87Reg((double)ctx->speed) * -0.01f, 0.5f);
    const u32 instruction = fetch(ctx);
    if (ctx->pc < 0) return;
    if ((instruction & kMask) != 0x80000000 || (instruction ^ 0x80000000) > 0x69) {
        ctx->pc = RSSE_PC_DEAD;
        return;
    }
    switch (instruction ^ 0x80000000) {
    case 1: global<u32>(0x0087919c) = 1; return;
    case 2:
        global<u32>(0x0087919c) = 0;
        ctx->budget_left = 0;
        return;
    case 3: {
        const u32 word = fetch(ctx);
        if (is_var(word)) {
            var(ctx, word) = value(ctx);
            ctx->result = var(ctx, word);
        }
        return;
    }
    case 4: ctx->result = (s32)fetch(ctx); return;
    case 5: {
        const u32 word = fetch(ctx);
        const u32 left = fetch(ctx), right = fetch(ctx);
        const s32 a = RSSE_OperandValue(ctx, left);
        set_result(ctx, word, (s32)((u32)a - (u32)RSSE_OperandValue(ctx, right)));
        return;
    }
    case 6: ctx->budget_left = 0; return;
    case 7: {
        const u32 word = fetch(ctx);
        set_result(ctx, word, (s32)now());
        return;
    }
    case 8: {
        u8 *const node = new_node(ctx);
        const s32 what = value(ctx), v = value(ctx);
        g_env->triggerNode(reinterpret_cast<u8 *>(ctx), node, (u32)what, 1000, (u32)v);
        return;
    }
    case 9: {
        u8 *const node = new_node(ctx);
        const s32 what = value(ctx), extra = value(ctx), v = value(ctx);
        g_env->triggerNode(reinterpret_cast<u8 *>(ctx), node, (u32)what, (u32)extra, (u32)v);
        return;
    }
    case 0xa: {
        const s32 key = value(ctx);
        remove_nodes(ctx, key, g_env->soundForward6);
        return;
    }
    case 0xb: {
        const s32 key = value(ctx);
        remove_nodes(ctx, key, g_env->soundStop);
        return;
    }
    case 0xc: {
        const s32 key = value(ctx), a = value(ctx), b = value(ctx);
        for (u8 *node = static_cast<u8 *>(ctx->objects); node != nullptr; node = at<u8 *>(node, 0)) {
            const u32 kind = at<u32>(node, 8);
            if (key == at<s32>(node, 0x18) && kind >= 3 && kind <= 10) at<u32>(node, 0xc) = g_env->soundSetParam(at<u32>(node, 0xc), (u32)a, (u32)b);
        }
        return;
    }
    case 0xd: {
        const s32 kind = value(ctx), key = value(ctx), what = value(ctx);
        g_env->trigger(reinterpret_cast<u8 *>(ctx), kind, key, (u32)what, 1000);
        return;
    }
    case 0xe: {
        const s32 kind = value(ctx), key = value(ctx), what = value(ctx), extra = value(ctx);
        g_env->trigger(reinterpret_cast<u8 *>(ctx), kind, key, (u32)what, (u32)extra);
        return;
    }
    case 0xf:
        if (ctx->owner != 0) g_env->clearQueue(ride(ctx), 0);
        return;
    case 0x10: {
        const s32 a = value(ctx), b = value(ctx);
        u32 word = fetch(ctx);
        rate_1000(ctx);
        played(ctx, play(ctx, (u32)a, (u32)b, 0, factor, 0), word);
        ctx->anim_end = end_time(ctx->result, factor);
        ctx->loop_anim = 0xffff;
        return;
    }
    case 0x11: {
        const s32 a = value(ctx), b = value(ctx);
        if (ctx->wait_deadline != 0) {
            wait_on(ctx);
            return;
        }
        rate_1000(ctx);
        const u32 left = (u32)play(ctx, (u32)a, (u32)b, 0, factor, 0) - 300;
        ctx->wait_deadline = (u32)ftol(X87Reg((double)left) / factor);
        if (ctx->wait_deadline < 300) ctx->wait_deadline = 300;
        ctx->wait_deadline += now();
        end_slice(ctx, 3);
        ctx->anim_end = 0;
        ctx->loop_anim = 0xffff;
        return;
    }
    case 0x12: {
        const s32 a = value(ctx), b = value(ctx);
        const s32 key = (s32)((u32)b * 0x10000 + (u32)a);
        if (ctx->loop_anim != key) {
            rate_1000(ctx);
            play(ctx, (u32)a, (u32)b, 1, factor, 0);
            ctx->anim_end = 0;
            ctx->loop_anim = key;
        }
        return;
    }
    case 0x13: {
        const s32 a = value(ctx), b = value(ctx);
        u32 word = fetch(ctx);
        if (at<u32>(ctx, 0xbc) != 0) {
            two_step_wait(ctx, word, 0);
            return;
        }
        rate_1000(ctx);
        played(ctx, play(ctx, (u32)a, (u32)b, 0, factor, 0), word);
        ctx->anim_end = end_time(ctx->result, factor);
        ctx->loop_anim = 0xffff;
        at<u32>(ctx, 0xbc) = (u32)a + 1;
        ctx->pc -= 4;
        return;
    }
    case 0x14: {
        const u32 word = fetch(ctx);
        anim_state(ctx, word, 0);
        return;
    }
    case 0x15: {
        const s32 a = value(ctx), b = value(ctx);
        u32 word = fetch(ctx);
        const s32 c = value(ctx);
        at<s16>(ctx, 0xe4) = (s16)c;
        const float speed = (float)(X87Reg(X87_MulIntDouble(c, (double)factor)) * 0.001f);
        played(ctx, play(ctx, (u32)a, (u32)b, 0, speed, 0), word);
        const u32 clock = now();
        ctx->anim_end = (u32)((s32)((u32)ctx->result * 1000) / c) + clock;
        ctx->loop_anim = 0xffff;
        return;
    }
    case 0x16: {
        const s32 index = value(ctx);
        if (ctx->owner != 0) g_env->clearQueue(ride(ctx), (u32)index);
        return;
    }
    case 0x17: {
        const s32 a = value(ctx), b = value(ctx);
        u32 word = fetch(ctx);
        const s32 index = value(ctx);
        rate_1000(ctx);
        played(ctx, play(ctx, (u32)a, (u32)b, 0, factor, (u32)index), word);
        ctx->anim_end = end_time(ctx->result, factor);
        return;
    }
    case 0x18: {
        const s32 a = value(ctx), b = value(ctx);
        const u32 index = fetch(ctx);
        if (ctx->wait_deadline != 0) {
            wait_on(ctx);
            return;
        }
        rate_1000(ctx);
        ctx->wait_deadline = (u32)play(ctx, (u32)a, (u32)b, 0, factor, index) - 300;
        if (ctx->wait_deadline < 300) ctx->wait_deadline = 300;
        ctx->wait_deadline += now();
        end_slice(ctx, 3);
        ctx->anim_end = 0;
        return;
    }
    case 0x19: {
        const s32 a = value(ctx), b = value(ctx);
        const u32 index = fetch(ctx);
        rate_1000(ctx);
        play(ctx, (u32)a, (u32)b, 1, factor, index);
        ctx->anim_end = 0;
        return;
    }
    case 0x1a: {
        const s32 a = value(ctx), b = value(ctx);
        u32 word = fetch(ctx);
        const s32 index = value(ctx);
        if (at<u32>(ctx, 0xbc) != 0) {
            two_step_wait(ctx, word, (u32)index);
            return;
        }
        rate_1000(ctx);
        played(ctx, play(ctx, (u32)a, (u32)b, 0, factor, (u32)index), word);
        ctx->anim_end = end_time(ctx->result, factor);
        at<u32>(ctx, 0xbc) = (u32)a + 1;
        ctx->pc -= 4;
        return;
    }
    case 0x1b: {
        const u32 word = fetch(ctx);
        anim_state(ctx, word, (u32)value(ctx));
        return;
    }
    case 0x1c: {
        const u32 word = fetch(ctx);
        const s32 range = (s16)fetch(ctx);
        s32 r = rand_half() % (range + 1);
        if (r < 0) r = -r;
        set_result(ctx, word, r);
        return;
    }
    case 0x1d: {
        const u32 word = fetch(ctx);
        push(ctx, ctx->pc | (s32)kLabel);
        if ((word & kMask) == kLabel) ctx->pc = (s32)(word ^ kLabel);
        return;
    }
    case 0x1e: {
        const u32 word = (u32)pop(ctx);
        if ((word & kMask) == kLabel) ctx->pc = (s32)(word ^ kLabel);
        return;
    }
    case 0x1f: jump(ctx, fetch(ctx)); return;
    case 0x20: {
        const u32 word = fetch(ctx);
        if (ctx->result == 0) jump(ctx, word);
        return;
    }
    case 0x21: {
        const u32 word = fetch(ctx);
        if (ctx->result != 0) jump(ctx, word);
        return;
    }
    case 0x22: {
        const u32 word = fetch(ctx);
        if (ctx->result < 0) jump(ctx, word);
        return;
    }
    case 0x23: {
        const u32 word = fetch(ctx);
        if (ctx->result > 0) jump(ctx, word);
        return;
    }
    case 0x24: fetch(ctx); return;
    case 0x25: {
        const u32 word = fetch(ctx);
        if ((word & kMask) == kString) ctx->name = reinterpret_cast<char *>((uintptr_t)(word ^ kString));
        return;
    }
    case 0x26: {
        const u32 word = fetch(ctx);
        if (is_var(word)) ctx->result = var(ctx, word);
        return;
    }
    case 0x27: {
        const u32 word = fetch(ctx);
        const s32 v = value(ctx);
        if (is_var(word)) ctx->result = (s32)((u32)var(ctx, word) - (u32)v);
        return;
    }
    case 0x28: {
        const s32 v = value(ctx);
        push(ctx, v);
        ctx->result = v;
        return;
    }
    case 0x29: {
        const u32 word = fetch(ctx);
        set_result(ctx, word, pop(ctx));
        return;
    }
    case 0x2a: {
        const u32 word = fetch(ctx);
        if (ctx->stack != nullptr && ctx->hp >= 0 && ctx->hp < (s32)ctx->stack_size) {
            const s32 v = RSSE_OperandValue(ctx, word);
            ctx->stack[ctx->hp] = v;
            ctx->result = v;
            ctx->hp++;
        }
        return;
    }
    case 0x2b: {
        const u32 word = fetch(ctx);
        if (ctx->stack == nullptr || ctx->hp < 1 || (s32)ctx->stack_size < ctx->sp) return;
        ctx->hp--;
        set_result(ctx, word, ctx->stack[ctx->hp]);
        return;
    }
    case 0x2c:
        if (ctx->wait_deadline != 0) {
            wait_operand(ctx);
            return;
        }
        ctx->wait_deadline = end_time(value(ctx), factor);
        end_slice(ctx, 2);
        return;
    case 0x2d:
        if (ctx->wait_deadline != 0) {
            wait_operand(ctx);
            return;
        }
        {
            const s32 v = value(ctx);
            ctx->wait_deadline = now() + (u32)v;
        }
        end_slice(ctx, 2);
        return;
    case 0x2e:
        if (ctx->anim_end != 0) {
            const u32 clock = now();
            if (ctx->anim_end <= clock) ctx->anim_end = 0;
            else end_slice(ctx, 1);
        }
        return;
    case 0x2f: {
        const u32 word = fetch(ctx), add = fetch(ctx);
        if (is_var(word)) {
            const s32 v = RSSE_OperandValue(ctx, add);
            var(ctx, word) = (s32)((u32)var(ctx, word) + (u32)v);
            ctx->result = var(ctx, word);
        }
        return;
    }
    case 0x30: {
        const u32 word = fetch(ctx);
        const s32 a = value(ctx), b = value(ctx);
        set_result(ctx, word, (s32)((u32)a * (u32)b));
        return;
    }
    case 0x31: {
        const u32 word = fetch(ctx);
        const s32 a = value(ctx), b = value(ctx);
        set_result(ctx, word, b == 0 ? 0 : a / b);
        return;
    }
    case 0x32: {
        const u32 word = fetch(ctx);
        const s32 a = value(ctx), b = value(ctx);
        set_result(ctx, word, b == 0 ? 0 : a % b);
        return;
    }
    case 0x33: ctx->turbo = (u8)fetch(ctx); return;
    case 0x34: ctx->pc = RSSE_PC_DEAD; return;
    case 0x35: flying(ctx); return;
    case 0x36: track(ctx); return;
    case 0x37: coaster(ctx); return;
    case 0x38: {
        const s32 v = value(ctx);
        u32 *const entries = at<u32 *>(ctx, 0x30);
        if (entries == nullptr) return;
        const s32 count = at<s32>(ctx, 0x4c);
        bool empty = false;
        for (s32 i = 0; i < count; ++i) {
            if (entries[i] == 0) empty = true;
        }
        if (!empty) return;
        s32 pick;
        do {
            pick = rand_half() % count;
        } while (entries[pick] != 0);
        entries[pick] = (u32)v;
        const u32 part = g_env->findPart(ride(ctx), 0x80, (u32)pick + 1);
        if (part != 0xffffffff) g_env->activate(at<u8 *>(ride(ctx), 8), part, (u32)v);
        return;
    }
    case 0x39: {
        const u32 v = (u32)value(ctx);
        u32 *const entries = at<u32 *>(ctx, 0x30);
        if (entries == nullptr) return;
        for (s32 i = 0; i < at<s32>(ctx, 0x4c); ++i) {
            if (entries[i] != v) continue;
            const u32 part = g_env->findPart(ride(ctx), 0x80, (u32)i + 1);
            if (part != 0xffffffff) g_env->release(at<u8 *>(ride(ctx), 8), part);
            entries[i] = 0;
        }
        return;
    }
    case 0x3a: {
        const s32 guest = value(ctx), wait = value(ctx);
        ctx->result = 0;
        for (s32 i = 0; i < (s32)ctx->limbo_capacity; ++i) {
            RsseLimboSlot &slot = ctx->limbo[i];
            if (slot.guest != 0) continue;
            slot.guest = guest;
            slot.release_time = now() + (u32)wait * 1000;
            ctx->limbo_count++;
            ctx->result = 1;
            return;
        }
        return;
    }
    case 0x3b: {
        const u32 word = fetch(ctx);
        const s32 clock = (s32)now();
        s32 out = 0;
        for (s32 i = 0; i < (s32)ctx->limbo_capacity; ++i) {
            RsseLimboSlot &slot = ctx->limbo[i];
            if (slot.guest != 0 && (s32)slot.release_time < clock) {
                ctx->limbo_count--;
                out = slot.guest;
                slot.guest = 0;
                break;
            }
        }
        set_result(ctx, word, out);
        return;
    }
    case 0x3c: {
        const u32 word = fetch(ctx);
        if (!is_var(word)) return;
        s32 out = 0;
        for (s32 i = 0; i < (s32)ctx->limbo_capacity; ++i) {
            RsseLimboSlot &slot = ctx->limbo[i];
            if (slot.guest != 0) {
                ctx->limbo_count--;
                out = slot.guest;
                slot.guest = 0;
                break;
            }
        }
        set_result(ctx, word, out);
        return;
    }
    case 0x3d: {
        const u32 word = fetch(ctx);
        set_result(ctx, word, (s32)ctx->limbo_count);
        return;
    }
    case 0x3e: {
        const u32 word = fetch(ctx);
        set_result(ctx, word, (s32)(ctx->limbo_capacity - ctx->limbo_count));
        return;
    }
    case 0x3f: {
        const u32 word = fetch(ctx);
        if ((word & kMask) != kString) return;
        const s32 id = (s32)load_child(ctx, word);
        ctx->child_id = id;
        if (id != 0) {
            RsseContext *const child = find_script(id);
            at<s32>(child, 0x10) = ctx->id;
            at<u16>(child, 0xac) = at<u16>(ctx, 0xac);
            child->sound = ctx->sound;
            at<u32>(child, 0xb4) = at<u32>(ctx, 0xb4);
            child->owner = ctx->owner;
        }
        return;
    }
    case 0x40: {
        const u32 word = fetch(ctx);
        if ((word & kMask) == kString) at<u32>(ctx, 0x14) = load_child(ctx, word);
        return;
    }
    case 0x41:
        if (ctx->child_id != 0) g_env->killScript(ctx->child_id, 0);
        ctx->child_id = 0;
        return;
    case 0x42: {
        const s32 index = value(ctx), v = value(ctx);
        if (ctx->child_id != 0) write_other(ctx, ctx->child_id, index, v);
        return;
    }
    case 0x43: {
        const u32 word = fetch(ctx);
        const s32 index = value(ctx);
        if (is_var(word) && ctx->child_id != 0) read_other(ctx, ctx->child_id, index, word);
        return;
    }
    case 0x44: {
        const s32 index = value(ctx), v = value(ctx);
        if (at<s32>(ctx, 0x10) != 0) write_other(ctx, at<s32>(ctx, 0x10), index, v);
        return;
    }
    case 0x45: {
        const u32 word = fetch(ctx);
        const s32 index = value(ctx);
        if (is_var(word) && at<s32>(ctx, 0x10) != 0) read_other(ctx, at<s32>(ctx, 0x10), index, word);
        return;
    }
    case 0x46: ctx->rider_node = (s32)fetch(ctx); return;
    case 0x47: at<s16>(ctx, 0x6e) = (s16)value(ctx); return;
    case 0x48: {
        const s32 guest = value(ctx), wait = value(ctx);
        ctx->result = 0;
        for (s32 i = 0; i < (s32)ctx->rider_capacity; ++i) {
            RsseRiderSlot &slot = ctx->riders[i];
            if (slot.guest != 0) continue;
            slot.guest = guest;
            slot.node = ctx->rider_node + i;
            slot.release_time = now() + (u32)wait * 1000;
            slot.start_time = now();
            at<s16>(ctx, 0x6c)++;
            ctx->result = 1;
            return;
        }
        return;
    }
    case 0x49: {
        const u32 word = fetch(ctx);
        rider_out(ctx, word, true);
        return;
    }
    case 0x4a: {
        const u32 word = fetch(ctx);
        rider_out(ctx, word, false);
        return;
    }
    case 0x4b: {
        const u32 word = fetch(ctx);
        set_result(ctx, word, at<s16>(ctx, 0x6c));
        return;
    }
    case 0x4c: {
        const s32 guest = value(ctx), from = value(ctx), to = value(ctx), a = value(ctx), b = value(ctx), kind = value(ctx), last = value(ctx);
        g_env->startWalker(reinterpret_cast<u8 *>(ctx), (u32)guest, from, to, (u32)a, (u32)b, (u32)kind, (u32)last);
        return;
    }
    case 0x4d: {
        const s32 v = value(ctx);
        g_env->walkerBack(reinterpret_cast<u8 *>(ctx), (u32)v);
        return;
    }
    case 0x4e: {
        const u32 word = fetch(ctx);
        set_result(ctx, word, (s32)g_env->takeFinished(reinterpret_cast<u8 *>(ctx)));
        return;
    }
    case 0x4f: {
        const s32 seconds = value(ctx), a = value(ctx), b = value(ctx);
        g_env->startTimer(ctx, (u32)seconds * 1000, (u32)a, (u32)b);
        return;
    }
    case 0x50: {
        const u32 word = fetch(ctx);
        set_result(ctx, word, at<s32>(ctx, 0x84));
        return;
    }
    case 0x51:
        if (at<u32>(ctx, 0x84) != 0) {
            const u32 first = now(), second = now();
            const u32 done = (first * 1000 - at<u32>(ctx, 0x80) * 1000) / at<u32>(ctx, 0x84);
            at<u32>(ctx, 0x84) = 1000;
            at<u32>(ctx, 0x80) = second - done;
        }
        return;
    case 0x52: {
        const s32 part = value(ctx);
        g_env->meshClear(mesh(slot_key(ctx, part)));
        return;
    }
    case 0x53: {
        const s32 part = value(ctx);
        g_env->meshSet(mesh(slot_key(ctx, part)));
        return;
    }
    case 0x54: {
        const s32 part = value(ctx), scale = value(ctx);
        g_env->colourScaled(mesh(slot_key(ctx, part)), hundredths(scale));
        return;
    }
    case 0x55: {
        const s32 part = value(ctx), r = value(ctx), g = value(ctx), b = value(ctx);
        const u32 key = slot_key(ctx, part);
        g_env->colour(mesh(key), float_bits(hundredths(r)), float_bits(hundredths(g)), float_bits(hundredths(b)));
        return;
    }
    case 0x56: {
        const s32 level = value(ctx), a = value(ctx);
        float x, y, z;
        g_env->centre(&ctx->owner, &x, &y, &z);
        const s32 sz = ftol(X87Reg(z)), sy = ftol(X87Reg(y)), sx = ftol(X87Reg(x));
        const u32 scream = g_env->startScream(at<u32>(ctx, 0xd0), level, a, ctx->speed, sx, sy, sz);
        ctx->result = (s32)scream;
        at<u32>(ctx, 0xd0) = scream;
        return;
    }
    case 0x57:
        if (at<u32>(ctx, 0xd0) != 0) g_env->soundStop(at<u32>(ctx, 0xd0));
        at<u32>(ctx, 0xd0) = 0;
        return;
    case 0x58: {
        const s32 level = value(ctx), a = value(ctx);
        float x, y, z;
        g_env->centre(&ctx->owner, &x, &y, &z);
        const s32 sz = ftol(X87Reg(z)), sy = ftol(X87Reg(y)), sx = ftol(X87Reg(x));
        if (a < 0) g_env->kidPlay(level, sx, sy, sz);
        else g_env->kidGraded(level, a, ctx->speed, sx, sy, sz);
        return;
    }
    case 0x59: {
        const s32 v = value(ctx);
        at<u32>(ctx, 0xd0) = g_env->screamVolume(at<u32>(ctx, 0xd0), v, ctx->speed);
        return;
    }
    case 0x5a: {
        const u32 word = fetch(ctx), out = fetch(ctx);
        pick_script(ctx, word, out);
        return;
    }
    case 0x5b: {
        const u32 word = fetch(ctx);
        const s32 id = value(ctx), index = value(ctx);
        ctx->result = 0;
        const RsseContext *const p = reinterpret_cast<const RsseContext *>(g_env->findScript(id));
        if (p != nullptr && index >= 0 && index < (s32)p->var_count) ctx->result = p->vars[index];
        set_var(ctx, word, ctx->result);
        return;
    }
    case 0x5c: {
        const s32 id = value(ctx), index = value(ctx), v = value(ctx);
        RsseContext *const p = reinterpret_cast<RsseContext *>(g_env->findScript(id));
        if (p != nullptr && index >= 0 && index < (s32)p->var_count) {
            p->vars[index] = v;
            ctx->result = v;
        }
        return;
    }
    case 0x5d: {
        const s32 v = value(ctx);
        g_env->setEffect(reinterpret_cast<u8 *>(ctx), (u32)v);
        float x, y, z;
        if (v != 0) {
            g_env->getVector(ride(ctx), reinterpret_cast<u32 *>(&x), reinterpret_cast<u32 *>(&y), reinterpret_cast<u32 *>(&z));
            const s32 sz = ftol(X87Reg(z)), sy = ftol(X87Reg(y)), sx = ftol(X87Reg(x));
            g_env->playAt(0, global<u32>(0x00803a28), 0x6f, sx, sy, sz);
            g_env->playAt(0, global<u32>(0x00803a28), 0xd3, sx, sy, sz);
        } else if ((s32)ctx->var_count > 5) {
            const s32 level = ctx->vars[5];
            g_env->getVector(ride(ctx), reinterpret_cast<u32 *>(&x), reinterpret_cast<u32 *>(&y), reinterpret_cast<u32 *>(&z));
            const s32 sz = ftol(X87Reg(z)), sy = ftol(X87Reg(y)), sx = ftol(X87Reg(x));
            g_env->kidGraded(level, 1, 1, sx, sy, sz);
        }
        return;
    }
    case 0x5e: {
        const u32 word = fetch(ctx);
        value(ctx);
        set_result(ctx, word, 0);
        return;
    }
    case 0x5f: {
        const s32 v = value(ctx);
        at<u32>(ctx, 0xc4) = now() + (u32)v;
        return;
    }
    case 0x60: {
        const u32 word = fetch(ctx);
        const s32 left = (s32)(at<u32>(ctx, 0xc4) - now());
        set_result(ctx, word, left < 0 ? 0 : left);
        return;
    }
    case 0x61: time_field(ctx, 0x14, 0); return;
    case 0x62: time_field(ctx, 0x10, 1); return;
    case 0x63: time_field(ctx, 0xc, 0); return;
    case 0x64: time_field(ctx, 8, 0); return;
    case 0x65: time_field(ctx, 4, 0); return;
    case 0x66: time_field(ctx, 0, 0); return;
    case 0x67: at<s32>(ctx, 0xd8) = value(ctx); return;
    case 0x68: {
        const s32 v = value(ctx);
        at<u8>(ctx, 0xb9) = (u8)v;
        g_env->soundSet((u32)v);
        return;
    }
    case 0x69: {
        at<u16>(ctx, 0xdc) = (u16)value(ctx);
        at<u16>(ctx, 0xde) = (u16)value(ctx);
        value(ctx);
        value(ctx);
        float place[3];
        g_env->partPlace(reinterpret_cast<const u8 *>(ctx), place, nullptr, at<s16>(ctx, 0xdc), 0x100);
        g_env->partPlace(reinterpret_cast<const u8 *>(ctx), place, nullptr, at<s16>(ctx, 0xde), 0x100);
        return;
    }
    default: return;
    }
}
}
