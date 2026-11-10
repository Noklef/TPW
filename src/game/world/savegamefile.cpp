// A saved game's file: see savegamefile.h.
#include "savegamefile.h"

#include <cstdint>
#include <cstring>

#include "../core/vcall.h"
#include "../files/handles.h"
#include "../core/imports.h"

namespace {

AddrCalls g_calls;

const u32 kFileVtable = (u32)(uintptr_t)FileStream_vtable, kMemoryVtable = 0x006fd6a0, kUi = 0x007b51f0, kClock = 0x00785970;

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u32 level() { return global<u32>(0x007cf83c); }
u32 alloc(u32 size) { return g_calls.Cd(0x0067acc0, size); }
void release(u32 block) { g_calls.Cd(0x0067acb0, block); }
bool failed(u32 status) { return *reinterpret_cast<const s32 *>((uintptr_t)status) < 0; }

struct LocalStream {
    u32 vtable;
    void *file;
};

u32 self_of(LocalStream &stream) { return (u32)(uintptr_t)&stream; }
void make(LocalStream &stream, u32 vtable) {
    g_calls.Th(0x005f7180, self_of(stream));
    stream.vtable = vtable;
}
void close(LocalStream &stream) { g_calls.Th(0x005f71f0, self_of(stream)); }
void destroy(LocalStream &stream) { g_calls.Th(0x005f71e0, self_of(stream)); }
void seek(LocalStream &stream, u32 whence) { g_calls.Th(0x005f7210, self_of(stream), 0u, whence); }
s32 readInto(LocalStream &stream, u32 to, u32 size) { return stream.file ? vcall<s32>(stream.file, 4, to, size) : -1; }
s32 writeFrom(LocalStream &stream, u32 from, u32 size) { return stream.file ? vcall<s32>(stream.file, 8, from, size) : -1; }

// A header's field in full (the stream's status is 0 or 1: never the negative the original tests).
bool put(Stream *stream, const void *from, s32 size) {
    s32 status, done = 0;
    Stream_Write(&status, stream, from, size, &done);
    return done == size;
}
bool get(Stream *stream, void *to, s32 size) {
    s32 status, done = 0;
    Stream_Read(&status, stream, to, size, &done);
    return done == size;
}

const u32 kLanguageNames[] = {0x0074bdc8, 0x0074bdc0, 0x0074bdb8, 0x0074bdb0, 0x0074bda8, 0x0074bda0, 0x0074bd94};
const u32 kLanguages = 0x0078a460, kGameId = 0x00802080;

u8 languageNumber(u8 *text) {
    for (u8 i = 0; i < 7; ++i) {
        if ((u8)g_calls.Th(0x005f7980, (u32)(uintptr_t)text, kLanguageNames[i])) return i;
    }
    return 0;
}

bool headerWritten(Stream *stream, u8 *park, u8 *text) {
    u8 wide[0x500] = {};
    const u32 language = languageNumber(text);
    const u32 entry = kLanguages + language * 0x14;
    memcpy(wide, global<const void *>(entry + 8), global<u32>(entry + 4) * 2);
    const u8 number = (u8)language;
    if (!put(stream, &number, 1) || !put(stream, wide, 0x500)) return false;
    u8 id[0x100] = {};
    memcpy(id, reinterpret_cast<const void *>((uintptr_t)kGameId), 0x20);
    if (!put(stream, id, 0x100)) return false;
    const u32 version = global<u32(__stdcall *)(u32)>(IMPORT(Wsock32_htonl))(global<u32>(0x00749870));
    if (!put(stream, &version, 4)) return false;
    const u32 hasPark = park ? 1 : 0;
    if (!put(stream, &hasPark, 4)) return false;
    return !park || g_calls.Th(0x00418da0, (u32)(uintptr_t)park, stream, 1u, 1u, 0u) != 1;
}

}  // namespace

