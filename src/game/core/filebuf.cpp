#include "filebuf.h"

#include <cstdint>
#include <cstring>

#include "accessors.h"
#include "stdstring.h"
#include "vcall.h"

namespace {

template <typename T>
T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(p) + offset);
}

const FileBufEnv kGameEnv = {reinterpret_cast<s32(__cdecl *)(s32, void *)>((uintptr_t)0x0067e570), reinterpret_cast<u32(__cdecl *)(const void *, u32, u32, void *)>((uintptr_t)0x0067b6d0),
                             reinterpret_cast<s32(__cdecl *)(void *, const void *)>((uintptr_t)0x0067cf70), reinterpret_cast<s32(__cdecl *)(void *, s32, s32)>((uintptr_t)0x0067c100),
                             reinterpret_cast<s32(__cdecl *)(void *, void *)>((uintptr_t)0x0067cf40)};
const FileBufEnv *g_env = &kGameEnv;

u32 &NoPos() { return at<u32>(nullptr, 0x006fe5ac); }

StdString *Text(u8 *self) { return at<StdString *>(self, 0x44); }

// The string's first [0x6fe5ac] characters (at most all) erased.
void EraseFront(StdString *s) {
    u32 count = NoPos();
    if (s->len < count) count = s->len;
    if (!count) return;
    std::memmove(s->ptr, s->ptr + count, s->len - count);  // memmove (0x67bb90)
    const u32 left = s->len - count;
    if (StdString_Grow(s, nullptr, left, 0)) {
        s->len = left;
        s->ptr[left] = 0;
    }
}

}  // namespace

void FileBuf_SetEnv(const FileBufEnv *env) { g_env = env ? env : &kGameEnv; }

s32 __fastcall FileBuf_Overflow(u8 *self, void *, s32 c) {
    if (c == -1) return 0;
    const u32 next = *at<u32 *>(self, 0x20);
    if (next) {
        s32 *const room = at<s32 *>(self, 0x30);
        if (next < next + static_cast<u32>(*room)) {
            --*room;
            u8 **const put = at<u8 **>(self, 0x20);
            u8 *const out = *put;
            *put = out + 1;
            *out = static_cast<u8>(c);
            return c;
        }
    }
    if (!at<void *>(self, 0x50)) return -1;
    if (!at<u8 *>(self, 0x38)) return g_env->fputc(static_cast<s8>(c), at<void *>(self, 0x50)) != -1 ? c : -1;

    u8 byte = static_cast<u8>(c);
    StdString_Split(Text(self), nullptr);
    EraseFront(Text(self));
    StdString *s = Text(self);
    if (NoPos() - s->len <= 8) StdStrings_Xlen();
    const u32 grown = s->len + 8;
    if (StdString_Grow(s, nullptr, grown, 0)) {
        std::memset(s->ptr + s->len, 0, 8);
        s->len = grown;
        s->ptr[grown] = 0;
    }
    s = Text(self);
    StdString_Freeze(s, nullptr);
    u8 *const toEnd = s->ptr ? reinterpret_cast<u8 *>(s->ptr) + s->len : nullptr;
    s = Text(self);
    StdString_Freeze(s, nullptr);
    u8 *fromNext;
    u8 *toNext;
    const s32 result = vcall<s32>(at<u8 *>(self, 0x38), 0x14, self + 0x40, &byte, &byte + 1, &fromNext, s->ptr, toEnd, &toNext);
    if (result == 3) return g_env->fputc(static_cast<s8>(byte), at<void *>(self, 0x50)) != -1 ? c : -1;
    if (result != 0 && (result != 1 || fromNext == &byte)) return -1;
    s = Text(self);
    StdString_Freeze(s, nullptr);
    const u32 made = static_cast<u32>(toNext - reinterpret_cast<u8 *>(s->ptr));
    s = Text(self);
    StdString_Freeze(s, nullptr);
    return g_env->fwrite(s->ptr, made, 1, at<void *>(self, 0x50)) == made ? c : -1;
}

u32 *__fastcall FileBuf_SeekPos(u8 *self, void *, u32 *out, s32 offset, u32, u32 low, u32 high, u32 state, u32, u32) {
    u32 position[2] = {low, high};
    void *const file = at<void *>(self, 0x50);
    if (!file || g_env->fsetpos(file, position) != 0 || (offset != 0 && g_env->fseek(at<void *>(self, 0x50), offset, 1) != 0) || g_env->fgetpos(at<void *>(self, 0x50), position) != 0) {
        Obj_Init4f1520(reinterpret_cast<u8 *>(out), nullptr, 0xffffffff);
        return out;
    }
    StdString *const s = Text(self);
    if (s) {
        at<u32>(self, 0x40) = state;
        const char *const old = s->ptr;
        if (old && old[-1] != 0 && static_cast<u8>(old[-1]) != 0xff) {
            StdString_Tidy(s, nullptr, 1);
            const u32 length = static_cast<u32>(std::strlen(old));
            if (StdString_Grow(s, nullptr, length, 1)) {
                std::memcpy(s->ptr, old, length);
                StdString_Eos(s, nullptr, length);
            }
        }
        EraseFront(s);
    }
    out[2] = position[0];
    out[0] = 0;
    out[3] = position[1];
    out[4] = at<u32>(self, 0x40);
    return out;
}
