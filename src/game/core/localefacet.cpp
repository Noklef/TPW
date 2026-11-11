#include "localefacet.h"

#include <cstdint>

#include "crt.h"
#include "crttime.h"
#include "globals.h"
#include "initializers.h"
#include "iosbase.h"
#include "locinfo.h"
#include "moneypunct.h"
#include "sharedglobals.h"
#include "stdlocale.h"
#include "stdstring.h"
#include "vtables.h"
#include "wstream.h"
#include "narrowfilebuf.h"
#include "narrowwriter.h"
#include "nstream.h"
#include "wfilebuf.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const u32 kCtypeVtable = (u32)(uintptr_t)NCtype_vtable;
const u32 kNumGetVtable = (u32)(uintptr_t)NumGet_vtable;
const u32 kNumPutVtable = (u32)(uintptr_t)NumPut_vtable;
const u32 kNumpunctVtable = (u32)(uintptr_t)Numpunct_vtable;
const u32 kCodecvtVtable = (u32)(uintptr_t)NCodecvt_vtable;
const u32 kCtypeId = (u32)(uintptr_t)&gLocalefacet_fb1da4;
const u32 kNumGetId = (u32)(uintptr_t)&gLocalefacet_fb1da0;
const u32 kNumPutId = (u32)(uintptr_t)&gLocalefacet_fb1dac;
const u32 kNumpunctId = (u32)(uintptr_t)&gLocalefacet_fb1da8;
const u32 kCodecvtId = (u32)(uintptr_t)&gLocalefacet_fb275c;
const u32 kNpos = 0xffffffff;

u32 *alloc_facet(u32 size) { return static_cast<u32 *>(IosBase_Env().memAlloc(size)); }

LocaleFacet *as_facet(void *object) { return static_cast<LocaleFacet *>(object); }

void throw_missing() {
    u32 bad[3];
    BadCast_Ctor(bad, nullptr, gLocalefacet_MissingFacet_75c734);
    IosBase_Env().cxxThrow(bad, reinterpret_cast<const void *>((uintptr_t)0x007193a8));
}

// The id variable's value, given the next id when 0, under the lock.
u32 locked_id(u32 variable) {
    u32 lock[2];
    IosBase_Env().lockit(lock);
    const u32 id = Id_Get(&global<u32>(variable));
    IosBase_Env().unlockit(lock);
    return id;
}

// The locale's facet of the id. A missing one comes from the classic locale: make() once, kept in the dword at cache; other locales throw.
template <typename Make> LocaleFacet *facet_of(u8 **loc, u32 id_variable, u32 cache, Make make) {
    u32 lock[2];
    IosBase_Env().lockit(lock);
    LocaleFacet *facet = reinterpret_cast<LocaleFacet *>(Locale_Getfacet(loc, nullptr, Id_Get(&global<u32>(id_variable)), 1));
    if (!facet) {
        if (Locale_Iscloc(loc)) {
            facet = global<LocaleFacet *>(cache);
            if (!facet) {
                facet = make();
                global<LocaleFacet *>(cache) = facet;
            }
        } else {
            throw_missing();
        }
    }
    IosBase_Env().unlockit(lock);
    return facet;
}

// use_facet's shape: fill() makes the object at the first use of the classic facet; hold has it counted, freed at exit by at_exit.
template <typename Fill> u32 *use_facet(u8 **loc, u32 flag, u32 id_variable, u32 cache, u32 hold, u32 size, u32 at_exit, Fill fill) {
    u32 outer[2];
    IosBase_Env().lockit(outer);
    u32 *facet = reinterpret_cast<u32 *>(Locale_Getfacet(loc, nullptr, locked_id(id_variable), 1));
    if (!facet) {
        if (!static_cast<u8>(flag) || !Locale_Iscloc(loc)) {
            throw_missing();
        } else {
            facet = global<u32 *>(cache);
            if (!facet) {
                facet = alloc_facet(size);
                if (facet) {
                    facet[1] = 0;
                    fill(facet);
                }
                u32 lock[2];
                IosBase_Env().lockit(lock);
                global<u32 *>(hold) = facet;
                u32 count[2];
                IosBase_Env().lockit(count);
                if (facet[1] < kNpos) ++facet[1];
                IosBase_Env().unlockit(count);
                IosBase_Env().atexit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)at_exit));
                IosBase_Env().unlockit(lock);
                global<u32 *>(cache) = facet;
            }
        }
    }
    IosBase_Env().unlockit(outer);
    return facet;
}

void money_fill(u32 *facet, u8 international, u32 base, u32 vtable, void(__fastcall *init)(u8 *, void *, const void *)) {
    u32 info[0x10];
    Locinfo_Ctor(info, nullptr, "C");
    reinterpret_cast<u8 *>(facet)[0x28] = international;
    facet[0] = base;
    init(reinterpret_cast<u8 *>(facet), nullptr, info);
    Locinfo_Dtor(info);
    facet[0] = vtable;
}

u32 text_length(const char *text) {
    u32 n = 0;
    while (text[n]) ++n;
    return n;
}

// The string's text, or the fallback when it has none (a counted but null string: _Nullstr's).
const char *names_of(const StdString *s, u32 fallback) {
    if (!s->len) return reinterpret_cast<const char *>((uintptr_t)fallback);
    return s->ptr ? s->ptr : reinterpret_cast<const char *>((uintptr_t)0x006fe5b0);
}

// The strings at the _Locinfo's +0 and +0x10 given the C library's texts (each freed).
const char *days_of(u8 *info) {
    StdString *const s = reinterpret_cast<StdString *>(info);
    if (char *const days = CRT_Getdays()) {
        StdString_AssignChars(s, nullptr, days, text_length(days));
        Crt().free(days);
    }
    return names_of(s, 0x00709e78);
}

const char *months_of(u8 *info) {
    StdString *const s = reinterpret_cast<StdString *>(info + 0x10);
    if (char *const months = CRT_Getmonths()) {
        StdString_AssignChars(s, nullptr, months, text_length(months));   // (0x6aa200 and 0x6a3470 have it inlined, short of its length check)
        Crt().free(months);
    }
    return names_of(s, (u32)(uintptr_t)&gLocalefacet_709df0);
}