void SaveGame_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u32 __fastcall SaveGame_Save(u8 *self, void *, const char *name, u32 header, u32 mode) {
    u32 &error = *reinterpret_cast<u32 *>(self + 4);
    const u32 me = (u32)(uintptr_t)self;
    s32 status = 0;
    Stream *start = nullptr;
    if (failed(g_calls.Cd(0x005f5ad0, &status, &start, name, 0x40000010u))) {
        error = 1;
        return 0;
    }
    Stream_Write(&status, start, reinterpret_cast<const void *>((uintptr_t)0x006fd928), 4, nullptr);   // (its status is 0 or 1: never the negative the original tests)
    if (!g_calls.Th(0x00415f50, me, start, header)) {
        g_calls.Cd(0x005f5b80, &status, start);
        g_calls.Cd(0x005f5d60, &status, name);
        error = 3;
        return 0;
    }
    g_calls.Cd(0x005f5b80, &status, start);

    LocalStream memory;
    make(memory, kMemoryVtable);
    status = 0;
    if (failed(g_calls.Th(0x005f72c0, self_of(memory), &status, 0x200u))) {
        error = 2;
        destroy(memory);
        return 0;
    }
    g_calls.Th(0x00402e60, kClock);
    if (!g_calls.Th(0x004164c0, me, &memory, mode)) {
        close(memory);
        g_calls.Cd(0x005f5d60, &status, name);
        error = 5;
        destroy(memory);
        return 0;
    }
    const u32 size = g_calls.Th(0x00639100, self_of(memory));
    const u32 plain = alloc(size);
    const u32 packed = plain ? alloc(size) : 0;
    if (!packed) {
        close(memory);
        g_calls.Cd(0x005f5d60, &status, name);
        error = 2;
        destroy(memory);
        return 0;
    }
    seek(memory, 0);
    readInto(memory, plain, size);
    close(memory);
    const u32 length = g_calls.Cd(0x005f8050, plain, packed, size, 0u);
    release(plain);

    LocalStream file;
    make(file, kFileVtable);
    u32 result = 0;
    if (failed(g_calls.Th(0x005f7ed0, self_of(file), &status, name, 0xc0000030u))) {
        g_calls.Cd(0x005f5d60, &status, name);
        error = 1;
    } else {
        seek(file, 2);
        const u32 wrote = (u32)writeFrom(file, packed, length);
        release(packed);
        close(file);
        if (wrote == length) result = 1;
        else error = 3;
    }
    destroy(file);
    destroy(memory);
    return result;
}

u32 __fastcall SaveGame_Load(u8 *self, void *, const char *name, u32 header, u32 mode) {
    u32 &error = *reinterpret_cast<u32 *>(self + 4);
    const u32 me = (u32)(uintptr_t)self;
    LocalStream file;
    make(file, kFileVtable);
    s32 magic = 0;   // (the open's status too)
    if (failed(g_calls.Th(0x005f7ed0, self_of(file), &magic, name, 0x80000000u))) {
        error = 1;
        destroy(file);
        return 0;
    }
    if (readInto(file, (u32)(uintptr_t)&magic, 4) != 4) {
        close(file);
        error = 1;
        destroy(file);
        return 0;
    }
    if (mode == 2 && magic > 0x1f4) {
        error = 9;
        close(file);
        destroy(file);
        return 0;
    }
    if (!g_calls.Th(0x00416240, me, &file, mode, header)) {
        close(file);
        error = 7;
        destroy(file);
        return 0;
    }
    if (!mode) {
        destroy(file);
        return 1;
    }
    const u32 end = g_calls.Th(0x00639100, self_of(file));
    const u32 left = end - g_calls.Th(0x005f7210, self_of(file), 0u, 1u);
    if (!left) {
        error = 6;
        destroy(file);
        return 0;
    }
    const u32 packed = alloc(left);
    if ((u32)readInto(file, packed, left) != left) {
        release(packed);
        error = 4;
        destroy(file);
        return 0;
    }
    close(file);
    const u32 size = g_calls.Cd(0x005f8120, packed);
    if (!size) {
        release(packed);
        error = 6;
        destroy(file);
        return 0;
    }
    const u32 plain = alloc(size);
    g_calls.Cd(0x005f8100, packed, plain, 0u);
    release(packed);

    LocalStream memory;
    make(memory, kMemoryVtable);
    s32 status = 0;
    u32 result = 0;
    if (failed(g_calls.Th(0x005f72c0, self_of(memory), &status, size))) {
        release(plain);
        error = 2;
    } else if (writeFrom(memory, plain, size) < 0) {
        release(plain);
        close(memory);
        error = 3;
    } else {
        seek(memory, 0);
        release(plain);
        if (mode != 1) {
            g_calls.Cd(0x00457e10);
            g_calls.Th(0x004d8330, level() + 0x2d8);
            g_calls.Cd(0x0052f8d0, 0x80u, 0x80u);
            g_calls.Th(0x005191e0, level());
            g_calls.Th(0x00515fb0, level());
            g_calls.Th(0x00515f30, level());
            g_calls.Cd(0x0042a190);
            g_calls.Cd(0x005445d0);
            g_calls.Cd(0x00437150);
            g_calls.Cd(0x005584b0);
            g_calls.Cd(0x005996d0, 0u);
            g_calls.Th(0x004815d0, kUi);
        }
        const u32 read = g_calls.Th(0x00415270, me, &memory, mode);
        close(file);
        close(memory);
        if (mode != 1) SaveGame_AfterLoad(self);
        if (read) result = 1;
        else error = 6;
    }
    destroy(memory);
    destroy(file);
    return result;
}

