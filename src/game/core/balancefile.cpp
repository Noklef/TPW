#include "balancefile.h"

#include <cstdint>

#include "vcall.h"
#include "../files/file.h"

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

void __fastcall AdvisorScoring_Dtor(u8 *self) {
    at<u32>(self, 0xc) = 0x006fd958;
    BalanceFile_Release(self + 0xc);
}

void __cdecl AdvisorScoring_StaticDtor() { AdvisorScoring_Dtor(reinterpret_cast<u8 *>(0x00fb3540)); }
}