u16 *wide_copy(const char *text) {
    u32 n = text_length(text) + 1;
    u16 *const copy = static_cast<u16 *>(IosBase_Env().memAlloc(n * 2));
    u16 *to = copy;
    for (; n; --n) *to++ = static_cast<u16>(CRT_btowc(static_cast<u32>(static_cast<s32>(static_cast<signed char>(*text++)))));
    return copy;
}

u32 *refs_facet(u32 *self, u32 refs, u32 vtable) {
    self[1] = refs;
    self[0] = vtable;
    u32 info[0x10];
    Locinfo_Ctor(info, nullptr, gLocalefacet_C_75c74c);
    Locinfo_Dtor(info);
    return self;
}

// The small numeric facet: new when no locale is given, else the locale's own or the classic one.
template <typename Make> void add_numeric(Locimp *imp, u8 **loc, u32 vtable, u32 id_variable, u32 cache, Make make) {
    if (!loc) {
        u32 *const object = alloc_facet(8);
        if (object) {
            object[1] = 0;
            object[0] = vtable;
        }
        const u32 id = locked_id(id_variable);
        Locimp_Addfac(imp, nullptr, as_facet(object), id);
    } else {
        const u32 id = locked_id(id_variable);
        Locimp_Addfac(imp, nullptr, facet_of(loc, id_variable, cache, make), id);
    }
}

// A facet new from make_new() when no locale is given, else the locale's own or the classic one (make() once, kept at cache).
template <typename New, typename Make> void add_facet(Locimp *imp, u8 **loc, u32 id_variable, u32 cache, New make_new, Make make) {
    if (!loc) {
        LocaleFacet *const facet = make_new();
        const u32 id = locked_id(id_variable);
        Locimp_Addfac(imp, nullptr, facet, id);
    } else {
        const u32 id = locked_id(id_variable);
        Locimp_Addfac(imp, nullptr, facet_of(loc, id_variable, cache, make), id);
    }
}

// The id, under the lock or not (the original takes some without).
u32 id_of(u32 id_variable, bool locked) { return locked ? locked_id(id_variable) : Id_Get(&global<u32>(id_variable)); }

// A facet new from make_new() when no locale is given, else use's. The id is taken after make_new(), before use.
template <typename New> void add_used(Locimp *imp, u8 **loc, u32 id_variable, bool locked_new, bool locked_use, New make_new, u32 *(__cdecl *use)(u8 **, u32, u32)) {
    if (!loc) {
        LocaleFacet *const facet = make_new();
        const u32 id = id_of(id_variable, locked_new);
        Locimp_Addfac(imp, nullptr, facet, id);
    } else {
        const u32 id = id_of(id_variable, locked_use);
        Locimp_Addfac(imp, nullptr, as_facet(use(loc, 0, 1)), id);
    }
}

// collate's: refs, the vtable, _Getcoll's block at +8.
LocaleFacet *new_collate(u32 vtable) {
    u32 *const object = alloc_facet(0x10);
    if (object) {
        object[1] = 0;
        object[0] = vtable;
        const unsigned long long conversion = CRT_Getcoll();
        object[2] = static_cast<u32>(conversion);
        object[3] = static_cast<u32>(conversion >> 32);
    }
    return as_facet(object);
}

// The facets of 0x70a044 / 0x709ef4: +8 the _Timevec's block; the emptied _Timevec freed (null).
void take_tnames(u32 *self, const void *info) {
    u32 names;
    Locinfo_Gettnames(info, nullptr, &names);
    self[2] = names;
    Crt().free(nullptr);
}

void fill_tnames(u32 *facet, u32 vtable) {
    facet[2] = 0;
    facet[0] = vtable;
    u32 info[0x10];
    Locinfo_Ctor(info, nullptr, gLocalefacet_C_75c74c);
    take_tnames(facet, info);
    Locinfo_Dtor(info);
}

u32 *collate_ctor(u32 *self, u32 refs, u32 vtable) {
    self[1] = refs;
    self[0] = vtable;
    u32 info[0x10];
    Locinfo_Ctor(info, nullptr, gLocalefacet_C_75c74c);
    const unsigned long long conversion = CRT_Getcoll();
    self[2] = static_cast<u32>(conversion);
    self[3] = static_cast<u32>(conversion >> 32);
    Locinfo_Dtor(info);
    return self;
}

u32 *tnames_ctor(u32 *self, const void *info, u32 refs, u32 vtable) {
    self[1] = refs;
    self[2] = 0;
    self[0] = vtable;
    take_tnames(self, info);
    return self;
}

}  // namespace

extern "C" {

void __fastcall CtypeChar_Init(u32 *self, void *, const void *) {
    u32 lock[2];
    IosBase_Env().lockit(lock);
    u32 got[4];
    const u32 *const ctype = CRT_Getctype(got);
    self[2] = ctype[0];
    self[3] = ctype[1];
    self[4] = ctype[2];
    self[5] = ctype[3];
    if (!gLocalefacet_fb1dd8) {
        gLocalefacet_fb1dd8 = self[4];
        IosBase_Env().atexit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005cfc80));
        self[5] = 0;
    }
    IosBase_Env().unlockit(lock);
}

u32 *__fastcall NumPut_Ctor(u32 *self, void *, u32 refs) { return refs_facet(self, refs, kNumPutVtable); }

u32 *__fastcall NumGet_Ctor(u32 *self, void *, u32 refs) { return refs_facet(self, refs, kNumGetVtable); }

char *__cdecl Numpunct_Copy(const char *text) {
    u32 size = 0;
    while (text[size]) ++size;
    ++size;
    char *const copy = static_cast<char *>(IosBase_Env().memAlloc(size));
    for (u32 i = 0; i < size; ++i) copy[i] = text[i];
    return copy;
}

void __fastcall Numpunct_Init(u32 *self, void *, const void *) {
    char *const *const lconv = reinterpret_cast<char *const *>((uintptr_t)Global781fc8_Get_691530());
    reinterpret_cast<u8 *>(self)[0xc] = static_cast<u8>(*lconv[0]);
    reinterpret_cast<u8 *>(self)[0xd] = static_cast<u8>(*lconv[1]);
    const char *const grouping = lconv[2];
    self[2] = static_cast<u32>(reinterpret_cast<uintptr_t>(Numpunct_Copy(grouping)));
    self[4] = static_cast<u32>(reinterpret_cast<uintptr_t>(Numpunct_Copy("false")));
    self[5] = static_cast<u32>(reinterpret_cast<uintptr_t>(Numpunct_Copy("true")));
}

