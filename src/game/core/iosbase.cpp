#include "iosbase.h"

#include <cstdint>
#include <cstring>

#include "crt.h"
#include "initializers.h"
#include "narrowfilebuf.h"
#include "stdstring.h"
#include "vcall.h"
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
    game<void(__cdecl *)(void *, u32, void *, void *)>(0x00694120),
    game<void *(__fastcall *)(void *, void *, u32, u32, u32)>(0x005cfb60),
    game<void(__cdecl *)(void *, u32, void *, void *)>(0x006a67f0),
    game<void(__cdecl *)(void *, u32, void *, void *)>(0x0069a7a0),
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
// (A stream's first word is its vbtable, whose +4 is the offset of the virtual base's word.)
void set_stream_vtable(u8 *stream, u32 vtable) { at<u32>(stream + at<s32>(at<u8 *>(stream, 0), 4), 0) = vtable; }

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

const IosBaseEnv &IosBase_Env() { return *g_env; }

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

static u32 *error_copy(u32 *self, const u32 *from, u32 vtable) {
    CRT_exception_Copy(self, nullptr, from);
    StdString_CopyCtor(reinterpret_cast<StdString *>(self + 3), nullptr, reinterpret_cast<const StdString *>(from + 3));
    self[0] = vtable;
    return self;
}

static void error_raise(const u32 *from, u32 vtable, u32 info) {
    u32 copy[7];
    error_copy(copy, from, vtable);
    g_env->cxxThrow(copy, reinterpret_cast<const void *>((uintptr_t)info));
}

u32 *__fastcall LengthError_Copy(u32 *self, void *, const u32 *from) { return error_copy(self, from, 0x00709884); }
u32 *__fastcall Failure_Copy(u32 *self, void *, const u32 *from) { return error_copy(self, from, 0x007099fc); }
u32 *__fastcall Error709a44_Copy(u32 *self, void *, const u32 *from) { return error_copy(self, from, 0x00709a44); }

void __fastcall LengthError_Raise(const u32 *from) { error_raise(from, 0x00709884, 0x00720468); }
void __fastcall Failure_Raise(const u32 *from) { error_raise(from, 0x007099fc, 0x00720720); }
void __fastcall Error709a44_Raise(const u32 *from) { error_raise(from, 0x00709a44, 0x00720798); }

void __fastcall Error709a44_Dtor(u32 *self) {
    self[0] = 0x00709a44;
    StdString_Tidy(reinterpret_cast<StdString *>(self + 3), nullptr, 1);
    CRT_exception_Dtor(self);
}

u32 *__fastcall Error709a44_DeletingDtor(u32 *self, void *, u32 flags) {
    Error709a44_Dtor(self);
    if (flags & 1) g_env->memFree(self);
    return self;
}

static u8 *streambuf_ctor(u8 *self, u32 vtable) {
    LocaleRef_Ctor(reinterpret_cast<u32 *>(self + 0x34));
    at<u32>(self, 0) = vtable;
    at<u8 *>(self, 0xc) = self + 4;
    at<u8 *>(self, 0x10) = self + 8;
    at<u8 *>(self, 0x1c) = self + 0x14;
    at<u8 *>(self, 0x20) = self + 0x18;
    at<u8 *>(self, 0x2c) = self + 0x24;
    at<u8 *>(self, 0x30) = self + 0x28;
    at<u32>(self, 8) = 0;
    at<u32>(self, 0x18) = 0;
    at<u32>(self, 0x28) = 0;
    at<u32>(self, 4) = 0;
    at<u32>(self, 0x14) = 0;
    at<u32>(self, 0x24) = 0;
    return self;
}

u8 *__fastcall Streambuf_Ctor(u8 *self) { return streambuf_ctor(self, 0x00709914); }
u8 *__fastcall StreambufW_Ctor(u8 *self) { return streambuf_ctor(self, 0x007099a4); }

static void ios_init_rest(u8 *self, u32 isstd) {
    IosBase_Init(self);
    if (!at<u32>(self, 0x28)) IosBase_Clear(self, nullptr, at<u32>(self, 4) | 4, 0);
    if (static_cast<u8>(isstd)) IosBase_Addstd(self);
}

