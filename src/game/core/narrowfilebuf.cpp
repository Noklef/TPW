#include "narrowfilebuf.h"

#include <cstdint>
#include <cstring>

#include "buffers.h"
#include "fieldops.h"
#include "initializers.h"
#include "crt.h"
#include "gameheap.h"
#include "globals.h"
#include "iosbase.h"
#include "sharedglobals.h"
#include "stdstring.h"
#include "streambuf.h"
#include "vcall.h"
#include "vobj3.h"
#include "wfilebuf.h"

namespace {

const NarrowFileBufEnv kGameEnv = {reinterpret_cast<s32(__cdecl *)(void *)>((uintptr_t)0x0067b620), reinterpret_cast<s32(__cdecl *)(u32, u32)>((uintptr_t)0x0067cbe0), reinterpret_cast<s32(__cdecl *)(u32)>((uintptr_t)0x0067cb90)};
const NarrowFileBufEnv *g_env = &kGameEnv;

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u32 npos() { return global<u32>(0x006fe5ac); }

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

void lose_use(u8 *counted) {
    if (!counted) return;
    if (u8 *const dead = Counted_Release(counted)) vcall<void>(dead, 0, (u32)1);
}

void set_slot_pointers(u8 *buf) {
    static const u8 kSlots[6][2] = {{0xc, 4}, {0x1c, 0x14}, {0x20, 0x18}, {0x10, 8}, {0x2c, 0x24}, {0x30, 0x28}};
    for (const auto &slot : kSlots) at<u8 *>(buf, slot[0]) = buf + slot[1];
}

void point_at_file(u8 *buf, u8 *file) {
    const u32 base = (u32)(uintptr_t)file;
    Fn_5d0e00(buf, nullptr, base + 8, base, base + 4, base + 8, base, base + 4);
}

void set_ios_vtable(u8 *stream, u32 vtable) { at<u32>(stream, at<u32>(at<u8 *>(stream, 0), 4)) = vtable; }

void keep_file(u8 *buf, u8 *file) {
    at<u8 *>(buf, 0x50) = file;
    at<u32>(buf, 0x40) = global<u32>(0x00fb2760);
    at<u32>(buf, 0x3c) = global<u32>(0x00fb2760);
    at<u32>(buf, 0x38) = 0;
}

}  // namespace