u16 *__cdecl Facet_WidenedCopy(const char *text) {
    u32 size = 0;
    while (text[size]) ++size;
    ++size;
    u16 *const copy = static_cast<u16 *>(IosBase_Env().memAlloc(size * 2));
    for (u32 i = 0; i < size; ++i) copy[i] = static_cast<u16>(CRT_btowc(static_cast<u32>(static_cast<s32>(text[i]))));
    return copy;
}

void __fastcall WNumpunct_Init(u32 *self, void *, const void *) {
    char *const *const lconv = reinterpret_cast<char *const *>((uintptr_t)Global781fc8_Get_691530());
    u16 *const marks = reinterpret_cast<u16 *>(self + 3);
    marks[0] = static_cast<u16>(CRT_btowc(static_cast<u32>(static_cast<s32>(*lconv[0]))));
    marks[1] = static_cast<u16>(CRT_btowc(static_cast<u32>(static_cast<s32>(*lconv[1]))));
    const char *const grouping = lconv[2];
    self[2] = static_cast<u32>(reinterpret_cast<uintptr_t>(Numpunct_Copy(grouping)));
    self[4] = static_cast<u32>(reinterpret_cast<uintptr_t>(Facet_WidenedCopy(reinterpret_cast<const char *>((uintptr_t)(u32)(uintptr_t)&gLocalefacet_709b68))));
    self[5] = static_cast<u32>(reinterpret_cast<uintptr_t>(Facet_WidenedCopy(reinterpret_cast<const char *>((uintptr_t)(u32)(uintptr_t)&gLocalefacet_709b60))));
}

u32 *__fastcall NumPutW_Ctor(u32 *self, void *, u32 refs) { return refs_facet(self, refs, (u32)(uintptr_t)NumPutW_vtable); }

u32 *__fastcall NumGetW_Ctor(u32 *self, void *, u32 refs) { return refs_facet(self, refs, (u32)(uintptr_t)NumGetW_vtable); }

u32 *__fastcall WCodecvt_Ctor(u32 *self, void *, const void *, u32 refs) {
    self[1] = refs;
    self[0] = (u32)(uintptr_t)CodecvtW_vtable;
    const unsigned long long conversion = CRT_Getcvt();
    self[2] = static_cast<u32>(conversion);
    self[3] = static_cast<u32>(conversion >> 32);
    return self;
}

u32 *__fastcall Numpunct_Ctor(u32 *self, void *, u32 refs) {
    self[1] = refs;
    self[0] = kNumpunctVtable;
    u32 info[0x10];
    Locinfo_Ctor(info, nullptr, gLocalefacet_C_75c74c);
    Numpunct_Init(self, nullptr, info);
    Locinfo_Dtor(info);
    return self;
}

u32 *__fastcall WNumpunct_Ctor(u32 *self, void *, u32 refs) {
    self[1] = refs;
    self[0] = (u32)(uintptr_t)WNumpunct_vtable;
    u32 info[0x10];
    Locinfo_Ctor(info, nullptr, gLocalefacet_C_75c74c);
    WNumpunct_Init(self, nullptr, info);
    Locinfo_Dtor(info);
    return self;
}

static void numpunct_tidy(u32 *self, u32 vtable) {
    self[0] = vtable;
    IosBase_Env().memFree(reinterpret_cast<void *>((uintptr_t)self[2]));
    IosBase_Env().memFree(reinterpret_cast<void *>((uintptr_t)self[4]));
    IosBase_Env().memFree(reinterpret_cast<void *>((uintptr_t)self[5]));
    self[0] = (u32)(uintptr_t)LocaleFacet_vtable;
}

void __fastcall Numpunct_Tidy(u32 *self, void *) { numpunct_tidy(self, kNumpunctVtable); }

void __fastcall WNumpunct_Tidy(u32 *self, void *) { numpunct_tidy(self, (u32)(uintptr_t)WNumpunct_vtable); }

void __fastcall WFacet709c0c_Init(u32 *self, void *, const void *) {
    self[2] = static_cast<u32>(reinterpret_cast<uintptr_t>(Facet_WidenedCopy(reinterpret_cast<const char *>((uintptr_t)(u32)(uintptr_t)&gLocalefacet_709d0c))));
    self[3] = static_cast<u32>(reinterpret_cast<uintptr_t>(Facet_WidenedCopy(reinterpret_cast<const char *>((uintptr_t)(u32)(uintptr_t)&gLocalefacet_709d08))));
}

u32 *__fastcall WFacet709c0c_Ctor(u32 *self, void *, u32 refs) {
    self[1] = refs;
    self[0] = (u32)(uintptr_t)WFacet709c0c_vtable;
    u32 info[0x10];
    Locinfo_Ctor(info, nullptr, gLocalefacet_C_75c74c);
    WFacet709c0c_Init(self, nullptr, info);
    Locinfo_Dtor(info);
    return self;
}

void __fastcall WFacet709c0c_Tidy(u32 *self, void *) {
    self[0] = (u32)(uintptr_t)WFacet709c0c_vtable;
    IosBase_Env().memFree(reinterpret_cast<void *>((uintptr_t)self[2]));
    IosBase_Env().memFree(reinterpret_cast<void *>((uintptr_t)self[3]));
    self[0] = (u32)(uintptr_t)LocaleFacet_vtable;
}

u32 *__fastcall WFacet709bfc_Ctor(u32 *self, void *, u32 refs) { return refs_facet(self, refs, (u32)(uintptr_t)MoneyGetW_vtable); }

void __fastcall Facet709fac_Init(u32 *self, void *, const void *) {
    self[2] = static_cast<u32>(reinterpret_cast<uintptr_t>(Numpunct_Copy(reinterpret_cast<const char *>((uintptr_t)(u32)(uintptr_t)&gLocalefacet_709d0c))));
    self[3] = static_cast<u32>(reinterpret_cast<uintptr_t>(Numpunct_Copy(reinterpret_cast<const char *>((uintptr_t)(u32)(uintptr_t)&gLocalefacet_709d08))));
}

