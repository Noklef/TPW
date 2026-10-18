#include "balancefile.h"

#include <cstdint>
#include <cstring>

#include "vcall.h"
#include "../files/file.h"
#include "crt.h"
#include "gameerror.h"
#include "gameheap.h"
#include "string.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const u8 *key(u8 *self, s32 i) { return vcall<const u8 *>(self, 0, i); }

u32 type(u8 *self, s32 i) { return at<u32>(key(self, i), 0); }

// A key record's name (+4) against a string, as the original's inline strcmp: true when equal.
bool named(const u8 *record, const char *text) {
    for (const u8 *a = record + 4;; ++a, ++text) {
        if (*a != static_cast<u8>(*text)) return false;
        if (!*a) return true;
    }
}

// The entry of key record i (the values at +4 of the file, re-read each time as the original does).
u8 *entry(const u8 *self, s32 i) { return at<u8 *>(self, 4) + i * 0xc; }

// Value k of an entry: its values (+0) plus k strides (+4) of dwords, in 32-bit arithmetic.
u32 &value(const u8 *e, s32 k) { return *reinterpret_cast<u32 *>(at<u32>(e, 0) + (u32)at<s32>(e, 4) * (u32)k * 4); }

void clear_flag(const u8 *self, s32 i) {
    if (u32 *const p = at<u32 *>(entry(self, i), 8)) *p = 0;
}

// A static balance file torn down at exit: its vtable restored, then released.
void destroy(u32 address, u32 vtable) {
    at<u32>(reinterpret_cast<void *>((uintptr_t)address), 0) = vtable;
    BalanceFile_Release(reinterpret_cast<u8 *>((uintptr_t)address));
}

}  // namespace

