#include "wstream.h"

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

extern "C" u32 __cdecl CRT_wcslen(const u16 *text);   // (online/chatlists)

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const WStreamEnv kGameEnv = {reinterpret_cast<void(__stdcall *)(void *, const void *)>((uintptr_t)0x0067d3a0),
                             reinterpret_cast<void *(__fastcall *)(void *, void *, const char *)>((uintptr_t)0x00698fc0),
                             reinterpret_cast<void *(__fastcall *)(void *)>((uintptr_t)0x00699100),
                             reinterpret_cast<const u32 *(__cdecl *)(u32 *)>((uintptr_t)0x00699bd0),
                             reinterpret_cast<u32(__fastcall *)(u8 *)>((uintptr_t)0x005d1190)};
const WStreamEnv *g_env = &kGameEnv;

u32 base_of(const u8 *stream) { return at<u32>(at<u8 *>(stream, 0), 4); }
u8 *ios_of(u8 *stream) { return stream + base_of(stream); }
u8 *buffer_of(u8 *stream) { return at<u8 *>(ios_of(stream), 0x28); }

// The facet (see WCtype_UseFacet).
u8 *use_ctype(u8 **locale, u8 transparent) {
    const u32 id = Id_Get(reinterpret_cast<u32 *>((uintptr_t)0x00fb1df8));
    u8 *const facet = Locale_Getfacet(locale, nullptr, id, 1);
    if (facet) return facet;
    if (transparent && Locale_Iscloc(locale)) {
        u8 *&kept = global<u8 *>(0x007ced3c);
        if (kept) return kept;
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x18));
        if (block) WCtype_Ctor(block, nullptr, 0);
        Global7ced44_Set(block);
        kept = block;
        return block;
    }
    u32 bad_cast[3];
    BadCast_Ctor(bad_cast, nullptr, reinterpret_cast<const char *>((uintptr_t)0x0075c734));
    g_env->cxxThrow(bad_cast, reinterpret_cast<const void *>((uintptr_t)0x007193a8));
    return nullptr;
}

// The share of the locale given up: at no uses left, its slot 0 (delete).
void release_locale(u8 *held) {
    if (!held) return;
    u8 *const dead = Counted_Release(held);
    if (dead) vcall<u32>(dead, 0, 1u);
}

// sbumpc: the unit under the position, which moves on, or slot 0x14.
u32 take_unit(u8 *buffer) {
    const u32 position = *at<u32 *>(buffer, 0x1c);
    if (position) {
        u32 *const left = at<u32 *>(buffer, 0x2c);
        const u32 count = *left;
        if (position < position + count * 2) {
            *left = count - 1;
            u32 *const cursor = at<u32 *>(buffer, 0x1c);
            const u32 now = *cursor;
            *cursor = now + 2;
            return *reinterpret_cast<const u16 *>((uintptr_t)now);
        }
    }
    return vcall<u32>(buffer, 0x14);
}

// The next unit after the one taken, 0xffff at the end. (The high half is the peek's, for the facet's call.)
u32 next_unit(u8 *stream) {
    u8 *const buffer = buffer_of(stream);
    return static_cast<u16>(take_unit(buffer)) == 0xffff ? 0xffffu : WBuf_Peek(buffer);
}

// sputc of the fill; only the low half of the answer counts. (The high half of the unit passed to slot 4 is the base offset's, sic.)
u16 put_fill(u8 *stream) {
    const u32 base = base_of(stream);
    u8 *const ios = stream + base;
    u8 *const buffer = at<u8 *>(ios, 0x28);
    const u32 unit = (base & 0xffff0000u) | at<u16>(ios, 0x30);
    const u32 position = *at<u32 *>(buffer, 0x20);
    if (position) {
        u32 *const left = at<u32 *>(buffer, 0x30);
        const u32 count = *left;
        if (position < position + count * 2) {
            *left = count - 1;
            u32 *const cursor = at<u32 *>(buffer, 0x20);
            const u32 now = *cursor;
            *cursor = now + 2;
            *reinterpret_cast<u16 *>((uintptr_t)now) = static_cast<u16>(unit);
            return static_cast<u16>(unit);
        }
    }
    return static_cast<u16>(vcall<u32>(buffer, 4, unit));
}

}  // namespace

