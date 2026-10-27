#include "visitoroption.h"

#include <cstdint>
#include <cstring>

#include "../math/x87.h"
#include "mapcell.h"
#include "object.h"
#include "objectmsg.h"
#include "person.h"
#include "things.h"
#include "world.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

float float_of(u32 bits) {
    float f;
    std::memcpy(&f, &bits, 4);
    return f;
}

// A stat's __ftol (64-bit, truncating) low byte.
u32 stat(const u8 *visitor, u32 offset) { return (u8)(long long)at<float>(visitor, offset); }

// The byte of the 11 by 11 table 0x75d0f8 for the definition's byte and the stat, each over 10.
u32 match(const u8 *visitor, u8 *object, u32 statAt, u32 defAt) {
    const u32 value = stat(visitor, statAt);
    const u32 wanted = at<u8>(Object_GetDef(object), defAt);
    return global<u8>(0x0075d0f8 + (wanted / 10) * 11 + value / 10);
}

// The byte of 0x75d178 for (the stat + 4) / 5 when the object's +0x32 has bit 0, else 0.
u32 need(const u8 *visitor, const u8 *object, u32 statAt) {
    if (!(at<u8>(object, 0x32) & 1)) return 0;
    return global<u8>(0x0075d178 + (stat(visitor, statAt) + 4) / 5);
}

}  // namespace

