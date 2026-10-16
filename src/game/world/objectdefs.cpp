#include "objectdefs.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__fastcall ObjectDefs_Find(u8 *defs, void *, u16 id) {
    if (id == at<u16>(defs, 0x730)) return at<u8 *>(defs, 0x734);
    const s32 count = at<s32>(defs, 0x528);
    for (s32 i = 0; i < count; ++i) {
        u8 *def = at<u8 *>(defs, i * 4);
        if (at<u16>(def, 0) == id) {
            at<u16>(defs, 0x730) = id;
            at<u8 *>(defs, 0x734) = def;
            return def;
        }
    }
    return nullptr;
}

u8 *__fastcall ObjectControls_Find(u8 *table, void *, u32 id, u32) {
    if ((u16)id == at<u16>(table, 0x12c4))
        return at<u8 *>(table, 0x12c8);
    const s32 count = at<s32>(table, 0x12c0);
    for (s32 i = 0; i < count; ++i) {
        u8 *record = table + 0x20 * (u32)i;
        if ((u16)id == at<u16>(record, 0)) {
            at<u16>(table, 0x12c4) = (u16)id;
            at<u8 *>(table, 0x12c8) = record;
            return record;
        }
    }
    return nullptr;
}

u8 *__fastcall ObjectControls_FindUncached(u8 *table, void *, u32 id, u32) {
    if ((u16)id == at<u16>(table, 0x12c4))
        return at<u8 *>(table, 0x12c8);
    const s32 count = at<s32>(table, 0x12c0);
    for (s32 i = 0; i < count; ++i) {
        u8 *record = table + 0x20 * (u32)i;
        if ((u16)id == at<u16>(record, 0))
            return record;
    }
    return nullptr;
}

void __fastcall ObjectControls_Init(u8 *table) {
    for (u32 i = 0; i < 0x4b3; ++i)
        at<u32>(table, 4 * i) = 0;
    ObjectControls_Collect(table);
}

u32 __stdcall ObjectControls_GetWord18(u32 id) {
    u8 *game = *reinterpret_cast<u8 **>(0x0080239c);
    return at<u32>(ObjectControls_FindUncached(game + 0x1d943c, nullptr, id, 0), 0x18);
}

u8 *__fastcall Obj_FindControls(const u8 *object) {
    u8 *game = *reinterpret_cast<u8 **>(0x0080239c);
    return ObjectControls_Find(game + 0x1d943c, nullptr, at<u16>(object, 0xe), 0);
}

void __fastcall ObjectControls_Collect(u8 *table) {
    u8 *defs = reinterpret_cast<u8 *>(0x007890a0);
    for (u8 *def = ObjectDefs_First(defs); def; def = ObjectDefs_Next(defs)) {
        const u16 id = at<u16>(def, 0);
        u8 *record = nullptr;
        if (id == at<u16>(table, 0x12c4)) {
            record = at<u8 *>(table, 0x12c8);
        } else {
            const s32 count = at<s32>(table, 0x12c0);
            for (s32 i = 0; i < count; ++i) {
                if (at<u16>(table, 0x20 * (u32)i) == id) {
                    at<u16>(table, 0x12c4) = id;
                    record = table + 0x20 * (u32)i;
                    at<u8 *>(table, 0x12c8) = record;
                    break;
                }
            }
        }
        if (record)
            continue;
        const u32 count = at<u32>(table, 0x12c0);
        record = table + 0x20 * count;
        at<u32>(table, 0x12c0) = count + 1;
        at<u16>(record, 0) = at<u16>(def, 0);
        at<u32>(record, 4) = at<u32>(def, 0x1b8);
        at<u32>(record, 8) = at<u32>(def, 0xe8);
        at<u32>(record, 0xc) = at<u32>(def, 0x16c);
        at<u32>(record, 0x14) = 0;
        at<u8>(record, 0x10) = at<u32>(def, 0x180) == 0 ? 1 : 0;
        at<u32>(record, 0x18) = 0;
        at<u32>(record, 0x1c) = 0;
    }
}

u8 *__fastcall ObjectDefs_FindAt390(u8 *defs, void *, u16 id) {
    u8 *def = ObjectDefs_Find(defs, nullptr, id);
    return def ? def + 0x390 : nullptr;
}

u8 *__fastcall ObjectDefs_FindAt410(u8 *defs, void *, u16 id) {
    u8 *def = ObjectDefs_Find(defs, nullptr, id);
    return def ? def + 0x410 : nullptr;
}

u8 *__fastcall ObjectDefs_First(u8 *defs) {
    at<s32>(defs, 0x72c) = 0;
    return at<u8 *>(defs, 0);
}

u8 *__fastcall ObjectDefs_Next(u8 *defs) {
    s32 &cursor = at<s32>(defs, 0x72c);
    if (cursor >= at<s32>(defs, 0x528) - 1) return nullptr;
    return at<u8 *>(defs, ++cursor * 4);
}

const u8 *__fastcall ObjectDef_GetSchema(const void *, void *, u32 index) {
    return reinterpret_cast<const u8 *>(0x00744b30) + index * 0x3c;
}

const u8 *__fastcall TrackType_GetSchema(const void *, void *, u32 index) {
    return reinterpret_cast<const u8 *>(0x00739ea8) + index * 0x3c;
}
}
