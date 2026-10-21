#include "headquarters.h"

#include "../core/accessors.h"
#include "../core/idmaps.h"
#include "../core/savegame.h"
#include "../core/vcall.h"
#include "advisor.h"
#include "advisorscore.h"
#include "messagecentre.h"
#include "staff.h"
#include "things.h"
#include "world.h"

namespace {

// The thing of a handle: the game table 0x7cfb90 (20-byte entries)'s dword +0.
u8 *TableThing(u32 handle) { return *reinterpret_cast<u8 **>((uintptr_t)(0x007cfb90 + handle * 20)); }

}  // namespace

extern "C" {

u32 __fastcall MechanicHQ_Serialize(u8 *hq, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    if (Thing_Serialize(hq, nullptr, stream, save, version, unused))
        return 1;
    if (version < 0 || version > 1)
        return 0;
    return Save_Field(stream, save, hq + 0xc, 2, 0x0075b2f0) ? 0 : 1;
}

u32 __fastcall StaffHQ_Serialize(u8 *hq, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    if (Thing_Serialize(hq, nullptr, stream, save, version, unused))
        return 1;
    if (version < 0 || version > 1)
        return 0;
    if (!Save_Field(stream, save, hq + 0x64, 4, 0x0075e944) || !Save_Field(stream, save, hq + 0x60, 2, 0x0075e92c))
        return 1;
    for (u32 i = 0; i < 5; ++i)
        if (!Save_Field(stream, save, hq + 0x24 + i * 0xc, 0xc, 0x0075e91c))
            return 1;
    return StaffTraining_Serialize(hq + 0xc, nullptr, stream, save, version, unused) != 0 ? 1 : 0;
}

u8 *__fastcall MechanicHQ_Ctor(u8 *hq) {
    Thing_Construct(hq, nullptr, 1, 0xa);
    *reinterpret_cast<u16 *>(hq + 0xc) = 0;
    MessageCentre_AddThing(MessageCentre_Instance(), nullptr, hq, 7);
    return hq;
}

u8 *__fastcall StaffHQ_Ctor(u8 *hq) {
    Thing_Construct(hq, nullptr, 1, 9);
    StaffTraining_Ctor(hq + 0xc);
    for (u32 offset = 0x24; offset < 0x24 + 5 * 0xc; offset += 0xc) {
        *reinterpret_cast<u32 *>(hq + offset) = 0;
        *reinterpret_cast<u32 *>(hq + offset + 4) = 0;
        *reinterpret_cast<u32 *>(hq + offset + 8) = 0;
    }
    *reinterpret_cast<u16 *>(hq + 0x60) = 0;
    *reinterpret_cast<u32 *>(hq + 0x64) = 0;
    MessageCentre_AddThing(MessageCentre_Instance(), nullptr, hq, 0xc);
    MessageCentre_AddThing(MessageCentre_Instance(), nullptr, hq, 0xe);
    return hq;
}

void __fastcall MechanicHQ_HandleMessage(u8 *, void *, u8 *event) {
    if (vcall<u32>(event, 0) != 7) {
        vcall<u32>(event, 0);   // (the kind again, for the empty Debug_Printf)
        return;
    }
    u8 *const world = *reinterpret_cast<u8 **>((uintptr_t)0x0080239c);
    const u16 who = *reinterpret_cast<const u16 *>(event + 8);
    u16 handle = *reinterpret_cast<const u16 *>(world + 0x1da73e);
    u32 id = 0x44;
    if (handle) {
        for (;;) {
            const u8 *const hq = TableThing(handle);
            const u16 mechanic = *reinterpret_cast<const u16 *>(hq + 0x218);
            if (mechanic == who) {
                if (Get64_4e03d0(TableThing(who))) return;
                id = 0x46;
                break;
            }
            if (mechanic == 0 && Mode19c_ToIndex(hq) != 3 && Mode19c_ToIndex(hq) != 4) return;
            handle = *reinterpret_cast<const u16 *>(hq + 0x210);
            if (!handle) {
                id = 0x45;
                break;
            }
        }
    }
    u8 message[0x18];   // (its bytes +2, +3 and +0x17 never written, sic)
    AdvisorMsg_Init(message, nullptr, id, who, *reinterpret_cast<const u16 *>(TableThing(who) + 0xe), 0xffffffffu, 0, -1);
    Advisor_Post(reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da71c_5194d0(world)), nullptr, message);
}
}
