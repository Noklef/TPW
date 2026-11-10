#include "worldsave.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/circbuf.h"
#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/initializers.h"
#include "../core/savegame.h"
#include "../core/trivial.h"
#include "../files/file.h"
#include "../sprite/draw.h"
#include "advisor.h"
#include "advisorrecords.h"
#include "analyser.h"
#include "bankaccount.h"
#include "challenges.h"
#include "headquarters.h"
#include "mapcell.h"
#include "mousemode.h"
#include "object.h"
#include "objectdefs.h"
#include "person.h"
#include "research.h"
#include "staff.h"
#include "things.h"
#include "weather.h"
#include "world.h"
#include "worldsave.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const WorldSaveEnv kGameEnv = {
    &Things_InitTable,
    &ObjectControls_Serialize,
    &WorldGlobals_Serialize,
    &Map_Serialize,
    {&Visitor_Serialize, nullptr, &Object_Serialize, &Mechanic_Serialize, &Handyman_Serialize, &Entertainer_Serialize, &Guard_Serialize,
     &Researcher_Serialize, &StaffHQ_Serialize, &MechanicHQ_Serialize, &Advisor_Serialize, &ThingBasic_Serialize, &ParkAnalyser_Serialize,
     &ResearchLab_Serialize, &Weather_Serialize, &BankAccount_Serialize, &ThingBasic_Serialize, nullptr, &Challenges_Serialize}};
const WorldSaveEnv *g_env = &kGameEnv;

struct WorldField {
    u32 offset, size, name;
};
const WorldField kWorldFields[] = {
    {0x1da72c, 2, (u32)&gText_Worldsave_7629fc}, {0x1da72e, 2, (u32)&gText_Worldsave_7629e4}, {0x1da730, 2, (u32)&gText_Worldsave_7629cc}, {0x1da726, 2, (u32)&gText_Worldsave_7629bc}, {0x1da72a, 2, (u32)&gText_Worldsave_7629a4},
    {0x1da70c, 4, (u32)&gText_Worldsave_762998}, {0x1da71a, 2, (u32)&gText_Worldsave_762988}, {0x1da720, 2, (u32)&gText_Worldsave_762978}, {0x1da710, 4, (u32)&gText_Worldsave_762968}, {0x1da714, 4, (u32)&gText_Worldsave_76294c},
    {0x1da732, 2, (u32)&gText_Worldsave_762940}, {0x1da734, 2, (u32)&gText_Worldsave_762930}, {0x1da708, 4, 0x762924}, {0x1da722, 2, (u32)&gText_Worldsave_762914}, {0x1da718, 2, (u32)&gText_Worldsave_762908},
    {0x1da71e, 2, (u32)&gText_Worldsave_7628fc}, {0x1da728, 2, (u32)&gText_Worldsave_7628ec}, {0x1da724, 2, (u32)&gText_Worldsave_7628e0}, {0x1da738, 4, (u32)&gText_Worldsave_7628d0}, {0x1da73c, 2, (u32)&gText_Worldsave_7628c0},
    {0x1da73e, 2, (u32)&gText_Worldsave_7628b0}, {0x1da740, 2, (u32)&gText_Worldsave_76289c}, {0x1da744, 2, 0x762890}, {0x1da742, 2, (u32)&gText_Worldsave_76287c}, {0x1da746, 2, (u32)&gText_Worldsave_76286c},
};
const u32 kFirstDwordName = 0x748744, kFirstThingName = 0x7627e4, kNextThingName = (u32)&gText_Worldsave_7627d4, kKindName = (u32)&gText_Worldsave_7627c8;
const u32 kSlots = 0x007cfb90, kSlotSize = 20;

bool worldFields(u8 *world, Stream *stream, u32 save) {
    for (const WorldField &f : kWorldFields)
        if (!Save_Field(stream, save, world + f.offset, (s32)f.size, f.name)) return false;
    return true;
}

template <typename F> void vector(u8 *array, u32 size, s32 count, F ctor) {
    CRT_VectorCtorIteratorNoEh(array, size, count, reinterpret_cast<void(__fastcall *)(void *)>(ctor));
}

u8 *alloc(u32 size) { return static_cast<u8 *>(GameHeap_Alloc(size)); }

