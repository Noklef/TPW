#include "crtstartup.h"

#include <cstdint>

namespace {

bool lead_byte(char c) { return reinterpret_cast<const u8 *>((uintptr_t)0x00fb1669)[static_cast<u8>(c)] & 4; }

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

}  // extern "C"
