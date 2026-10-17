#include "actionrec.h"

#include "vcall.h"

namespace {

bool writing(const ActionRec *r) { return r->playback == 0 && r->recording != 0; }

void put(ActionRec *r, const void *buffer, s32 count) { vcall<s32>(&r->stream[0], 0xc, buffer, count); }

void put8(ActionRec *r, u32 v) {
    const u8 b = (u8)v;
    put(r, &b, 1);
}

void put16(ActionRec *r, u32 v) {
    const u16 w = (u16)v;
    put(r, &w, 2);
}

}  // namespace

extern "C" {

void __fastcall ActionRec_Write(ActionRec *r, void *, u32 code) {
    if (!writing(r)) return;
    put8(r, code);
}

void __fastcall ActionRec_WriteBB(ActionRec *r, void *, u32 code, u32 a, u32 b) {
    if (!writing(r)) return;
    const u8 bytes[3] = {(u8)code, (u8)a, (u8)b};
    put(r, bytes, 3);
}

void __fastcall ActionRec_WriteBBW(ActionRec *r, void *, u32 code, u32 a, u32 b, u32 c) {
    if (!writing(r)) return;
    const u8 bytes[3] = {(u8)code, (u8)a, (u8)b};
    put(r, bytes, 3);
    put16(r, c);
}

void __fastcall ActionRec_WriteW(ActionRec *r, void *, u32 code, u32 a) {
    if (!writing(r)) return;
    put8(r, code);
    put16(r, a);
}

void __fastcall ActionRec_WriteWW(ActionRec *r, void *, u32 code, u32 a, u32 b) {
    if (!writing(r)) return;
    put8(r, code);
    put16(r, a);
    put16(r, b);
}

void __fastcall ActionRec_WriteWWW(ActionRec *r, void *, u32 code, u32 a, u32 b, u32 c) {
    if (!writing(r)) return;
    put8(r, code);
    put16(r, a);
    put16(r, b);
    put16(r, c);
}

void __fastcall ActionRec_WriteWB(ActionRec *r, void *, u32 code, u32 a, u32 b) {
    if (!writing(r)) return;
    put8(r, code);
    put16(r, a);
    put8(r, b);
}
}