u32 *__fastcall Facet709fac_Ctor(u32 *self, void *, u32 refs) {
    self[1] = refs;
    self[0] = (u32)(uintptr_t)Facet709fac_vtable;
    u32 info[0x10];
    Locinfo_Ctor(info, nullptr, gLocalefacet_C_75c74c);
    Facet709fac_Init(self, nullptr, info);
    Locinfo_Dtor(info);
    return self;
}

u32 *__fastcall Facet709f9c_Ctor(u32 *self, void *, u32 refs) { return refs_facet(self, refs, (u32)(uintptr_t)Facet709f9c_vtable); }

u32 *__fastcall Facet709f8c_Ctor(u32 *self, void *, u32 refs) { return refs_facet(self, refs, (u32)(uintptr_t)Facet709f8c_vtable); }

u32 *__fastcall Codecvt_Ctor(u32 *self, void *, const void *, u32 refs) {
    self[1] = refs;
    self[0] = kCodecvtVtable;
    const unsigned long long conversion = CRT_Getcvt();
    self[2] = static_cast<u32>(conversion);
    self[3] = static_cast<u32>(conversion >> 32);
    return self;
}

u32 *__cdecl UseFacet_Numpunct(u8 **loc, u32, u32 flag) {
    return use_facet(loc, flag, kNumpunctId, (u32)(uintptr_t)&gLocalefacet_fb1db8, (u32)(uintptr_t)&gLocalefacet_fb1dbc, 0x18, 0x00699770, [](u32 *facet) {
        u32 info[0x10];
        Locinfo_Ctor(info, nullptr, "C");
        facet[0] = kNumpunctVtable;
        Numpunct_Init(facet, nullptr, nullptr);
        Locinfo_Dtor(info);
    });
}

u32 *__cdecl UseFacet_WNumpunct(u8 **loc, u32, u32 flag) {
    return use_facet(loc, flag, (u32)(uintptr_t)&gLocalefacet_fb1ddc, (u32)(uintptr_t)&gLocalefacet_fb1e1c, (u32)(uintptr_t)&gLocalefacet_fb1e30, 0x18, 0x006a5c00, [](u32 *facet) {
        u32 info[0x10];
        Locinfo_Ctor(info, nullptr, gLocalefacet_C_75c74c);
        facet[0] = (u32)(uintptr_t)WNumpunct_vtable;
        WNumpunct_Init(facet, nullptr, info);
        Locinfo_Dtor(info);
    });
}

u32 *__cdecl UseFacet_Codecvt(u8 **loc, u32, u32 flag) {
    return use_facet(loc, flag, kCodecvtId, (u32)(uintptr_t)&gLocalefacet_f827e4, (u32)(uintptr_t)&gLocalefacet_f827e8, 0x10, 0x005d1670, [](u32 *facet) {
        u32 info[0x10];
        Locinfo_Ctor(info, nullptr, "C");
        facet[0] = kCodecvtVtable;
        const unsigned long long conversion = CRT_Getcvt();
        facet[2] = static_cast<u32>(conversion);
        facet[3] = static_cast<u32>(conversion >> 32);
        Locinfo_Dtor(info);
    });
}

u32 *__cdecl UseFacet_MoneypunctWIntl(u8 **loc, u32, u32 flag) {
    return use_facet(loc, flag, (u32)(uintptr_t)&gLocalefacet_fb1de8, (u32)(uintptr_t)&gLocalefacet_fb1e2c, (u32)(uintptr_t)&gLocalefacet_fb1e34, 0x2c, 0x006a5e40, [](u32 *facet) { money_fill(facet, 1, (u32)(uintptr_t)MoneypunctW_vtable, (u32)(uintptr_t)Class709da4_vtable, MoneypunctW_Init); });
}

u32 *__cdecl UseFacet_MoneypunctW(u8 **loc, u32, u32 flag) {
    return use_facet(loc, flag, (u32)(uintptr_t)&gLocalefacet_fb1e0c, (u32)(uintptr_t)&gLocalefacet_fb1e28, (u32)(uintptr_t)&gLocalefacet_fb1e38, 0x2c, 0x006a5f60, [](u32 *facet) { money_fill(facet, 0, (u32)(uintptr_t)MoneypunctW_vtable, (u32)(uintptr_t)Class709bbc_vtable, MoneypunctW_Init); });
}

u32 *__cdecl UseFacet_MoneypunctCIntl(u8 **loc, u32, u32 flag) {
    return use_facet(loc, flag, (u32)(uintptr_t)&gLocalefacet_fb1e80, (u32)(uintptr_t)&gLocalefacet_fb1eb0, (u32)(uintptr_t)&gLocalefacet_fb1eb4, 0x2c, 0x006ac1d0, [](u32 *facet) { money_fill(facet, 1, (u32)(uintptr_t)MoneypunctC_vtable, (u32)(uintptr_t)Class709f2c_vtable, MoneypunctC_Init); });
}

u32 *__cdecl UseFacet_MoneypunctC(u8 **loc, u32, u32 flag) {
    return use_facet(loc, flag, (u32)(uintptr_t)&gLocalefacet_fb1e90, (u32)(uintptr_t)&gLocalefacet_fb1eac, (u32)(uintptr_t)&gLocalefacet_fb1eb8, 0x2c, 0x006ac2f0, [](u32 *facet) { money_fill(facet, 0, (u32)(uintptr_t)MoneypunctC_vtable, (u32)(uintptr_t)Class709f5c_vtable, MoneypunctC_Init); });
}

u32 *__cdecl UseFacet_709bec(u8 **loc, u32, u32 flag) {
    return use_facet(loc, flag, (u32)(uintptr_t)&gLocalefacet_fb1e08, (u32)(uintptr_t)&gLocalefacet_fb1e40, (u32)(uintptr_t)&gLocalefacet_fb1e3c, 8, 0x006a6360, [](u32 *facet) {
        u32 info[0x10];
        Locinfo_Ctor(info, nullptr, "C");
        facet[0] = (u32)(uintptr_t)MoneyPutW_vtable;
        Locinfo_Dtor(info);
    });
}

