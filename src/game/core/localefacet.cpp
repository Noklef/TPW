#include "localefacet.h"

#include <cstdint>

#include "crt.h"
#include "globals.h"
#include "initializers.h"
#include "iosbase.h"
#include "locinfo.h"
#include "sharedglobals.h"
#include "stdlocale.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const u32 kCtypeVtable = 0x00709b1c;
const u32 kNumGetVtable = 0x00709aec;
const u32 kNumPutVtable = 0x00709acc;
const u32 kNumpunctVtable = 0x00709aac;
const u32 kCodecvtVtable = 0x00709b8c;
const u32 kCtypeId = 0x00fb1da4;
const u32 kNumGetId = 0x00fb1da0;
const u32 kNumPutId = 0x00fb1dac;
const u32 kNumpunctId = 0x00fb1da8;
const u32 kCodecvtId = 0x00fb275c;
const u32 kNpos = 0xffffffff;

u32 *alloc_facet(u32 size) { return static_cast<u32 *>(IosBase_Env().memAlloc(size)); }

LocaleFacet *as_facet(void *object) { return static_cast<LocaleFacet *>(object); }

void throw_missing() {
    u32 bad[3];
    BadCast_Ctor(bad, nullptr, reinterpret_cast<const char *>((uintptr_t)0x0075c734));
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

u32 *refs_facet(u32 *self, u32 refs, u32 vtable) {
    self[1] = refs;
    self[0] = vtable;
    u32 info[0x10];
    Locinfo_Ctor(info, nullptr, "C");
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
    if (!global<u32>(0x00fb1dd8)) {
        global<u32>(0x00fb1dd8) = self[4];
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

u32 *__fastcall Codecvt_Ctor(u32 *self, void *, const void *, u32 refs) {
    self[1] = refs;
    self[0] = kCodecvtVtable;
    const unsigned long long conversion = CRT_Getcvt();
    self[2] = static_cast<u32>(conversion);
    self[3] = static_cast<u32>(conversion >> 32);
    return self;
}

u32 *__cdecl UseFacet_Numpunct(u8 **loc, u32, u32 flag) {
    return use_facet(loc, flag, kNumpunctId, 0x00fb1db8, 0x00fb1dbc, 0x18, 0x00699770, [](u32 *facet) {
        u32 info[0x10];
        Locinfo_Ctor(info, nullptr, "C");
        facet[0] = kNumpunctVtable;
        Numpunct_Init(facet, nullptr, nullptr);
        Locinfo_Dtor(info);
    });
}

u32 *__cdecl UseFacet_Codecvt(u8 **loc, u32, u32 flag) {
    return use_facet(loc, flag, kCodecvtId, 0x00f827e4, 0x00f827e8, 0x10, 0x005d1670, [](u32 *facet) {
        u32 info[0x10];
        Locinfo_Ctor(info, nullptr, "C");
        facet[0] = kCodecvtVtable;
        const unsigned long long conversion = CRT_Getcvt();
        facet[2] = static_cast<u32>(conversion);
        facet[3] = static_cast<u32>(conversion >> 32);
        Locinfo_Dtor(info);
    });
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
            LocaleFacet *const facet = facet_of(loc, kCtypeId, 0x00f827e0, []() {
                void *const object = IosBase_Env().memAlloc(0x18);
                void *const made = object ? IosBase_Env().ctypeCtor(object, nullptr, 0, 0, 0) : nullptr;
                return as_facet(Globalf827dc_Set(static_cast<u8 *>(made)));
            });
            Locimp_Addfac(imp, nullptr, facet, id);
        }
    }
    const u32 numeric = cats & 8;
    if (numeric) {
        add_numeric(imp, loc, kNumGetVtable, kNumGetId, 0x00fb1db4, []() {
            u32 *const object = alloc_facet(8);
            return as_facet(Globalfb1dc4_Set(reinterpret_cast<u8 *>(object ? NumGet_Ctor(object, nullptr, 0) : nullptr)));
        });
        add_numeric(imp, loc, kNumPutVtable, kNumPutId, 0x00fb1db0, []() {
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

}  // extern "C"