extern "C" {

u32 __fastcall Visitor_ScoreOption(u8 *visitor, void *, u8 *object) {
    u16 word;   // (the original's out words: its argument's stack slot and a local)
    Object_BackOfQueueTo(object, nullptr, &word);
    const u32 ox = (u8)(*Object_BackOfQueueTo(object, nullptr, &word) - 1) & 0x7f;
    const u32 oy = ((u32)*Object_BackOfQueueTo(object, nullptr, &word) - 1) >> 7;   // (a back of 0: 0x1ffffff)
    const u32 id = (u16)((at<u8>(visitor, 7) << 7) + at<u8>(visitor, 5) + 1);
    const u32 vx = (id - 1) & 0x7f, vy = (id - 1) >> 7;
    Thing_GetHandle(object, nullptr, &word);
    Thing_GetHandle(visitor, nullptr, &word);
    Object_GetDef(object);
    if (const u16 last = at<u16>(visitor, 0x1e0)) {
        const u8 *const ride = global<u8 *>(0x007cfb90 + 20 * last);
        if (ride && at<u16>(ride, 0xe) == at<u16>(object, 0xe)) return 0;
    }

    const u32 dx = vx - ox, dy = vy - (u32)(s32)(s16)oy;
    const u32 d2 = dx * dx + dy * dy;
    u32 near = (u32)((s32)(d2 * 100) / 450);
    if (near > 100) near = 100;
    u32 distance = 100 - near;
    const s32 split = at<s16>(Map_RECell(global<u8 *>(0x008023a0), nullptr, (u16)((oy << 7) + ox + 1)), 8);
    if (split != 0) distance /= (u32)split;

    u32 queue = 0, queueWeight = 0;
    if ((s32)d2 <= 8) {
        u32 room = at<u32>(object, 0x40);
        if (!room) room = 1;
        const u32 place = (u32)Object_QueuePositionOfNone(object) * 100 / (room * 4);
        queueWeight = global<u32>(0x007850b4);
        queue = 100 - place;
    }

    const u32 wanted = global<u8>(0x007850e4 + 12 * at<u8>(visitor, 0x1f0));
    s32 off = (s32)wanted - (s32)Object_Rating(object, nullptr, 0);
    if (off < 0) off = -off;
    if (off > 50) off = 50;
    const u32 rating = 100 - 2 * (u32)off;
    const u32 ratingWeight = Object_Rating(object, nullptr, 1) ? (u16)global<u32>(0x007850b8) : 0;

    const u32 first = match(visitor, object, 0x1a4, 0x144);
    const u32 second = match(visitor, object, 0x1a8, 0x148);
    const u32 third = need(visitor, object, 0x1ac);
    const u32 fourth = need(visitor, object, 0x1b0);

    const u32 wDistance = (u16)global<u32>(0x007850b0), wFirst = (u16)global<u32>(0x007850bc), wSecond = (u16)global<u32>(0x007850c0);
    const u32 wThird = (u16)global<u32>(0x007850c4), wFourth = (u16)global<u32>(0x007850c8);
    const u32 sum = rating * ratingWeight + distance * wDistance + wSecond * second + wFirst * first + wThird * third + wFourth * fourth +
                    queue * queueWeight;
    u32 score = sum / (queueWeight + wDistance + wSecond + wFirst + wFourth + wThird + ratingWeight);

    if ((u32)Object_AgeDays(object) <= global<u32>(0x00785090)) score *= global<u32>(0x00785094);
    if (at<s32>((const void *)(uintptr_t)GameTable7cfb90_Get1da724_519590(global<u8 *>(0x0080239c)), 0x30) > 0 &&
        at<u32>(Object_GetDef(object), 0x114) != 0)
        score *= global<u32>(0x00785098);
    if (at<s32>(Object_GetDef(object), 0xc4) > 0) {
        const s32 n = at<s32>(Object_GetDef(object), 0xc4) + 1;
        const X87Reg factor = X87_SubR(X87_MulIntDouble(n, -0.1), X87Reg(1.0));   // fild; fmul -0.1; fsubr 1.0
        score = (u32)(long long)X87_MulIntDouble((s32)score, factor.v);
    } else if (at<s32>(Object_GetDef(object), 0x1b8) != 0) {
        const X87Reg part = X87_MulIntDouble(at<s32>(Object_GetDef(object), 0x1b8), (double)float_of(0x39aec33e));   // (about 1/3000)
        if (part.v > 1.0) {   // (fcom 1.0: neither below, equal nor unordered)
            const X87Reg factor = X87_SubR(part * X87Reg((double)-0.1f), X87Reg(1.0));
            score = (u32)(long long)X87_MulIntDouble((s32)score, factor.v);
        }
    }

    // the rides the visitor has been on lately (newest first): the first that is this one, then each of the later four that is
    if (at<u16>(visitor, 0x1e0) == *Thing_GetHandle(object, nullptr, &word))
        score = (u32)((s32)score / 5);
    else if (at<u16>(visitor, 0x1e2) == *Thing_GetHandle(object, nullptr, &word))
        score = (u32)((s32)score / 4);
    else if (at<u16>(visitor, 0x1e4) == *Thing_GetHandle(object, nullptr, &word))
        score = (u32)((s32)score / 3);
    else if (at<u16>(visitor, 0x1e6) == *Thing_GetHandle(object, nullptr, &word))
        score = (u32)((s32)score / 2);
    static const u32 kDivisors[] = {5, 4, 3, 2};
    for (u32 i = 0; i < 4; ++i)
        if (at<u16>(visitor, 0x1e8 + 2 * i) == *Thing_GetHandle(object, nullptr, &word)) score = (u32)((s32)score / (s32)kDivisors[i]);
    Thing_GetHandle(object, nullptr, &word);
    return score;
}

void __fastcall Visitor_ChooseMajorDest(u8 *visitor) {
    s32 best = 0;
    at<u16>(visitor, 0x1dc) = 0;
    u16 handle = at<u16>(global<u8 *>(0x0080239c), 0x1da746);
    while (handle) {
        u8 *const object = global<u8 *>(0x007cfb90 + 20u * handle);
        u16 back;
        if ((at<u8>(object, 0x32) & 4) && *Object_BackOfQueueTo(object, nullptr, &back) != 0) {
            const u32 score = Visitor_ScoreOption(visitor, nullptr, object);
            if (score >= 10 && ((s32)score > best || ((s32)score == best && (at<u8>(global<u8 *>(0x0080239c), 0x1da70c) & 1))) &&
                Object_QueueOpen(object) &&
                Sprite_SetDestinationCell(visitor, nullptr, *Object_BackOfQueueTo(object, nullptr, &back))) {
                best = (s32)score;
                at<u16>(visitor, 0x1dc) = handle;
            }
        }
        handle = at<u16>(object, 0xc);
    }
}
}