u32 *__cdecl UseFacet_CodecvtW(u8 **loc, u32, u32 flag) {
    return use_facet(loc, flag, (u32)(uintptr_t)&gLocalefacet_fb1e00, (u32)(uintptr_t)&gLocalefacet_7ced40, (u32)(uintptr_t)&gLocalefacet_7ced48, 0x10, 0x004f7a10, [](u32 *facet) {
        u32 info[0x10];
        Locinfo_Ctor(info, nullptr, "C");
        facet[0] = (u32)(uintptr_t)CodecvtW_vtable;
        const unsigned long long conversion = CRT_Getcvt();
        facet[2] = static_cast<u32>(conversion);
        facet[3] = static_cast<u32>(conversion >> 32);
        Locinfo_Dtor(info);
    });
}

u32 *__cdecl UseFacet_TimeC(u8 **loc, u32, u32 flag) {
    return use_facet(loc, flag, (u32)(uintptr_t)&gLocalefacet_fb1e84, (u32)(uintptr_t)&gLocalefacet_fb1ec0, (u32)(uintptr_t)&gLocalefacet_fb1ebc, 0x10, 0x006ac6f0, [](u32 *facet) {
        u32 info[0x10];
        facet[0] = (u32)(uintptr_t)TimeGet_vtable;
        Locinfo_Ctor(info, nullptr, "C");
        u8 *const held = reinterpret_cast<u8 *>(info);
        facet[2] = static_cast<u32>(reinterpret_cast<uintptr_t>(Numpunct_Copy(days_of(held))));
        facet[3] = static_cast<u32>(reinterpret_cast<uintptr_t>(Numpunct_Copy(months_of(held))));
        Locinfo_Dtor(info);
    });
}

u32 *__fastcall TimeC_Ctor(u32 *self, void *, u8 *info, u32 refs) {
    self[1] = refs;
    self[0] = (u32)(uintptr_t)TimeGet_vtable;
    self[2] = static_cast<u32>(reinterpret_cast<uintptr_t>(Numpunct_Copy(days_of(info))));
    self[3] = static_cast<u32>(reinterpret_cast<uintptr_t>(Numpunct_Copy(months_of(info))));
    return self;
}

u32 *__fastcall TimeW_Ctor(u32 *self, void *, u8 *info, u32 refs) {
    self[1] = refs;
    self[0] = (u32)(uintptr_t)WTimeGet_vtable;
    self[2] = static_cast<u32>(reinterpret_cast<uintptr_t>(wide_copy(days_of(info))));
    self[3] = static_cast<u32>(reinterpret_cast<uintptr_t>(wide_copy(months_of(info))));
    return self;
}

StdString *__fastcall Locinfo_GetName(const u8 *self, void *, StdString *out) {
    const StdString *const name = reinterpret_cast<const StdString *>(self + 0x30);
    *reinterpret_cast<u8 *>(&out->allocator) = self[0x30];
    out->ptr = nullptr;
    out->len = 0;
    out->res = 0;
    StdString_AssignSub(out, nullptr, name, 0, kNpos);
    return out;
}

Locimp *__cdecl Locimp_Makeloc(void *info, u32 cats, Locimp *imp, u8 **loc) {
    u32 lock[2];
    IosBase_Env().lockit(lock);
    const u32 ctype = cats & 2;
    if (ctype) {
        if (!loc) {
            u8 *const object = static_cast<u8 *>(IosBase_Env().memAlloc(0x18));
            if (object) {
                Init_4f1890(object, nullptr, 0);
                *reinterpret_cast<u32 *>(object) = kCtypeVtable;
                CtypeChar_Init(reinterpret_cast<u32 *>(object), nullptr, info);
            }
            const u32 id = locked_id(kCtypeId);
            Locimp_Addfac(imp, nullptr, as_facet(object), id);
        } else {
            const u32 id = locked_id(kCtypeId);
            LocaleFacet *const facet = facet_of(loc, kCtypeId, (u32)(uintptr_t)&gLocalefacet_f827e0, []() {
                void *const object = IosBase_Env().memAlloc(0x18);
                void *const made = object ? IosBase_Env().ctypeCtor(object, nullptr, 0, 0, 0) : nullptr;
                return as_facet(Globalf827dc_Set(static_cast<u8 *>(made)));
            });
            Locimp_Addfac(imp, nullptr, facet, id);
        }
    }
    const u32 numeric = cats & 8;
    if (numeric) {
        add_numeric(imp, loc, kNumGetVtable, kNumGetId, (u32)(uintptr_t)&gLocalefacet_fb1db4, []() {
            u32 *const object = alloc_facet(8);
            return as_facet(Globalfb1dc4_Set(reinterpret_cast<u8 *>(object ? NumGet_Ctor(object, nullptr, 0) : nullptr)));
        });
        add_numeric(imp, loc, kNumPutVtable, kNumPutId, (u32)(uintptr_t)&gLocalefacet_fb1db0, []() {
            u32 *const object = alloc_facet(8);
            return as_facet(Globalfb1dc0_Set(reinterpret_cast<u8 *>(object ? NumPut_Ctor(object, nullptr, 0) : nullptr)));
        });
        if (!loc) {
            u32 *const object = alloc_facet(0x18);
            if (object) {
                object[1] = 0;
                object[0] = kNumpunctVtable;
                Numpunct_Init(object, nullptr, info);
            }
            const u32 id = locked_id(kNumpunctId);
            Locimp_Addfac(imp, nullptr, as_facet(object), id);
        } else {
            const u32 id = locked_id(kNumpunctId);
            Locimp_Addfac(imp, nullptr, as_facet(UseFacet_Numpunct(loc, 0, 1)), id);
        }
    }
    if (ctype) {
        if (!loc) {
            u32 *const object = alloc_facet(0x10);
            u32 *const facet = object ? Codecvt_Ctor(object, nullptr, info, 0) : nullptr;
            const u32 id = Id_Get(&global<u32>(kCodecvtId));
            Locimp_Addfac(imp, nullptr, as_facet(facet), id);
        } else {
            const u32 id = Id_Get(&global<u32>(kCodecvtId));
            Locimp_Addfac(imp, nullptr, as_facet(UseFacet_Codecvt(loc, 0, 1)), id);
        }
    }
    IosBase_Env().makexloc(info, cats, imp, loc);
    IosBase_Env().makewloc(info, cats, imp, loc);
    imp->categories |= cats;
    StdString name;
    Locinfo_GetName(static_cast<const u8 *>(info), nullptr, &name);
    StdString_AssignSub(&imp->name, nullptr, &name, 0, kNpos);
    StdString_Tidy(&name, nullptr, 1);
    IosBase_Env().unlockit(lock);
    return imp;
}

