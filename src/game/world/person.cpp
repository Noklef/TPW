#include "person.h"

#include <cstdint>
#include <cstring>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/globals.h"
#include "../core/initializers.h"
#include "../core/rings.h"
#include "../core/savegame.h"
#include "../core/soundplay.h"
#include "../math/x87.h"
#include "../render/dynfaces.h"
#include "../render/particles.h"
#include "../sprite/script.h"
#include "../sprite/scriptref.h"
#include "../sprite/sprite.h"
#include "../terrain/land.h"
#include "analyser.h"
#include "messagecentre.h"
#include "nav.h"
#include "object.h"
#include "objectdefs.h"
#include "personscripts.h"
#include "random.h"
#include "things.h"
#include "world.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u16 map_id(const u8 *thing) { return (u16)((at<u8>(thing, 7) << 7) + at<u8>(thing, 5) + 1); }

// The float at p = v kept to 0..100 (fcom 100.0: above it 100; fcom 0.0: below it, or unordered, 0).
void set100(void *p, X87Reg v) {
    if (v.v > 100.0)
        at<u32>(p, 0) = 0x42c80000;
    else if (!(v.v >= 0.0))
        at<u32>(p, 0) = 0;
    else
        at<float>(p, 0) = (float)v.v;
}

// a * b / 100 as the original's IMUL and signed divide by 100 (the product wrapping).
s32 MulDiv100(s32 a, s32 b) { return (s32)((u32)a * (u32)b) / 100; }

// The park analyser's (the game table entry of [0x7cf83c]'s +0x1da720) ring of a price's margin: ((willing + 5) - price) * 10, at most 100 (unsigned).
void RecordMargin(u32 ring, u32 willing, u32 price) {
    u32 value = (willing + 5) * 10 - price * 10;
    if (value >= 100)
        value = 100;
    u8 *const analyser = reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(*reinterpret_cast<u8 **>(0x007cf83c)));
    ParkAnalyser_Record(analyser, nullptr, ring, value);
}

// m * ln(a / b) / ln(2.0 at 0x700758) as the original's x87 sequence (fild; fidiv; fldln2; fxch; fyl2x; fldln2; fld; fyl2x; fdivp; fild; fmulp),
// under the caller's control word (the game's 24-bit precision; fyl2x is not rounded to it): the register stored to a double (exact).
double sideshow_log2(s32 a, s32 b, s32 m) {
    double result;
    __asm {
        fild a
        fidiv b
        fldln2
        fxch st(1)
        fyl2x
        fldln2
        mov eax, 0x00700758
        fld qword ptr [eax]
        fyl2x
        _emit 0xde   // fdivp st(1), st
        _emit 0xf9
        fild m
        _emit 0xde   // fmulp st(1), st
        _emit 0xc9
        fstp result
    }
    return result;
}

}  // namespace

