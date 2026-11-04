#include "swearfilter.h"

#include <cstdint>

#include "crt.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u16 *const kNullStr = reinterpret_cast<u16 *>(0x006fd6c4);  // _Nullstr

const SwearFilterEnv kGameEnv = {reinterpret_cast<u32(__cdecl *)(u32)>((uintptr_t)0x0067e5d0)};
const SwearFilterEnv *g_env = &kGameEnv;

u16 lower(u16 c) { return static_cast<u16>(g_env->lower(c)); }

bool isSpace(u16 c) { return Crt().iswspace(c) != 0; }

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
}