u32 *__fastcall Locinfo_Gettnames(const void *, void *, u32 *out) {
    *out = static_cast<u32>(reinterpret_cast<uintptr_t>(CRT_Gettnames()));
    return out;
}

static void facet_dtor_two(u32 *self, u32 table) {
    self[0] = table;
    IosBase_Env().memFree(reinterpret_cast<void *>((uintptr_t)self[2]));
    IosBase_Env().memFree(reinterpret_cast<void *>((uintptr_t)self[3]));
    self[0] = (u32)(uintptr_t)LocaleFacet_vtable;
}

static void facet_dtor_one(u32 *self, u32 table) {
    self[0] = table;
    Crt().free(reinterpret_cast<void *>((uintptr_t)self[2]));
    self[0] = (u32)(uintptr_t)LocaleFacet_vtable;
}

void __fastcall Facet709ed4_Dtor(u32 *self, void *) { facet_dtor_two(self, (u32)(uintptr_t)WTimeGet_vtable); }
void __fastcall Facet709fac_Dtor(u32 *self, void *) { facet_dtor_two(self, (u32)(uintptr_t)Facet709fac_vtable); }
void __fastcall Facet70a024_Dtor(u32 *self, void *) { facet_dtor_two(self, (u32)(uintptr_t)TimeGet_vtable); }
void __fastcall Facet709ef4_Dtor(u32 *self, void *) { facet_dtor_one(self, (u32)(uintptr_t)WTimePut_vtable); }
void __fastcall Facet70a044_Dtor(u32 *self, void *) { facet_dtor_one(self, (u32)(uintptr_t)TimePut_vtable); }
void __fastcall Timevec_Dtor(void **self, void *) { Crt().free(*self); }

s32 __fastcall WideRange_Compare(void *, void *, const u16 *a, const u16 *a_end, const u16 *b, const u16 *b_end) {
    for (; a != a_end; ++a, ++b) {
        if (b == b_end) return 1;
        if (*b > *a) return -1;
        if (*b < *a) return 1;
    }
    return b == b_end ? 0 : -1;
}

static u32 *deleted(u32 *self, u32 flags) {
    if (flags & 1) Crt().free(self);
    return self;
}

u32 *__fastcall Facet709ed4_DeletingDtor(u32 *self, void *, u32 flags) { Facet709ed4_Dtor(self, nullptr); return deleted(self, flags); }
u32 *__fastcall Facet709fac_DeletingDtor(u32 *self, void *, u32 flags) { Facet709fac_Dtor(self, nullptr); return deleted(self, flags); }
u32 *__fastcall Facet70a024_DeletingDtor(u32 *self, void *, u32 flags) { Facet70a024_Dtor(self, nullptr); return deleted(self, flags); }
u32 *__fastcall Facet709ef4_DeletingDtor(u32 *self, void *, u32 flags) { Facet709ef4_Dtor(self, nullptr); return deleted(self, flags); }
u32 *__fastcall Facet70a044_DeletingDtor(u32 *self, void *, u32 flags) { Facet70a044_Dtor(self, nullptr); return deleted(self, flags); }
u32 *__fastcall MoneypunctC_DeletingDtor(u32 *self, void *, u32 flags) { MoneypunctC_Tidy(self, nullptr); return deleted(self, flags); }
u32 *__fastcall MoneypunctW_DeletingDtor(u32 *self, void *, u32 flags) { MoneypunctW_Tidy(self, nullptr); return deleted(self, flags); }
u32 *__fastcall Numpunct_DeletingDtor(u32 *self, void *, u32 flags) { Numpunct_Tidy(self, nullptr); return deleted(self, flags); }
u32 *__fastcall WNumpunct_DeletingDtor(u32 *self, void *, u32 flags) { WNumpunct_Tidy(self, nullptr); return deleted(self, flags); }
u32 *__fastcall WFacet709c0c_DeletingDtor(u32 *self, void *, u32 flags) { WFacet709c0c_Tidy(self, nullptr); return deleted(self, flags); }

u8 *__fastcall Vtbl709b44_DeletingDtor(u8 *self, void *, u32 flags) {
    Vtbl709b44_Set_6a9180(self);
    if (flags & 1) Crt().free(self);
    return self;
}

u8 *__fastcall Vtbl70a0a8_DeletingDtor(u8 *self, void *, u32 flags) {
    *reinterpret_cast<u32 *>(self) = (u32)(uintptr_t)Class70a0a8_vtable;
    if (flags & 1) Crt().free(self);
    return self;
}

u32 *__fastcall CollateC_Ctor(u32 *self, void *, u32 refs) { return collate_ctor(self, refs, (u32)(uintptr_t)CollateC_vtable); }
u32 *__fastcall CollateW_Ctor(u32 *self, void *, u32 refs) { return collate_ctor(self, refs, (u32)(uintptr_t)CollateW_vtable); }
u32 *__fastcall Facet70a044_Ctor(u32 *self, void *, const void *info, u32 refs) { return tnames_ctor(self, info, refs, (u32)(uintptr_t)TimePut_vtable); }
u32 *__fastcall Facet709ef4_Ctor(u32 *self, void *, const void *info, u32 refs) { return tnames_ctor(self, info, refs, (u32)(uintptr_t)WTimePut_vtable); }

u32 *__cdecl UseFacet_70a044(u8 **loc, u32, u32 flag) {
    return use_facet(loc, flag, (u32)(uintptr_t)&gLocalefacet_fb1e88, (u32)(uintptr_t)&gLocalefacet_fb1ec8, (u32)(uintptr_t)&gLocalefacet_fb1ec4, 0xc, 0x006ac790, [](u32 *facet) { fill_tnames(facet, (u32)(uintptr_t)TimePut_vtable); });
}