u32 __fastcall SaveGame_WriteHeader(void *, void *, Stream *stream, u8 *park) {
    u8 text[0x14];
    g_calls.Th(0x005f60b0, (u32)(uintptr_t)text, g_calls.Th(0x0041a820, 0x0078b530u));
    const bool written = headerWritten(stream, park, text);
    g_calls.Th(0x005f61e0, (u32)(uintptr_t)text);
    return written ? 1 : 0;
}

u32 __fastcall SaveGame_ReadHeader(u8 *self, void *, Stream *stream, u32 mode, u8 *park) {
    u8 number;
    u8 wide[0x500];
    if (!get(stream, &number, 1) || !get(stream, wide, 0x500)) return 0;
    if ((u8)g_calls.Th(0x005f7e60, kLanguages + (u32)number * 0x14, wide)) return 0;
    u8 id[0x100];
    if (!get(stream, id, 0x100)) return 0;
    if (!g_calls.Th(0x0051ab60, level(), id, mode == 2 ? 1u : 0u)) {
        *reinterpret_cast<u32 *>(self + 4) = 8;
        return 0;
    }
    memcpy(reinterpret_cast<void *>((uintptr_t)kGameId), id, 0x20);
    u32 version = 0;
    if (!get(stream, &version, 4)) return 0;
    if (global<u32(__stdcall *)(u32)>(IMPORT(Wsock32_ntohl))(version) != global<u32>(0x00749870)) {
        *reinterpret_cast<u32 *>(self + 4) = 6;
        return 0;
    }
    u32 hasPark;
    if (!get(stream, &hasPark, 4)) return 0;
    if (!hasPark) return 1;
    u8 local[0x3d4];
    u8 *into = hasPark == 1 && !park ? local : park;
    return g_calls.Th(0x00418da0, (u32)(uintptr_t)into, stream, 0u, 1u, 0u) ? 0 : 1;
}

void __fastcall SaveGame_AfterLoad(void *) {
    g_calls.Cd(0x005508c0);
    g_calls.Cd(0x00550930);
    g_calls.Cd(0x0055e3b0);
    g_calls.Th(0x00519120, level());
    g_calls.Th(0x004d83a0, level() + 0x2d8);
    g_calls.Th(0x004d3e00, level() + 0x1d943c);
    g_calls.Cd(0x00450d10);
    global<u32>(0x00879070) = 1;
    g_calls.Th(0x00402e80, kClock);
    g_calls.Cd(0x00473440);
    g_calls.Cd(0x00468500);
    u32 &last = global<u32>(0x00789c20);
    s32 status;
    for (u32 font = global<u32>(0x00789c1c); font != last; font += 0x14) {
        const char *path = *reinterpret_cast<const char **>((uintptr_t)font + 8);
        global<s32(__stdcall *)(const char *)>(IMPORT(Gdi32_RemoveFontResourceA))(path);
        g_calls.Cd(0x005f5d60, &status, *reinterpret_cast<const char **>((uintptr_t)font + 8));
    }
    const u32 first = global<u32>(0x00789c1c), end = last;
    for (u32 font = first; font != end; font += 0x14) vcall<u32>(reinterpret_cast<void *>((uintptr_t)font), 0xc, 0u);
    last = first;
    g_calls.Th(0x004815e0, kUi);
    g_calls.Cd(0x005996d0, 1u);
    g_calls.Cd(0x0051fd80, global<u32>(0x0078d8e0) ? 0u : 1u);
    g_calls.Cd(0x00475f40);
    g_calls.Cd(0x0051c1c0, 0u);
}
}