void NarrowFileBuf_SetEnv(const NarrowFileBufEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u8 *__fastcall NFileBuf_Ctor(u8 *buf, void *, u8 *file) {
    LocaleRef_Ctor(reinterpret_cast<u32 *>(buf + 0x34));
    set_slot_pointers(buf);
    at<u32>(buf, 0) = 0x00709914;
    at<u32>(buf, 8) = 0;
    at<u32>(buf, 0x18) = 0;
    at<u32>(buf, 0x28) = 0;
    Targets3_Set(buf, nullptr, 0, 0, 0);
    LocaleRef_Ctor(reinterpret_cast<u32 *>(buf + 0x4c));
    at<u32>(buf, 0) = 0x007098dc;
    buf[0x48] = 0;
    lose_use(at<u8 *>(buf, 0x4c));
    LocaleRef_Ctor(reinterpret_cast<u32 *>(buf + 0x4c));
    set_slot_pointers(buf);
    at<u32>(buf, 0x44) = 0;
    Targets2_Set(buf, nullptr, 0, 0);
    Targets3_Set(buf, nullptr, 0, 0, 0);
    if (file) point_at_file(buf, file);
    keep_file(buf, file);
    return buf;
}

u8 *__fastcall NFileBuf_Close(u8 *buf) {
    u8 *const file = at<u8 *>(buf, 0x50);
    if (!file || g_env->fclose(file)) return nullptr;
    buf[0x48] = 0;
    Wstreambuf_InitPointers(buf);
    keep_file(buf, nullptr);
    return buf;
}

void __fastcall NFileBuf_Init(u8 *buf, void *, u8 *file, u32 mode) {
    buf[0x48] = mode == 1;
    if (mode == 0) {
        lose_use(at<u8 *>(buf, 0x4c));
        LocaleRef_Ctor(reinterpret_cast<u32 *>(buf + 0x4c));
        at<u32>(buf, 0x44) = 0;
    }
    Wstreambuf_InitPointers(buf);
    if (file && !buf[0x48]) point_at_file(buf, file);
    keep_file(buf, file);
}

u8 *__fastcall NFileBuf_DeletingDtor(u8 *buf, void *, u32 flags) {
    NFileBuf_Dtor(buf);
    if (flags & 1) GameHeap_Free(buf);
    return buf;
}

void __fastcall NFileBuf_Dtor(u8 *buf) {
    at<u32>(buf, 0) = 0x007098dc;
    if (buf[0x48] && at<void *>(buf, 0x50) && g_env->fclose(at<void *>(buf, 0x50)) == 0) {
        buf[0x48] = 0;
        Wstreambuf_InitPointers(buf);
        keep_file(buf, nullptr);
    }
    if (u8 *const name = at<u8 *>(buf, 0x44)) {
        StdString_Tidy(reinterpret_cast<StdString *>(name), nullptr, 1);
        GameHeap_Free(name);
    }
    lose_use(at<u8 *>(buf, 0x4c));
    at<u32>(buf, 0) = 0x00709914;
    lose_use(at<u8 *>(buf, 0x34));
}

void __fastcall NIfstream_DtorOnIos(u8 *ios) {
    u8 *const stream = ios - 0x5c;
    set_ios_vtable(stream, 0x00702960);
    NFileBuf_Dtor(stream + 8);
    set_ios_vtable(stream, 0x0070994c);
}

void __fastcall NOfstream_DtorOnIos(u8 *ios) {
    u8 *const stream = ios - 0x58;
    set_ios_vtable(stream, 0x00702970);
    NFileBuf_Dtor(stream + 4);
    set_ios_vtable(stream, 0x0070995c);
}

s32 __fastcall NFileBuf_PutBack(u8 *buf, void *, s32 c) {
    const u8 *const get = *at<const u8 **>(buf, 0x1c);
    if (get && *at<const u8 **>(buf, 0xc) < get && (c == -1 || get[-1] == (u32)c)) {
        ++*at<u32 *>(buf, 0x2c);
        *at<const u8 **>(buf, 0x1c) -= 1;
        return c == -1 ? 0 : c;
    }
    if (!at<u32>(buf, 0x50) || c == -1) return -1;
    if (!at<u8 *>(buf, 0x38)) return g_env->ungetc(c & 0xff, at<u32>(buf, 0x50)) != -1 ? c : -1;
    StdString *const text = at<StdString *>(buf, 0x44);
    u32 left = text->len;
    if (!left) return -1;
    make_own(text, left);
    const u32 file = at<u32>(buf, 0x50);
    const u8 *from = reinterpret_cast<const u8 *>(text->ptr) + left;
    do {
        if (g_env->ungetc(*--from, file) == -1) return -1;
    } while (--left);
    // As StdString_Split, inline: a text others use is tidied and assigned again from its own bytes.
    StdString *const own = at<StdString *>(buf, 0x44);
    const char *const shared = own->ptr;
    if (shared && shared[-1] != 0 && shared[-1] != '\xff') {
        StdString_Tidy(own, nullptr, 1);
        StdString_AssignChars(own, nullptr, shared, static_cast<u32>(std::strlen(shared)));
    }
    erase_front(own);
    at<u32>(buf, 0x40) = at<u32>(buf, 0x3c);
    return c;
}

s32 __fastcall NFileBuf_Uflow(u8 *buf) {
    u8 *const get = *at<u8 **>(buf, 0x1c);
    if (get) {
        u32 *const left = at<u32 *>(buf, 0x2c);
        if (reinterpret_cast<uintptr_t>(get) < static_cast<u32>(reinterpret_cast<uintptr_t>(get) + *left)) {
            --*left;
            *at<u8 **>(buf, 0x1c) = get + 1;
            return *get;
        }
    }
    const u32 file = at<u32>(buf, 0x50);
    if (!file) return -1;
    u8 *const convert = at<u8 *>(buf, 0x38);
    if (!convert) {
        const s32 got = g_env->fgetc(file);
        return got == -1 ? -1 : static_cast<u8>(got);
    }
    at<u32>(buf, 0x3c) = at<u32>(buf, 0x40);
    StdString *const first = at<StdString *>(buf, 0x44);
    StdString_Split(first, nullptr);
    erase_front(first);
    s32 got = g_env->fgetc(at<u32>(buf, 0x50));
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
        StdString *const string = at<StdString *>(buf, 0x44);
        StdString_Freeze(string, nullptr);
        char *const end = string->ptr ? string->ptr + string->len : nullptr;
        StdString *const again = at<StdString *>(buf, 0x44);
        StdString_Freeze(again, nullptr);
        u8 to[4];   // (the original's local byte, whatever the conversion leaves in it)
        char *from_next;
        u8 *to_next;
        const s32 answer = vcall<s32>(at<u8 *>(buf, 0x38), 0x10, buf + 0x40, again->ptr, end, &from_next, &to[0], &to[1], &to_next);
        if (answer == 0) return to[0];
        if (answer == 3) {
            StdString *const own = at<StdString *>(buf, 0x44);
            if (own->len >= 1) {
                make_own(own, own->len);
                return static_cast<u8>(own->ptr[0]);
            }
        } else if (answer != 1) {
            return -1;
        }
        got = g_env->fgetc(at<u32>(buf, 0x50));
    }
    return -1;
}

}  // extern "C"