void __fastcall Ios_Init(u8 *self, void *, void *buffer, u32 isstd) {
    at<void *>(self, 0x28) = buffer;
    at<u32>(self, 0x2c) = 0;
    at<u8>(self, 0x30) = 0x20;
    ios_init_rest(self, isstd);
}

void __fastcall IosW_Init(u8 *self, void *, void *buffer, u32 isstd) {
    at<void *>(self, 0x28) = buffer;
    at<u32>(self, 0x2c) = 0;
    at<u16>(self, 0x30) = static_cast<u16>(CRT_btowc(0x20));
    ios_init_rest(self, isstd);
}

// (vbtable, the slot of the zeroed field, the slot of the buffer's vtable, its value, the ios's vtable)
static u8 *stream_uninit(u8 *self, u32 derived, u32 vbtable, u32 zero_at, u32 slot_at, u32 slot, u32 ios_vtable) {
    if (derived) {
        at<u32>(self, 0) = vbtable;
        at<u32>(self, zero_at) = 0;
        at<u32>(self, slot_at) = slot;
    }
    set_stream_vtable(self, ios_vtable);
    IosBase_Addstd(self + at<u32>(at<u8 *>(self, 0), 4));
    return self;
}

u8 *__fastcall Istream_CtorUninit(u8 *self, void *, u32, u32 derived) { return stream_uninit(self, derived, 0x00702988, 0x2c, 8, 0x00709954, 0x0070994c); }
u8 *__fastcall Ostream_CtorUninit(u8 *self, void *, u32, u32 derived) { return stream_uninit(self, derived, 0x00702980, 0x28, 4, 0x00709954, 0x0070995c); }
u8 *__fastcall IstreamW_CtorUninit(u8 *self, void *, u32, u32 derived) { return stream_uninit(self, derived, 0x00700650, 0x2c, 8, 0x007099e4, 0x007099dc); }
u8 *__fastcall OstreamW_CtorUninit(u8 *self, void *, u32, u32 derived) { return stream_uninit(self, derived, 0x00700648, 0x28, 4, 0x007099e4, 0x007099ec); }

static bool init_first(u32 counter) {
    u32 lock;
    g_env->lockit(&lock);
    s32 &count = global<s32>(counter);
    const bool first = count < 0;
    count = first ? 1 : count + 1;
    g_env->unlockit(&lock);
    return first;
}

static u8 *std_address(u32 address) { return reinterpret_cast<u8 *>((uintptr_t)address); }
static u8 *std_ios(u8 *stream) { return stream + at<u32>(at<u8 *>(stream, 0), 4); }

static void std_tie(u8 *stream, u32 to) { at<u32>(std_ios(stream), 0x2c) = to; }

static u8 *std_stream(u32 address, u32 vbtable) {
    u8 *const stream = std_address(address);
    at<u32>(stream, 0) = vbtable;
    return stream;
}

static void narrow_filebuf(u32 address, u32 file) {
    u8 *const buf = std_address(address);
    Streambuf_Ctor(buf);
    LocaleRef_Ctor(reinterpret_cast<u32 *>(buf + 0x4c));
    at<u32>(buf, 0) = 0x007098dc;
    NFileBuf_Init(buf, nullptr, std_address(file), 0);
}

static void wide_filebuf(u32 address, u32 file) {
    u8 *const buf = std_address(address);
    StreambufW_Ctor(buf);
    LocaleRef_Ctor(reinterpret_cast<u32 *>(buf + 0x4c));
    at<u32>(buf, 0) = 0x0070996c;
    WFileBuf_Init(buf, nullptr, file, 0);
}

static void narrow_finish(u8 *stream, u32 vtable, u32 buffer) {
    set_stream_vtable(stream, vtable);
    Ios_Init(std_ios(stream), nullptr, std_address(buffer), 1);
}

static void wide_finish(u8 *stream, u32 vtable, u32 buffer) {
    set_stream_vtable(stream, vtable);
    IosW_Init(std_ios(stream), nullptr, std_address(buffer), 1);
}

