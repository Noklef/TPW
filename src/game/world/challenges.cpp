#include "challenges.h"

#include <cstdint>
#include <cstring>

#include "../core/accessors.h"
#include "../core/balancefile.h"
#include "../core/filetime.h"
#include "../core/funnytime.h"
#include "../core/idmaps.h"
#include "../core/savegame.h"
#include "messagecentre.h"
#include "money.h"
#include "object.h"
#include "objectdefs.h"
#include "ridesearch.h"
#include "thingmsg.h"
#include "things.h"
#include "thingscan.h"
#include "world.h"
#include "challenges.h"
#include "balancelevel.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// mNextChallengeEventTime (+0x3d0) = the world's funny time now + days (signed, the 64-bit ticks wrapping).
void scheduleIn(u8 *challenges, s32 days) {
    const unsigned long long ticks = (unsigned long long)(long long)days * 864000000000ULL;  // (_allmul)
    const FileTime span = {(u32)ticks, (u32)(ticks >> 32)};
    FileTime now;
    FunnyTime_Now(global<u8 *>(0x0080239c) + 0x2a0, nullptr, &now, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                  nullptr);
    FileTime_Add(&now, nullptr, &at<FileTime>(challenges, 0x3d0), &span);
}

}  // namespace

extern "C" {

void __fastcall BalanceChallenges_Dtor(u8 *file) {
    at<u32>(file, 0) = 0x00700398;
    BalanceFile_Release(file);
}

void __fastcall Challenges_LoadSam(u8 *challenges) {
    u8 *const list = challenges + 0xc;
    for (s32 i = 0; i < 0x14; ++i) at<u32>(list, i * 0x30) = 0;
    u8 file[0x970];
    at<u32>(file, 0) = 0x00700398;
    BalanceFile_Init(file);
    BalanceFile_Load(file, nullptr, reinterpret_cast<const char *>(0x0075a1f4), 0);  // "data/challenges.sam" (a failure only asserts)
    for (const s32 *index = gBalanceLevel.mChallengesInThisLevelChallengeType; index < reinterpret_cast<const s32 *>(gBalanceLevel.unk_7e4); ++index) {
        const s32 k = *index;
        if (k <= 0) continue;
        const u8 *const row = file + 0xc + (u32)k * 0x28;
        u8 rec[0x30] = {};  // (sic: +0x1e, +0x1f and +0x2d are uninitialised stack in the original)
        at<u8>(rec, 0x1d) = at<u32>(row, 0x24) != 0;  // mIndependent
        at<u32>(rec, 0) = at<u32>(row, 0);            // mType
        at<u32>(rec, 4) = at<u32>(row, 8);            // mTargetTime
        at<u32>(rec, 8) = at<u32>(row, 0xc);          // mTargetVal
        at<u16>(rec, 0xc) = at<u16>(row, 0x10);       // mTargetObj
        at<u16>(rec, 0xe) = at<u16>(row, 0x14);       // mTargetObj2
        at<u32>(rec, 0x10) = at<u32>(row, 0x18);      // mTargetStaffType
        at<u32>(rec, 0x14) = at<u32>(row, 0x1c);      // mPrize
        at<u32>(rec, 0x18) = at<u32>(row, 4);         // mDoubleOrNothingFollowup
        at<u8>(rec, 0x1c) = at<u32>(row, 0x20) != 0;  // mCheckAtEndOnly
        s32 slot = 0;
        while (at<u32>(list, slot * 0x30) != 0) {
            if (++slot >= 0x14) break;
        }
        if (slot >= 0x14) continue;
        memcpy(list + slot * 0x30, rec, 0x30);
    }
    Challenges_DropOrphans(challenges);
    BalanceChallenges_Dtor(file);
}

void __fastcall Challenges_DropOrphans(u8 *challenges) {
    u8 *const list = challenges + 0xc;
    for (s32 i = 0; i < 0x14; ++i) {
        u8 *const c = list + i * 0x30;
        if (at<u32>(c, 0) == 0) continue;
        const u32 needs = at<u32>(c, 0x18);
        if (needs == 0) continue;
        bool found = false;
        for (s32 j = 0; j < 0x14; ++j) {
            if (j == i) continue;
            const u32 id = at<u32>(list, j * 0x30);
            if (id != 0 && id == needs) {
                found = true;
                break;
            }
        }
        if (!found) at<u32>(c, 0) = 0;
    }
}

u8 *__fastcall Challenges_Current(u8 *challenges) {
    const s32 index = at<s32>(challenges, 0x3cc);
    if (index < 0 || index >= 0x14) return nullptr;
    return challenges + 0xc + index * 0x30;
}

u8 __fastcall Challenge_Met(u8 *challenge, void *, s32 *value, u32 flag) {
    if (at<u8>(challenge, 0x1c) && !(u8)flag) return 0;
    const s32 target = at<s32>(challenge, 8);
    // (a word of the challenge goes to the calls in the flag's slot, its high half staying)
    const u32 first = (flag & 0xffff0000) | at<u16>(challenge, 0xc);
    u16 handle = (u16)flag;
    switch (at<u32>(challenge, 0)) {
    case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8:
    case 0xb: case 0xc: case 0xd:
    case 0x10: case 0x11: case 0x14: case 0x16:
        return *value >= target;
    case 9:
    case 0xa: {
        const s32 people = Things_CountKind1OnPlainCell();
        if (people <= 0) return 0;
        const s32 some = at<u32>(challenge, 0) == 9 ? Things_CountKind1OnPlainCellWith210() : Things_CountKind1OnPlainCellMode2();
        *value = (s32)((u32)some * 100) / people;
        return *value >= target;
    }
    case 0xe:
        if ((s32)Things_CoasterMostFlaggedCars(&handle, 0) < target) return 0;
        at<u16>(challenge, 0x2e) = handle;
        return 1;
    case 0xf:
        if ((s32)Rides_BestKind11Count(&handle) < target) return 0;
        at<u16>(challenge, 0x2e) = handle;
        return 1;
    case 0x12:
        handle = (u16)first;
        if ((s32)Things_CoasterMostFlaggedCars(&handle, first) < target) return 0;
        at<u16>(challenge, 0x2e) = handle;
        return 1;
    case 0x13:
        handle = (u16)first;
        return (s32)Things_CoasterMostFlaggedCars(&handle, first) >= 3;
    case 0x15:
        handle = (u16)first;
        return (s32)Rides_BestPiecesOfKinds(&handle, first) >= target;
    case 0x18: case 0x19: case 0x1b:
        return *value / at<s32>(challenge, 4) >= target;
    case 0x1c:
        return Things_MaxKind3Byte50(first) >= target;
    case 0x1d: {
        u32 kind;
        switch (at<u32>(challenge, 0x10)) {
        case 0: kind = 5; break;
        case 1: kind = 4; break;
        case 2: kind = 6; break;
        case 3: kind = 7; break;
        case 4: kind = 8; break;
        default: kind = flag; break;
        }
        *value = Things_AverageOfKind(kind);
        return *value >= target;
    }
    case 0x1e: case 0x1f: case 0x20:
        return (s32)ObjectControls_GetWord18(first) > 0;
    case 0x21:
        return (s32)ObjectControls_GetWord18(first) > 0 && (s32)ObjectControls_GetWord18((flag & 0xffff0000) | at<u16>(challenge, 0xe)) > 0;
    default:
        return 0;
    }
}

void __fastcall Challenge_Finish(u8 *challenge) {
    at<u8>(challenge, 0x2c) = 1;
    if (at<u32>(challenge, 0x18)) return;
    u8 msg[0x30];   // (bytes 0x11-0x13 left as the stack had them, as the original's)
    ThingMsg19_Ctor(msg, nullptr, Kind_ToIdC(reinterpret_cast<const u32 *>(challenge)), 0x110, 0, Get14_6bb1a0(challenge), 0, 0, 0, 0, 0, 0,
                    0);
    Object_PostMessage(msg);
}

void __fastcall Challenge_Attempt(u8 *challenge, void *, u32 thing) {
    ++at<u32>(challenge, 0x24);
    u8 msg[0x30];   // (bytes 0x11-0x13 left as the stack had them, as the original's)
    ThingMsg19_Ctor(msg, nullptr, Kind_ToIdD(reinterpret_cast<const u32 *>(challenge)), 0x111, 0, Get14_6bb1a0(challenge),
                    Get4_5dc650(challenge), Get8_51c050(challenge), 0, thing, 0, 0, 0);
    Object_PostMessage(msg);
}

u8 __fastcall Challenges_Announce(u8 *, void *, u8 *challenge, u32 offer) {
    if (!challenge) return 0;
    u8 msg[0x30];   // (bytes 0x11-0x13 left as the stack had them, as the original's)
    ThingMsg19_Ctor(msg, nullptr, Kind_ToIdA(reinterpret_cast<const u32 *>(challenge)), (u8)offer ? 0x158 : 0x10e, 1, at<u32>(challenge, 0x14),
                    at<u32>(challenge, 4), at<u32>(challenge, 8), 0, 0, at<u16>(challenge, 0xc), at<u16>(challenge, 0xe), at<u32>(challenge, 0x10));
    Object_PostMessage(msg);
    return 1;
}

void __fastcall Challenges_OfferNext(u8 *challenges, void *, u32 offer) {
    if (at<u8>(challenges, 0x3d9) || at<u8>(challenges, 0x3da)) return;
    const s32 current = at<s32>(challenges, 0x3cc);
    at<u32>(challenges, 0x3dc) = 0;
    const s32 most = gBalanceLevel.mDeclinesToForfeit;
    // (32-bit wrapping counts, compared signed, as the original's)
    for (u32 i = current >= 0 ? (u32)current + 1 : 0; (s32)i <= (s32)((u32)at<s32>(challenges, 0x3cc) + 0x14); ++i) {
        const s32 k = (s32)i % 0x14;
        const u8 *const c = challenges + 0xc + k * 0x30;
        if (!at<u32>(c, 0) || at<u8>(c, 0x2c) || at<s32>(c, 0x28) >= most || !at<u8>(c, 0x1d)) continue;
        at<s32>(challenges, 0x3cc) = k;
        if (Challenges_Announce(challenges, nullptr, Challenges_Current(challenges), offer)) at<u8>(challenges, 0x3da) = 1;
        return;
    }
}

u8 __fastcall Challenges_OfferFollowup(u8 *challenges) {
    if (at<u8>(challenges, 0x3d9) || !Challenges_Current(challenges)) return 0;
    const u32 followup = at<u32>(Challenges_Current(challenges), 0x18);
    if (!followup) return 0;
    for (s32 i = 0; i < 0x14; ++i) {
        const u8 *const c = challenges + 0xc + i * 0x30;
        if (!at<u32>(c, 0) || at<u8>(c, 0x2c) || at<s32>(c, 0x28) >= gBalanceLevel.mDeclinesToForfeit || at<u32>(c, 0) != followup) continue;
        at<s32>(challenges, 0x3cc) = i;
        if (Challenges_Announce(challenges, nullptr, Challenges_Current(challenges), 0)) {
            at<u32>(challenges, 0x3dc) = 0;
            at<u8>(challenges, 0x3da) = 1;
            return 1;
        }
    }
    return 0;
}

void __fastcall Challenges_AcceptOffer(u8 *challenges) {
    if (!at<u8>(challenges, 0x3da)) return;
    const u8 *const current = Challenges_Current(challenges);
    if (!current) return;
    at<u8>(challenges, 0x3da) = 0;  // mWaitingForReply
    at<u8>(challenges, 0x3d9) = 1;  // mChallengeOn
    scheduleIn(challenges, at<s32>(current, 4));  // mTargetTime
}

void __fastcall Challenges_DeclineOffer(u8 *challenges) {
    if (!at<u8>(challenges, 0x3da)) return;
    at<u8>(challenges, 0x3da) = 0;
    at<u8>(challenges, 0x3d9) = 0;
    const u8 *const current = Challenges_Current(challenges);  // (null for an index out of 0-19: the original reads +0x1d all the same)
    if (at<u8>(current, 0x1d) == 0) {  // mIndependent
        u8 *const money = reinterpret_cast<u8 *>(GameTable7cfb90_Get1da726_5195d0(global<u8 *>(0x0080239c)));
        Money_Credit(money, nullptr, at<s32>(current, 0x14) / 2);  // half mPrize
        at<u16>(challenges, 0x3e0) = 0;  // mActualThing
    }
    scheduleIn(challenges, gBalanceLevel.mDaysAfterDeclinedChallenge);  // gBalanceLevel.mDaysAfterDeclinedChallenge
}

void __fastcall Challenges_Won(u8 *challenges) {
    at<u8>(challenges, 0x3d9) = 0;
    scheduleIn(challenges, gBalanceLevel.mDaysAfterCompletedChallenge);
    const u8 *const current = Challenges_Current(challenges);
    if (!current) return;
    if (at<u32>(current, 0x18)) {
        at<u16>(challenges, 0x3e0) = at<u16>(current, 0x2e);
        if (Challenges_OfferFollowup(challenges)) at<u16>(Challenges_Current(challenges), 0x2e) = at<u16>(challenges, 0x3e0);
    } else {
        at<u16>(challenges, 0x3e0) = 0;
        u8 *const money = reinterpret_cast<u8 *>(GameTable7cfb90_Get1da726_5195d0(global<u8 *>(0x0080239c)));
        Money_Credit(money, nullptr, at<s32>(current, 0x14));
    }
    Challenge_Finish(Challenges_Current(challenges));
}

void __fastcall Challenges_Lost(u8 *challenges) {
    at<u8>(challenges, 0x3d9) = 0;
    scheduleIn(challenges, gBalanceLevel.mDaysAfterCompletedChallenge);
    u8 *const current = Challenges_Current(challenges);
    if (current) Challenge_Attempt(current, nullptr, at<u32>(challenges, 0x3dc));
    at<u16>(challenges, 0x3e0) = 0;
}

const u8 *__fastcall BalanceChallenges_GetSchema(const u8 *, void *, u32 index) {
    return reinterpret_cast<const u8 *>(0x00739b98) + index * 0x3c;
}
u32 __fastcall Challenge_Serialize(u8 *challenge, void *, Stream *stream, u32 save, s32 version, u32) {
    static const SaveSpec kFields[] = {{0x0, 4, (u32)&gText_Challenges_75a034},  {0x4, 4, 0x0075a168},  {0x8, 4, (u32)&gText_Challenge_Serialize_75a15c},  {0xc, 2, (u32)&gText_Challenge_Serialize_75a150},  {0xe, 2, (u32)&gText_Challenge_Serialize_75a144},
                                       {0x10, 4, (u32)&gText_Challenge_Serialize_75a130}, {0x14, 4, (u32)&gText_Challenge_Serialize_75a128}, {0x18, 4, (u32)&gText_Challenge_Serialize_75a10c}, {0x1c, 1, (u32)&gText_Challenge_Serialize_75a0fc}, {0x20, 4, (u32)&gText_Challenge_Serialize_75a0e8},
                                       {0x24, 4, (u32)&gText_Challenge_Serialize_75a0d8}, {0x28, 4, (u32)&gText_Challenge_Serialize_75a0c4}, {0x2c, 1, (u32)&gText_Challenge_Serialize_75a0b8}, {0x1d, 1, (u32)&gText_Challenge_Serialize_75a0a8}, {0x2e, 2, (u32)&gText_Challenge_Serialize_75a094}};
    if (version < 0 || version > 1)
        return 0;
    return Save_Fields(stream, save, challenge, kFields, 15) ? 0 : 1;
}

u32 __fastcall Challenges_Serialize(u8 *challenges, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    static const SaveSpec kFields[] = {{0x3cc, 4, (u32)&gText_Challenges_Serialize_75a2b0}, {0x3d0, 8, 0x0075a298}, {0x3d8, 1, (u32)&gText_Challenges_Serialize_75a284}, {0x3d9, 1, (u32)&gText_Challenges_Serialize_75a274},
                                       {0x3da, 1, (u32)&gText_Challenges_Serialize_75a260}, {0x3dc, 4, (u32)&gText_Challenges_Serialize_75a258}, {0x3e0, 2, (u32)&gText_Challenges_Serialize_75a248}};
    Thing_Serialize(challenges, nullptr, stream, save, version, unused);  // sic: the result is ignored
    if (version < 0 || version > 1)
        return 0;
    for (u32 i = 0; i < 0x14; ++i)
        Challenge_Serialize(challenges + 0xc + i * 0x30, nullptr, stream, save, version, unused);  // sic: the results are ignored
    return Save_Fields(stream, save, challenges, kFields, 7) ? 0 : 1;
}

u8 *__fastcall Challenges_Ctor(u8 *challenges) {
    Thing_Construct(challenges, nullptr, 1, 0x13);
    for (u32 offset = 0xc; offset < 0xc + 0x14 * 0x30; offset += 0x30) {
        at<u32>(challenges, offset) = 0;
        at<u32>(challenges, offset + 0x20) = 0;
        at<u32>(challenges, offset + 0x24) = 0;
        at<u32>(challenges, offset + 0x28) = 0;
        at<u8>(challenges, offset + 0x2c) = 0;
    }
    at<u32>(challenges, 0x3cc) = 0xffffffff;
    at<u32>(challenges, 0x3d0) = 0;
    at<u32>(challenges, 0x3d4) = 0;
    at<u8>(challenges, 0x3d8) = 0;
    at<u8>(challenges, 0x3d9) = 0;
    at<u8>(challenges, 0x3da) = 0;
    at<u32>(challenges, 0x3dc) = 0;
    at<u16>(challenges, 0x3e0) = 0;
    MessageCentre_AddThing(MessageCentre_Instance(), nullptr, challenges, 0x1a);
    MessageCentre_AddThing(MessageCentre_Instance(), nullptr, challenges, 0xb);
    MessageCentre_AddThing(MessageCentre_Instance(), nullptr, challenges, 0x11);
    Challenges_LoadSam(challenges);
    return challenges;
}
}