u32 *__cdecl UseFacet_709ef4(u8 **loc, u32, u32 flag) {
    return use_facet(loc, flag, (u32)(uintptr_t)&gLocalefacet_fb1e04, (u32)(uintptr_t)&gLocalefacet_fb1e50, (u32)(uintptr_t)&gLocalefacet_fb1e4c, 0xc, 0x006a64a0, [](u32 *facet) { fill_tnames(facet, (u32)(uintptr_t)WTimePut_vtable); });
}

u32 *__cdecl UseFacet_TimeW(u8 **loc, u32, u32 flag) {
    return use_facet(loc, flag, (u32)(uintptr_t)&gLocalefacet_fb1df4, (u32)(uintptr_t)&gLocalefacet_fb1e48, (u32)(uintptr_t)&gLocalefacet_fb1e44, 0x10, 0x006a6400, [](u32 *facet) {
        u32 info[0x10];
        facet[0] = (u32)(uintptr_t)WTimeGet_vtable;
        Locinfo_Ctor(info, nullptr, gLocalefacet_C_75c74c);
        u8 *const held = reinterpret_cast<u8 *>(info);
        facet[2] = static_cast<u32>(reinterpret_cast<uintptr_t>(wide_copy(days_of(held))));
        facet[3] = static_cast<u32>(reinterpret_cast<uintptr_t>(Facet_WidenedCopy(months_of(held))));
        Locinfo_Dtor(info);
    });
}

void __cdecl Locimp_Makexloc(void *info, u32 cats, Locimp *imp, u8 **loc) {
    u32 lock[2];
    IosBase_Env().lockit(lock);
    if (cats & 1) {
        add_facet(imp, loc, (u32)(uintptr_t)&gLocalefacet_fb1e98, (u32)(uintptr_t)&gLocalefacet_fb1ea8, []() { return new_collate((u32)(uintptr_t)CollateC_vtable); }, []() {
            u32 *const object = alloc_facet(0x10);
            return as_facet(Globalfb1ecc_Set(reinterpret_cast<u8 *>(object ? CollateC_Ctor(object, nullptr, 0) : nullptr)));
        });
    }
    if (cats & 0x20) {
        add_facet(imp, loc, (u32)(uintptr_t)&gLocalefacet_fb1e8c, (u32)(uintptr_t)&gLocalefacet_fb1ea4, [info]() {
            u32 *const object = alloc_facet(0x10);
            if (object) {
                Init_69fef0(reinterpret_cast<u8 *>(object), nullptr, 0);
                object[0] = (u32)(uintptr_t)Facet709fac_vtable;
                Facet709fac_Init(object, nullptr, info);
            }
            return as_facet(object);
        }, []() {
            u32 *const object = alloc_facet(0x10);
            return as_facet(Globalfb1ed0_Set(reinterpret_cast<u8 *>(object ? Facet709fac_Ctor(object, nullptr, 0) : nullptr)));
        });
    }
    if (cats & 4) {
        add_numeric(imp, loc, (u32)(uintptr_t)Facet709f9c_vtable, (u32)(uintptr_t)&gLocalefacet_fb1e94, (u32)(uintptr_t)&gLocalefacet_fb1ea0, []() {
            u32 *const object = alloc_facet(8);
            return as_facet(Globalfb1ed4_Set(reinterpret_cast<u8 *>(object ? Facet709f9c_Ctor(object, nullptr, 0) : nullptr)));
        });
        add_numeric(imp, loc, (u32)(uintptr_t)Facet709f8c_vtable, (u32)(uintptr_t)&gLocalefacet_fb1e7c, (u32)(uintptr_t)&gLocalefacet_fb1e9c, []() {
            u32 *const object = alloc_facet(8);
            return as_facet(Globalfb1ed8_Set(reinterpret_cast<u8 *>(object ? Facet709f8c_Ctor(object, nullptr, 0) : nullptr)));
        });
    }
    if (cats & 8) {
        add_used(imp, loc, (u32)(uintptr_t)&gLocalefacet_fb1e90, true, true, [info]() {
            u8 *const object = reinterpret_cast<u8 *>(alloc_facet(0x2c));
            if (object) {
                MoneypunctC_Ctor(object, nullptr, info, 0, 0);
                *reinterpret_cast<u32 *>(object) = (u32)(uintptr_t)Class709f5c_vtable;
            }
            return as_facet(object);
        }, UseFacet_MoneypunctC);
        add_used(imp, loc, (u32)(uintptr_t)&gLocalefacet_fb1e80, true, false, [info]() {
            u8 *const object = reinterpret_cast<u8 *>(alloc_facet(0x2c));
            if (object) {
                MoneypunctC_Ctor(object, nullptr, info, 0, 1);
                *reinterpret_cast<u32 *>(object) = (u32)(uintptr_t)Class709f2c_vtable;
            }
            return as_facet(object);
        }, UseFacet_MoneypunctCIntl);
    }
    if (cats & 0x10) {
        add_used(imp, loc, (u32)(uintptr_t)&gLocalefacet_fb1e84, false, false, [info]() {
            u32 *const object = alloc_facet(0x10);
            return as_facet(object ? TimeC_Ctor(object, nullptr, static_cast<u8 *>(info), 0) : nullptr);
        }, UseFacet_TimeC);
        add_used(imp, loc, (u32)(uintptr_t)&gLocalefacet_fb1e88, false, false, [info]() {
            u32 *const object = alloc_facet(0xc);
            return as_facet(object ? Facet70a044_Ctor(object, nullptr, info, 0) : nullptr);
        }, UseFacet_70a044);
    }
    IosBase_Env().unlockit(lock);
}