u8 *__fastcall Ios_InitCtor(u8 *self) {
    if (!init_first(0x007822b0)) return self;
    narrow_filebuf(0x00fb1ad0, 0x0077e800);
    narrow_filebuf(0x00fb19a8, 0x0077e820);
    u8 *const cin = std_stream(0x00fb1a90, 0x00702988);
    Init_5cfd80(cin + 8);
    at<u32>(cin, 4) = 0;
    narrow_finish(cin, 0x0070994c, 0x00fb1ad0);
    u8 *const cout = std_stream(0x00fb1a00, 0x00702980);
    Init_5cfd80(cout + 4);
    narrow_finish(cout, 0x0070995c, 0x00fb19a8);
    std_tie(cin, 0x00fb1a00);
    narrow_filebuf(0x00fb1a38, 0x0077e840);
    u8 *const cerr = std_stream(0x00fb1b28, 0x00702980);
    Init_4ef830(cerr + 4);
    at<u32>(cerr, 4) = 0x00709954;
    narrow_finish(cerr, 0x0070995c, 0x00fb1a38);
    std_tie(cerr, 0x00fb1a00);
    at<u32>(std_ios(cerr), 0xc) |= 2;
    u8 *const clog = std_stream(0x00fb1970, 0x00702980);
    Init_4ef830(clog + 4);
    at<u32>(clog, 4) = 0x00709954;
    narrow_finish(clog, 0x0070995c, 0x00fb1a38);
    std_tie(clog, 0x00fb1a00);
    return self;
}

u8 *__fastcall Ios_WinitCtor(u8 *self) {
    if (!init_first(0x0078240c)) return self;
    wide_filebuf(0x00fb1c70, 0x0077e800);
    wide_filebuf(0x00fb1c18, 0x0077e820);
    wide_filebuf(0x00fb1d00, 0x0077e840);
    u8 *const cin = std_stream(0x00fb1bd8, 0x00700650);
    Init_4f1bd0(cin + 8);
    at<u32>(cin, 4) = 0;
    wide_finish(cin, 0x007099dc, 0x00fb1c70);
    u8 *const cout = std_stream(0x00fb1cc8, 0x00700648);
    Init_4f1bd0(cout + 4);
    wide_finish(cout, 0x007099ec, 0x00fb1c18);
    std_tie(cin, 0x00fb1cc8);
    u8 *const cerr = std_stream(0x00fb1b60, 0x00700648);
    Init_4ef830(cerr + 4);
    at<u32>(cerr, 4) = 0x007099e4;
    wide_finish(cerr, 0x007099ec, 0x00fb1d00);
    std_tie(cerr, 0x00fb1cc8);
    at<u32>(std_ios(cerr), 0xc) |= 2;
    u8 *const clog = std_stream(0x00fb1ba0, 0x00700648);
    Init_4ef830(clog + 4);
    at<u32>(clog, 4) = 0x007099e4;
    wide_finish(clog, 0x007099ec, 0x00fb1d00);
    std_tie(clog, 0x00fb1cc8);
    return self;
}

static void flush_streams(u32 counter, u32 a, u32 b, u32 c) {
    u32 lock;
    g_env->lockit(&lock);
    const bool last = --*reinterpret_cast<u32 *>((uintptr_t)counter) == 0;
    g_env->unlockit(&lock);
    if (!last) return;
    const u32 streams[] = {a, b, c};
    for (const u32 address : streams) {
        u8 *const stream = reinterpret_cast<u8 *>((uintptr_t)address);
        const auto ios = [stream] { return stream + at<u32>(at<u8 *>(stream, 0), 4); };
        if (at<u8>(ios(), 4) & 6) continue;
        u8 *const buffer = at<u8 *>(ios(), 0x28);
        if (at<s32(__fastcall *)(u8 *)>(at<u8 *>(buffer, 0), 0x2c)(buffer) == -1)
            IosBase_Clear(ios(), nullptr, at<u32>(ios(), 4) | 4, 0);
    }
}

void __fastcall Ios_InitDtor(void *) { flush_streams(0x007822b0, 0x00fb1a00, 0x00fb1b28, 0x00fb1970); }

void __fastcall Ios_WinitDtor(void *) { flush_streams(0x0078240c, 0x00fb1cc8, 0x00fb1b60, 0x00fb1ba0); }

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

