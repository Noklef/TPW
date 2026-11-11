#include "nstream.h"

#include <cstdint>
#include <cstring>

#include "accessors.h"
#include "buffers.h"
#include "crt.h"
#include "gameheap.h"
#include "globals.h"
#include "iosbase.h"
#include "sharedglobals.h"
#include "stdlocale.h"
#include "vcall.h"
#include "vobj3.h"
#include "vstream.h"
#include "localefacet.h"
#include "addrcalls.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const NStreamEnv kGameEnv = {GameAddress<void(__stdcall *)(void *, const void *)>(0x0067d3a0),
                             GameAddress<void *(__fastcall *)(void *, void *, const char *)>(0x00698fc0),
                             GameAddress<void *(__fastcall *)(void *)>(0x00699100),
                             GameAddress<const u32 *(__cdecl *)(u32 *)>(0x00699bd0),
                             &CRT_atexit,
                             &IosBase_Init,
                             &IosBase_Addstd,
                             GameAddress<void(__fastcall *)(u8 *)>(0x005cde40),
                             GameAddress<void(__fastcall *)(u8 *)>(0x005ce920),
                             GameAddress<void(__fastcall *)(u8 *)>(0x005cf630),
                             &CRT_TolowerLocking,
                             &CRT_ToupperLocking};
const NStreamEnv *g_env = &kGameEnv;

u32 base_of(const u8 *stream) { return at<u32>(at<u8 *>(stream, 0), 4); }
u8 *ios_of(u8 *stream) { return stream + base_of(stream); }
u8 *buffer_of(u8 *stream) { return at<u8 *>(ios_of(stream), 0x28); }

u8 *use_ctype(u8 **locale) {
    const u32 id = Id_Get(&gLocalefacet_fb1da4);
    u8 *const facet = Locale_Getfacet(locale, nullptr, id, 1);
    if (facet) return facet;
    if (Locale_Iscloc(locale)) {
        u8 *&kept = reinterpret_cast<u8 *&>(gLocalefacet_f827e0);
        if (kept) return kept;
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x18));
        kept = Globalf827dc_Set(block ? NCtype_Ctor(block, nullptr, nullptr, 0, 0) : nullptr);
        return kept;
    }
    u32 bad_cast[3];
    BadCast_Ctor(bad_cast, nullptr, gLocalefacet_MissingFacet_75c734);
    g_env->cxxThrow(bad_cast, reinterpret_cast<const void *>((uintptr_t)0x007193a8));
    return nullptr;
}

// sbumpc: the unit under the position, which moves on, or slot 0x14.
u32 take_unit(u8 *buffer) {
    const u32 position = *at<u32 *>(buffer, 0x1c);
    if (position) {
        u32 *const left = at<u32 *>(buffer, 0x2c);
        const u32 count = *left;
        if (position < position + count) {
            *left = count - 1;
            u32 *const cursor = at<u32 *>(buffer, 0x1c);
            const u32 now = *cursor;
            *cursor = now + 1;
            return *reinterpret_cast<const u8 *>((uintptr_t)now);
        }
    }
    return vcall<u32>(buffer, 0x14);
}

void init_ios(u8 *ios, u8 *buffer, u32 addstd) {
    at<u8 *>(ios, 0x28) = buffer;
    at<u32>(ios, 0x2c) = 0;
    at<u8>(ios, 0x30) = 0x20;
    g_env->iosInit(ios);
    if (!at<u32>(ios, 0x28)) IosBase_Clear(ios, nullptr, at<u32>(ios, 4) | 4, 0);
    if (static_cast<u8>(addstd)) g_env->iosAddstd(ios);
}

void flush_stream(u8 *stream) {
    u32 state = 0;
    u8 *const ios = ios_of(stream);
    if (!(at<u8>(ios, 4) & 6) && vcall<s32>(at<u8 *>(ios, 0x28), 0x2c) == -1) state = 4;
    if (!state) return;
    u8 *const after = ios_of(stream);
    u32 next = at<u32>(after, 4) | state;
    if (!at<u32>(after, 0x28)) next |= 4;
    IosBase_Clear(after, nullptr, next, 0);
}

