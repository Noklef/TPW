#include "objectdefs.h"

#include <cstdint>
#include <cstring>

#include "../core/balancefile.h"
#include "../core/string.h"
#include "../files/direntry.h"
#include "../files/handles.h"
#include "../core/crt.h"
#include "../core/defids.h"
#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "../files/file.h"
#include "../files/wtext.h"
#include "../core/savegame.h"
#include "../core/soundset.h"
#include "../render/particles.h"
#include "../core/textarg.h"
#include "objectdefs.h"

namespace {

// From a line's start: past its end and its line break (CR LF and LF CR as one); there when that line has more than spaces and tabs,
// else on to the next non-blank line (WText_NextLine).
const u16 *nextLine(const u16 *q) {
    while (*q && *q != 0xa && *q != 0xd) ++q;
    if (!*q) return q;
    if ((q[0] == 0xa && q[1] == 0xd) || (q[0] == 0xd && q[1] == 0xa))
        q += 2;
    else
        ++q;
    for (const u16 *r = q; *r && *r != 0xa && *r != 0xd; ++r)
        if (*r != 0x20 && *r != 9) return q;
    return WText_NextLine(q);
}

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// A definition's cell grid (+0x18): the column count, the row count, then cells of 8 bytes (a kind and a byte), 20 to a row.
u8 *grid_cell(u8 *grid, s32 column, s32 row) { return grid + 8 + 8 * column + 0xa0 * row; }

// The first cell with the kind, column by column (the row count read once, before the scan).
bool find_cell(u8 *grid, u32 kind, s32 *column, s32 *row) {
    const s32 columns = at<s32>(grid, 0);
    if (columns <= 0) return false;
    const s32 rows = at<s32>(grid, 4);
    for (s32 i = 0; i < columns; ++i)
        for (s32 j = 0; j < rows; ++j)
            if (at<u32>(grid_cell(grid, i, j), 0) == kind) {
                *column = i;
                *row = j;
                return true;
            }
    return false;
}

// The object text table's row for the id (0x7488d8 .. 0x749850, 0xc bytes a row; an id 0 ends it), or none.
const u8 *object_text_row(u32 id) {
    for (u32 row = 0x007488d8; row < 0x00749850; row += 0xc) {
        const u32 k = global<u32>(row);
        if (k == 0) break;
        if (k == id) return reinterpret_cast<const u8 *>((uintptr_t)row);
    }
    return nullptr;
}

// A text of the game's text table (0x78bacc) into to (size chars).
void format_text(u16 *to, u32 size, u32 text) { TextFormat().format(global<void *>(0x0078bacc), to, size, text); }

// A text ending in '-' loses it (true).
bool cut_dash(u16 *text) {
    const s32 n = (s32)Crt().wcslen(text);
    if (n > 0 && text[n - 1] == '-') {
        text[n - 1] = 0;
        return true;
    }
    return false;
}

// A definition's effects (+0x4e0-+0x4ec, ParticleEffect_Clear118) and sound category (+0x4f0, SoundCategory_Release) let go, each that
// is set (0x412cb0's and 0x4145f0's, inlined in both).
void ReleaseEffects(const u8 *def) {
    for (u32 offset = 0x4e0; offset < 0x4f0; offset += 4)
        if (at<u32>(def, offset)) ParticleEffect_Clear118(at<u32>(def, offset));
    if (at<u32>(def, 0x4f0)) SoundCategory_Release(at<u32>(def, 0x4f0));
}

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

u32 __fastcall ObjectControls_Serialize(u8 *table, void *, Stream *stream, u32 save, s32 version, u32) {
    if (version < 0 || version > 1)
        return 0;
    for (u32 i = 0; i < 0x96; ++i)
        if (!Save_Field(stream, save, table + i * 0x20, 0x20, (u32)&gText_ObjectControls_Serialize_75a3c8))
            return 1;
    if (!Save_Field(stream, save, table + 0x12c0, 4, (u32)&gText_ObjectControls_Serialize_75a3b4) || !Save_Field(stream, save, table + 0x12c4, 2, (u32)&gText_ObjectControls_Serialize_75a3a0))
        return 1;
    if (save == 0) {
        at<u32>(table, 0x12c8) = 0;
        at<u16>(table, 0x12c4) = 0;  // sic: the search key just loaded
    }
    return 0;
}

s32 __cdecl ObjectDefs_PercentWithControls() {
    u8 *defs = reinterpret_cast<u8 *>(0x007890a0);
    s32 counted = 0, withControls = 0;
    for (u8 *def = ObjectDefs_First(defs); def; def = ObjectDefs_Next(defs)) {
        if (at<s32>(def, 0xc4) <= 0)
            continue;
        ++counted;
        u8 *game = *reinterpret_cast<u8 **>(0x0080239c);
        if (at<s32>(ObjectControls_FindUncached(game + 0x1d943c, nullptr, at<u16>(def, 0), 0), 0x18) > 0)
            ++withControls;
    }
    return counted == 0 ? 0 : withControls * 100 / counted;
}

namespace {
// The control record of a definition (by its id +0), quiet, in the game's table.
const u8 *controls_of(const u8 *def) {
    u8 *game = *reinterpret_cast<u8 **>(0x0080239c);
    return ObjectControls_FindUncached(game + 0x1d943c, nullptr, at<u16>(def, 0), 0);
}

s32 count_kind_controls10(u32 kind) {
    u8 *defs = reinterpret_cast<u8 *>(0x007890a0);
    s32 count = 0;
    for (u8 *def = ObjectDefs_First(defs); def; def = ObjectDefs_Next(defs))
        if (at<u32>(def, 0x4ac) == kind && at<u8>(controls_of(def), 0x10)) ++count;
    return count;
}
}  // namespace

s32 __stdcall ObjectDefs_CountKind0WithControls14(u32) {
    u8 *defs = reinterpret_cast<u8 *>(0x007890a0);
    s32 count = 0;
    for (u8 *def = ObjectDefs_First(defs); def; def = ObjectDefs_Next(defs))
        if (at<u32>(def, 0x4ac) == 0 && at<s32>(controls_of(def), 0x14) > 0) ++count;
    return count;
}

s32 __stdcall ObjectDefs_CountKind1WithControls10(u32) { return count_kind_controls10(1); }
s32 __stdcall ObjectDefs_CountKind2WithControls10(u32) { return count_kind_controls10(2); }
s32 __stdcall ObjectDefs_CountKind3WithControls10(u32) { return count_kind_controls10(3); }

s32 __stdcall ObjectDefs_CountKind0WithControls10(u32 flag) {
    u8 *defs = reinterpret_cast<u8 *>(0x007890a0);
    s32 count = 0;
    for (u8 *def = ObjectDefs_First(defs); def; def = ObjectDefs_Next(defs))
        if (at<u32>(def, 0x4ac) == 0 && at<u8>(controls_of(def), 0x10) && (at<u32>(def, 0xc4) == 0 || (u8)flag)) ++count;
    return count;
}

s32 __stdcall ObjectDefs_CountKindWithControls18(u32 kind, u32 flag) {
    u8 *defs = reinterpret_cast<u8 *>(0x007890a0);
    s32 count = 0;
    for (u8 *def = ObjectDefs_First(defs); def; def = ObjectDefs_Next(defs))
        if (at<u32>(def, 0x4ac) == kind && at<s32>(controls_of(def), 0x18) > 0 && ((u8)flag || at<u32>(def, 0xc4) == 0)) ++count;
    return count;
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

u8 __stdcall Table76e300_GetByte(u32 id) {
    if (id == 0x160 || id == 0x15f)
        return 0;
    const u32 slot = 0x0076e300 + 56 * id;
    if (global<u32>(slot + 4) == id)
        return global<u8>(slot + 0x20);
    for (u32 p = 0x0076e304; (s32)p < 0x00772fcc; p += 0x38) {
        if (global<u32>(p) == id)
            return global<u8>(slot + 0x20);
    }
    return 0;
}

u32 __fastcall Table76e300_GetObjectDword(const u8 *object, void *, u32 id) {
    const u32 record = Table76e300_FindRecord(id);
    if (!record)
        return 0;
    return at<u32>(object, 0x24 + 12 * global<u32>(record + 8));
}

u8 __fastcall Table76e300_ObjectDwordSet(const u8 *object, void *, u32 id) {
    const u32 record = Table76e300_FindRecord(id);
    if (!record)
        return 0;
    return at<u32>(object, 0x28 + 12 * global<u32>(record + 8)) != 0;
}

extern "C" u32 Table76e300_FindRecord(u32 id) {
    if (id == 0x160 || id == 0x15f)
        return 0;
    const u32 slot = 0x0076e300 + 56 * id;
    if (global<u32>(slot + 4) == id)
        return slot;
    for (u32 p = 0x0076e304; (s32)p < 0x00772fcc; p += 0x38) {
        if (global<u32>(p) == id)
            return slot;
    }
    return 0;
}

u8 __fastcall Table76e300_LimitCheck(const u8 *object, void *, u32 id, s32 limit) {
    const u32 record = Table76e300_FindRecord(id);
    const u32 index = record ? global<u32>(record + 8) : 10;
    if (index == 10)
        return 0;
    const s32 value = at<s32>(object, 0x2c + 12 * index);
    if (value == 0)
        return 0;
    return limit >= value;
}

u32 __fastcall Obj_DefKind123(const u8 *object) {
    u8 *defs = reinterpret_cast<u8 *>(0x007890a0);
    if (at<u32>(ObjectDefs_Find(defs, nullptr, at<u16>(object, 0xe)), 0x9c) == 1)
        return 1;
    if (at<u32>(ObjectDefs_Find(defs, nullptr, at<u16>(object, 0xe)), 0x9c) == 2)
        return 1;
    return at<u32>(ObjectDefs_Find(defs, nullptr, at<u16>(object, 0xe)), 0x9c) == 3 ? 1 : 0;
}

u8 __stdcall Record_IsActive(u32 id) {
    if (id == 0x160 || id == 0x15f) return 0;
    const u8 *const record = reinterpret_cast<const u8 *>(0x0076e300 + id * 0x38);
    if (at<u32>(record, 4) != id) {
        bool found = false;
        for (u32 scan = 0x0076e304; (s32)scan < 0x00772fcc; scan += 0x38) {
            if (global<u32>(scan) == id) { found = true; break; }
        }
        if (!found) return 0;  // (the original prints that it was not found)
    }
    return at<u32>(record, 8) == 1 ? 1 : 0;
}

u32 __stdcall DefField24_59c240(u32 v0) {
    const u8 *record = ObjectDef_FindById(v0); return record ? at<u32>(record, 0x24) : 0x266;
}

u32 __stdcall DefField28_59c2b0(u32 v0) {
    const u8 *record = ObjectDef_FindById(v0); return record ? at<u32>(record, 0x28) : 0x0;
}

u32 __stdcall DefField2c_59c490(u32 v0) {
    const u8 *record = ObjectDef_FindById(v0); return record ? at<u32>(record, 0x2c) : 0x5;
}

u32 __stdcall DefField8_59c500(u32 v0) {
    const u8 *record = ObjectDef_FindById(v0); return record ? at<u32>(record, 0x8) : 0xa;
}
void __fastcall ObjectDef_FindMarkedCells(void *, void *, u8 **def) {
    u8 *object = *def;
    s32 column, row;
    if (find_cell(at<u8 *>(object, 0x18), 9, &column, &row)) {
        const u8 mark = at<u8>(grid_cell(at<u8 *>(object, 0x18), column, row), 4);
        at<s32>(object, 0x494) = column;
        at<s32>(object, 0x498) = row;
        at<u8>(*def, 0x49c) = mark;
        object = *def;
        u8 *const grid = at<u8 *>(object, 0x18);
        s32 column2, row2;
        if (find_cell(grid, 0xa, &column2, &row2)) {
            const u8 mark2 = at<u8>(grid_cell(grid, column2, row2), 4);
            at<s32>(*def, 0x4a0) = column2;
            at<s32>(*def, 0x4a4) = row2;
            at<u8>(*def, 0x4a8) = mark2;
        } else {
            at<s32>(object, 0x4a0) = column;
            at<s32>(object, 0x4a4) = row;
            at<u8>(*def, 0x4a8) = mark;
        }
    } else {
        at<s32>(object, 0x494) = 0;
        at<s32>(object, 0x498) = 0;
        at<u8>(*def, 0x49c) = 1;
        at<s32>(*def, 0x4a0) = 0;
        at<s32>(*def, 0x4a4) = 0;
        at<u8>(*def, 0x4a8) = 0x10;
    }
    object = *def;
    at<u32>(object, 0x4b0) = at<u32>(object, 0x30);
    at<u32>(object, 0x4b4) = at<u32>(object, 0x34);
    at<u16>(*def, 0) = at<u16>(*def, 0x10);
    at<u32>(*def, 0x4ac) = at<u32>(*def, 0x4c);
}

u8 *__fastcall ObjectDef_Ctor(u8 *def) {
    at<u32>(def, 4) = 0x006fd8a8;
    BalanceFile_Init(def + 4);
    static const u32 kCleared[] = {0x494u, 0x498u, 0x4a0u, 0x4a4u, 0x4b0u, 0x4b4u, 0x4f4u, 0x4f8u, 0x4fcu, 0x500u, 0x4e0u, 0x4e4u, 0x4e8u, 0x4ecu, 0x4f0u};
    for (const u32 offset : kCleared)
        at<u32>(def, offset) = 0;
    return def;
}

void __fastcall ObjectDef_Dtor(u8 *def) {
    ReleaseEffects(def);
    ObjectDefBalance_Dtor(def + 4);   // (inlined)
}

void __fastcall ObjectDefs_ReleaseEffects(u8 *defs) {
    at<s32>(defs, 0x72c) = 0;
    for (const u8 *def = at<const u8 *>(defs, 0); def;) {
        ReleaseEffects(def);
        if (at<s32>(defs, 0x72c) < (s32)(at<u32>(defs, 0x528) - 1)) {
            const s32 next = ++at<s32>(defs, 0x72c);
            def = at<const u8 *>(defs, (u32)next * 4);
        } else {
            def = nullptr;
        }
    }
}

void __fastcall ObjectDef_LoadText(u8 *def, void *, Stream *stream) {
    u16 **const fields = reinterpret_cast<u16 **>(def + 0x4f4);
    for (u32 i = 0; i < 4; ++i) fields[i] = reinterpret_cast<u16 *>((uintptr_t)0x006fd880);
    const s32 size = vcall<s32>(stream, 0x14);
    if (size % 2 != 0) return;
    u16 *const text = static_cast<u16 *>(Mem_GlobalAlloc((u32)(size + 2)));
    const s32 got = vcall<s32>(stream, 8, text, size);
    if (!text || !got) return;
    text[size / 2] = 0;
    const u16 *const *const keys = reinterpret_cast<const u16 *const *>((uintptr_t)0x007487f8);
    const u16 *p = text[0] == 0xfeff ? text + 1 : text;
    while (*p) {
        s32 index = -1;
        for (s32 i = 0; keys[i]; ++i)
            if (WText_WordEquals(p, keys[i])) {
                index = i;
                break;
            }
        if (index == -1) break;
        const u16 *const value = nextLine(p);
        u32 n = 0;
        while (value[n] && value[n] != 0xa && value[n] != 0xd) ++n;
        u16 *const copy = static_cast<u16 *>(Mem_GlobalAlloc(2 * n + 2));
        fields[index] = copy;
        for (u32 k = 0; k < n; ++k) copy[k] = value[k];
        copy[n] = 0;
        p = nextLine(value);
    }
    Mem_GlobalFree(text);
}

u8 __fastcall ObjectText_FullName(u16 *self, void *, u32 id, u16 *out, s32 size) {
    const u16 *const none = &global<u16>(0x0078a374);  // (an empty bss string, the format)
    if (const u8 *row = object_text_row(id))
        format_text(self, 0x40, at<u32>(row, 4));
    else
        Crt().swprintf(self, none);
    u16 *const suffix = self + 0x40;
    if (const u8 *row = object_text_row(id))
        format_text(suffix, 0x40, at<u32>(row, 8));
    else
        Crt().swprintf(suffix, none);
    u16 *const full = self + 0x80;
    if (!cut_dash(self)) {
        const u16 *const gap = *suffix ? reinterpret_cast<const u16 *>(L" ") : &global<u16>(0x00786de4);
        Crt().swprintf(full, reinterpret_cast<const u16 *>(L"%s%s%s"), self, gap, suffix);
    } else {
        Crt().swprintf(full, reinterpret_cast<const u16 *>(L"%s%s"), self, suffix);
    }
    Crt().wcsncpy(out, full, (u32)(size > 0x80 ? 0x80 : size));
    return 1;
}

u8 __fastcall ObjectText_Name(void *, void *, u32 id, u16 *out, u32 size, u32 trim) {
    const u8 *row = object_text_row(id);
    if (!row) return 0;
    format_text(out, size, at<u32>(row, 4));
    if ((u8)trim == 1) cut_dash(out);
    return 1;
}

u8 __fastcall ObjectText_Suffix(void *, void *, u32 id, u16 *out, u32 size) {
    const u8 *row = object_text_row(id);
    if (!row) return 0;
    format_text(out, size, at<u32>(row, 8));
    return 1;
}

void __fastcall ObjectDefs_GetFullName(u8 *defs, void *, u32 id, u16 *out, u32 size) {
    if (u8 *def = ObjectDefs_Find(defs, nullptr, (u16)id)) {
        const u16 *text = at<const u16 *>(def, 0x4f4);
        if (text && text != reinterpret_cast<const u16 *>((uintptr_t)0x006fd880)) {
            Crt().wcsncpy(out, text, size);
            return;
        }
    }
    ObjectText_FullName(reinterpret_cast<u16 *>(defs + 0x52c), nullptr, id & 0xffff, out, (s32)size);
}

void __fastcall ObjectDefs_GetName(u8 *defs, void *, u32 id, u16 *out, u32 size) {
    if (ObjectText_Name(defs + 0x52c, nullptr, id & 0xffff, out, size, 0)) return;
    u8 *def = ObjectDefs_Find(defs, nullptr, (u16)id);
    Crt().wcsncpy(out, at<const u16 *>(def, 0x4f8), size);  // (no definition: a fault, sic)
}

void __fastcall ObjectDefs_GetSuffix(u8 *defs, void *, u32 id, u16 *out, u32 size) {
    if (ObjectText_Suffix(defs + 0x52c, nullptr, id & 0xffff, out, size)) return;
    u8 *def = ObjectDefs_Find(defs, nullptr, (u16)id);
    Crt().wcsncpy(out, at<const u16 *>(def, 0x4fc), size);  // (no definition: a fault, sic)
}

void __fastcall ObjectDefs_Free(u8 *defs) {
    ObjectDefs_ReleaseEffects(defs);   // (inlined)
    for (s32 i = 0; i < at<s32>(defs, 0x528); ++i) {   // (the count re-read each step)
        u8 **const slot = reinterpret_cast<u8 **>(defs + 4 * i);
        u16 **const texts = reinterpret_cast<u16 **>(*slot + 0x4f4);   // (a null entry: a fault, sic)
        for (u32 k = 0; k < 4; ++k) {
            if (texts[k] && texts[k] != reinterpret_cast<u16 *>((uintptr_t)0x006fd880)) {   // (not the shared "MISSING STRING")
                Mem_GlobalFree(texts[k]);
                texts[k] = nullptr;
            }
        }
        if (u8 *const def = *slot) {
            ObjectDef_Dtor(def);
            GameHeap_Free(def);
        }
        *slot = nullptr;
    }
    at<s32>(defs, 0x528) = 0;
    at<u16>(defs, 0x730) = 0;
    at<u32>(defs, 0x734) = 0;
}

void __cdecl ObjectDefs_FreeAll() {
    ObjectDefs_Free(reinterpret_cast<u8 *>(0x007890a0));   // (inlined, the table's words read as globals)
}

s32 __stdcall ObjectDefs_ListBuiltNumbers(u16 *out) {
    std::memset(out, 0, 0x200);
    u8 *const defs = reinterpret_cast<u8 *>(0x007890a0);
    const u8 *const built = reinterpret_cast<const u8 *>(0x00802080);
    u32 count = 0;
    for (u32 index = 0; index < 0x20; ++index) {
        for (u32 bit = 0; bit < 8; ++bit) {
            if (!(built[index] >> bit & 1)) continue;
            const u32 id = index * 8 + bit + 1;
            u8 *def = ObjectDefs_First(defs);
            while (def && *reinterpret_cast<u32 *>(def + 0x94) != id) def = ObjectDefs_Next(defs);
            if (!def) continue;
            const char *const name = CRT_strrchr(reinterpret_cast<const char *>(def + 0x28c), 0x5c);
            const char *const digits = CRT_strpbrk(name, gText_ObjectDefs_ListBuiltNumbers_762b18);
            const char *end;
            out[count++] = static_cast<u16>(CRT_strtol(digits, &end, 10));
        }
    }
    return 0;
}
}

namespace {

AddrCalls g_loadCalls;

u32 address(const void *p) { return static_cast<u32>(reinterpret_cast<uintptr_t>(p)); }

const char *const kPathFormat = reinterpret_cast<const char *>((uintptr_t)0x007488ac);    // "%s\\%s"
const char *const kPrefixedFormat = reinterpret_cast<const char *>((uintptr_t)0x007488a4);   // "%s\\%s%s"

const char *handleFolder(u8 *handle) { return vcall<const char *>(handle, 0x18); }

// The setting's mode, read at the first use (a static with its destructor registered).
u32 settingMode() {
    u8 &made = gObjectdefs_fb3b78;
    if (!(made & 1)) {
        made |= 1;
        g_loadCalls.Th(0x00550ca0, 0x00fb3b7cu);
        g_loadCalls.Cd(0x0067a7d0, 0x005d1750u);
    }
    return global<u32>(0x00fb3b7c);
}

struct DefLoad {
    u8 *source;
    StreamBase *stream;
    char *name;