// The blocks and the constructors World_Load inlines, by kind (a failed allocation stays null, or goes straight on for those with none).
u8 *newThing(u32 kind) {
    u8 *o;
    switch (kind) {
    case 1:
        o = alloc(0x22c);
        if (!o) return nullptr;
        Person_PartsCtor(o, nullptr, 0);   // (the original's argument an unread stack address, as for Person_BaseCtor below)
        for (u32 off = 0x19c; off <= 0x1b8; off += 4)
            if (off != 0x1a0) at<u32>(o, off) = 0;
        vector(o + 0x1e8, 2, 4, &Empty_Ctor);
        at<u32>(o, 0x20c) = 0;
        return o;
    case 3:
        o = alloc(0x450);
        if (!o) return nullptr;
        at<u32>(o, 0x18) = at<u32>(o, 0x1c) = 0;
        at<u32>(o, 0x44) = at<u32>(o, 0x48) = at<u32>(o, 0x4c) = 0;
        CircBufU32_30_Ctor(o + 0x70);
        CircBufU32_30_Ctor(o + 0xf8);
        CircBufU32_30_Ctor(o + 0x1a8);
        CircBufU32_30_Ctor(o + 0x230);
        CircBufU32_30_Ctor(o + 0x2b8);
        CircBufU32_30_Ctor(o + 0x340);
        return o;
    case 4:
    case 5:
        o = alloc(0x220);
        if (o) Person_BaseCtor(o, nullptr, 0);
        return o;
    case 6:
    case 7:
    case 8:
        o = alloc(0x218);
        if (o) Person_BaseCtor(o, nullptr, 0);
        return o;
    case 9:
        o = alloc(0x68);
        if (!o) return nullptr;
        StaffTraining_Ctor(o + 0xc);
        vector(o + 0x24, 0xc, 5, &AdvisorEntry_Ctor);
        return o;
    case 0xa:
        return alloc(0xe);
    case 0xb:
        o = alloc(0x16f0);
        if (!o) return nullptr;
        at<u32>(o, 0xc) = 0;
        vector(o + 0x14, 0x18, 8, &AdvisorSlot_Ctor);
        vector(o + 0xe0, 0x10, 0x15f, &AdvisorRecord_Ctor);
        at<u8>(o, 0x16e4) = 0;
        return o;
    case 0xc:
        return alloc(0x80e);
    case 0xd:
        o = alloc(0x21c50);
        if (!o) return nullptr;
        vector(o + 0x20, 0x7c, 0x400, &AnalyserCell_Ctor);
        vector(o + 0x1f020, 2, 0x24, &Empty_Ctor);
        vector(o + 0x1f068, 0x14, 4, &CircBufU16_3_Ctor);
        for (u32 i = 0; i < 6; ++i) Record999_Ctor(o + 0x1f0b8 + 0xc * i);
        for (u32 i = 0; i < 16; ++i) CircBufU32_144_Ctor(o + 0x1f100 + 0x250 * i);
        CircBufU32_50_Ctor(o + 0x21600);
        CircBufU32_5_Ctor(o + 0x216d8);
        vector(o + 0x216fc, 0x40, 0x14, &CircBufU8_50_Ctor);
        for (u32 off = 0x21c10; off <= 0x21c3c; off += 4) at<u32>(o, off) = 0;
        return o;
    case 0xe:
        o = alloc(0x146c);
        if (!o) return nullptr;
        vector(o + 0xc, 0x14, 0x100, &Obj_Self);
        vector(o + 0x140c, 0x10, 5, &ResearchGroup_Ctor);
        return o;
    case 0xf:
        o = alloc(0x68);
        if (!o) return nullptr;
        at<u32>(o, 0x10) = at<u32>(o, 0x14) = at<u32>(o, 0x18) = at<u32>(o, 0x1c) = 0;
        Init_513290(o + 0x20);
        return o;
    case 0x10:
        return alloc(0x128);
    case 0x11:
        return alloc(0xc);
    default:   // (0x13)
        o = alloc(0x3e8);
        if (!o) return nullptr;
        for (u32 i = 0; i < 0x14; ++i) Init_4d0f70(o + 0xc + 0x30 * i);
        at<u32>(o, 0x3d0) = at<u32>(o, 0x3d4) = 0;
        return o;
    }
}

}  // namespace

void WorldSave_SetEnv(const WorldSaveEnv *env) { g_env = env ? env : &kGameEnv; }

