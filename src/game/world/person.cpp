#include "person.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/globals.h"
#include "../core/savegame.h"
#include "../math/x87.h"
#include "../sprite/script.h"
#include "../sprite/scriptref.h"
#include "analyser.h"
#include "nav.h"
#include "object.h"
#include "objectdefs.h"
#include "things.h"
#include "world.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

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
}