extern "C" {

u16 *__fastcall Thing_MapId(const u8 *thing, void *, u16 *out) {
    *out = map_id(thing);
    return out;
}

u16 *__fastcall Sprite_RecordMapId(u8 *sprite, void *, u16 *out) {
    const u16 last = at<u16>(sprite, 0xcc);
    at<u16>(sprite, 0xcc) = map_id(sprite);
    *out = last;
    return out;
}

u32 __fastcall Visitor_TimeInPark(const u8 *visitor) {
    const u8 *world = *reinterpret_cast<u8 *const *>(0x0080239c);
    return (at<u32>(world, 0x1da70c) - at<u32>(visitor, 0x1d4)) >> 2;
}

void __fastcall Visitor_PushPreviousRide(u8 *visitor, void *, u32 ride) {
    u16 *const rides = &at<u16>(visitor, 0x1e8);
    for (int i = 3; i != 0; --i) rides[i] = rides[i - 1];
    rides[0] = (u16)ride;
}

u32 __fastcall Visitor_IsState11AtQueuePos0(const u8 *visitor) {
    return at<s32>(visitor, 0x220) == 11 && at<u8>(visitor, 0x1f1) == 0;
}

u8 *__fastcall Thoughts_Ctor(u8 *thoughts) {
    at<u32>(thoughts, 0) = 0;
    at<u32>(thoughts, 0x84) = 0;
    at<u32>(thoughts, 0x8c) = 0;
    for (u32 i = 0; i != 32; ++i) {
        at<u16>(thoughts, 4 + i * 4) = 0;
        at<u16>(thoughts, 6 + i * 4) = 0;
    }
    at<u32>(thoughts, 0x88) = 0;
    return thoughts;
}

u32 __fastcall Visitor_IsState11To14(const u8 *visitor) {
    s32 state = at<s32>(visitor, 0x220);
    if (state == 8) state = at<s32>(visitor, 0x224);
    return state >= 11 && state <= 14;
}

u32 __fastcall Person_HasWordInState(const u8 *object, void *, u32 word) {
    if (!Person_HasWord(object, nullptr, word))
        return 0;
    const u8 kind = at<u8>(object, 2);
    if (kind == 5 && at<u32>(object, 0x19c) == 0xb)
        return 1;
    if (kind == 4 && at<u32>(object, 0x19c) == 0xd)
        return 1;
    return 0;
}

void __fastcall Visitor_Set1f8One(u8 *object) { at<u32>(object, 0x1f8) = 1; }

u32 __fastcall Person_HasWord(const u8 *object, void *, u32 word) {
    const u8 kind = at<u8>(object, 2);
    if (kind == 4)
        return at<u16>(object, 0x218) == (u16)word;
    if (kind == 5)
        return at<u16>(object, 0x21a) == (u16)word;
    return 0;
}

u8 *__fastcall Person_BaseCtor(u8 *object, void *, u32) {
    Thoughts_Ctor(object + 0x30);
    Nav_Ctor(object + 0xd4);
    at<u32>(object, 0x1f0) = 0;
    at<u32>(object, 0x1f4) = 0;
    at<u32>(object, 0x1f8) = 0;
    at<u32>(object, 0x1fc) = 0;
    return object;
}

u8 *__fastcall MovingPerson_Ctor(u8 *sprite, void *, u32 cell, u32 kind) {
    Thing_Construct(sprite, nullptr, cell, kind);
    Init_4d3ed0(sprite + 0xc);
    at<u32>(sprite, 0x1c) = 0;
    at<u32>(sprite, 0x20) = 0;
    at<u32>(sprite, 0x24) = 0;
    at<u32>(sprite, 0x28) = 0;
    at<u8>(sprite, 0x2c) = 0;
    Thoughts_Ctor(sprite + 0x30);
    at<u16>(sprite, 0xc0) = 0;
    at<u16>(sprite, 0xc2) = 0;
    at<u16>(sprite, 0xc4) = 0;
    at<u32>(sprite, 0xc8) = 0;
    at<u16>(sprite, 0xcc) = 0;
    at<u32>(sprite, 0xd0) = 0;
    Nav_Ctor(sprite + 0xd4);
    at<u32>(sprite, 0x190) = 0;
    at<u32>(sprite, 0x194) = 0;
    at<u32>(sprite, 0x198) = 0;
    const u32 pick = (u32)World_Rand(global<u8 *>(0x0080239c)) % 5;
    at<u16>(sprite, 0xc0) = global<u16>(0x0075c7f8 + pick * 2);
    at<u16>(sprite, 0xc2) = global<u16>(0x0075c7f0);
    MessageCentre_AddThing(MessageCentre_Instance(), nullptr, sprite, 0xa);
    at<u16>(sprite, 0xcc) = (u16)cell;
    const u32 id = cell & 0xffff;
    const u32 x = (((id << 16) - 0x10000) & 0x7f0000) + 0x8000;
    const u32 y = ((id - 1) >> 7 << 16) + 0x8000;
    Nav_SetPosVel(sprite + 0xd4, nullptr, x, y, 0, 0);
    Nav_GetPos(sprite + 0xd4, nullptr, &at<u32>(sprite, 0x190), &at<u32>(sprite, 0x194));
    at<u32>(sprite, 0x28) = 0;
    at<u32>(sprite, 0xd0) = 0;
    at<u32>(sprite, 0x24) = 0;
    return sprite;
}

u8 *__fastcall Person_PartsCtor(u8 *object, void *, u32) {
    Thoughts_Ctor(object + 0x30);
    Nav_Ctor(object + 0xd4);
    return object;
}

u32 __fastcall Thoughts_Serialize(u8 *thoughts, void *, Stream *stream, u32 save, s32 version, u32) {
    if (version < 0 || version > 1)
        return 0;
    if (!Save_Field(stream, save, thoughts + 0x88, 4, 0x00761de8))
        return 1;
    for (u32 i = 0; i < 0x20; ++i)
        if (!Save_Field(stream, save, thoughts + 4 + i * 4, 4, 0x00761dd4))
            return 1;
    if (!Save_Field(stream, save, thoughts, 4, 0x00761dc4) || !Save_Field(stream, save, thoughts + 0x84, 4, 0x00761db4) ||
        !Save_Field(stream, save, thoughts + 0x8c, 4, 0x00761da0))
        return 1;
    return 0;
}

u32 __fastcall Sprite_SetDestination(u8 *sprite, void *, s32 x, s32 y) {
    if (Counter7cdb98_Inc() < at<u32>(sprite, 0x198))
        at<u32>(sprite, 0x198) = 0;
    if (at<u32>(sprite, 0x198) != 0 && Obj_BlockValuesBelowLimit(sprite))
        return 0;
    at<u16>(sprite, 0x1a) = (u16)y;
    at<u32>(sprite, 0x198) = 0;
    at<u16>(sprite, 0x18) = (u16)x;
    return Nav_PathToPoint(sprite + 0xd4, nullptr, (s32)((u32)(s32)(s16)x << 8), (s32)((u32)(s32)(s16)y << 8));
}

void __fastcall Sprite_SetStranded(u8 *sprite, void *, u32 flag) { at<u32>(sprite, 0x198) = flag ? Counter7cdb98_Inc() : 0; }

u32 __fastcall Sprite_SetDestinationCell(u8 *sprite, void *, u32 cell) {
    const u32 index = cell - 1;
    const u32 x = (index & 0x7f) << 8 | 0x80;
    const u32 y = (index >> 7 & 0xff) << 8 | 0x80;
    return Sprite_SetDestination(sprite, nullptr, (s32)x, (s32)y);
}

u32 __fastcall Sprite_SetDestinationMarked(u8 *sprite, void *, s32 x, s32 y) {
    if (Sprite_SetDestination(sprite, nullptr, x, y) == 0)
        return 0;
    at<u32>(sprite, 0x39c) = 1;
    Set4_4217f0(sprite + 0xc, nullptr, 1);
    return 1;
}

u32 __fastcall Sprite_SetDestinationCellMarked(u8 *sprite, void *, u32 cell) {
    if (Sprite_SetDestinationCell(sprite, nullptr, cell) == 0)
        return 0;
    at<u32>(sprite, 0x39c) = 1;
    Set4_4217f0(sprite + 0xc, nullptr, 1);
    return 1;
}

u32 __fastcall Sprite_Serialize(u8 *sprite, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    if (Thing_Serialize(sprite, nullptr, stream, save, version, unused) != 0)
        return 1;
    if (SpriteAnim_Serialize(sprite + 0xc, nullptr, stream, save, version, unused) != 0)
        return 1;
    if (version >= 0 && version <= 1) {
        static const SaveSpec kHead[] = {{0x18, 2, 0x0075ca24}, {0x1a, 2, 0x0075ca14}, {0xc4, 2, 0x0075ca04}, {0xc0, 2, 0x0075c9f8},
                                         {0x2c, 1, 0x0075c9f0}, {0x24, 4, 0x0075c9e4}, {0xcc, 2, 0x0075c9d0}};
        static const SaveSpec kTail[] = {{0xc8, 4, 0x0075c9c0},  {0x190, 4, 0x0075c9b4}, {0x194, 4, 0x0075c9a8}, {0x198, 4, 0x0075c998},
                                         {0xc2, 2, 0x0075c988},  {0xd0, 4, 0x0075c970},  {0x1c, 4, 0x0075c960},  {0x20, 4, 0x0075c954},
                                         {0x28, 4, 0x0075c93c}};
        if (save != 0) {
            if (!Save_Fields(stream, save, sprite, kHead, 7))
                return 1;
            if (Nav_Serialize(sprite + 0xd4, nullptr, stream, save, version, unused) != 0)
                return 1;
            if (!Save_Fields(stream, save, sprite, kTail, 9))
                return 1;
        } else {
            if (!Save_Fields(stream, 0, sprite, kHead, 4))
                return 1;
            if (Save_ReadU8(stream, sprite + 0x2c, reinterpret_cast<const char *>(0x0075c9f0), 0) ||
                Save_ReadU32(stream, sprite + 0x24, reinterpret_cast<const char *>(0x0075c9e4), 0) ||
                Save_ReadU16(stream, sprite + 0xcc, reinterpret_cast<const char *>(0x0075c9d0), 0))
                return 1;
            if (Nav_SerializeFwd(sprite + 0xd4, nullptr, stream, 0, version, unused) != 0)
                return 1;
            for (u32 i = 0; i < 8; ++i) {
                const SaveSpec &f = kTail[i];
                const char *name = reinterpret_cast<const char *>(f.name);
                if (f.size == 2 ? Save_ReadU16(stream, sprite + f.at, name, 0) : Save_ReadU32(stream, sprite + f.at, name, 0))
                    return 1;
            }
            at<u32>(sprite, 0x20) %= (u32)SpriteFolder_Count(at<u32>(sprite, 0x24));  // (DIV: faults on a count of 0)
            if (Save_ReadU32(stream, sprite + 0x28, reinterpret_cast<const char *>(0x0075c93c), 0))
                return 1;
        }
        if (Thoughts_Serialize(sprite + 0x30, nullptr, stream, save, version, unused) != 0)
            return 1;
    }
    Sprite_SetDestination(sprite, nullptr, at<u16>(sprite, 0x18), at<u16>(sprite, 0x1a));
    return 0;
}

u32 __fastcall Visitor_Serialize(u8 *visitor, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    if (Sprite_Serialize(visitor, nullptr, stream, save, version, unused) != 0)
        return 1;
    if (version < 0 || version > 1)
        return 0;
    static const SaveSpec kHead[] = {{0x1d4, 4, 0x0075d380}, {0x1d8, 4, 0x0075d370}, {0x210, 4, 0x0075d360}, {0x1f8, 4, 0x0075d350}, {0x1a0, 4, 0x0075d348},
                                     {0x1bc, 4, 0x0075d33c}, {0x19c, 4, kSaveStatName},  {0x1a8, 4, kSaveStatName},  {0x218, 4, 0x0075d330}, {0x21c, 4, 0x0075d324},
                                     {0x1b4, 4, kSaveStatName},  {0x1dc, 2, 0x0075d318}, {0x1c4, 4, 0x0075d30c}, {0x1c8, 4, 0x0075d300}, {0x1cc, 4, 0x0075d2f0},
                                     {0x1d0, 4, 0x0075d2dc}, {0x204, 4, 0x0075d2cc}, {0x200, 4, 0x0075d2b4}, {0x1f0, 1, 0x0075d2a8}, {0x1c0, 1, 0x0075d298}};
    static const SaveSpec kTail[] = {{0x228, 2, 0x0075d260}, {0x22a, 2, 0x0075d258}, {0x1f4, 4, 0x0075d248}, {0x1f1, 1, 0x0075d23c}, {0x214, 4, 0x0075d224},
                                     {0x1de, 2, 0x0075d214}, {0x224, 4, 0x0075d208}, {0x220, 4, 0x0075b5b0}, {0x1a4, 4, kSaveStatName},  {0x208, 4, 0x0075d1f4},
                                     {0x1fc, 4, 0x0075d1e0}, {0x1b8, 4, kSaveStatName},  {0x1ac, 4, kSaveStatName},  {0x1b0, 4, kSaveStatName}};
    if (!Save_StatFields(stream, save, visitor, kHead, 20))
        return 1;
    for (u32 i = 0; i < 4; ++i)  // the previous rides and the previous temporary rides, a pair at a time
        if (!Save_Field(stream, save, visitor + 0x1e0 + i * 2, 2, 0x0075d284) || !Save_Field(stream, save, visitor + 0x1e8 + i * 2, 2, 0x0075d268))
            return 1;
    return Save_StatFields(stream, save, visitor, kTail, 14) ? 0 : 1;
}

s32 __fastcall Visitor_RatingGap(const u8 *visitor, void *, u8 *object) {
    if (Object_InPoorCondition(object)) return 100;
    const u8 wanted = *reinterpret_cast<const u8 *>(0x007850e4 + 12 * (u32)at<u8>(visitor, 0x1f0));
    const s32 gap = (s32)wanted - (s32)Object_Rating(object, nullptr, 0);
    const s32 sign = gap >= 0 ? -1 : 1;
    const s32 size = gap < 0 ? -gap : gap;
    return (size > 50 ? 50 : size) * sign;
}

u32 __fastcall Visitor_WontPay(const u8 *visitor, void *, u8 *object) {
    const u32 price = Obj_Get194(object);
    if (price == 0)
        return 0;
    const u8 *const def = Object_GetDef(object);
    // the stats' __ftol low bytes against the definition's effects, each product over 100 (signed)
    const auto stat = [visitor](u32 offset) { return (s32)(u8)(long long)at<float>(visitor, offset); };
    u32 mood = (u32)MulDiv100(at<s32>(def, 0x150), 100 - stat(0x19c));
    mood += (u32)MulDiv100(at<s32>(def, 0x144), stat(0x1a4));
    mood += (u32)MulDiv100(at<s32>(def, 0x148), stat(0x1a8));
    mood = mood - (u32)MulDiv100(at<s32>(def, 0x14c), stat(0x1b0)) + 100;
    const s32 cost = MulDiv100(at<s32>(def, 0x140), 0x73);
    u32 willing = mood * (u32)cost / 100;  // (the products wrap; the divisions by 100 from here on unsigned)
    Obj_Get194(object);
    const u32 quality = GetByte190_4e21b0(object);
    const s32 extra = at<s32>(Object_GetDef(object), 0x4ac) == 2 ? (s32)Obj_Get188(object) : 0;
    const s32 controls = at<s32>(Obj_FindControls(object), 0xc);
    willing = ((u32)MulDiv100(extra, (s32)quality) + willing) * ((u32)controls + 100) / 100;
    willing = willing * (u32)(stat(0x19c) + 100) / 100;
    // the analyser rings of the price margins: an ingredient (1-4) and an appearance (6, 7) for a kind-1 definition, 9 for a kind-2 one
    u32 ring = 0;
    const s32 kind = at<s32>(Object_GetDef(object), 0x4ac);
    if (kind == 2) {
        ring = 9;
    } else if (kind == 1) {
        const u32 ingredient = at<u32>(def, 0x158);
        if (ingredient >= 1 && ingredient <= 4)
            RecordMargin(ingredient, willing, price);
        const s32 appearance = at<s32>(def, 0x15c);
        ring = appearance == 1 ? 6 : appearance == 2 ? 7 : 0;
    }
    if (ring != 0)
        RecordMargin(ring, willing, price);
    return price > willing || at<u32>(visitor, 0x1a0) < price ? 1 : 0;
}

u32 __fastcall Visitor_IsState6To10(const u8 *visitor) {
    const s32 state = at<s32>(visitor, 0x220);
    return state == 6 || state == 7 || state == 8 || state == 10 || state == 9 ? 1 : 0;
}

void __fastcall Visitor_Raise19c(u8 *visitor, void *, u32 index) {
    set100(visitor + 0x19c, X87Reg((double)Bytes785058_Get(index)) + X87Reg(at<float>(visitor, 0x19c)));
}

void __fastcall Visitor_Lower19c(u8 *visitor, void *, u32 index) {
    set100(visitor + 0x19c, X87Reg(-(double)Bytes785058_Get(index)) + X87Reg(at<float>(visitor, 0x19c)));   // fild; fchs; fadd
}

void __fastcall Stat100_Add(float *stat, void *, float delta) { set100(stat, X87Reg(delta) + X87Reg(*stat)); }

void __fastcall Sprite_SpawnParticle(u8 *sprite, void *, s32 effect) {
    float x = 0.0f, y = 0.0f, z = 0.0f;   // (the original's are its stack's leftovers when the script is not there)
    SpriteScript_GetPosition(at<s32>(sprite, 0xc), reinterpret_cast<u32 *>(&x), reinterpret_cast<u32 *>(&y), reinterpret_cast<u32 *>(&z), nullptr);
    x = (float)(X87Reg((double)(s32)Global785584_Get_519410()) * global<float>(0x00700698) + x);   // fild; fmul 10.0; fadd x; fstp
    z = (float)(X87Reg((double)(s32)Global785588_Get_519420()) * global<float>(0x00700698) + z);
    y = (float)Terrain_HeightAt(x, z);
    const s32 px = (s32)(long long)(X87Reg(x) * global<float>(0x007006ec)).v;   // fld; fmul 1024.0; __ftol (low half)
    const s32 py = (s32)(long long)(X87Reg(y) * global<float>(0x007006ec)).v;
    const s32 pz = (s32)(long long)(X87Reg(z) * global<float>(0x007006ec)).v;
    Particle_Spawn(effect, px, py, pz);
}

void __fastcall Sprite_SetBlockedMark(u8 *sprite, void *, u32 on) {
    if (!on) {
        at<u32>(sprite, 0x198) = 0;
        return;
    }
    at<u32>(sprite, 0x198) = Counter7cdb98_Inc();
    u16 handle;
    Thing_GetHandle(sprite, nullptr, &handle);   // (for the empty Debug_Printf)
}

void __fastcall Visitor_RideFeelings(u8 *visitor, void *, const u8 *object) {
    if (!Object_Rating(object, nullptr, 0)) return;
    const s32 wanted = *reinterpret_cast<const u8 *>(0x7850e4 + 12 * visitor[0x1f0]);
    s32 gap = wanted - Object_Rating(object, nullptr, 0);
    if (gap < 0) gap = -gap;
    const u32 step = gap < 5 ? 0x785064 : gap < 15 ? 0x785068 : gap < 0x28 ? 0x78506c : 0;
    if (step) set100(visitor + 0x19c, X87Reg((double)*reinterpret_cast<const s32 *>(step)) + at<float>(visitor, 0x19c));   // fild; fadd [mem]
    const s32 divisor = *reinterpret_cast<const s32 *>(0x785070);
    const s32 rating = Object_Rating(object, nullptr, 0);
    const s32 share = rating / divisor;   // (idiv: 0 faults)
    const s32 calm = (100 - static_cast<s32>(static_cast<u8>(static_cast<long long>(at<float>(visitor, 0x1a8))))) / 20;
    set100(visitor + 0x1b0, X87Reg((double)(calm * share)) + at<float>(visitor, 0x1b0));
}

void __fastcall Visitor_UseObject(u8 *visitor, void *, u8 *object) {
    u8 *const ring = visitor + 0x30;
    u16 handle;
    Ring_PushUnique(ring, nullptr, 8, *Thing_GetHandle(object, nullptr, &handle));
    if (at<s32>(Object_GetDef(object), 0x4ac) == 2) {
        Object_PayRunningCost(object, nullptr, (s32)Obj_Get188(object));
        at<u32>(visitor, 0x1a0) += Obj_Get188(object);
    } else if (at<s32>(Object_GetDef(object), 0x4ac) == 1) {
        Object_PayRunningCost(object, nullptr, (s32)Object_CurrentSettingsValue(object));
    }
    Visitor_RideFeelings(visitor, nullptr, object);
    const u8 *const def = Object_GetDef(object);
    // the effects: thirst +0x1a4 and hunger +0x1a8 down by +0x144 / +0x148, vomit +0x1b0, happiness +0x19c and litter +0x1b4 up (fild; fadd [mem])
    set100(visitor + 0x1a4, X87Reg(-(double)at<s32>(def, 0x144)) + at<float>(visitor, 0x1a4));
    set100(visitor + 0x1a8, X87Reg(-(double)at<s32>(def, 0x148)) + at<float>(visitor, 0x1a8));
    if (at<s32>(def, 0x144) > at<s32>(def, 0x148))
        Sprite_PlaySound(visitor, nullptr, global<u32>(0x00803a24), 0x83);
    else if (at<s32>(def, 0x144) < at<s32>(def, 0x148))
        Sprite_PlaySound(visitor, nullptr, global<u32>(0x00803a24), 0x84);
    set100(visitor + 0x1b0, X87Reg((double)at<s32>(def, 0x14c)) + at<float>(visitor, 0x1b0));
    set100(visitor + 0x19c, X87Reg((double)at<s32>(def, 0x150)) + at<float>(visitor, 0x19c));
    set100(visitor + 0x1b4, X87Reg((double)at<s32>(def, 0x154)) + at<float>(visitor, 0x1b4));
    const u32 quality = GetByte198_4e21f0(object);
    // a small happiness drop when the effect plus quality plus a random 0-7 stays below 30 (unsigned): hunger first, then thirst
    const auto drop = [&](u32 offset) {
        if (at<s32>(def, offset) != 0 && ((u32)World_Rand(global<u8 *>(0x0080239c)) & 7) + quality + at<u32>(def, offset) < 0x1e)
            Stat100_Add(&at<float>(visitor, 0x19c), nullptr, (float)-(double)global<u8>(0x00785058));
    };
    drop(0x148);
    drop(0x144);
    set100(visitor + 0x19c, X87Reg((double)MulDiv100((s32)quality, at<s32>(def, 0x150))) + at<float>(visitor, 0x19c));
    switch (at<u32>(def, 0x158)) {   // the ingredient
    case 1:
        set100(visitor + 0x1ac, X87Reg((double)quality) + at<float>(visitor, 0x1ac));
        break;
    case 2:
        set100(visitor + 0x1a4, X87Reg((double)quality) + at<float>(visitor, 0x1a4));
        break;
    case 3:
        set100(visitor + 0x1a4, X87Reg((double)MulDiv100((s32)quality, at<s32>(def, 0x144))) + at<float>(visitor, 0x1a4));
        break;
    case 4:
        at<u16>(visitor, 0xc4) += (u16)MulDiv100((s32)quality, 6);
        break;
    }
    const s32 appearance = at<s32>(def, 0x15c);
    if (appearance == 1) {   // a balloon (the Debug_Printf calls left out)
        World_SetSeed(global<u8 *>(0x0080239c), nullptr, *Thing_GetHandle(visitor, nullptr, &handle));
        const u32 sprite = SpriteFolder_RandomSprite(10);
        const u32 anim = Sprite_RandomAnim(10, sprite);
        at<s32>(visitor, 0x210) = SpriteScript_Start(0x0074f480, 10, sprite, anim, 0, 0, 0);
        u32 life = (u32)Obj_GetByte18c(object) * global<u32>(0x0075d0f4) / 100;   // (the product wraps)
        at<u32>(visitor, 0x214) = life;
        if (life < global<u32>(0x0075d0f0))
            life = global<u32>(0x0075d0f0);
        else if (life > global<u32>(0x0075d0f4))
            life = global<u32>(0x0075d0f4);
        at<u32>(visitor, 0x214) = life;
        Ring_PushUnique(ring, nullptr, 0xc, *Thing_GetHandle(object, nullptr, &handle));
    } else if (appearance == 2) {   // a costume: put on, or returned
        if (at<u32>(visitor, 0x24) == 2) {
            at<u32>(visitor, 0x24) = 0;
            World_SetSeed(global<u8 *>(0x0080239c), nullptr, *Thing_GetHandle(visitor, nullptr, &handle));
            at<u32>(visitor, 0x20) = SpriteFolder_RandomSprite(at<u32>(visitor, 0x24));
        } else {
            at<u32>(visitor, 0x24) = 2;
            at<u32>(visitor, 0x20) = SpriteFolder_RandomSprite(2);
            Ring_PushUnique(ring, nullptr, 0xb, *Thing_GetHandle(object, nullptr, &handle));
        }
    }
    if (at<u8>(object, 0x32) & 1) {   // a toilet
        Object_Wear(object, nullptr, (u8)(long long)at<float>(visitor, 0x1ac));
        at<u32>(visitor, 0x1ac) = 0;
        Ring_PushUnique(ring, nullptr, 0x11, *Thing_GetHandle(object, nullptr, &handle));
        if ((u8)(long long)at<float>(visitor, 0x1b0) > 0x5a) {
            Ring_PushUnique(ring, nullptr, 0x12, 0);
            at<u32>(visitor, 0x1b0) = 0;
        }
        at<u16>(visitor, 0xc2) = global<u16>(0x0075c7f2);
    }
    if (at<s32>(Object_GetDef(object), 0x4ac) == 2) {   // a sideshow won
        ++at<u32>(visitor, 0x1d0);
        const s32 prize = (s32)Obj_Get188(object);
        const s32 price = (s32)Obj_Get194(object);
        const s32 gain = (s32)(long long)sideshow_log2(prize, price, global<u8>(0x0078505c));
        set100(visitor + 0x19c, X87Reg((double)gain) + at<float>(visitor, 0x19c));
        World_Rand(global<u8 *>(0x0080239c));
    }
}

void __fastcall Sprite_InterpolatedPos(const u8 *sprite, void *, float t, u32 unit, float *x, u32 *height, float *z, u32 *frame) {
    if (unit)
        t = !(t >= 0.0f) ? 0.0f : t > 1.0f ? 1.0f : t;
    else
        t = !(t >= -1.0f) ? -1.0f : t > 2.0f ? 2.0f : t;
    u32 posX, posY;
    Nav_GetPos(sprite + 0xd4, nullptr, &posX, &posY);
    // the last position plus t of the way to the nav's (fild; fmul t; fiadd; __ftol's low dword)
    const auto lerp = [t](u32 pos, s32 last) {
        return (s32)(long long)X87_Add(X87_MulDouble((double)(s32)(pos - (u32)last), t), (double)last);
    };
    const s32 lastY = at<s32>(sprite, 0x194);
    const s32 y = lerp(posY, lastY);
    const s32 lastX = at<s32>(sprite, 0x190);
    const s32 nx = lerp(posX, lastX);
    // 16.16 cells to the scene's units: times 10 (rounded at 24 bits), over 65536 (exact), stored to floats
    *x = (float)(X87_MulDouble((double)nx, 10.0) / 65536.0);
    *height = 0;
    *z = (float)(X87_MulDouble((double)y, 10.0) / 65536.0);
    *frame = at<u32>(sprite, 0x1c);
}

u32 __fastcall Sprite_PlaySound(const u8 *sprite, void *, u32 category, u32 sound) {
    u32 x = 0, y = 0, z = 0, r15;   // (the original's are uninitialised when the script is missing)
    SpriteScript_GetPosition(at<s32>(sprite, 0xc), &x, &y, &z, &r15);
    const auto whole = [](u32 bits) {
        float f;
        std::memcpy(&f, &bits, 4);
        return (s32)(long long)f;
    };
    return Sound_PlayAt(0, category, sound, whole(x), whole(y), whole(z));
}

void __fastcall Person_PlaceSprite(u8 *person, void *, float t) {
    const auto bits = [](X87Reg v) {   // (fstp m32)
        const float f = (float)v.v;
        u32 b;
        std::memcpy(&b, &f, 4);
        return b;
    };
    float x0, z0, x1, z1;
    u32 h0, frame0, h1, frame1;
    Sprite_InterpolatedPos(person, nullptr, (float)(X87Reg(t) - X87Reg(0.35f)).v, 0, &x0, &h0, &z0, &frame0);
    Sprite_InterpolatedPos(person, nullptr, t, 1, &x1, &h1, &z1, &frame1);
    frame1 = (frame1 - 0x380) & 0x7ff;
    u32 x1b, z1b;
    std::memcpy(&x1b, &x1, 4);
    std::memcpy(&z1b, &z1, 4);
    if (!at<u32>(person, 0x28)) SpriteScript_SetPosition(at<s32>(person, 0xc), x1b, h1, z1b, frame1 >> 8);
    if (Counter7cdb98_Inc() < at<u32>(person, 0x198)) at<u32>(person, 0x198) = 0;
    if (at<u32>(person, 0x198)) {   // (a square face on the person's cell)
        const u32 cell = (u32)map_id(person) - 1;
        DynFaces_AddSquare(bits(X87_MulInt(cell & 0x7f, 10.0f)), 0x3f800000, bits(X87_MulInt(cell >> 7, 10.0f)), 0, 1, 0, 0x3f800000, 0, 0, 0, 0);
    }
    if (const s32 slot = at<s32>(person, 0xb4)) {
        float h;
        std::memcpy(&h, &h1, 4);
        SpriteScript_SetPosition(slot, x1b, bits(X87Reg(h) + X87Reg(2.5f)), z1b, 0);
    }
    if (at<u8>(person, 2) != 1 || !at<u32>(person, 0x210)) return;
    // the script +0x210 bobs: a row of the table 0x75c810 (12-byte rows of three floats, the count the dword 0x75c80c) by the handle and
    // the row 0x7cedd4, lower the further the person moved (|dx| + |dz| over the 0.35 since the earlier position)
    u16 handle;
    const u32 rows = global<u32>(0x0075c80c);
    const u32 row = ((u32)*Thing_GetHandle(person, nullptr, &handle) + global<u32>(0x007cedd4)) % rows;
    float h0f;
    std::memcpy(&h0f, &h0, 4);
    const X87Reg moved = (X87Reg(std::fabs((double)(X87Reg(x0) - X87Reg(x1)).v)) + X87Reg(std::fabs((double)(X87Reg(z0) - X87Reg(z1)).v))) / X87Reg(0.5f);
    const float *const bob = &global<float>(0x0075c810 + 12 * row);
    const X87Reg c = X87Reg(bob[2]) * X87Reg(1.5f) + X87Reg(z0);
    const X87Reg b = X87_SubR(moved, X87Reg(1.0f)) + X87Reg(bob[1]) * X87Reg(1.5f) + X87Reg(h0f);
    const X87Reg a = X87Reg(bob[0]) * X87Reg(1.5f) + X87Reg(x0);
    Person_MoveScript(person, nullptr, bits(a), bits(b), bits(c), 0);
    float &time = global<float>(0x007cedd8);
    if (!((double)X87Reg(time).v >= 1.0)) {   // (fcomp 1.0: below or unordered)
        time = (float)(X87Reg(0.1f) + X87Reg(time)).v;
    } else {
        global<u32>(0x007cedd4) = (global<u32>(0x007cedd4) + 1) % rows;
        global<u32>(0x007cedd8) = 0;
    }
}

void __fastcall World_PlaceSprites(u8 *, void *, float t) {
    const u8 *const first = global<const u8 *>(0x007cf56c);
    u16 handle = first ? at<u16>(first, 4) : 0;
    while (handle) {
        // (the debug print of the table's first thing missing is left out)
        u8 *const thing = global<u8 *>(0x007cfb90 + handle * 20u);
        if (thing && !thing[3]) {
            const u8 kind = thing[2];
            if (kind == 1 || kind == 4 || kind == 5 || kind == 6 || kind == 8 || kind == 7 || kind == 0x12) Person_PlaceSprite(thing, nullptr, t);
        }
        u16 own;
        const u16 at_handle = *Thing_GetHandle(thing, nullptr, &own);
        const u8 *const next = global<const u8 *>(0x007cfb98 + at_handle * 20u);
        handle = next ? at<u16>(next, 4) : 0;
    }
}

u32 __fastcall Person_DumpActionHistory(u8 *person) {
    auto read = [](const void *p, u32 offset) { (void)*reinterpret_cast<const volatile u16 *>((uintptr_t)p + offset); };
    for (s32 i = 0x1f; i >= 0; --i) {
        const u8 *const entry = Ring_FromEnd(person + 0x30, nullptr, (u32)i);
        const u16 handle = *reinterpret_cast<const volatile u16 *>(entry + 2);
        if (handle != 0) {
            const u8 *const thing = *reinterpret_cast<const u8 *const volatile *>(0x007cfb90 + handle * 20u);
            if (thing != nullptr) {
                if (*reinterpret_cast<const volatile u8 *>(thing + 2) == 3) read(thing, 0xe);
                read(thing, 5);  // (+5 and +7: the cell)
                read(thing, 6);
            }
        }
        read(entry, 0);  // the action, printed by its kind
    }
    return 0xffffffff;  // (the loop counter's last value, left in EAX)
}

void __fastcall Person_TickSpeed(u8 *person) {
    Person_UpdateSpeed(person);
    const u32 ticks = at<u32>(global<u8 *>(0x0080239c), 0x1da70c);
    u16 handle;
    if (((*Thing_GetHandle(person, nullptr, &handle) ^ ticks) & 3) == 0) Person_KillScript84After(person + 0x30);
}

void __fastcall Person_UpdateSpeed(u8 *person) {
    const u32 sum = (u32)at<u16>(person, 0xc2) + at<u16>(person, 0xc0) + at<u16>(person, 0xc4);
    const X87Reg pace = X87Reg((double)(s32)sum) / X87Reg((double)(global<u32>(0x0075c7fc) & 0xffff));
    const float speed = (float)((pace - X87Reg(at<float>(person, 0xc8)) * X87Reg(-3.0)) * X87Reg(0.25));
    at<float>(person, 0xc8) = speed;
    Nav_SetSpeed(person + 0xd4, nullptr, speed);
    at<u16>(person, 0xc4) = (u16)((u16)(at<u16>(person, 0xc4) * 99u) / 100u);
    Nav_GetPos(person + 0xd4, nullptr, &at<u32>(person, 0x190), &at<u32>(person, 0x194));
    ScriptRef_Apply(person + 0xc);
}

void __fastcall Person_UpdateSpeedParticle(u8 *person) {
    Person_UpdateSpeed(person);
    Sprite_SpawnParticle(person, nullptr, at<u8>(person, 0x3a0) ? 0x35 : 0x39);
}
}
