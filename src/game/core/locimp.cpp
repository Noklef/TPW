#include "locimp.h"

#include <cstdint>

#include "crt.h"
#include "iosbase.h"
#include "locinfo.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const u32 bad_alloc_vtable = 0x00709f1c;
const u32 none = 0xffffffff;

// A count taken, unless frozen at -1.
void count_in(LocaleFacet *facet) {
    u32 lock[2];
    IosBase_Env().lockit(lock);
    if (facet->refs < none) ++facet->refs;
    IosBase_Env().unlockit(lock);
}

// A count dropped, unless 0 or frozen at -1.
void count_out(LocaleFacet *facet) {
    if (facet->refs > 0 && facet->refs < none) --facet->refs;
}

}  // namespace

u32 *__fastcall BadAlloc_Ctor(u32 *self, void *, const char *text) {
    CRT_exception_Ctor(self, nullptr, &text);
    self[0] = bad_alloc_vtable;
    return self;
}

u32 *__fastcall BadAlloc_Copy(u32 *self, void *, const u32 *from) {
    CRT_exception_Copy(self, nullptr, from);
    self[0] = bad_alloc_vtable;
    return self;
}

void __fastcall BadAlloc_Dtor(u32 *self) {
    self[0] = bad_alloc_vtable;
    CRT_exception_Dtor(self);
}

void __cdecl Nomemory() {
    u32 *const made = &global<u32>(0x00fb1e70);
    u8 &flag = global<u8>(0x00fb1e68);
    if (!(flag & 1)) {
        flag |= 1;
        BadAlloc_Ctor(made, nullptr, "bad allocation");
        IosBase_Env().atexit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x006a66e0));
    }
    u32 copy[3];
    BadAlloc_Copy(copy, nullptr, made);
    IosBase_Env().cxxThrow(copy, reinterpret_cast<const void *>((uintptr_t)0x00721c88));
}

void __fastcall Locimp_Addfac(Locimp *self, void *, LocaleFacet *facet, u32 id) {
    u32 lock[2];
    IosBase_Env().lockit(lock);
    if (self->count <= id) {
        u32 room = id + 1;
        if (room < 0x20) room = 0x20;
        LocaleFacet **const grown = static_cast<LocaleFacet **>(CrtInner().realloc(self->facets, room * 4));
        if (!grown) Nomemory();
        self->facets = grown;
        while (self->count < room) self->facets[self->count++] = nullptr;
    }
    count_in(facet);
    LocaleFacet *const old = self->facets[id];
    if (old) {
        u32 inner[2];
        IosBase_Env().lockit(inner);
        count_out(old);
        LocaleFacet *const dead = old->refs ? nullptr : old;
        IosBase_Env().unlockit(inner);
        if (dead) dead->vtable[0](dead, nullptr, 1);
    }
    self->facets[id] = facet;
    IosBase_Env().unlockit(lock);
}

Locimp *__fastcall Locimp_Copy(Locimp *self, void *, const Locimp *from) {
    self->facet.refs = 1;
    self->facet.vtable = reinterpret_cast<void(__fastcall *const *)(LocaleFacet *, void *, u32)>((uintptr_t)0x00709b44);
    self->facets = nullptr;
    self->count = from->count;
    self->categories = from->categories;
    self->xparse = from->xparse;
    *reinterpret_cast<u8 *>(&self->name.allocator) = *reinterpret_cast<const u8 *>(&from->name.allocator);
    self->name.ptr = nullptr;
    self->name.len = 0;
    self->name.res = 0;
    StdString_AssignSub(&self->name, nullptr, &from->name, 0, none);
    self->facet.vtable = reinterpret_cast<void(__fastcall *const *)(LocaleFacet *, void *, u32)>((uintptr_t)0x00709bac);
    u32 lock[2];
    IosBase_Env().lockit(lock);
    if (reinterpret_cast<uintptr_t>(from) == global<u32>(0x00fb1dcc)) {
        u32 info[0x10];
        Locinfo_Ctor(info, nullptr, "C");
        IosBase_Env().makeloc(info, 0x3f, self, nullptr);
        Locinfo_Dtor(info);
    } else {
        u32 inner[2];
        IosBase_Env().lockit(inner);
        if (self->count) {
            self->facets = static_cast<LocaleFacet **>(CRT_malloc(self->count * 4));
            if (!self->facets) Nomemory();
            for (u32 i = self->count; i;) {
                --i;
                LocaleFacet *const facet = from->facets[i];
                self->facets[i] = facet;
                if (facet) count_in(facet);
            }
        }
        IosBase_Env().unlockit(inner);
    }
    IosBase_Env().unlockit(lock);
    return self;
}

LocaleHandle *__fastcall Locale_Addfac(LocaleHandle *self, void *, LocaleFacet *facet, u32 id, u32 mask) {
    Locimp *const imp = self->imp;
    if (imp->facet.refs > 1) {
        u32 lock[2];
        IosBase_Env().lockit(lock);
        count_out(&imp->facet);
        IosBase_Env().unlockit(lock);
        void *const block = IosBase_Env().memAlloc(sizeof(Locimp));
        self->imp = block ? Locimp_Copy(static_cast<Locimp *>(block), nullptr, self->imp) : nullptr;
    }
    Locimp_Addfac(self->imp, nullptr, facet, id);
    if (mask) {
        StdString *const name = &self->imp->name;
        const u8 owners = name->ptr ? static_cast<u8>(name->ptr[-1]) : 0;
        if (owners && owners != 0xff) {
            StdString_Copy(name, nullptr, 1);
        } else if (name->res > 0x1f || name->res < 1) {
            StdString_Tidy(name, nullptr, 1);
            StdString_Copy(name, nullptr, 1);
        }
        name->ptr[0] = '*';
        name->len = 1;
        name->ptr[1] = 0;
    }
    return self;
}