extern "C" {

u32 __fastcall BalanceFile_FindKey(u8 *self, void *, const char *group, const char *name, s32 *index, u32 grouped, s32 row) {
    s32 i = 0;
    while (type(self, i) != 0xc) ++i;
    for (; i >= 0; --i)
        if ((type(self, i) == 1 || type(self, i) == 3) && named(key(self, i), group)) break;
    if (i < 0) return 0;
    if (grouped) {
        if (type(self, i) == 1) return 0;
        if (row >= at<s32>(key(self, i), 0x34)) return 0;  // (an assert, left out)
    } else if (type(self, i) == 3) {
        return 0;
    }
    while (--i >= 0) {
        if (type(self, i) == 0 || type(self, i) == 2) return 0;
        if (named(key(self, i), name)) {
            *index = i;
            return 1;
        }
    }
    return 0;
}

void __fastcall BalanceFile_SetDefaults(u8 *self, void *, u32 keep) {
    s32 n = 0;
    while (at<u32>(key(self, n), 0) != 0xc) ++n;
    s32 i = 0;
    while (i < n) {
        s32 count;
        if (at<u32>(key(self, i), 0) == 2) {
            s32 j = i;
            if (i < n)
                while (at<u32>(key(self, j), 0) != 3) ++j;
            count = at<s32>(key(self, j), 0x34);
        } else {
            count = 1;
        }
        ++i;
        bool stop = false;
        while (i < n && !stop) {
            switch (at<u32>(key(self, i), 0)) {
            case 1:
            case 3:
                stop = true;
                break;
            case 4:
            case 5:
            case 7:
            case 8:
                for (s32 k = 0; k < count; ++k) value(entry(self, i), k) = 0;
                clear_flag(self, i);
                break;
            case 6:
            case 9: {
                const u32 field = at<u32>(key(self, i), 0) == 6 ? 0x24 : 0x2c;
                for (s32 k = 0; k < count; ++k) {
                    u8 *const e = entry(self, i);
                    value(e, k) = at<u32>(key(self, i), field);
                }
                clear_flag(self, i);
                break;
            }
            case 10:
            case 11:
                for (s32 k = 0; k < count; ++k) {
                    u32 &v = value(entry(self, i), k);
                    if (keep == 1) {
                        v = 0;
                    } else if (v) {
                        Mem_GlobalFree(reinterpret_cast<void *>((uintptr_t)v));
                        v = 0;
                    }
                }
                clear_flag(self, i);
                break;
            default:  // (0 and 2: the original's "InitialiseBalanceValues internal" assert)
                break;
            }
            ++i;
        }
    }
}

void __fastcall BalanceFile_Release(u8 *self) {
    if (at<void *>(self, 4)) BalanceFile_SetDefaults(self, nullptr, 0);
    if (void *const values = at<void *>(self, 4)) {
        Mem_GlobalFree(values);
        at<void *>(self, 4) = nullptr;
    }
}

const u8 *__fastcall BalanceHoardings_GetSchema(const void *, void *, u32 index) { return reinterpret_cast<const u8 *>(0x0073fb40) + index * 0x3c; }
const u8 *__fastcall BalanceLevel_GetSchema(const void *, void *, u32 index) { return reinterpret_cast<const u8 *>(0x0073fc70) + index * 0x3c; }
const u8 *__fastcall BalanceOnline_GetSchema(const void *, void *, u32 index) { return reinterpret_cast<const u8 *>(0x00743f00) + index * 0x3c; }
const u8 *__fastcall BalanceGraphics_GetSchema(const void *, void *, u32 index) { return reinterpret_cast<const u8 *>(0x007442c0) + index * 0x3c; }
const u8 *__fastcall BalanceResolution_GetSchema(const void *, void *, u32 index) { return reinterpret_cast<const u8 *>(0x00744a40) + index * 0x3c; }
const u8 *__fastcall BalanceSound_GetSchema(const void *, void *, u32 index) { return reinterpret_cast<const u8 *>(0x00746a20) + index * 0x3c; }

void __fastcall BalanceFile_Init(u8 *self) {
    s32 n = 0;
    while (at<u32>(key(self, n), 0) != 0xc) ++n;
    at<void *>(self, 4) = nullptr;
    void *const values = Mem_GlobalAlloc((u32)n * 0xc);
    at<void *>(self, 4) = values;
    if (!values) return;
    s32 next = 1, count = 0, members = 0;
    u32 group = 0;
    for (s32 i = 0; i < n; ++i) {
        switch (at<u32>(key(self, i), 0)) {
        case 0:
        case 1:
            break;
        case 2: {
            s32 j = i + 1;
            for (; j < n; ++j) {
                const u32 type = at<u32>(key(self, j), 0);
                if (type == 3 || type == 0 || type == 1 || type == 2) break;
            }
            // (no type-3 record: the original's "Group start without group end" assert)
            members = j - i - 1;
            count = at<s32>(key(self, j), 0x34);
            const u32 base = (u32)(uintptr_t)vcall<u8 *>(self, 4);
            group = base + ((u32)count * (u32)members + (u32)next) * 4;
            break;
        }
        case 3:
            next = (s32)((u32)next + ((u32)count - 1) * (u32)members + 1);
            group = 0;
            count = 0;
            members = 0;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11: {
            const u32 base = (u32)(uintptr_t)vcall<u8 *>(self, 4);
            at<u32>(entry(self, i), 0) = base + (u32)next * 4;
            ++next;
            at<s32>(entry(self, i), 4) = members;
            at<u32>(entry(self, i), 8) = group;
            continue;
        }
        default:
            continue;
        }
        at<u32>(entry(self, i), 0) = 0;  // (types 0-3: an entry without values)
        at<u32>(entry(self, i), 4) = 0;
        at<u32>(entry(self, i), 8) = 0;
    }
    BalanceFile_SetDefaults(self, nullptr, 1);
}

void __cdecl BalanceHoardingsA_StaticDtor() { destroy(0x00784e30, 0x006fd618); }
void __cdecl BalanceHoardingsB_StaticDtor() { destroy(0x00784c20, 0x006fd618); }
void __cdecl BalanceLevel_StaticDtor() { destroy(0x00785040, 0x006fd620); }
void __cdecl BalanceOnline_StaticDtor() { destroy(0x00785828, 0x006fd628); }
void __cdecl BalanceGraphics_StaticDtor() { destroy(0x00785868, 0x006fd630); }
void __cdecl BalanceResolution_StaticDtor() { destroy(0x007858e0, 0x006fd638); }
void __cdecl BalanceSound_StaticDtor() { destroy(0x007858f0, 0x006fd640); }

void __fastcall ObjectDefBalance_Dtor(u8 *self) {
    at<u32>(self, 0) = 0x006fd8a8;
    BalanceFile_Release(self);
}
void __fastcall TrackTypeBalance_Dtor(u8 *self) {
    at<u32>(self, 0) = 0x006fd8b0;
    BalanceFile_Release(self);
}
void __fastcall BalanceGraphics_Dtor(u8 *self) {
    at<u32>(self, 0) = 0x006fd630;
    BalanceFile_Release(self);
}

const u8 *__fastcall AdvisorScoring_GetSchema(const void *, void *, u32 index) { return reinterpret_cast<const u8 *>(0x0072e120) + index * 0x3c; }

void __fastcall ThemeRecord_Dtor(u8 *self) {
    if (u8 *const file = at<u8 *>(self, 0x1c)) {
        at<u32>(file, 0) = 0x00702800;
        BalanceFile_Release(file);
        GameHeap_Free(file);
    }
    String_Dtor(reinterpret_cast<String *>(self + 0x20));
}

void __fastcall AdvisorScoring_Dtor(u8 *self) {
    at<u32>(self, 0xc) = 0x006fd958;
    BalanceFile_Release(self + 0xc);
}

void __fastcall AdvisorScoringFile_Dtor(u8 *self) {
    at<u32>(self, 0) = 0x006fd958;
    BalanceFile_Release(self);
}

void __cdecl AdvisorScoring_StaticDtor() { AdvisorScoring_Dtor(reinterpret_cast<u8 *>(0x00fb3540)); }

u32 __cdecl BalanceFile_ParseMap(const char **text, u8 **map, const char **error, const u8 *symbols, s32 count) {
    u8 grid[0xc88] = {};  // +0 the width, +4 the height, +8 20 rows of 20 cells {the symbol's +4 dword, its +8 byte, 3 zeros}
    *error = nullptr;
    const char *s = *text;
    while (*s && Crt().iswspace((u32)(s32)(s16)(s8)*s)) ++s;
    while (*s && *s != '\n') ++s;
    if (!*s) {
        *error = reinterpret_cast<const char *>(0x00739b80);  // "Missing map description"
        return 0;
    }
    if (s[1] != '-' || s[2] != '-' || s[3] != '-') {
        *error = reinterpret_cast<const char *>(0x00739b08);  // "Missing map description initial marker"
        return 0;
    }
    const char *p = s + 3;
    while (*p && *p != '\n') ++p;  // (from the third dash on)
    if (*p == '\n') ++p;
    s32 width = 0, rows = 0, base = 0;
    for (;;) {
        char c = *p;
        if (c == '-' && p[1] == '-' && p[2] == '-') {  // the closing marker
            const char *end = p + 2;
            while (*end && *end != '\n') ++end;
            if (*end == '\n') ++end;
            *reinterpret_cast<s32 *>(grid) = width;
            *reinterpret_cast<s32 *>(grid + 4) = rows;
            u8 *top = grid + 8, *bottom = grid + 8 + (rows - 1) * 0xa0;  // the rows turned upside down
            for (s32 n = rows / 2; n > 0; --n, top += 0xa0, bottom -= 0xa0) {
                u8 row[0xa0];
                for (u32 i = 0; i < 0xa0; ++i) row[i] = top[i];
                for (u32 i = 0; i < 0xa0; ++i) top[i] = bottom[i];
                for (u32 i = 0; i < 0xa0; ++i) bottom[i] = row[i];
            }
            u8 *const block = static_cast<u8 *>(Mem_GlobalAlloc(0xc88));
            if (!block) {
                GameError_Set(reinterpret_cast<u8 *>(0x00785de8), nullptr, 8, 0, nullptr);
                return 0;
            }
            for (u32 i = 0; i < 0xc88; ++i) block[i] = grid[i];
            *map = block;
            *text = end;
            return 1;
        }
        if (!c) {
            *error = reinterpret_cast<const char *>(0x00739b68);  // "Missing map delimter"
            return 0;
        }
        s32 column = 0;
        while (c != '\n') {
            if (c != ' ') {
                if (base == 0x190) {
                    *error = reinterpret_cast<const char *>(0x00739b58);  // "Map too high"
                    return 0;
                }
                s32 i = 0;
                if (count > 0)
                    while (i < count && (char)symbols[i * 0xc] != c) ++i;
                if (i == count) {  // (a negative count takes the first symbol, sic)
                    *error = reinterpret_cast<const char *>(0x00739b40);  // "Illegal map character"
                    return 0;
                }
                if (column == 0x14) {
                    *error = reinterpret_cast<const char *>(0x00739b30);  // "Map too wide"
                    return 0;
                }
                u8 *const cell = grid + 8 + (column + base) * 8;
                *reinterpret_cast<u32 *>(cell) = *reinterpret_cast<const u32 *>(symbols + i * 0xc + 4);
                cell[4] = symbols[i * 0xc + 8];
                ++column;
            }
            c = *++p;
            if (!c) break;
        }
        if (column > width) width = column;
        ++rows;
        base += 0x14;
        while (*p && *p != '\n') ++p;
        if (*p == '\n') ++p;
    }
}
}

