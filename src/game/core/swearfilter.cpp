#include "swearfilter.h"

#include <cstdint>
#include <cstring>

#include "buffers.h"
#include "crt.h"
#include "gameheap.h"
#include "initializers.h"
#include "iosbase.h"
#include "narrowfilebuf.h"
#include "nstream.h"
#include "stdlist.h"
#include "wtraits.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u16 *const kNullStr = reinterpret_cast<u16 *>(0x006fd6c4);  // _Nullstr

const SwearFilterEnv kGameEnv = {reinterpret_cast<u32(__cdecl *)(u32)>((uintptr_t)0x0067e5d0), &NFileBuf_Open, &CRT_fclose};
const SwearFilterEnv *g_env = &kGameEnv;

u16 lower(u16 c) { return static_cast<u16>(g_env->lower(c)); }

bool isSpace(u16 c) { return Crt().iswspace(c) != 0; }

void set_ios_vtable(u8 *stream, u32 vtable) { at<u32>(stream, at<u32>(at<u8 *>(stream, 0), 4)) = vtable; }

const u32 kLoadFrame = 0x210, kLoadLine = 0x90;   // (the original's frame: the stream, then the two buffers, to its SEH record)

// The load of a file in a frame of the caller's. Only the two buffers' old contents matter: a read that fails leaves the unit it looks at as it was.
u8 LoadInFrame(u8 *frame, const char *path, u8 *list, u32 flag);

void unlink(u8 *node) {
    at<u8 *>(at<u8 *>(node, 4), 0) = at<u8 *>(node, 0);
    at<u8 *>(at<u8 *>(node, 0), 4) = at<u8 *>(node, 4);
}

// Whether an exception of the list [+0x10] stands at the place.
bool excepted(const u8 *self, const u16 *place) {
    for (const u8 *node = at<const u8 *>(at<const u8 *>(self, 0x10), 0); node != at<const u8 *>(self, 0x10); node = at<const u8 *>(node, 0)) {
        const u16 *const word = at<const u16 *>(node, 0xc);
        if (Crt().wcsnicmp(place, word ? word : kNullStr, at<u32>(node, 0x10)) == 0)
            return true;
    }
    return false;
}

u16 *past(u16 *place, const u16 *word) {
    for (; *word; ++word)
        ++place;
    return place;
}

u16 *starred(u16 *place, const u16 *word, bool star) {
    for (; *word; ++word, ++place)
        if (star)
            *place = '*';
    return place;
}

}  // namespace

void SwearFilter_SetEnv(const SwearFilterEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u16 *__cdecl SwearFilter_Find(u16 *text, const u16 *word) {
    if (!*word)
        return text;
    u16 *start = nullptr;
    const u16 *rest = nullptr;
    for (; *text; ++text) {
        const u16 c = lower(*text);
        if (rest) {
            if (!*rest)
                return start;
            if (c != lower(*rest)) {
                start = nullptr;
                rest = nullptr;
            }
        } else if (c == lower(*word)) {
            rest = word;
            start = text;
        }
        if (rest)
            ++rest;
    }
    return rest && !*rest ? start : nullptr;
}

u8 __fastcall SwearFilter_Apply(const u8 *self, void *, StdWString *text, u32 flag) {
    const bool star = static_cast<u8>(flag) != 0;
    u8 result = 0;
    const u8 *const list = at<const u8 *>(self, 4);
    for (const u8 *node = at<const u8 *>(list, 0); node != at<const u8 *>(self, 4); node = at<const u8 *>(node, 0)) {
        const u16 *word = at<const u16 *>(node, 0xc);
        if (!word)
            word = kNullStr;
        enum { kWhole, kPart, kAnywhere } mode = kPart;
        if (*word == ' ') {
            mode = kWhole;
            ++word;
        } else if (*word == '*') {
            mode = kAnywhere;
            ++word;
        }
        u16 *from = at<u16 *>(text, 4);
        if (!from)
            from = kNullStr;
        while (*word) {
            u16 *place = SwearFilter_Find(from, word);
            if (!place)
                break;
            u16 *const found = place;
            if (mode == kAnywhere) {
                if (excepted(self, place)) {
                    place = past(place, word);
                } else {
                    result = 1;
                    place = starred(place, word, star);
                }
            } else {
                if (mode == kPart) {
                    while (place != from && !isSpace(*place))
                        --place;
                    if (isSpace(*place))
                        ++place;
                }
                if (place != from && !isSpace(place[-1])) {
                    place = past(found, word);
                } else if (excepted(self, place)) {
                    while (place != found)   // (sic: a word that starts with a space leaves it past the place: then round the address space)
                        ++place;
                    place = past(place, word);
                } else {
                    result = 1;
                    for (; place != found; ++place)
                        if (star)
                            *place = '*';
                    place = starred(place, word, star);
                    for (u16 c = *place; c && !isSpace(c); c = *++place)
                        if (star)
                            *place = '*';
                }
            }
            from = place;
        }
    }
    return result;
}

u8 __fastcall SwearFilter_LoadFile(u8 *, void *, const char *path, u8 *list, u32 flag) {
    u8 frame[kLoadFrame];
    memset(frame + kLoadLine, 0, kLoadFrame - kLoadLine);
    return LoadInFrame(frame, path, list, flag);
}

u8 __fastcall SwearFilter_Load(u8 *self, void *, const char *swears, const char *allowed, u32 flag) {
    u8 *const head = at<u8 *>(self, 4);
    for (u8 *node = at<u8 *>(head, 0); node != head;) {
        u8 *const gone = node;
        node = at<u8 *>(node, 0);
        unlink(gone);
        CountedBuf_Deleter(gone + 8, nullptr, 0);
        GameHeap_Free(gone);
        --at<u32>(self, 8);
    }
    u8 *const head2 = at<u8 *>(self, 0x10);
    for (u8 *node = at<u8 *>(head2, 0); node != head2;) {
        u8 *const gone = node;
        node = at<u8 *>(node, 0);
        unlink(gone);
        CountedBuf_Reset(gone + 8, nullptr, 1);
        GameHeap_Free(gone);
        --at<u32>(self, 0x14);
    }
    u8 frame[kLoadFrame];   // (one for both: the original's two calls have the same stack place, and a second file's failed read sees the first's units there)
    memset(frame + kLoadLine, 0, kLoadFrame - kLoadLine);
    if (!LoadInFrame(frame, swears, self, flag))
        return 0;
    LoadInFrame(frame, allowed, self + 0xc, flag);
    return 1;
}
}

