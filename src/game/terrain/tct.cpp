#include "tct.h"

#include <cstdint>
#include <cstring>

#include "../core/crt.h"
#include "../core/globalmem.h"
#include "../core/vwrap.h"
#include "../files/textscan.h"
#include "../files/vfs.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

bool digit(char c) { return (s8)c >= '0' && (s8)c <= '9'; }
bool letter(char c) { return ((s8)c >= 'A' && (s8)c <= 'Z') || ((s8)c >= 'a' && (s8)c <= 'z'); }

// To the end of a comment's line: past its '\n' (0), or 1 when the text ends first.
u32 past_line(TextScan &scan) {
    for (;;) {
        if (*scan.next == '\n') {
            if (scan.pos >= scan.length)
                return 1;
            ++scan.next;
            ++scan.pos;
            return 0;
        }
        if (scan.pos >= scan.length)
            return 1;
        ++scan.next;
        ++scan.pos;
    }
}

// A run of characters for which take holds into token (up to 255): true when the text ran out.
template <typename Take> bool read_token(TextScan &scan, char *token, Take take) {
    u32 n = 0;
    token[0] = 0;
    if (scan.pos >= scan.length)
        return true;
    for (;;) {
        if (!take())
            return false;
        token[n++] = *scan.next;
        token[n] = 0;
        if (TextScan_Step(&scan))
            return true;
        if (n >= 0xff)
            return false;
    }
}
}

extern "C" u32 __cdecl TerrainSets_Load(const char *name, u8 *sets) {
    const CrtEnv &crt = Crt();
    u32 tiles = 0;
    u8 *cursor = nullptr;
    for (u32 pass = 0; pass < 2; ++pass) {
        const bool place = pass != 0;
        if (!place) {
            tiles = 0;
        } else {
            const u32 bytes = at<u32>(sets, 0xc);
            at<u32>(sets, 0x14) = 0;
            at<u32>(sets, 8) = tiles;
            cursor = static_cast<u8 *>(GlobalMem_Alloc(bytes, 1));
            at<u8 *>(sets, 0x10) = cursor;
            u8 *group = at<u8 *>(sets, 4);
            for (u32 n = at<u32>(sets, 0); n; --n, group += 0xc) {
                const u32 count = at<u32>(group, 4);
                at<u8 *>(group, 8) = cursor;
                cursor += count * 0xc;
            }
        }
        u8 *group = at<u8 *>(sets, 4);
        for (u32 n = at<u32>(sets, 0); n; --n, group += 0xc)
            at<u32>(group, 4) = 0;
        u8 *const stream = reinterpret_cast<u8 *>(VFS_Open(name));   // (0x74c8ec, "r", pushed too)
        if (!stream)
            continue;
        const u32 size = Object_CallSlot14(stream);
        u8 *const text = static_cast<u8 *>(GlobalMem_Alloc(size, 1));
        Object_CallSlot8Product((u32)(uintptr_t)text, size, 1, stream);
        Object_FinishAndDelete(stream);
        TextScan scan = {reinterpret_cast<const char *>(text), 0, size};
        const char *const *const at_next = &scan.next;
        u8 *current = nullptr;
        char token[0x100];
        for (;;) {
            const char c = *scan.next;
            if (c != '#' && c != ';' && !TextScan_IsAlpha(at_next) && !digit(*scan.next) && !TextScan_IsPunct(at_next)) {
                if (TextScan_Step(&scan))
                    break;
                continue;
            }
            const char first = *scan.next;
            if (first == '#' || first == ';') {
                if (past_line(scan))
                    break;
                continue;
            }
            if (letter(first)) {   // a group's name
                const bool end = read_token(scan, token, [&] { return TextScan_IsAlpha(at_next) || digit(*scan.next); });
                current = nullptr;
                u8 *g = at<u8 *>(sets, 4);
                for (u32 n = at<u32>(sets, 0); n; --n, g += 0xc)
                    if (!crt.stricmp(at<const char *>(g, 0), token)) {
                        current = g;
                        break;
                    }
                if (end)
                    break;
                continue;
            }
            if (digit(first)) {   // a tile: its number and name
                char number[0xc];
                const u32 out = TextScan_Read(&scan, number, 0xc, 0, 1, 1, 0);
                const s32 value = crt.atoi(number);
                if (out)
                    break;
                bool stop = false;
                for (;;) {
                    const char d = *scan.next;
                    if (d == '#' || d == ';' || letter(d) || TextScan_IsDigit(at_next) || TextScan_IsPunct(at_next))
                        break;
                    if (TextScan_Step(&scan)) {
                        stop = true;
                        break;
                    }
                }
                if (stop)
                    break;
                const bool end = read_token(scan, token, [&] {
                    return TextScan_IsAlpha(at_next) || TextScan_IsDigit(at_next) || TextScan_IsPunct(at_next);
                });
                if (current) {
                    const u32 length = (u32)std::strlen(token);
                    if (place) {
                        at<s32>(at<u8 *>(current, 8), 0xc * at<u32>(current, 4)) = value;
                        at<u8 *>(at<u8 *>(current, 8), 0xc * at<u32>(current, 4) + 4) = cursor;
                        std::memcpy(at<u8 *>(at<u8 *>(current, 8), 0xc * at<u32>(current, 4) + 4), token, length + 1);
                        cursor += length + 1;
                        ++at<u32>(sets, 0x14);
                    } else {
                        at<u32>(sets, 0xc) += 0xc;
                        at<u32>(sets, 0xc) += length + 1;
                        ++tiles;
                    }
                    ++at<u32>(current, 4);
                }
                if (end)
                    break;
                continue;
            }
            u32 done;   // other punctuation: the line passed over
            for (;;) {
                if (*scan.next == '\n') {
                    done = TextScan_Step(&scan);
                    break;
                }
                if (TextScan_Step(&scan)) {
                    done = 1;
                    break;
                }
            }
            if (done)
                break;
        }
        GlobalMem_Free(text);
    }
    return 1;
}
