#include "uimsgreceiver.h"

#include "messagecentre.h"
#include "things.h"

extern "C" u8 *__fastcall UIMsgReceiver_Ctor(u8 *receiver) {
    Thing_Construct(receiver, nullptr, 1, 0x11);
    static const s32 kSlots[] = {0x12, 0xa, 7, 8, 0x1b, 0xf, 0x13, 0x1c};
    for (s32 slot : kSlots)
        MessageCentre_AddThing(MessageCentre_Instance(), nullptr, receiver, slot);
    return receiver;
}