namespace {

u8 LoadInFrame(u8 *frame, const char *path, u8 *list, u32 flag) {
    u8 *const stream = frame;
    u8 *const buffer = frame + 8;
    u8 *const ios = frame + 0x5c;
    u16 *const line = reinterpret_cast<u16 *>(frame + kLoadLine);
    u16 *const out = reinterpret_cast<u16 *>(frame + 0x110);

    at<u32>(stream, 0) = 0x00702968;
    Init_4ef830(ios);
    at<u32>(ios, 0) = 0x00709954;
    NIstream_Ctor(stream, nullptr, buffer, 0, 0);
    NFileBuf_Ctor(buffer, nullptr, nullptr);
    set_ios_vtable(stream, 0x00702960);
    if (!static_cast<u8>(flag)) {
        NIfstream_DtorOnIos(ios);
        at<u32>(ios, 0) = 0x00709954;
        IosBase_Dtor(ios);
        return 0;
    }
    if (!g_env->open(buffer, nullptr, path, 0x21))
        BasicIosW_Clear(ios, nullptr, at<u32>(ios, 4) | 2, 0);
    if (!at<u32>(buffer, 0x50)) {
        set_ios_vtable(stream, 0x00702960);
        NFileBuf_Dtor(buffer);
        set_ios_vtable(stream, 0x0070994c);
        at<u32>(ios, 0) = 0x00709954;
        IosBase_Dtor(ios);
        return 0;
    }
    u16 *cursor = line;
    while (!(at<u8>(ios, 4) & 1)) {
        NIstream_Read(stream, nullptr, reinterpret_cast<char *>(cursor), 2);
        const bool ended = *cursor == 10;
        ++cursor;
        if (!ended || static_cast<u8>(line[0]) == '#')   // (sic: the place stays on after a '#' line)
            continue;
        u16 *to = out;
        for (u16 c = line[0]; c != 0xd && c != 0xa; c = to[-0x40]) {   // (sic: 0x40 units on, a long line copies over itself)
            *to = c;
            ++to;
        }
        *to = 0;
        for (u16 *p = out; *p; ++p)
            *p = static_cast<u16>(~*p);
        StdWString text;
        text.allocator = 0;
        CountedBuf_Reset(reinterpret_cast<u8 *>(&text), nullptr, 0);
        const u32 n = Crt().wcslen(out);
        if (StdWString_Grow(&text, nullptr, n, 1)) {
            WTraits_Copy(text.ptr, out, n);
            ImeWString_SetLength(reinterpret_cast<u8 *>(&text), nullptr, n);
        }
        u8 *node;
        StdListWString_Insert(list, nullptr, &node, at<u8 *>(list, 4), &text);
        CountedBuf_Reset(reinterpret_cast<u8 *>(&text), nullptr, 1);
        cursor = line;
    }
    u8 *const file = at<u8 *>(buffer, 0x50);
    bool closed = false;
    if (file && g_env->fclose(file) == 0) {
        NFileBuf_Init(buffer, nullptr, nullptr, 2);
        closed = true;
    }
    if (!closed)
        BasicIosW_SetState(ios, nullptr, 2, 0);
    set_ios_vtable(stream, 0x00702960);
    NFileBuf_Dtor(buffer);
    set_ios_vtable(stream, 0x0070994c);
    at<u32>(ios, 0) = 0x00709954;
    IosBase_Dtor(ios);
    return 1;
}

}  // namespace
