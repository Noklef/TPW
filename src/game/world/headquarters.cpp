#include "headquarters.h"

#include "../core/savegame.h"
#include "messagecentre.h"
#include "staff.h"
#include "things.h"

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
}