// sputc of the ios's fill: at the put position, which moves on, or slot 4.
u32 put_fill(u8 *stream) {
    u8 *const ios = ios_of(stream);
    u8 *const buffer = at<u8 *>(ios, 0x28);
    const u8 fill = at<u8>(ios, 0x30);
    const u32 position = *at<u32 *>(buffer, 0x20);
    if (position) {
        u32 *const left = at<u32 *>(buffer, 0x30);
        const u32 count = *left;
        if (position < position + count) {
            *left = count - 1;
            u32 *const cursor = at<u32 *>(buffer, 0x20);
            const u32 now = *cursor;
            *cursor = now + 1;
            *reinterpret_cast<u8 *>((uintptr_t)now) = fill;
            return fill;
        }
    }
    return vcall<u32>(buffer, 4, static_cast<u32>(fill));
}

}  // namespace

void NStream_SetEnv(const NStreamEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u8 *__fastcall NCtype_Ctor(u8 *self, void *, u8 *table, u32 del, u32 refs) {
    at<u32>(self, 4) = refs;
    at<u32>(self, 0) = (u32)(uintptr_t)NCtype_vtable;
    u8 info[0x48];
    g_env->locinfoCtor(info, nullptr, gLocalefacet_C_75c74c);
    u32 out[4];
    const u32 *const ctype = g_env->getCtype(out);
    for (u32 i = 0; i < 4; ++i) at<u32>(self, 8 + 4 * i) = ctype[i];
    if (gLocalefacet_fb1dd8 == 0) {
        gLocalefacet_fb1dd8 = at<u32>(self, 0x10);
        g_env->atExit(reinterpret_cast<void *>((uintptr_t)0x005cfc80));
        at<u32>(self, 0x14) = 0;
    }
    g_env->locinfoDtor(info);
    if (at<u32>(self, 0x14) != 0) {
        Crt().free(at<void *>(self, 0x10));
        at<u32>(self, 0x14) = 0;
    }
    if (table) {
        at<u8 *>(self, 0x10) = table;
        at<u32>(self, 0x14) = static_cast<u8>(del);
    } else {
        at<u32>(self, 0x10) = gLocalefacet_fb1dd8;
    }
    return self;
}

void __fastcall NCtype_Dtor(u8 *self) {
    at<u32>(self, 0) = (u32)(uintptr_t)NCtype_vtable;
    if (at<u32>(self, 0x14)) Crt().free(at<void *>(self, 0x10));
    at<u32>(self, 0) = (u32)(uintptr_t)LocaleFacet_vtable;
}

u8 *__fastcall NCtype_DeletingDtor(u8 *self, void *, u32 flags) {
    NCtype_Dtor(self);
    if (flags & 1) GameHeap_Free(self);
    return self;
}

void __cdecl NCtype_FreeTable() { Crt().free(reinterpret_cast<void *>(gLocalefacet_fb1dd8)); }

s32 __fastcall NCtype_ToLower(u8 *self, void *, u32 c) { return g_env->tolower(c & 0xff, reinterpret_cast<const u32 *>(self + 8)); }

char *__fastcall NCtype_ToLowerRange(u8 *self, void *, char *first, char *last) {
    for (; first != last; ++first) *first = static_cast<char>(g_env->tolower(*first, reinterpret_cast<const u32 *>(self + 8)));
    return first;
}

s32 __fastcall NCtype_ToUpper(u8 *self, void *, u32 c) { return g_env->toupper(c & 0xff, reinterpret_cast<const u32 *>(self + 8)); }

char *__fastcall NCtype_ToUpperRange(u8 *self, void *, char *first, char *last) {
    for (; first != last; ++first) *first = static_cast<char>(g_env->toupper(*first, reinterpret_cast<const u32 *>(self + 8)));
    return first;
}

void __fastcall NIstream_DtorVb(u8 *self) {
    *reinterpret_cast<u32 *>(self + base_of(self)) = (u32)(uintptr_t)Class70994c_vtable;
    g_env->iosDtor(self + 8);
}

void __fastcall NOstream_DtorVb(u8 *self) {
    *reinterpret_cast<u32 *>(self + base_of(self)) = (u32)(uintptr_t)Class70995c_vtable;
    g_env->iosDtor(self + 4);
}

void __fastcall NIfstream_DtorVb(u8 *self) {
    g_env->ifstreamDtor(self + 0x5c);
    g_env->iosDtor(self + 0x5c);
}

void __fastcall NOfstream_DtorVb(u8 *self) {
    g_env->ofstreamDtor(self + 0x58);
    g_env->iosDtor(self + 0x58);
}

u8 *__fastcall NIos_DeletingDtor(u8 *self, void *, u32 flags) {
    g_env->iosDtor(self);
    if (flags & 1) GameHeap_Free(self);
    return self;
}

u8 *__fastcall NOstream_DeletingDtorOnIos(u8 *ios, void *, u32 flags) {
    u8 *const self = ios - 4;
    *reinterpret_cast<u32 *>(self + base_of(self)) = (u32)(uintptr_t)Class70995c_vtable;
    g_env->iosDtor(ios);
    if (flags & 1) GameHeap_Free(self);
    return self;
}

u8 *__fastcall NIstream_DeletingDtorOnIos(u8 *ios, void *, u32 flags) {
    u8 *const self = ios - 8;
    *reinterpret_cast<u32 *>(self + base_of(self)) = (u32)(uintptr_t)Class70994c_vtable;
    g_env->iosDtor(ios);
    if (flags & 1) GameHeap_Free(self);
    return self;
}

u8 *__fastcall NIfstream_DeletingDtorOnIos(u8 *ios, void *, u32 flags) {
    g_env->ifstreamDtor(ios);
    g_env->iosDtor(ios);
    u8 *const self = ios - 0x5c;
    if (flags & 1) GameHeap_Free(self);
    return self;
}

u8 *__fastcall NOfstream_DeletingDtorOnIos(u8 *ios, void *, u32 flags) {
    g_env->ofstreamDtor(ios);
    g_env->iosDtor(ios);
    u8 *const self = ios - 0x58;
    if (flags & 1) GameHeap_Free(self);
    return self;
}

u8 __fastcall NIos_Widen(u8 *ios, void *, u32 c) {
    u8 *held;
    u8 **const locale = Obj_ShareMember20(ios, nullptr, &held);
    use_ctype(locale);
    if (held) {
        u8 *const dead = Counted_Release(held);
        if (dead) vcall<u32>(dead, 0, 1u);
    }
    return static_cast<u8>(c);
}

u8 *__fastcall NOstream_Ctor(u8 *self, void *, u8 *buffer, u32 addstd, u32 init, u32 derived) {
    if (derived) {
        at<u32>(self, 0) = 0x00702980;
        at<u32>(self, 0x28) = 0;
        at<u32>(self, 4) = (u32)(uintptr_t)NIos_vtable;
    }
    *reinterpret_cast<u32 *>(self + base_of(self)) = (u32)(uintptr_t)Class70995c_vtable;
    if (static_cast<u8>(init)) init_ios(self + base_of(self), buffer, addstd);
    return self;
}

u8 *__fastcall NIstream_Ctor(u8 *self, void *, u8 *buffer, u32 addstd, u32 derived) {
    if (derived) {
        at<u32>(self, 0) = 0x00702988;
        at<u32>(self, 0x2c) = 0;
        at<u32>(self, 8) = (u32)(uintptr_t)NIos_vtable;
    }
    at<u32>(self, 4) = 0;
    *reinterpret_cast<u32 *>(self + base_of(self)) = (u32)(uintptr_t)Class70994c_vtable;
    init_ios(self + base_of(self), buffer, addstd);
    return self;
}

u8 __fastcall NOstream_Opfx(u8 *self) {
    u8 *const tied = at<u8 *>(ios_of(self), 0x2c);
    if (at<u32>(ios_of(self), 4) == 0 && tied) flush_stream(tied);
    return at<u32>(ios_of(self), 4) == 0;
}

void __fastcall NOstream_Sentry(u8 *sentry) {
    if (Fn_6a65e0()) return;
    u8 *const stream = at<u8 *>(sentry, 4);
    if (at<u8>(ios_of(stream), 0xc) & 2) flush_stream(stream);
}

u8 __fastcall NIstream_Ipfx(u8 *self, void *, u32 noskip) {
    if (at<u32>(ios_of(self), 4) == 0) {
        u8 *const tied = at<u8 *>(ios_of(self), 0x2c);
        if (tied) {
            u32 state = 0;
            if (!(at<u8>(ios_of(tied), 4) & 6) && vcall<s32>(buffer_of(tied), 0x2c) == -1) state = 4;
            BasicIosW_SetState(ios_of(tied), nullptr, state, 0);
        }
        if (!static_cast<u8>(noskip) && (at<u8>(ios_of(self), 0xc) & 1)) {
            u8 *held;
            u8 **const locale = Obj_ShareMember20(ios_of(self), nullptr, &held);
            u8 *const facet = use_ctype(locale);
            if (held) {
                u8 *const dead = Counted_Release(held);
                if (dead) vcall<u32>(dead, 0, 1u);
            }
            u32 unit = Buf_Peek(buffer_of(self));
            while (unit != 0xffffffffu && (at<const u8 *>(facet, 0x10)[static_cast<u8>(unit) * 2] & 0x48)) {
                u8 *const buffer = buffer_of(self);
                unit = take_unit(buffer) == 0xffffffffu ? 0xffffffffu : Buf_Peek(buffer);
            }
        }
        if (at<u32>(ios_of(self), 4) == 0) return 1;
    }
    BasicIosW_SetState(ios_of(self), nullptr, 2, 0);
    return 0;
}

u8 *__fastcall NIstream_Getline(u8 *self, void *, char *text, u32 count, u32 delimiter) {
    u32 state = 0;
    s32 left = static_cast<s32>(count);
    at<u32>(self, 4) = 0;
    if (NIstream_Ipfx(self, nullptr, 1) && left > 0) {
        const u32 wanted = delimiter & 0xff;
        u32 unit = Buf_Peek(buffer_of(self));
        for (;;) {
            if (unit == 0xffffffffu) {
                state |= 1;
                break;
            }
            if (unit == wanted) {
                ++at<u32>(self, 4);
                u8 *const buffer = buffer_of(self);
                if (take_unit(buffer) != 0xffffffffu) Buf_Peek(buffer);
                break;
            }
            if (--left <= 0) {
                state |= 2;
                break;
            }
            ++at<u32>(self, 4);
            *text++ = static_cast<char>(unit);
            u8 *const buffer = buffer_of(self);
            unit = take_unit(buffer) == 0xffffffffu ? 0xffffffffu : Buf_Peek(buffer);
        }
    }
    *text = 0;
    if (at<u32>(self, 4) == 0) state |= 2;
    BasicIosW_SetState(ios_of(self), nullptr, state, 0);
    return self;
}

u8 *__fastcall NIstream_Read(u8 *self, void *, char *text, u32 count) {
    u32 state = 0;
    at<u32>(self, 4) = 0;
    if (NIstream_Ipfx(self, nullptr, 1)) {
        const u32 got = vcall<u32>(buffer_of(self), 0x18, text, count);
        at<u32>(self, 4) += got;
        if (got != count) state = 3;
    }
    BasicIosW_SetState(ios_of(self), nullptr, state, 0);
    return self;
}

u8 *__cdecl NOstream_PutText(u8 *stream, const char *text) {
    u32 state = 0;
    const u32 length = static_cast<u32>(std::strlen(text));
    const s32 width = at<s32>(ios_of(stream), 0x14);
    u32 pad = width > 0 && static_cast<u32>(width) > length ? static_cast<u32>(width) - length : 0;
    if (!NOstream_Opfx(stream)) {
        state = 4;
    } else {
        if ((at<u32>(ios_of(stream), 0xc) & 0x1c0) != 0x40) {
            for (; pad; --pad) {
                if (put_fill(stream) == 0xffffffffu) {
                    state = 4;
                    break;
                }
            }
        }
        if (!state && vcall<u32>(buffer_of(stream), 0x1c, text, length) != length) state = 4;
        if (!state) {
            for (; pad; --pad) {
                if (put_fill(stream) == 0xffffffffu) {
                    state = 4;
                    break;
                }
            }
        }
        at<u32>(ios_of(stream), 0x14) = 0;
    }
    BasicIosW_SetState(ios_of(stream), nullptr, state, 0);
    if (!Fn_6a65e0() && (at<u8>(ios_of(stream), 0xc) & 2)) flush_stream(stream);
    return stream;
}
}