void WStream_SetEnv(const WStreamEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u8 *__fastcall WCtype_Ctor(u8 *self, void *, u32 refs) {
    at<u32>(self, 4) = refs;
    at<u32>(self, 0) = 0x00709cac;
    u8 info[0x48];
    g_env->locinfoCtor(info, nullptr, reinterpret_cast<const char *>((uintptr_t)0x0075c74c));
    u32 out[4];
    const u32 *const ctype = g_env->getCtype(out);
    for (u32 i = 0; i < 4; ++i) at<u32>(self, 8 + 4 * i) = ctype[i];
    g_env->locinfoDtor(info);
    return self;
}

u8 *__cdecl WCtype_UseFacet(u8 **locale, u32, u32 transparent) { return use_ctype(locale, static_cast<u8>(transparent)); }

u32 __fastcall WIos_Widen(u8 *self, void *, u32 c) {
    u8 *held;
    u8 **const locale = Obj_ShareMember20(self, nullptr, &held);
    u8 *const facet = use_ctype(locale, 1);
    release_locale(held);
    return vcall<u32>(facet, 0x28, c);
}

u8 __fastcall WIstream_Ipfx(u8 *self, void *, u32 noskip) {
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
            u8 *const facet = use_ctype(locale, 1);
            release_locale(held);
            u32 unit = WBuf_Peek(buffer_of(self));
            while (static_cast<u16>(unit) != 0xffff && static_cast<u8>(vcall<u32>(facet, 8, 0x48u, unit))) unit = next_unit(self);
        }
        if (at<u32>(ios_of(self), 4) == 0) return 1;
    }
    BasicIosW_SetState(ios_of(self), nullptr, 2, 0);
    return 0;
}

u8 *__fastcall WIstream_Getline(u8 *self, void *, u16 *text, s32 count, u32 delimiter) {
    u32 state = 0;
    at<u32>(self, 4) = 0;
    if (WIstream_Ipfx(self, nullptr, 1) && count > 0) {
        u32 unit = WBuf_Peek(buffer_of(self));
        for (;;) {
            if (static_cast<u16>(unit) == 0xffff) {
                state |= 1;
                break;
            }
            if (static_cast<u16>(unit) == static_cast<u16>(delimiter)) {
                ++at<u32>(self, 4);
                u8 *const buffer = buffer_of(self);
                if (static_cast<u16>(take_unit(buffer)) != 0xffff) WBuf_Peek(buffer);
                break;
            }
            if (--count <= 0) {
                state |= 2;
                break;
            }
            ++at<u32>(self, 4);
            *text++ = static_cast<u16>(unit);
            unit = next_unit(self);
        }
    }
    *text = 0;
    if (at<u32>(self, 4) == 0) state |= 2;
    BasicIosW_SetState(ios_of(self), nullptr, state, 0);
    return self;
}

u8 *__cdecl WOstream_PutText(u8 *stream, const u16 *text) {
    u32 state = 0;
    const u32 length = CRT_wcslen(text);
    const s32 width = at<s32>(ios_of(stream), 0x14);
    u32 pad = width > 0 && static_cast<u32>(width) > length ? static_cast<u32>(width) - length : 0;
    if (!static_cast<u8>(g_env->ostreamPrefix(stream))) {
        state = 4;
    } else {
        if ((at<u32>(ios_of(stream), 0xc) & 0x1c0) != 0x40) {
            for (; pad; --pad) {
                if (put_fill(stream) == 0xffff) {
                    state = 4;
                    break;
                }
            }
        }
        if (!state && vcall<u32>(buffer_of(stream), 0x1c, text, length) != length) state = 4;
        if (!state) {
            for (; pad; --pad) {
                if (put_fill(stream) == 0xffff) {
                    state = 4;
                    break;
                }
            }
        }
        at<u32>(ios_of(stream), 0x14) = 0;
    }
    BasicIosW_SetState(ios_of(stream), nullptr, state, 0);
    if (!Fn_6a65e0() && (at<u8>(ios_of(stream), 0xc) & 2)) {
        u32 flush = 0;
        if (!(at<u8>(ios_of(stream), 4) & 6) && vcall<s32>(buffer_of(stream), 0x2c) == -1) flush = 4;
        BasicIosW_SetState(ios_of(stream), nullptr, flush, 0);
    }
    return stream;
}

}  // extern "C"