u8 *__fastcall BasicIosW_DeletingDtor(u8 *self, void *, u32 flags) {
    BasicIosW_Dtor(self);
    if (flags & 1) g_env->memFree(self);
    return self;
}

void __fastcall BasicOstreamW_VbaseDtor(u8 *stream) {
    set_stream_vtable(stream, 0x007099ec);
    BasicIosW_Dtor(stream + 4);
}

void __fastcall BasicIstreamW_VbaseDtor(u8 *stream) {
    set_stream_vtable(stream, 0x007099dc);
    BasicIosW_Dtor(stream + 8);
}

u8 *__fastcall BasicOstreamW_DeletingDtor(u8 *ios, void *, u32 flags) {
    u8 *const stream = ios - 4;
    BasicOstreamW_VbaseDtor(stream);
    if (flags & 1) g_env->memFree(stream);
    return stream;
}

u8 *__fastcall BasicIstreamW_DeletingDtor(u8 *ios, void *, u32 flags) {
    u8 *const stream = ios - 8;
    BasicIstreamW_VbaseDtor(stream);
    if (flags & 1) g_env->memFree(stream);
    return stream;
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

u32 *__fastcall BadCast_DeletingDtor(u32 *self, void *, u32 flags) {
    BadCast_Dtor(self);
    if (flags & 1) g_env->memFree(self);
    return self;
}

void __fastcall BadCast_Throw(const u32 *from) {
    u32 copy[3];
    BadCast_Copy(copy, nullptr, from);
    g_env->cxxThrow(copy, reinterpret_cast<const void *>((uintptr_t)0x007193a8));
}

u32 __fastcall CtypeW_Widen(void *, void *, u32 c) { return CRT_btowc((u32)(s32)(s8)c); }

const char *__fastcall CtypeW_WidenRange(void *, void *, const char *first, const char *last, u16 *to) {
    for (; first != last; ++first, ++to) *to = (u16)CRT_btowc((u32)(s32)(s8)*first);
    return first;
}

u32 __fastcall CtypeW_ToLower(u8 *self, void *, u32 c) {
    const s32 narrow = static_cast<s8>(vcall<u32>(self, 0x30, c, 0u));
    c = (c & 0xffffff00u) | static_cast<u8>(CRT_TolowerLocking(narrow, reinterpret_cast<const u32 *>(self + 8)));
    return vcall<u32>(self, 0x28, c);
}

u16 *__fastcall CtypeW_ToLowerRange(u8 *self, void *, u16 *first, u16 *last) {
    for (; first != last; ++first) *first = static_cast<u16>(CRT_TolowerLocking(*first, reinterpret_cast<const u32 *>(self + 8)));
    return first;
}

u32 __fastcall CtypeW_ToUpper(u8 *self, void *, u32 c) {
    const s32 narrow = static_cast<s8>(vcall<u32>(self, 0x30, c, 0u));
    c = (c & 0xffffff00u) | static_cast<u8>(CRT_ToupperLocking(narrow, reinterpret_cast<const u32 *>(self + 8)));
    return vcall<u32>(self, 0x28, c);
}

u16 *__fastcall CtypeW_ToUpperRange(u8 *self, void *, u16 *first, u16 *last) {
    for (; first != last; ++first) *first = static_cast<u16>(CRT_ToupperLocking(*first, reinterpret_cast<const u32 *>(self + 8)));
    return first;
}

s32 __fastcall CtypeW_Narrow(void *, void *, u32 c, u32) { return CRT_Wctob(c); }

const u16 *__fastcall CtypeW_NarrowRange(void *, void *, const u16 *first, const u16 *last, u32, char *to) {
    for (; first != last; ++first, ++to) *to = static_cast<char>(CRT_Wctob(*first));
    return first;
}

void __fastcall CtypeW_Dtor(u8 *self) {
    at<u32>(self, 0) = 0x00709cac;
    if (at<u32>(self, 0x14)) Crt().free(at<void *>(self, 0x10));
    at<u32>(self, 0) = 0x00709b44;
}

u8 *__fastcall CtypeW_DeletingDtor(u8 *self, void *, u32 flags) {
    CtypeW_Dtor(self);
    if (flags & 1) g_env->memFree(self);
    return self;
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