namespace {

// The classifiers as the original calls them: the char sign-extended.
bool space(char c) { return Crt().isspace((s32)c) != 0; }
bool digit(char c) { return Crt().isdigit((s32)c) != 0; }
bool alpha(char c) { return Crt().isalpha((s32)c) != 0; }
bool alnum(char c) { return Crt().isalnum((s32)c) != 0; }

// A line's names, adjacent as on the original's stack: a group name of all 64 chars runs on into the first field (and the 16th
// field into the original's return address, which no key record's name can match).
struct LineNames {
    char group[0x40];
    char fields[0x10][0x40];
};

// Where key record index's value goes: row of it when grouped (the entry's row count at +8 raised to row + 1 first), else
// its single value. The record is fetched once more after, as the original does.
u32 *value_slot(u8 *self, s32 index, u32 grouped, s32 row) {
    u32 *slot;
    if (grouped) {
        u8 *const e = entry(self, index);
        s32 *const rows = at<s32 *>(e, 8);
        if (*rows < row + 1) *rows = row + 1;
        slot = &value(e, row);
    } else {
        slot = at<u32 *>(entry(self, index), 0);
    }
    key(self, index);
    return slot;
}

// A block (a string or a map) stored over the slot's old one, which is freed.
void replace(u32 *slot, void *block) {
    if (*slot) Mem_GlobalFree(reinterpret_cast<void *>((uintptr_t)*slot));
    *slot = (u32)(uintptr_t)block;
}

// One "Group.Field.Field value value" line (or "Group[row].Field ..."), from p (its first char past the white space). False on a
// parse error, whose message (to Debug_Printf) is left out. A map moves line itself.
bool parse_line(u8 *self, char *&line, char *p, LineNames &names, const char *&map_error) {
    const CrtEnv &crt = Crt();
    std::memset(names.group, 0, sizeof names.group);
    s32 length = 0;
    while (alnum(*p) && length < 0x40) names.group[length++] = *p++;
    if (length == 0 || (*p != '.' && *p != '[')) return false;
    u32 grouped = 0;
    s32 row = 0;
    if (*p == '[') {
        ++p;
        if (!digit(*p)) return false;
        row = crt.atoi(p);
        while (digit(*p)) ++p;
        if (*p != ']' || *++p != '.') return false;
        ++p;
        grouped = 1;
    } else {
        ++p;
    }
    std::memset(names.fields, 0, sizeof names.fields);
    s32 count = 0;
    if (!alpha(*p)) return false;
    for (;;) {
        char *const field = names.fields[count];
        s32 k = 0;
        do {
            if (!alnum(*p)) break;
            field[k++] = *p++;
        } while (k < 0x40);
        ++count;
        if (*p != '.' && !space(*p)) return false;
        if (*p++ == '.' && count < 0x10) {
            if (!alpha(*p)) return false;
            continue;
        }
        if (count == 0x10 && *p == '.') return false;  // "Too many fields have been specified"
        break;
    }
    for (s32 i = 0; i < count; ++i) {
        s32 index;
        if (!BalanceFile_FindKey(self, nullptr, names.group, names.fields[i], &index, grouped, row)) return false;
        const u32 kind = type(self, index);
        if (kind == 0xb) {  // a map: the only field of its line
            if (count != 1) return false;
            u8 *map;
            u32 ok = 0;
            switch (at<u32>(key(self, index), 0x38)) {
            case 1:
                ok = BalanceFile_ParseMap(const_cast<const char **>(&line), &map, &map_error, reinterpret_cast<const u8 *>(0x007396c8), 0x13);
                break;
            case 2:
                ok = BalanceFile_ParseMap(const_cast<const char **>(&line), &map, &map_error, reinterpret_cast<const u8 *>(0x007397b0), 0x10);
                break;
            default:  // (asserts: no map_type or an unknown one)
                break;
            }
            if (!ok) {
                // The message takes the map's error text (strcat): none (an allocation failed) faults (sic).
                static_cast<void>(*static_cast<const volatile char *>(map_error));
                return false;
            }
            replace(value_slot(self, index, grouped, row), map);
        } else if (kind == 0xa) {  // a string: "text" up to its closing quote on the line
            while (space(*p)) ++p;
            if (*p != '"') return false;
            const char *const from = ++p;
            u32 n = 0;
            const char *end = from;
            while (*end != '"' && *end != '\n' && *end) {
                ++end;
                ++n;
            }
            if (*end != '"') return false;
            char *const text = static_cast<char *>(Mem_GlobalAlloc(n + 1));
            if (!text) return false;
            char *to = text;
            for (end = from; *end != '"' && *end != '\n' && *end; ++end) *to++ = *end;
            *to = 0;
            p = const_cast<char *>(end) + 1;
            replace(value_slot(self, index, grouped, row), text);
        } else if (kind == 7 || kind == 8 || kind == 9) {  // a float: [-]digits with up to one point, fewer than 100 chars
            while (space(*p)) ++p;
            char *end = p;
            if (*end == '-') ++end;
            s32 points = 0;
            while (digit(*end) || (*end == '.' && points < 2)) {
                if (*end == '.') ++points;
                ++end;
            }
            if (!space(*end) && *end) return false;
            if (points > 1 && *end != '.') return false;
            if (end - p >= 0x64) return false;
            const float f = static_cast<float>(crt.atof(p));
            p = end;
            u32 *const slot = value_slot(self, index, grouped, row);
            const u8 *const record = key(self, index);
            const u32 t = at<u32>(record, 0);
            if (t == 8 && !(f >= 0.0f)) return false;  // "invalid -ve value"
            if (t == 9 && (!(f >= at<float>(record, 0x2c)) || f >= at<float>(record, 0x30))) return false;  // "bounded value out of range"
            std::memcpy(slot, &f, 4);
        } else {  // an integer: [-]digits
            while (space(*p)) ++p;
            char *end = p;
            if (*end == '-') ++end;
            while (digit(*end)) ++end;
            if (!space(*end) && *end) return false;
            const s32 v = crt.atoi(p);
            p = end;
            u32 *const slot = value_slot(self, index, grouped, row);
            const u8 *const record = key(self, index);
            const u32 t = at<u32>(record, 0);
            if (t == 5 && v < 0) return false;
            if (t == 6 && (v < at<s32>(record, 0x24) || v >= at<s32>(record, 0x28))) return false;
            *slot = (u32)v;
        }
    }
    return true;
}

}  // namespace

extern "C" {

u32 __fastcall BalanceFile_Parse(u8 *self, void *, const char *, char *text, u32 keep) {
    if (!keep) BalanceFile_SetDefaults(self, nullptr, 0);
    char *to = text;
    for (const char *from = text; *from; ++from)
        if (*from != '\r') *to++ = *from;
    *to = 0;
    LineNames names;
    const char *map_error = nullptr;  // (sic: the original's is uninitialised until a map has been parsed)
    for (char *line = text; *line;) {
        char *p = line;
        while (space(*p)) ++p;  // (across line ends too)
        if (*p && *p != '#' && *p != '\n' && !parse_line(self, line, p, names, map_error)) return 0;
        while (*line && *line != '\n') ++line;
        if (*line == '\n') ++line;
    }
    return 1;
}
}
