#include "wfilebuf.h"

#include <cstdint>
#include <cstring>

#include "crt.h"
#include "gameheap.h"
#include "iosbase.h"
#include "stdstring.h"
#include "streambuf.h"
#include "vcall.h"
#include "vobj3.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const WFileBufEnv kGameEnv = {reinterpret_cast<u32(__cdecl *)()>((uintptr_t)0x00699da0),
                              CRT_fclose,
                              reinterpret_cast<u32(__cdecl *)(const void *, u32, u32, u32)>((uintptr_t)0x0067b6d0),
                              reinterpret_cast<u32(__cdecl *)(u32, u32)>((uintptr_t)0x0067c9f0),
                              reinterpret_cast<u32(__cdecl *)(u32, u32)>((uintptr_t)0x0067cca0),
                              reinterpret_cast<s32(__cdecl *)(u32, u32)>((uintptr_t)0x0067cbe0),
                              reinterpret_cast<s32(__cdecl *)(u32)>((uintptr_t)0x0067cb90),
                              reinterpret_cast<u32(__cdecl *)(u32)>((uintptr_t)0x0067cdf0)};
const WFileBufEnv *g_env = &kGameEnv;
constexpr u32 kFile = 0x50;

u32 npos() { return global<u32>(0x006fe5ac); }

// The text unshared when others use it (its use count neither 0 nor 0xff, by a Grow to its own length), then marked as not shareable (0xff), when there is a text.
void make_own(StdString *s, u32 length) {
    if (s->ptr && s->ptr[-1] != 0 && s->ptr[-1] != '\xff') StdString_Grow(s, nullptr, length, 0);
    if (s->ptr) s->ptr[-1] = '\xff';
}

void erase_front(StdString *s) {
    u32 count = npos();
    if (s->len < count) count = s->len;
    if (!count) return;
    std::memmove(s->ptr, s->ptr + count, s->len - count);
    const u32 n = s->len - count;
    if (StdString_Grow(s, nullptr, n, 0)) {
        s->len = n;
        s->ptr[n] = 0;
    }
}

void erase_all(StdString *s) {
    StdString_Split(s, nullptr);
    erase_front(s);
}

// As StdString_Split, inline in 0x4f0c50: a text others use is tidied and assigned again from its own bytes up to its first 0.
void erase_all_unshared(StdString *s) {
    const char *const shared = s->ptr;
    if (shared && shared[-1] != 0 && shared[-1] != '\xff') {
        StdString_Tidy(s, nullptr, 1);
        StdString_AssignChars(s, nullptr, shared, static_cast<u32>(std::strlen(shared)));
    }
    erase_front(s);
}

void set_slot_pointers(u8 *buf) {
    static const u8 kSlots[6][2] = {{0xc, 4}, {0x1c, 0x14}, {0x20, 0x18}, {0x10, 8}, {0x2c, 0x24}, {0x30, 0x28}};
    for (const auto &slot : kSlots) at<u8 *>(buf, slot[0]) = buf + slot[1];
}

void lose_use(u8 *counted) {
    if (!counted) return;
    if (u8 *const dead = Counted_Release(counted)) vcall<void>(dead, 0, (u32)1);
}

// (A stream's first word is its vbtable, whose +4 is the offset of the virtual base's word.)
void set_stream_vtable(u8 *stream, u32 vtable) { at<u32>(stream + at<s32>(at<u8 *>(stream, 0), 4), 0) = vtable; }

}  // namespace