void __cdecl Locimp_Makewloc(void *info, u32 cats, Locimp *imp, u8 **loc) {
    u32 lock[2];
    IosBase_Env().lockit(lock);
    if (cats & 2) {
        add_facet(imp, loc, (u32)(uintptr_t)&gLocalefacet_fb1df8, (u32)(uintptr_t)&gLocalefacet_7ced3c, []() {
            u8 *const object = static_cast<u8 *>(IosBase_Env().memAlloc(0x18));
            if (object) {
                Init_4f1890(object, nullptr, 0);
                *reinterpret_cast<u32 *>(object) = (u32)(uintptr_t)WCtype_vtable;
                u32 got[4];
                const u32 *const ctype = CRT_Getctype(got);
                for (u32 i = 0; i < 4; ++i) reinterpret_cast<u32 *>(object)[2 + i] = ctype[i];
            }
            return as_facet(object);
        }, []() {
            u8 *const object = static_cast<u8 *>(IosBase_Env().memAlloc(0x18));
            return as_facet(Global7ced44_Set(object ? WCtype_Ctor(object, nullptr, 0) : nullptr));
        });
    }
    if (cats & 8) {
        add_numeric(imp, loc, (u32)(uintptr_t)NumGetW_vtable,(u32)(uintptr_t)&gLocalefacet_fb1dfc, (u32)(uintptr_t)&gLocalefacet_fb1e24, []() {
            u32 *const object = alloc_facet(8);
            return as_facet(Globalfb1e58_Set(reinterpret_cast<u8 *>(object ? NumGetW_Ctor(object, nullptr, 0) : nullptr)));
        });
        add_numeric(imp, loc, (u32)(uintptr_t)NumPutW_vtable, (u32)(uintptr_t)&gLocalefacet_fb1dec, (u32)(uintptr_t)&gLocalefacet_fb1e20, []() {
            u32 *const object = alloc_facet(8);
            return as_facet(Globalfb1e54_Set(reinterpret_cast<u8 *>(object ? NumPutW_Ctor(object, nullptr, 0) : nullptr)));
        });
        add_facet(imp, loc, (u32)(uintptr_t)&gLocalefacet_fb1ddc, (u32)(uintptr_t)&gLocalefacet_fb1e1c, [info]() {
            u32 *const object = alloc_facet(0x18);
            if (object) {
                object[1] = 0;
                object[0] = (u32)(uintptr_t)WNumpunct_vtable;
                WNumpunct_Init(object, nullptr, info);
            }
            return as_facet(object);
        }, []() {
            u32 *const object = alloc_facet(0x18);
            return as_facet(Globalfb1e30_Set(reinterpret_cast<u8 *>(object ? WNumpunct_Ctor(object, nullptr, 0) : nullptr)));
        });
    }
    if (cats & 1) {
        add_facet(imp, loc, (u32)(uintptr_t)&gLocalefacet_fb1df0, (u32)(uintptr_t)&gLocalefacet_fb1e18, []() { return new_collate((u32)(uintptr_t)CollateW_vtable); }, []() {
            u32 *const object = alloc_facet(0x10);
            return as_facet(Globalfb1e5c_Set(reinterpret_cast<u8 *>(object ? CollateW_Ctor(object, nullptr, 0) : nullptr)));
        });
    }
    if (cats & 0x20) {
        add_facet(imp, loc, (u32)(uintptr_t)&gLocalefacet_fb1de4, (u32)(uintptr_t)&gLocalefacet_fb1e14, [info]() {
            u32 *const object = alloc_facet(0x10);
            if (object) {
                Init_69fef0(reinterpret_cast<u8 *>(object), nullptr, 0);
                object[0] = (u32)(uintptr_t)WFacet709c0c_vtable;
                WFacet709c0c_Init(object, nullptr, info);
            }
            return as_facet(object);
        }, []() {
            u32 *const object = alloc_facet(0x10);
            return as_facet(Globalfb1e60_Set(reinterpret_cast<u8 *>(object ? WFacet709c0c_Ctor(object, nullptr, 0) : nullptr)));
        });
    }
    if (cats & 4) {
        add_numeric(imp, loc, (u32)(uintptr_t)MoneyGetW_vtable, (u32)(uintptr_t)&gLocalefacet_fb1de0, (u32)(uintptr_t)&gLocalefacet_fb1e10, []() {
            u32 *const object = alloc_facet(8);
            return as_facet(Globalfb1e64_Set(reinterpret_cast<u8 *>(object ? WFacet709bfc_Ctor(object, nullptr, 0) : nullptr)));
        });
        add_used(imp, loc, (u32)(uintptr_t)&gLocalefacet_fb1e08, true, true, []() {
            u32 *const object = alloc_facet(8);
            if (object) {
                object[1] = 0;
                object[0] = (u32)(uintptr_t)MoneyPutW_vtable;
            }
            return as_facet(object);
        }, UseFacet_709bec);
    }
    if (cats & 8) {
        add_used(imp, loc, (u32)(uintptr_t)&gLocalefacet_fb1e0c, true, false, [info]() {
            u8 *const object = reinterpret_cast<u8 *>(alloc_facet(0x2c));
            if (object) {
                MoneypunctW_Ctor(object, nullptr, info, 0, 0);
                *reinterpret_cast<u32 *>(object) = (u32)(uintptr_t)Class709bbc_vtable;
            }
            return as_facet(object);
        }, UseFacet_MoneypunctW);
        add_used(imp, loc, (u32)(uintptr_t)&gLocalefacet_fb1de8, false, false, [info]() {
            u8 *const object = reinterpret_cast<u8 *>(alloc_facet(0x2c));
            return as_facet(object ? MoneypunctWIntl_Ctor(object, nullptr, info, 0) : nullptr);
        }, UseFacet_MoneypunctWIntl);
    }
    if (cats & 0x10) {
        add_used(imp, loc, (u32)(uintptr_t)&gLocalefacet_fb1df4, false, false, [info]() {
            u32 *const object = alloc_facet(0x10);
            return as_facet(object ? TimeW_Ctor(object, nullptr, static_cast<u8 *>(info), 0) : nullptr);
        }, UseFacet_TimeW);
        add_used(imp, loc, (u32)(uintptr_t)&gLocalefacet_fb1e04, false, false, [info]() {
            u32 *const object = alloc_facet(0xc);
            return as_facet(object ? Facet709ef4_Ctor(object, nullptr, info, 0) : nullptr);
        }, UseFacet_709ef4);
    }
    if (cats & 2) {
        add_used(imp, loc, (u32)(uintptr_t)&gLocalefacet_fb1e00, false, false, [info]() {
            u32 *const object = alloc_facet(0x10);
            return as_facet(object ? WCodecvt_Ctor(object, nullptr, info, 0) : nullptr);
        }, UseFacet_CodecvtW);
    }
    IosBase_Env().unlockit(lock);
}

}  // extern "C"
