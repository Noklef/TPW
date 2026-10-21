#include "islandcamera.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/string.h"
#include "../core/vregs.h"
#include "../core/vwrap.h"
#include "../files/file.h"
#include "../files/vfs.h"

namespace {

// The float after the line's first '(' up to the delimiter (String_ReadToken, then atof as a float).
float number(const char **cursor, u32 delimiter) {
    String token;
    String_ReadToken(&token, cursor, delimiter);
    const float value = (float)Crt().atof(token.data);
    String_Dtor(&token);
    return value;
}

void open_paren(const char **cursor) {
    while (**cursor != '(') ++*cursor;
    ++*cursor;
}

}  // namespace

extern "C" {

void __fastcall IslandCamera_LoadSettings(u8 *settings) {
    String path;
    String_CtorCopy(&path, nullptr, reinterpret_cast<const StringView *>((uintptr_t)0x00f855b8));
    String_Append(&path, nullptr, reinterpret_cast<const StringView *>((uintptr_t)0x00f855d0));
    u8 *const stream = reinterpret_cast<u8 *>(VFS_Open(path.data));
    Stream_SeekFailed(stream, 0, 2);
    const u32 size = Object_CallSlot14(stream);
    char *const text = static_cast<char *>(Mem_GlobalAlloc(size));
    Stream_SeekFailed(stream, 0, 0);
    Object_CallSlot8Product((u32)(uintptr_t)text, 1, size, stream);
    Object_FinishAndDelete(stream);
    char *const end = text + size;
    for (char *p = text; p < end; ++p)
        if (*p == '\n') *p = 0;
    float *const f = reinterpret_cast<float *>(settings);
    for (const char *cursor = text; cursor < end;) {
        static const struct {
            u32 key, length, offset;
        } kFloats[] = {{0x00774d40, 10, 4}, {0x00774d34, 10, 8}, {0x00774d28, 11, 0xc}, {0x00774d18, 15, 0x10}, {0x00774d08, 15, 0x14}, {0x00774cf8, 14, 0x18}};
        bool matched = false;
        for (const auto &k : kFloats) {
            if (Crt().strncmp(reinterpret_cast<const char *>((uintptr_t)k.key), cursor, k.length) == 0) {   // ISLANDFOV( .. GLOBERADIUSIN(
                open_paren(&cursor);
                f[k.offset / 4] = number(&cursor, 0x29);
                matched = true;
                break;
            }
        }
        if (!matched && Crt().strncmp(reinterpret_cast<const char *>((uintptr_t)0x00774ce0), cursor, 0x15) == 0) {   // ISLANDCAMERAPOSITION(
            open_paren(&cursor);
            String token;
            String_ReadToken(&token, &cursor, 0x2c);
            const s32 index = Crt().atoi(token.data);
            String_Dtor(&token);
            const float a = number(&cursor, 0x2c);
            const float b = number(&cursor, 0x29);
            if (index >= 0 && index < 10) {
                f[0x1c / 4 + index] = a;
                f[0x44 / 4 + index] = b;
            }
        }
        while (*cursor) ++cursor;
        ++cursor;
    }
    Mem_GlobalFree(text);
    String_Dtor(&path);
}
}
