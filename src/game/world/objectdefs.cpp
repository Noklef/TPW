#include "objectdefs.h"

#include <cstdint>
#include <cstring>

#include "../core/balancefile.h"
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
        if (!Save_Field(stream, save, table + i * 0x20, 0x20, 0x0075a3c8))
            return 1;
    if (!Save_Field(stream, save, table + 0x12c0, 4, 0x0075a3b4) || !Save_Field(stream, save, table + 0x12c4, 2, 0x0075a3a0))
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
}