u32 __fastcall World_Save(u8 *world, void *, Stream *stream) {
    Stream_Tell(stream);
    u8 *const block = alloc(8);
    u8 *const mode = block ? MouseModeOnlineBase_Ctor(block) : nullptr;
    MouseModes_EnsureHolder();
    MouseMode_Set(mode);
    u32 tag = 2;
    if (!Save_Field(stream, 1, &tag, 4, kFirstDwordName)) return 0;
    if (!worldFields(world, stream, 1)) return 0;
    Stream_Tell(stream);
    Stream_Tell(stream);
    if (g_env->controls(world + 0x1d943c, nullptr, stream, 1, 1, tag)) return 0;
    Stream_Tell(stream);
    Stream_Tell(stream);
    if (g_env->globals(world, nullptr, stream, 1, 1, tag)) return 0;
    Stream_Tell(stream);
    Stream_Tell(stream);
    if (g_env->map(world + 0x2d8, nullptr, stream, 1, 1, tag)) return 0;
    Stream_Tell(stream);
    Stream_Tell(stream);
    if (!Save_Field(stream, 1, global<u8 *>(0x007cf56c) + 4, 4, kFirstThingName)) return 0;
    for (u8 *slot = global<u8 *>(0x007cf56c); slot; slot = at<u8 *>(slot, 8)) {
        u8 *const next = at<u8 *>(slot, 8);
        u32 nextHandle = next ? at<u32>(next, 4) : 0;
        if (!Save_Field(stream, 1, &nextHandle, 4, kNextThingName)) return 0;
        u32 kind = at<u8>(at<u8 *>(slot, 0), 2);
        if (!Save_Field(stream, 1, &kind, 4, kKindName)) return 0;
        if (kind - 1 <= 0x12 && g_env->kinds[kind - 1] && g_env->kinds[kind - 1](at<u8 *>(slot, 0), nullptr, stream, 1, 1, tag)) return 0;
    }
    Stream_Tell(stream);
    Stream_Tell(stream);
    return 1;
}

u32 __fastcall World_Load(u8 *world, void *, Stream *stream) {
    g_env->initTable();
    u32 tag;
    if (!Save_Field(stream, 0, &tag, 4, kFirstDwordName)) return 0;
    if (!worldFields(world, stream, 0)) return 0;
    if (g_env->controls(world + 0x1d943c, nullptr, stream, 0, 1, tag)) return 0;
    if (g_env->globals(world, nullptr, stream, 0, 1, tag)) return 0;
    if (g_env->map(world + 0x2d8, nullptr, stream, 0, 1, tag)) return 0;
    u32 first;
    if (!Save_Field(stream, 0, &first, 4, kFirstThingName)) return 0;
    u8 *slot = reinterpret_cast<u8 *>((uintptr_t)(kSlots + first * kSlotSize));
    global<u8 *>(0x007cf56c) = slot;
    u8 *previous = nullptr;
    while (slot) {
        u32 next;
        if (!Save_Field(stream, 0, &next, 4, kNextThingName)) return 0;
        at<u8 *>(slot, 8) = next ? reinterpret_cast<u8 *>((uintptr_t)(kSlots + next * kSlotSize)) : nullptr;
        at<u8 *>(slot, 0xc) = previous;
        at<u32>(slot, 0x10) = 0;
        u32 kind;
        if (!Save_Field(stream, 0, &kind, 4, kKindName)) return 0;
        const u32 handle = at<u32>(slot, 4);
        if (kind - 1 <= 0x12 && g_env->kinds[kind - 1]) {
            u8 *const thing = newThing(kind);
            thing[2] = (u8)kind;   // (a failed allocation faults here)
            Fn_50b080(thing, nullptr, (u16)handle);
            if (g_env->kinds[kind - 1](thing, nullptr, stream, 0, 1, tag)) return 0;
            at<u8 *>(slot, 0) = thing;
        }
        previous = slot;
        slot = at<u8 *>(slot, 8);
    }
    // (the free list, slot 0 aside)
    u32 lastFree = 0xffffffffu, index = 1;
    for (u8 *s = reinterpret_cast<u8 *>((uintptr_t)(kSlots + kSlotSize)); (uintptr_t)s < 0x00801b90; s += kSlotSize, ++index) {
        at<u32>(s, 0x10) = 0;
        if (at<u32>(s, 0)) continue;
        if (lastFree == 0xffffffffu) global<u8 *>(0x00801bbc) = s;
        else global<u8 *>(kSlots + 0x10 + lastFree * kSlotSize) = s;
        lastFree = index;
    }
    if (at<u32>(world, 0x1da738) == 4) SpriteDraw_Disable();
    else SpriteDraw_Enable();
    return 1;
}