    void namePath(const char *file) {
        Crt().sprintf(name, kPathFormat, handleFolder(source + 0x2c), file);
    }
    void namePrefixed(u32 prefixText) {
        const char *const file = at<const char *>(source, 0xc);
        const char *const prefix = global<const char *>(prefixText);
        Crt().sprintf(name, kPrefixedFormat, handleFolder(source + 0x2c), prefix, file);
    }
    // The stream read into a balance file; failed: the error, unless silent.
    bool read(u8 *balance, u32 keep, bool silent = false) {
        if (g_loadCalls.Th(0x00401700, address(balance), address(name), address(stream), keep)) {
            StreamBase_Close(stream);
            return true;
        }
        if (!silent) g_loadCalls.Th(0x00405710, 0x00785de8u, 3u, 0u, address(name));
        return false;
    }
};

u32 loadDefinition(u8 *defs, u8 *entry, u8 *opener, u8 *source, String *path, StreamBase *stream, char *name, u8 *balance) {
    if (at<u32>(defs, 0x528) == 0x14a) return 0;
    void *const block = GameHeap_Alloc(0x50c);
    const u32 made = block ? g_loadCalls.Th(0x00412c20, address(block)) : 0;
    at<u32>(defs, at<u32>(defs, 0x528) * 4) = made;
    const u32 index = at<u32>(defs, 0x528);
    u8 *def = at<u8 *>(defs, index * 4);
    at<u32>(defs, 0x528) = index + 1;
    {
        const char *from = path->data;
        const u32 size = static_cast<u32>(std::strlen(from)) + 1;
        std::memcpy(def + 0x28c, from, size);
    }
    Crt().swprintf(reinterpret_cast<u16 *>(def + 0x390), reinterpret_cast<const u16 *>((uintptr_t)0x007488b4), at<const char *>(source, 0xc));
    DefLoad load = {source, stream, name};
    u8 *const handle = source + 0x2c;
    const u32 streamAt = address(stream);
    if (g_loadCalls.Th(0x0041e660, address(entry), address(opener), streamAt)) {
        load.namePath(at<const char *>(source, 0xc));
        if (!load.read(def + 4, 0, true)) return 0;
        if (settingMode() == 1 && g_loadCalls.Th(0x0041e720, address(entry), address(opener), streamAt)) {
            load.namePath(at<const char *>(source, 0xc));
            if (!load.read(def + 4, 1)) return 0;
        }
    }
    load.namePath(at<const char *>(source, 0xc));
    if (g_loadCalls.Th(0x0041f170, address(source), streamAt) && !load.read(def + 4, 1)) return 0;
    if (settingMode() == 1) {
        load.namePrefixed(0x0078a338);
        if (g_loadCalls.Th(0x0041f260, address(source), streamAt) && !load.read(def + 4, 1)) return 0;
    } else if (settingMode() == 2) {
        load.namePrefixed(0x00788df0);
        if (g_loadCalls.Th(0x0041f190, address(source), streamAt) && !load.read(def + 4, 1)) return 0;
    }
    if (g_loadCalls.Th(0x0041f420, address(source))) at<u32>(def, 0x4f0) = g_loadCalls.Cd(0x0051b4f0, address(handleFolder(handle)), 0x0074787cu);
    if (g_loadCalls.Th(0x0041f630, address(source))) g_loadCalls.Th(0x00414420, address(def), address(handle));
    g_loadCalls.Th(0x00413410, address(defs), address(&def));
    if (g_loadCalls.Th(0x0041f350, address(source), streamAt)) {
        at<u32>(def, 0x504) |= 1;
        g_loadCalls.Th(0x00413600, address(def), streamAt);
    }
    StreamBase_Close(stream);
    // (The id's slot: pushed as the out pointer, then its low word set, Fn_409cd0.)
    g_loadCalls.Th(0x00412fb0, address(defs), (address(def + 0x410) & 0xffff0000u) | at<u16>(def, 0x10), address(def + 0x410), 0x40u);
    const u32 folder = address(path->data);
    const u32 sourceName = at<u32>(source, 0xc);
    if (at<u32>(def, 0x38)) {
        const u32 flags = at<u32>(def, 0x84) ? 0x450c00u : 0x50c00u;
        at<u32>(def, 0x490) = g_loadCalls.Cd(0x004629d0, folder, sourceName, at<u32>(def, 0x18), at<u32>(def, 0x1c), flags, static_cast<u32>(at<u16>(def, 0)), at<u32>(def, 0x170), 0u, 0u, at<u32>(def, 0x28), at<u32>(def, 0x2c));
    } else {
        const u32 flags = at<u32>(def, 0x84) ? 0x4b24a9u : 0xb24a9u;
        at<u32>(def, 0x490) = g_loadCalls.Cd(0x004629d0, folder, sourceName, at<u32>(def, 0x18), at<u32>(def, 0x1c), flags, static_cast<u32>(at<u16>(def, 0)), at<u32>(def, 0x170), 0u, 0u, 0u, 0u);
        if (at<u32>(def, 0x490) && at<u32>(def, 0x9c) == 3) {
            at<u32>(balance, 0) = 0x006fd8b0;
            g_loadCalls.Th(0x00401030, address(balance));
            Crt().sprintf(name, kPathFormat, handleFolder(handle), global<const char *>(0x0078a0b0));
            if (g_loadCalls.Th(0x0041f330, address(source), streamAt) && !load.read(balance, 0)) {
                at<u32>(balance, 0) = 0x006fd8b0;
                g_loadCalls.Th(0x00401250, address(balance));
                return 0;
            }
            g_loadCalls.Cd(0x0042fad0, at<u32>(def, 0x490), address(path->data), address(balance));
            at<u32>(balance, 0) = 0x006fd8b0;
            g_loadCalls.Th(0x00401250, address(balance));
        }
    }
    at<u32>(def, 0x508) = at<u32>(def, 0x490) ? 0 : 1;
    if (!at<u32>(def, 0x508)) {
        at<u32>(def, 0x4b8) = at<u32>(def, 0x264);
        for (s32 i = 0; i < at<s32>(def, 0x4b8); ++i)
            at<u32>(def, 0x4bc + i * 4) = g_loadCalls.Cd(0x004629d0, address(path->data), at<u32>(def, 0x244 + i * 4), 0u, 0u, 0xc0u, static_cast<u32>(i) + 1 + at<u16>(def, 0), 1u, 0u, 0u, 0u, 0u);
    }
    return 1;
}

// One named folder: found in the list [0x786b9c] (1 exactly), its definitions loaded. 0 for a load that failed.
bool load_named(u8 *defs, u32 key, s32 *folder) {
    return g_loadCalls.Th(0x00421770, global<u32>(0x00786b9c), key, folder) != 1 || g_loadCalls.Th(0x00413930, (u32)(uintptr_t)defs, *folder) != 0;
}

}  // namespace

void ObjectDefs_SetLoadCalls(const AddrCall *table, u32 count) { g_loadCalls.Set(table, count); }

extern "C" {
u32 __fastcall ObjectDefs_Load(u8 *defs, void *, u8 *entry, u8 *opener, u8 *source) {
    struct {
        char name[0x104];
        u8 balance[0x3b80];
    } frame;
    String path;
    String_Ctor(&path);
    StreamBase stream;
    StreamBase_Ctor(&stream);
    vcall<void>(&path, 0x18, source + 0x18);
    const StringView *const suffix = reinterpret_cast<const StringView *>((uintptr_t)0x007898c8);
    StringView view;
    String copy;
    String_CtorCopy(&copy, nullptr, String_Right(&path, nullptr, &view, suffix->length));
    const bool ends = Names_Equal(reinterpret_cast<const u8 *>(&copy), reinterpret_cast<const u8 *>(suffix)) == 1;
    String_Dtor(&copy);
    if (ends) {
        vcall<void>(&path, 0x1c, String_Left(&path, nullptr, &view, path.length - suffix->length));
        String_EnsureTrailingBackslash(&path);
    }
    const u32 result = loadDefinition(defs, entry, opener, source, &path, &stream, frame.name, frame.balance);
    StreamBase_Dtor(&stream);
    String_Dtor(&path);
    return result;
}

void __fastcall ObjectDef_LoadTrackTypes(u8 *def, void *, u8 *handle) {
    struct {
        u8 head[0x20];
        char *name;
        char text[0x104];
    } found;
    found.name = found.text;
    found.text[0] = 0;
    s32 result;
    if (*vcall<s32 *>(handle, 4, &result, global<const char *>(0x00788f40), &found) < 0) return;
    StreamBase stream;
    StreamBase_Ctor(&stream);
    String text;
    String_Ctor(&text);
    do {
        if (*vcall<s32 *>(handle, 0xc, &result, &stream, found.name, 0x80000001u, 0u) >= 0) {
            vcall<void>(&text, 0x14, found.name);
            vcall<void>(&text, 0x18, String_StripSuffix(reinterpret_cast<const String *>((uintptr_t)0x00789010), &text));
            StringView view;
            vcall<void>(&text, 0x1c, String_Right(&text, nullptr, &view, 1));
            char digit[4];
            {
                const u32 size = static_cast<u32>(std::strlen(text.data)) + 1;
                std::memcpy(digit, text.data, size);
            }
            const s32 index = CRT_atol(digit);
            at<u32>(def, 0x4dc + index * 4) = g_loadCalls.Cd(0x0051fa20, address(&stream));
        }
        StreamBase_Close(&stream);
    } while (*vcall<s32 *>(handle, 8, &result, &found) >= 0);
    String_Dtor(&text);
    StreamBase_Dtor(&stream);
}

u32 __fastcall ObjectDefs_LoadFolders(u8 *defs, void *, u8 *folders) {
    const u8 *const list = at<const u8 *>(folders, 0x2c);
    if (!list)
        return 0;
    struct {
        u8 head[0x20];
        char *name;
        char path[0x104];
    } entry;
    entry.name = entry.path;
    entry.path[0] = 0;
    StreamBase stream;
    StreamBase_Ctor(&stream);
    String unused;
    String_Ctor(&unused);
    u32 result = 1;
    const u8 *node = at<const u8 *>(list, 4);
    while (node && result) {
        u8 *const source = at<u8 *>(node, 0);
        node = at<const u8 *>(node, 4);
        u32 status;
        const s32 *found = vcall<const s32 *>(source, 4, &status, global<u32>(0x0078a2f0), &entry);
        while (*found >= 0) {
            if (DirEntry_IsRealDir(reinterpret_cast<const u8 *>(&entry)) || DirEntry_IsWad(reinterpret_cast<const DirEntry *>(&entry))) {
                String path;
                String_Ctor(&path);
                vcall<void>(&path, 0x18, folders + 0x18);
                vcall<void>(&path, 0x20, entry.name);
                void *const block = GameHeap_Alloc(0x38);
                u8 *const object = block ? reinterpret_cast<u8 *>((uintptr_t)g_loadCalls.Th(0x0041dc80, (u32)(uintptr_t)block, &entry, &path)) : nullptr;
                const u32 self = (u32)(uintptr_t)object;
                const auto add = [&] {
                    g_loadCalls.Th(0x0041eaa0, (u32)(uintptr_t)folders, self);
                    return g_loadCalls.Th(0x00413c10, (u32)(uintptr_t)defs, (u32)(uintptr_t)folders, (u32)(uintptr_t)source, self) != 0;
                };
                bool ok = object != nullptr;
                if (ok && g_loadCalls.Th(0x0041f170, self, &stream) == 1) {
                    StreamBase_Close(&stream);
                    if (settingMode() == 2) {
                        if (g_loadCalls.Th(0x0041f190, self, &stream) != 1)
                            vcall<void>(object, 0, 1u);
                        else {
                            StreamBase_Close(&stream);
                            // sic: an object whose name is there already is kept, not deleted
                            if (!g_loadCalls.Th(0x0041ede0, (u32)(uintptr_t)folders, self + 4))
                                ok = add();
                        }
                    } else if (g_loadCalls.Th(0x0041ede0, (u32)(uintptr_t)folders, self + 4)) {
                        vcall<void>(object, 0, 1u);
                    } else {
                        ok = add();
                    }
                } else if (ok) {
                    StreamBase_Close(&stream);
                    vcall<void>(object, 0, 1u);
                }
                String_Dtor(&path);
                if (!ok) {
                    result = 0;
                    break;
                }
            }
            u32 next;
            found = vcall<const s32 *>(source, 8, &next, &entry);
        }
    }
    String_Dtor(&unused);
    StreamBase_Dtor(&stream);
    return result;
}

u32 __fastcall ObjectDefs_ReloadAll(u8 *defs, void *, u32) {
    s32 folder = 0;
    g_loadCalls.Th(0x004145f0, (u32)(uintptr_t)defs);
    for (s32 i = 0; i < at<s32>(defs, 0x528); ++i) {
        u8 *const def = at<u8 *>(defs, 4 * i);
        g_loadCalls.Th(0x00413340, (u32)(uintptr_t)def);
        if (def) {
            g_loadCalls.Th(0x00412cb0, (u32)(uintptr_t)def);
            g_loadCalls.Cd(0x0067acb0, def);
        }
        at<u32>(defs, 4 * i) = 0;
    }
    at<u32>(defs, 0x528) = 0;
    at<u16>(defs, 0x730) = 0;
    at<u32>(defs, 0x734) = 0;
    if (!load_named(defs, 0x007898e0, &folder) || !load_named(defs, 0x00788e60, &folder))
        return 0;

    const u8 *const end = global<u8 *>(0x00789c20);
    u8 *const begin = global<u8 *>(0x00789c1c);
    for (u8 *font = begin; font != end; font += 0x14) {
        reinterpret_cast<s32(__stdcall *)(u32)>((uintptr_t)global<u32>(0x006fd058))(at<u32>(font, 8));   // RemoveFontResourceA
        s32 result;
        g_loadCalls.Cd(0x005f5d60, &result, at<u32>(font, 8));
    }
    for (u8 *font = begin; font != end; font += 0x14)
        g_loadCalls.Th(at<u32>(at<u8 *>(font, 0), 0xc), (u32)(uintptr_t)font, 0u);
    global<u8 *>(0x00789c20) = begin;

    return load_named(defs, 0x0078a2a0, &folder) && load_named(defs, 0x00789d00, &folder) && load_named(defs, 0x00789910, &folder) ? 1 : 0;
}
}