extern "C" {

void WFileBuf_SetEnv(const WFileBufEnv *env) { g_env = env ? env : &kGameEnv; }

u32 *__fastcall LocaleRef_Ctor(u32 *ref) {
    *ref = g_env->localeInit();
    u32 &uses = at<u32>(global<u8 *>(0x00fb1dd0), 4);
    if (uses < 0xffffffffu) ++uses;
    return ref;
}

void __fastcall WFileBuf_Init(u8 *buf, void *, u32 file, s32 which) {
    buf[0x48] = which == 1;
    if (which == 0) {
        u8 *const old = at<u8 *>(buf, 0x4c);
        if (old) {
            u8 *const dead = Counted_Release(old);
            if (dead) vcall<void>(dead, 0, (u32)1);
        }
        LocaleRef_Ctor(reinterpret_cast<u32 *>(buf + 0x4c));
        at<u32>(buf, 0x44) = 0;
    }
    static const u8 kSlots[6][2] = {{0xc, 4}, {0x1c, 0x14}, {0x20, 0x18}, {0x10, 8}, {0x2c, 0x24}, {0x30, 0x28}};
    for (const auto &slot : kSlots) {
        at<u8 *>(buf, slot[0]) = buf + slot[1];
        at<u32>(buf, slot[1]) = 0;
    }
    at<u32>(buf, 0x50) = file;
    at<u32>(buf, 0x40) = global<u32>(0x00fb34b4);
    at<u32>(buf, 0x3c) = global<u32>(0x00fb34b4);
    at<u32>(buf, 0x38) = 0;
}

u8 *__fastcall WFileBuf_Ctor(u8 *buf, void *, u32 file) {
    LocaleRef_Ctor(reinterpret_cast<u32 *>(buf + 0x34));
    set_slot_pointers(buf);
    at<u32>(buf, 0) = 0x007099a4;
    at<u32>(buf, 8) = 0;
    at<u32>(buf, 0x18) = 0;
    at<u32>(buf, 0x28) = 0;
    Wstreambuf_Setg(buf, nullptr, 0, 0, 0);
    LocaleRef_Ctor(reinterpret_cast<u32 *>(buf + 0x4c));
    at<u32>(buf, 0) = 0x0070996c;
    buf[0x48] = 0;
    lose_use(at<u8 *>(buf, 0x4c));
    LocaleRef_Ctor(reinterpret_cast<u32 *>(buf + 0x4c));
    set_slot_pointers(buf);
    at<u32>(buf, 0x44) = 0;
    Wstreambuf_Setp(buf, nullptr, 0, 0);
    Wstreambuf_Setg(buf, nullptr, 0, 0, 0);
    at<u32>(buf, 0x50) = file;
    at<u32>(buf, 0x40) = global<u32>(0x00fb34b4);
    at<u32>(buf, 0x3c) = global<u32>(0x00fb34b4);
    at<u32>(buf, 0x38) = 0;
    return buf;
}

void __fastcall WFileBuf_Dtor(u8 *buf) {
    at<u32>(buf, 0) = 0x0070996c;
    if (buf[0x48] && at<void *>(buf, 0x50) && g_env->fclose(at<void *>(buf, 0x50)) == 0) WFileBuf_Init(buf, nullptr, 0, 2);
    if (u8 *const name = at<u8 *>(buf, 0x44)) {
        StdString_Tidy(reinterpret_cast<StdString *>(name), nullptr, 1);
        GameHeap_Free(name);
    }
    lose_use(at<u8 *>(buf, 0x4c));
    at<u32>(buf, 0) = 0x007099a4;
    lose_use(at<u8 *>(buf, 0x34));
}

void __fastcall WOfstream_VbaseDtor(u8 *ios) {
    u8 *const stream = ios - 0x58;
    set_stream_vtable(stream, 0x0070062c);
    WFileBuf_Dtor(stream + 4);
    set_stream_vtable(stream, 0x007099ec);
}

void __fastcall WIfstream_VbaseDtor(u8 *ios) {
    u8 *const stream = ios - 0x5c;
    set_stream_vtable(stream, 0x00700638);
    WFileBuf_Dtor(stream + 8);
    set_stream_vtable(stream, 0x007099dc);
}

u16 __fastcall WFileBuf_Overflow(u8 *buf, void *, u32 c) {
    if (static_cast<u16>(c) == 0xffff) return 0;
    u16 *const put = *at<u16 **>(buf, 0x20);
    if (put) {
        u32 *const left = at<u32 *>(buf, 0x30);
        if (reinterpret_cast<uintptr_t>(put) < static_cast<u32>(reinterpret_cast<uintptr_t>(put) + *left * 2)) {
            --*left;
            *at<u16 **>(buf, 0x20) = put + 1;
            *put = static_cast<u16>(c);
            return static_cast<u16>(c);
        }
    }
    const u32 file = at<u32>(buf, kFile);
    if (!file) return 0xffff;
    u8 *const convert = at<u8 *>(buf, 0x38);
    if (!convert) return static_cast<u16>(g_env->fputwc(c, file)) != 0xffff ? static_cast<u16>(c) : 0xffff;
    erase_all(at<StdString *>(buf, 0x44));
    StdString *const text = at<StdString *>(buf, 0x44);
    if (npos() - text->len <= 8) Std_Xlen();
    const u32 grown = text->len + 8;
    if (StdString_Grow(text, nullptr, grown, 0)) {
        std::memset(text->ptr + text->len, 0, 8);
        text->len = grown;
        text->ptr[grown] = 0;
    }
    StdString_Freeze(text, nullptr);
    char *const end = text->ptr ? text->ptr + text->len : nullptr;
    StdString_Freeze(text, nullptr);
    u32 wide = c;
    u16 *from_next;
    char *to_next;
    const s32 answer = vcall<s32>(convert, 0x14, buf + 0x40, &wide, reinterpret_cast<u16 *>(&wide) + 1, &from_next, text->ptr, end, &to_next);
    if (answer == 3) return static_cast<u16>(g_env->fputwc(wide, at<u32>(buf, kFile))) != 0xffff ? static_cast<u16>(c) : 0xffff;
    if (answer != 0 && (answer != 1 || from_next == reinterpret_cast<u16 *>(&wide))) return 0xffff;
    StdString *const out = at<StdString *>(buf, 0x44);
    StdString_Freeze(out, nullptr);
    const u32 size = static_cast<u32>(to_next - out->ptr);
    StdString_Freeze(out, nullptr);
    return g_env->fwrite(out->ptr, size, 1, at<u32>(buf, kFile)) == size ? static_cast<u16>(c) : 0xffff;   // (sic: the count of items, 1, against the size)
}

u16 __fastcall WFileBuf_PutBack(u8 *buf, void *, u32 c) {
    const u16 *const get = *at<const u16 **>(buf, 0x1c);
    if (get && *at<const u16 **>(buf, 0xc) < get && (static_cast<u16>(c) == 0xffff || get[-1] == static_cast<u16>(c))) {
        ++*at<u32 *>(buf, 0x2c);
        *at<const u16 **>(buf, 0x1c) -= 1;
        return static_cast<u16>(c) == 0xffff ? 0 : static_cast<u16>(c);
    }
    const u32 file = at<u32>(buf, kFile);
    if (!file || static_cast<u16>(c) == 0xffff) return 0xffff;
    if (!at<u8 *>(buf, 0x38)) return static_cast<u16>(g_env->ungetwc(c, file)) != 0xffff ? static_cast<u16>(c) : 0xffff;
    StdString *const text = at<StdString *>(buf, 0x44);
    u32 left = text->len;
    if (!left) return 0xffff;
    make_own(text, left);
    const u8 *from = reinterpret_cast<const u8 *>(text->ptr) + left;
    do {
        if (g_env->ungetc(*--from, file) == -1) return 0xffff;
    } while (--left);
    erase_all_unshared(at<StdString *>(buf, 0x44));
    at<u32>(buf, 0x40) = at<u32>(buf, 0x3c);
    return static_cast<u16>(c);
}

u16 __fastcall WFileBuf_Uflow(u8 *buf) {
    u16 *const get = *at<u16 **>(buf, 0x1c);
    if (get) {
        u32 *const left = at<u32 *>(buf, 0x2c);
        if (reinterpret_cast<uintptr_t>(get) < static_cast<u32>(reinterpret_cast<uintptr_t>(get) + *left * 2)) {
            --*left;
            *at<u16 **>(buf, 0x1c) = get + 1;
            return *get;
        }
    }
    const u32 file = at<u32>(buf, kFile);
    if (!file) return 0xffff;
    u8 *const convert = at<u8 *>(buf, 0x38);
    if (!convert) {
        const u32 got = g_env->fgetwc(file);
        return static_cast<u16>(got) == 0xffff ? 0xffff : static_cast<u16>(got);
    }
    at<u32>(buf, 0x3c) = at<u32>(buf, 0x40);
    erase_all(at<StdString *>(buf, 0x44));
    s32 got = g_env->fgetc(at<u32>(buf, kFile));
    while (got != -1) {
        StdString *const text = at<StdString *>(buf, 0x44);
        if (npos() - text->len <= 1) Std_Xlen();
        const u32 grown = text->len + 1;
        if (StdString_Grow(text, nullptr, grown, 0)) {
            text->ptr[text->len] = static_cast<char>(got);
            text->len = grown;
            text->ptr[grown] = 0;
        }
        at<u32>(buf, 0x40) = at<u32>(buf, 0x3c);
        StdString_Freeze(text, nullptr);
        char *const end = text->ptr ? text->ptr + text->len : nullptr;
        StdString_Freeze(text, nullptr);
        u32 wide;   // (what the conversion leaves in its low word)
        char *from_next;
        u16 *to_next;
        const s32 answer = vcall<s32>(convert, 0x10, buf + 0x40, text->ptr, end, &from_next, &wide, reinterpret_cast<u16 *>(&wide) + 1, &to_next);
        if (answer == 0) return static_cast<u16>(wide);
        if (answer == 3) {
            StdString *const own = at<StdString *>(buf, 0x44);
            if (own->len >= 2) {
                make_own(own, own->len);
                return *reinterpret_cast<const u16 *>(own->ptr);
            }
        } else if (answer != 1) {
            return 0xffff;
        }
        got = g_env->fgetc(at<u32>(buf, kFile));
    }
    return 0xffff;
}
}
