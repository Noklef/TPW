#include "crtstartup.h"

#include <cstdint>

#include "crt.h"

namespace {

bool lead_byte(char c) { return reinterpret_cast<const u8 *>((uintptr_t)0x00fb1669)[static_cast<u8>(c)] & 4; }

template <typename T> T &at(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename F> F import_slot(u32 slot) { return at<F>(slot); }

using Init = void(__cdecl *)();
void initterm(u32 begin, u32 end) { CRT_initterm(reinterpret_cast<Init *>((uintptr_t)begin), reinterpret_cast<Init *>((uintptr_t)end)); }

}  // namespace

extern "C" {

void __cdecl CRT_parse_cmdline(const char *command, char **words, char *text, s32 *wordCount, s32 *textSize) {
    const char *from = command;
    *textSize = 0;
    *wordCount = 1;
    if (words) *words++ = text;
    if (*from == '"') {
        ++from;
        char c = *from;
        while (c != '"' && c) {
            if (lead_byte(c)) {
                ++*textSize;
                if (text) {
                    *text++ = *from;
                    ++from;
                }
            }
            ++*textSize;
            if (text) *text++ = *from;
            ++from;
            c = *from;
        }
        ++*textSize;
        if (text) *text++ = 0;
        if (*from == '"') ++from;
    } else {
        char c;
        do {
            ++*textSize;
            if (text) *text++ = *from;
            c = *from++;
            if (lead_byte(c)) {
                ++*textSize;
                if (text) *text++ = *from;
                ++from;
            }
        } while (c != ' ' && c && c != '\t');
        if (!c)
            --from;
        else if (text)
            text[-1] = 0;
    }
    bool quoted = false;
    while (*from) {
        while (*from == ' ' || *from == '\t') ++from;
        if (!*from) break;
        if (words) *words++ = text;
        ++*wordCount;
        for (;;) {
            bool keep = true;
            u32 slashes = 0;
            while (*from == '\\') {
                ++from;
                ++slashes;
            }
            if (*from == '"') {
                if (!(slashes & 1)) {
                    if (quoted && from[1] == '"')
                        ++from;
                    else
                        keep = false;
                    quoted = !quoted;
                }
                slashes >>= 1;
            }
            for (; slashes; --slashes) {
                if (text) *text++ = '\\';
                ++*textSize;
            }
            const char c = *from;
            if (!c || (!quoted && (c == ' ' || c == '\t'))) break;
            if (keep) {
                if (text) {
                    if (lead_byte(c)) {
                        *text++ = c;
                        ++from;
                        ++*textSize;
                    }
                    *text++ = *from;
                    ++*textSize;
                } else {
                    if (lead_byte(c)) {
                        ++from;
                        ++*textSize;
                    }
                    ++*textSize;
                }
            }
            ++from;
        }
        if (text) *text++ = 0;
        ++*textSize;
    }
    if (words) *words = nullptr;
    ++*wordCount;
}

void __cdecl CRT_cinit() {
    if (const Init fpinit = at<Init>(0x0077e718)) fpinit();
    initterm(0x0072e0ec, 0x0072e0fc);
    initterm(0x00727000, 0x0072e0e8);
}

void __cdecl CRT_doexit(s32 code, s32 quick, s32 retcaller) {
    CRT_lockexit();
    if (at<s32>(0x00fb13a0) == 1) import_slot<s32(__stdcall *)(u32, s32)>(0x006fd24c)(import_slot<u32(__stdcall *)()>(0x006fd250)(), code);
    at<s32>(0x00fb139c) = 1;
    at<u8>(0x00fb1398) = static_cast<u8>(retcaller);
    if (!quick) {
        if (at<u32>(0x00fb3cb4)) {
            for (u32 entry = at<u32>(0x00fb3cb0) - 4; entry >= at<u32>(0x00fb3cb4); entry -= 4)   // (unsigned: an end below 4 wraps, as the original's)
                if (const Init f = at<Init>(entry)) f();
        }
        initterm(0x0072e100, 0x0072e10c);
    }
    initterm(0x0072e110, 0x0072e118);
    if (retcaller) {
        CRT_unlockexit();
        return;
    }
    at<s32>(0x00fb13a0) = 1;
    import_slot<void(__stdcall *)(s32)>(0x006fd244)(code);   // (ExitProcess: the original returns if it does)
}

void __cdecl CRT_exit(s32 code) { CRT_doexit(code, 0, 0); }
void __cdecl CRT__exit(s32 code) { CRT_doexit(code, 1, 0); }
void __cdecl CRT_lockexit() { CrtInner().lock(0xd); }
void __cdecl CRT_unlockexit() { CrtInner().unlock(0xd); }

}  // extern "C"
