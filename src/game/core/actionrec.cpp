#include "actionrec.h"

#include <cstdint>

#include "../files/file.h"
#include "../files/handles.h"
#include "../world/world.h"
#include "gameheap.h"
#include "savegame.h"
#include "vcall.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

ActionRec *gActionRec() { return reinterpret_cast<ActionRec *>((uintptr_t)0x00785a08); }

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

StreamBase *stream(ActionRec *r, u32 i) { return reinterpret_cast<StreamBase *>(&r->stream[i]); }

void close(ActionRec *r, u32 i) { vcall<void>(stream(r, i), 4); }

// The streams opened (the first, the others in mode 0), then the start record.
void begin(ActionRec *r, u32 mode) {
    s32 result;
    StreamBase_OpenMemory(stream(r, 0), nullptr, &result, 0x200);
    if (mode == 0) {
        StreamBase_OpenMemory(stream(r, 1), nullptr, &result, 0x200);
        StreamBase_OpenMemory(stream(r, 2), nullptr, &result, 0x200);
    }
    u8 zeros[0x80] = {};
    if (writing(r)) {
        put8(r, 0xe7);
        put(r, zeros, 0x80);
    }
}

void set(ActionRec *r, u32 offset, u32 value) { *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(r) + offset) = value; }

void *const *const kStreamVtable = reinterpret_cast<void *const *>(0x6fd6a0);

}  // namespace

extern "C" {

ActionRec *__fastcall ActionRec_Ctor(ActionRec *r) {
    for (u32 i = 0; i < 3; ++i) {
        StreamBase_Ctor(stream(r, i));
        stream(r, i)->vtable = kStreamVtable;
    }
    r->playback = 0;
    static const u32 kSettings[][2] = {
        {0xd4, 2},  {0xe8, 2},  {0x110, 8},  {0x114, 8},  {0x134, 8}, {0x138, 8}, {0x14c, 9}, {0x160, 9}, {0x34, 1},  {0x174, 0xa},
        {0x188, 0xa}, {0x118, 4}, {0x11c, 4}, {0x98, 1},  {0x70, 0},  {0x84, 0},  {0x48, 3},  {0xac, 1},  {0xc0, 1},  {0x5c, 1},
        {0xfc, 1},  {0x120, 5}, {0x124, 7},  {0x128, 1},  {0x12c, 1}, {0x130, 6}, {0x13c, 1}, {0x140, 1}, {0x144, 1}, {0x148, 6},
        {0x3d0, 0xb}};
    for (const auto &s : kSettings) set(r, s[0], s[1]);
    r->mode = 0;
    r->recording = 1;
    begin(r, 0);
    r->unknown8 = 0;
    return r;
}

void __fastcall ActionRec_Dtor(ActionRec *r) {
    if (r->mode == 0) {
        close(r, 0);
        close(r, 1);
        close(r, 2);
    } else {
        close(r, 0);
    }
    StreamBase_Dtor(stream(r, 2));
    StreamBase_Dtor(stream(r, 1));
    StreamBase_Dtor(stream(r, 0));
}

void __fastcall ActionRec_Start(ActionRec *r, void *, u32 mode) {
    r->mode = mode;
    r->recording = 1;
    begin(r, mode);
}

void __fastcall ActionRec_Restart(ActionRec *r) {
    r->recording = 0;
    close(r, 0);
    if (r->mode == 0) {
        close(r, 1);
        close(r, 2);
    }
    const u32 mode = r->mode;
    r->recording = 1;
    begin(r, mode);
}

u32 __fastcall ActionRec_Serialize(ActionRec *r, void *, Stream *s, u32 save, s32 version, u32) {
    if (version < 0 || version > 1) return 0;
    const char *const value = reinterpret_cast<const char *>((uintptr_t)0x00747144);
    const char *const sized = reinterpret_cast<const char *>((uintptr_t)0x00747134);
    StreamBase *const first = stream(r, 0);
    s32 result, done = 0, size;
    if (save) {
        Save_FieldName(4, value, 0);
        if (*Stream_Write(&result, s, &r->unknown8, 4, &done) < 0 || done != 4) return 1;
        size = vcall<s32>(first, 0x14);
        Save_FieldName(4, sized, 0);
        done = 0;
        if (*Stream_Write(&result, s, &size, 4, &done) < 0 || done != 4) return 1;
        void *const bytes = GameHeap_Alloc((u32)size);
        done = 0;
        vcall<s32>(first, 0x10, 0, 0);
        vcall<s32>(first, 8, bytes, size);
        vcall<s32>(first, 0x10, 0, 2);
        const bool failed = *Stream_Write(&result, s, bytes, size, &done) < 0;
        GameHeap_Free(bytes);
        return failed ? 1 : 0;
    }
    r->recording = 0;
    close(r, 0);
    if (r->mode == 0) {
        close(r, 1);
        close(r, 2);
    }
    ActionRec_Start(r, nullptr, r->mode);
    Save_FieldName(4, value, 1);
    if (*Stream_Read(&result, s, &r->unknown8, 4, &done) < 0 || done != 4) return 1;
    Save_FieldName(4, sized, 1);
    done = 0;
    if (*Stream_Read(&result, s, &size, 4, &done) < 0 || done != 4) return 1;
    void *const bytes = GameHeap_Alloc((u32)size);
    done = 0;
    if (*Stream_Read(&result, s, bytes, size, &done) < 0) {
        GameHeap_Free(bytes);
        return 1;
    }
    vcall<s32>(first, 0xc, bytes, size);
    vcall<s32>(first, 0x10, 0, 2);
    GameHeap_Free(bytes);
    return 0;
}

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

void __fastcall ActionRec_WriteFW(ActionRec *r, void *, u32 code, u32 a, u32 b) {
    if (!writing(r)) return;
    put8(r, code);
    put(r, &a, 4);
    put16(r, b);
}

void __fastcall ActionRec_WriteFFW(ActionRec *r, void *, u32 code, u32 a, u32 b, u32 c) {
    if (!writing(r)) return;
    put8(r, code);
    put(r, &a, 4);
    put(r, &b, 4);
    put16(r, c);
}

void __cdecl ActionRec_RecordCoaster45(const u8 *coaster, u32 value) {
    ActionRec_WriteWW(gActionRec(), nullptr, 0x45, *reinterpret_cast<const u16 *>(coaster + 0x14), value);
}

void __cdecl Global81b0cc_SetRecorded(u32 value) {
    ActionRec_WriteW(gActionRec(), nullptr, 0x23, value);
    global<u32>(0x0081b0cc) = value;
}

void __cdecl Global81ecac_SetRecorded(u32 value) {
    ActionRec_WriteW(gActionRec(), nullptr, 0x1e, value);
    global<u32>(0x0081ecac) = value;
}

void __cdecl Global81d7a4_SetRecorded(u32 value, u32 force) {
    if (value != global<u32>(0x0081b134) || (u8)force != 0) ActionRec_WriteW(gActionRec(), nullptr, 0xa, value);
    global<u32>(0x0081b134) = global<u32>(0x0081d7a4);
    global<u32>(0x0081d7a4) = value;
}

void __cdecl Global8186e0_SetRecorded(const u16 *id, u32 flag) {
    ActionRec_WriteWB(gActionRec(), nullptr, 5, *id, flag);
    World_MarkRideBuilt(global<u8 *>(0x007cf83c), nullptr, *id);
    global<u16>(0x008186ec) = global<u16>(0x008186e0);
    global<u32>(0x008187f8) = flag;
    global<u16>(0x008186e0) = *id;
}
}
