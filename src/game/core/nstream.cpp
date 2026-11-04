#include "nstream.h"

#include <cstdint>

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

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const NStreamEnv kGameEnv = {reinterpret_cast<void(__stdcall *)(void *, const void *)>((uintptr_t)0x0067d3a0),
                             reinterpret_cast<void *(__fastcall *)(void *, void *, const char *)>((uintptr_t)0x00698fc0),
                             reinterpret_cast<void *(__fastcall *)(void *)>((uintptr_t)0x00699100),
                             reinterpret_cast<const u32 *(__cdecl *)(u32 *)>((uintptr_t)0x00699bd0),
                             &CRT_atexit,
                             &IosBase_Init,
                             &IosBase_Addstd};
const NStreamEnv *g_env = &kGameEnv;

u32 base_of(const u8 *stream) { return at<u32>(at<u8 *>(stream, 0), 4); }
u8 *ios_of(u8 *stream) { return stream + base_of(stream); }
u8 *buffer_of(u8 *stream) { return at<u8 *>(ios_of(stream), 0x28); }

u8 *use_ctype(u8 **locale) {
    const u32 id = Id_Get(reinterpret_cast<u32 *>((uintptr_t)0x00fb1da4));
    u8 *const facet = Locale_Getfacet(locale, nullptr, id, 1);
    if (facet) return facet;
    if (Locale_Iscloc(locale)) {
        u8 *&kept = global<u8 *>(0x00f827e0);
        if (kept) return kept;
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x18));
        kept = Globalf827dc_Set(block ? NCtype_Ctor(block, nullptr, nullptr, 0, 0) : nullptr);
        return kept;
    }
    u32 bad_cast[3];
    BadCast_Ctor(bad_cast, nullptr, reinterpret_cast<const char *>((uintptr_t)0x0075c734));
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

}  // namespace

void NStream_SetEnv(const NStreamEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u8 *__fastcall NCtype_Ctor(u8 *self, void *, u8 *table, u32 del, u32 refs) {
    at<u32>(self, 4) = refs;
    at<u32>(self, 0) = 0x00709b1c;
    u8 info[0x48];
    g_env->locinfoCtor(info, nullptr, reinterpret_cast<const char *>((uintptr_t)0x0075c74c));
    u32 out[4];
    const u32 *const ctype = g_env->getCtype(out);
    for (u32 i = 0; i < 4; ++i) at<u32>(self, 8 + 4 * i) = ctype[i];
    if (global<u32>(0x00fb1dd8) == 0) {
        global<u32>(0x00fb1dd8) = at<u32>(self, 0x10);
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
        at<u32>(self, 0x10) = global<u32>(0x00fb1dd8);
    }
    return self;
}

void __fastcall NCtype_Dtor(u8 *self) {
    at<u32>(self, 0) = 0x00709b1c;
    if (at<u32>(self, 0x14)) Crt().free(at<void *>(self, 0x10));
    at<u32>(self, 0) = 0x00709b44;
}

u8 *__fastcall NCtype_DeletingDtor(u8 *self, void *, u32 flags) {
    NCtype_Dtor(self);
    if (flags & 1) GameHeap_Free(self);
    return self;
}

void __cdecl NCtype_FreeTable() { Crt().free(global<void *>(0x00fb1dd8)); }

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
        at<u32>(self, 4) = 0x00709954;
    }
    *reinterpret_cast<u32 *>(self + base_of(self)) = 0x0070995c;
    if (static_cast<u8>(init)) init_ios(self + base_of(self), buffer, addstd);
    return self;
}

u8 *__fastcall NIstream_Ctor(u8 *self, void *, u8 *buffer, u32 addstd, u32 derived) {
    if (derived) {
        at<u32>(self, 0) = 0x00702988;
        at<u32>(self, 0x2c) = 0;
        at<u32>(self, 8) = 0x00709954;
    }
    at<u32>(self, 4) = 0;
    *reinterpret_cast<u32 *>(self + base_of(self)) = 0x0070994c;
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
}
