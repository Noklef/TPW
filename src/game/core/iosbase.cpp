#include "iosbase.h"

#include <cstdint>
#include <cstring>

#include "crt.h"
#include "stdstring.h"
#include "wfilebuf.h"

namespace {

template <typename F> F game(u32 address) { return reinterpret_cast<F>(address); }

const IosBaseEnv kGameEnv = {
    game<void *(__fastcall *)(void *)>(0x00691de0),
    game<void(__fastcall *)(void *)>(0x00691ea0),
    game<void(__cdecl *)(void *)>(0x0067acb0),
    game<void(__stdcall *)(void *, const void *)>(0x0067d3a0),
    game<void *(__cdecl *)(const char *, const char *)>(0x0067b8a0),
    game<s32(__cdecl *)(void *, s32, s32)>(0x0067c100),
    game<s32(__cdecl *)(void *)>(0x0067b620),
    game<void *(__cdecl *)(u32)>(0x0067acc0),
    game<void *(__fastcall *)(void *, void *, u32)>(0x00699f70),
    game<s32(__cdecl *)(void(__cdecl *)())>(0x0067a7d0),
};
const IosBaseEnv *g_env = &kGameEnv;

template <typename T> T &at(void *base, u32 offset) { return *reinterpret_cast<T *>(static_cast<u8 *>(base) + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

struct ThrownError {   // exception's vtable, text and owned flag, then the message
    u32 vtable;
    const char *text;
    u32 owned;
    StdString what;
};

// The standard errors' common body: the text in a string, an exception of an empty text (0x786de4) with a copy of it, the vtable, the throw. (Past it only when
// _CxxThrowException returns, which it does not: the string let go as the original's code there does.)
void throw_error(const char *text, u32 vtable, u32 info) {
    StdString s;
    const u32 n = (u32)strlen(text);
    s.ptr = nullptr;
    s.len = 0;
    s.res = 0;
    if (StdString_Grow(&s, nullptr, n, 1)) {
        memcpy(s.ptr, text, n);
        s.len = n;
        s.ptr[n] = 0;
    }
    ThrownError error;
    const char *const empty = reinterpret_cast<const char *>((uintptr_t)0x00786de4);
    CRT_exception_Ctor(reinterpret_cast<u32 *>(&error), nullptr, &empty);
    StdString_Tidy(&error.what, nullptr, 0);
    StdString_AssignSub(&error.what, nullptr, &s, 0, global<u32>(0x006fe5ac));
    error.vtable = vtable;
    g_env->cxxThrow(&error, reinterpret_cast<const void *>((uintptr_t)info));
    char *const p = s.ptr;
    if (!p) return;
    const u8 refs = (u8)p[-1];
    if (refs == 0 || refs == 0xff) {
        g_env->memFree(p - 1);
        return;
    }
    p[-1] = (char)(refs - 1);
}

void free_list(u8 *head) {
    while (head) {
        u8 *const next = *reinterpret_cast<u8 **>(head);
        g_env->memFree(head);
        head = next;
    }
}

}  // namespace

void IosBase_SetEnv(const IosBaseEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl Std_Xlen() { throw_error(reinterpret_cast<const char *>((uintptr_t)0x00709890), 0x00709884, 0x00720468); }

void __cdecl Std_Xran() { throw_error(reinterpret_cast<const char *>((uintptr_t)0x007098c0), 0x007098b4, 0x0071f298); }

void __fastcall IosBase_Clear(u8 *self, void *, u32 state, u32 reraise) {
    state &= 7;
    at<u32>(self, 4) = state;
    const u32 cause = at<u32>(self, 8) & state;
    if (!cause) return;
    if ((u8)reraise) {
        g_env->cxxThrow(nullptr, nullptr);
        return;
    }
    const u32 text = cause & 4 ? 0x00709a2c : cause & 2 ? 0x00709a18 : 0x00709a08;
    throw_error(reinterpret_cast<const char *>((uintptr_t)text), 0x007099fc, 0x00720720);
}

void __fastcall IosBase_Callfns(u8 *self, void *, u32 event) {
    for (u8 *node = at<u8 *>(self, 0x1c); node; node = at<u8 *>(node, 0))
        at<void(__cdecl *)(u32, u8 *, u32)>(node, 8)(event, self, at<u32>(node, 4));
}

void __fastcall IosBase_Addstd(u8 *self) {
    u32 lock;
    g_env->lockit(&lock);
    u32 &index = at<u32>(self, 0x24);
    if (index < 8) {
        u32 i = index;
        do {
            const u32 slot = *reinterpret_cast<u32 *>((uintptr_t)(0x00fb1d60 + i * 4));
            if (!slot || slot == (u32)(uintptr_t)self) break;
            index = ++i;
        } while (i < 8);
    }
    *reinterpret_cast<u8 **>((uintptr_t)(0x00fb1d60 + index * 4)) = self;
    ++*reinterpret_cast<u8 *>((uintptr_t)(0x00fb1d88 + index));
    g_env->unlockit(&lock);
}

void __fastcall IosBase_Init(u8 *self) {
    LocaleRef_Ctor(reinterpret_cast<u32 *>(self + 0x20));
    at<u32>(self, 8) = 0;
    at<u32>(self, 0xc) = 0x201;
    at<u32>(self, 0x10) = 6;
    at<u32>(self, 0x14) = 0;
    at<u32>(self, 0x18) = 0;
    at<u32>(self, 0x1c) = 0;
    IosBase_Clear(self, nullptr, 0, 0);
}

void __fastcall IosBase_Tidy(u8 *self) {
    IosBase_Callfns(self, nullptr, 0);
    free_list(at<u8 *>(self, 0x18));
    u8 *const calls = at<u8 *>(self, 0x1c);
    at<u32>(self, 0x18) = 0;
    free_list(calls);
    at<u32>(self, 0x1c) = 0;
}

void __fastcall IosBase_Dtor(u8 *self) {
    at<u32>(self, 0) = 0x00709964;
    bool tidy = true;
    if (at<u32>(self, 0x24) != 0) {
        s8 &opens = *reinterpret_cast<s8 *>((uintptr_t)(0x00fb1d88 + at<u32>(self, 0x24)));
        --opens;
        tidy = *reinterpret_cast<s8 *>((uintptr_t)(0x00fb1d88 + at<u32>(self, 0x24))) <= 0;
    }
    if (tidy) IosBase_Tidy(self);
    u8 *const locale = at<u8 *>(self, 0x20);
    if (!locale) return;
    u32 lock;
    g_env->lockit(&lock);
    u32 &uses = at<u32>(locale, 4);
    if (uses != 0 && uses < 0xffffffffu) --uses;
    u8 *const last = uses == 0 ? locale : nullptr;
    g_env->unlockit(&lock);
    if (last) (*at<void(__fastcall **)(u8 *, void *, u32)>(last, 0))(last, nullptr, 1);
}

void *__cdecl Std_FiopenTable(const char *name, u32 mode) {
    const u32 ate = mode & 4;
    const u32 wanted = mode & ~4u;
    const u32 *const modes = reinterpret_cast<const u32 *>((uintptr_t)0x00709a50);
    u32 i = 0;
    while (modes[i] && modes[i] != wanted) ++i;
    if (!modes[i]) return nullptr;
    void *const file = g_env->fopen(name, reinterpret_cast<const char *const *>((uintptr_t)0x00782590)[i]);
    if (!file) return nullptr;
    if (ate && g_env->fseek(file, 0, 2)) {
        g_env->fclose(file);
        return nullptr;
    }
    return file;
}

void *__cdecl Std_Fiopen(const char *name, u32 mode) { return Std_FiopenTable(name, mode); }

u8 *__fastcall IosBase_DeletingDtor(u8 *self, void *, u32 flags) {
    IosBase_Dtor(self);
    if (flags & 1) g_env->memFree(self);
    return self;
}

void __fastcall BasicIosW_Dtor(u8 *self) {
    at<u32>(self, 0) = 0x007099e4;
    IosBase_Dtor(self);
}

void __fastcall BasicIosW_SetState(u8 *self, void *, u32 state, u32 reraise) {
    if (!state) return;
    u32 now = at<u32>(self, 4) | state;
    if (!at<u32>(self, 0x28)) now |= 4;
    IosBase_Clear(self, nullptr, now, reraise);
}

void __fastcall BasicIosW_Clear(u8 *self, void *, u32 state, u32 reraise) { IosBase_Clear(self, nullptr, at<u32>(self, 0x28) ? state : state | 4, reraise); }

namespace {

// basic_ostream's and basic_istream's common end: the virtual base's stream buffer, fill and init.
void init_stream_ios(u8 *self, void *buffer, u32 isstd) {
    u8 *const ios = self + at<u32>(at<u8 *>(self, 0), 4);
    at<void *>(ios, 0x28) = buffer;
    at<u32>(ios, 0x2c) = 0;
    at<u16>(ios, 0x30) = (u16)CRT_btowc(0x20);
    IosBase_Init(ios);
    if (!at<u32>(ios, 0x28)) IosBase_Clear(ios, nullptr, at<u32>(ios, 4) | 4, 0);
    if ((u8)isstd) IosBase_Addstd(ios);
}

}  // namespace

u8 *__fastcall BasicOstreamW_Ctor(u8 *self, void *, void *buffer, u32 isstd, u32 init, u32 mostDerived) {
    if (mostDerived) {
        at<u32>(self, 0) = 0x00700648;
        at<u32>(self, 0x28) = 0;
        at<u32>(self, 4) = 0x007099e4;
    }
    at<u32>(self + at<u32>(at<u8 *>(self, 0), 4), 0) = 0x007099ec;
    if ((u8)init) init_stream_ios(self, buffer, isstd);
    return self;
}

u8 *__fastcall BasicIstreamW_Ctor(u8 *self, void *, void *buffer, u32 isstd, u32 mostDerived) {
    if (mostDerived) {
        at<u32>(self, 0) = 0x00700650;
        at<u32>(self, 0x2c) = 0;
        at<u32>(self, 8) = 0x007099e4;
    }
    at<u32>(self, 4) = 0;
    at<u32>(self + at<u32>(at<u8 *>(self, 0), 4), 0) = 0x007099dc;
    init_stream_ios(self, buffer, isstd);
    return self;
}

u32 *__fastcall BadCast_Ctor(u32 *self, void *, const char *text) {
    CRT_exception_Ctor(self, nullptr, &text);
    self[0] = 0x00709b34;
    return self;
}

u32 *__fastcall BadCast_Copy(u32 *self, void *, const u32 *from) {
    CRT_exception_Copy(self, nullptr, from);
    self[0] = 0x00709b34;
    return self;
}

void __fastcall BadCast_Dtor(u32 *self) {
    self[0] = 0x00709b34;
    CRT_exception_Dtor(self);
}

u32 __fastcall CtypeW_Widen(void *, void *, u32 c) { return CRT_btowc((u32)(s32)(s8)c); }

const char *__fastcall CtypeW_WidenRange(void *, void *, const char *first, const char *last, u16 *to) {
    for (; first != last; ++first, ++to) *to = (u16)CRT_btowc((u32)(s32)(s8)*first);
    return first;
}

void __fastcall CtypeW_Dtor(u8 *self) {
    at<u32>(self, 0) = 0x00709cac;
    if (at<u32>(self, 0x14)) Crt().free(at<void *>(self, 0x10));
    at<u32>(self, 0) = 0x00709b44;
}

void *__cdecl Std_LocaleInit() {
    u32 lock;
    g_env->lockit(&lock);
    u8 *&classic = global<u8 *>(0x00fb1dd0);
    if (!classic) {
        void *const block = g_env->memAlloc(0x28);
        classic = block ? static_cast<u8 *>(g_env->locimpCtor(block, nullptr, 0)) : nullptr;
        g_env->atexit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x00699ee0));
        at<u32>(classic, 0x10) = 0x3f;
        StdString &name = at<StdString>(classic, 0x18);
        const char *const text = reinterpret_cast<const char *>((uintptr_t)0x0075c74c);
        const u32 n = (u32)strlen(text);
        if (StdString_Grow(&name, nullptr, n, 1)) {
            memcpy(name.ptr, text, n);
            name.len = n;
            name.ptr[n] = 0;
        }
        u8 *const held = classic;
        global<u8 *>(0x00fb1dcc) = held;
        u32 inner;
        g_env->lockit(&inner);
        u32 &uses = at<u32>(held, 4);
        if (uses < 0xffffffffu) ++uses;
        g_env->unlockit(&inner);
        global<u8 *>(0x00fb1dc8) = global<u8 *>(0x00fb1dcc);
    }
    u8 *const answer = classic;
    g_env->unlockit(&lock);
    return answer;
}

}  // extern "C"
