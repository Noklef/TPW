#include "screen.h"

#include "../core/globalmem.h"
#include "../files/quickload.h"
#include "../math/sintable.h"
#include "frame.h"

namespace {

template <typename T = u32> T &global(u32 address) { return *reinterpret_cast<T *>(address); }

struct Size {
    u32 width, height;
};

// The size from the option flags (0x78d968 == 1), resolution setting or 0x78d8dc; false: keep.
bool pick(Size &size) {
    const Size k160{0xa0, 0x78}, k320{0x140, 0xf0}, k400{400, 300}, k512{0x200, 0x180}, k640{0x280, 0x1e0},
        k800{800, 600}, k1024{0x400, 0x300}, k1280{0x500, 0x400}, k1600{0x640, 0x4b0}, k2048{0x800, 0x600};
    if (global<s32>(0x0078d968) == 1) {
        const u32 options = global(0x007a1a8c);
        if (options & 0x20000000 && !(options & 1)) size = k160;
        else if (options & 0x40000000) size = k320;
        else if (options & 0x80000000) size = k400;
        else if (options & 8) size = k512;
        else if (options & 0x10) size = k640;
        else if (options & 0x20) size = k800;
        else if (options & 0x40) size = k1024;
        else if (options & 0x80) size = k1280;
        else if (options & 0x100) size = k1600;
        else size = k640;
        return true;
    }
    const s32 resolution = global<s32>(0x007858ec);  // gBalanceResolution.mRESOLUTION
    if (resolution <= 0) {
        const s32 fallback = global<s32>(0x0078d8dc);
        size = fallback == 0 ? k512 : fallback == 2 ? k800 : k640;
        return true;
    }
    switch (resolution) {
    case 1: size = k512; return true;
    case 2: size = k640; return true;
    case 3: size = k800; return true;
    case 4: size = k1024; return true;
    case 5: size = k1280; return true;
    case 6: size = k1600; return true;
    case 7: size = k2048; return true;
    default: return false;  // sic: the size stays
    }
}

}  // namespace

extern "C" {

void __cdecl Screen_Init() {
    global(0x0079d73c) = global(0x0079cb6c) + 1;
    char **const table = static_cast<char **>(GlobalMem_Alloc(4, 1));
    global<char **>(0x0079cb4c) = table;
    char *const text = static_cast<char *>(GlobalMem_Alloc(global(0x0079d73c), 1));
    global<char *>(0x0079d25c) = text;
    *table = text;
    const char *from = global<const char *>(0x0079cb70);
    char *to = *global<char **>(0x0079cb4c);
    do *to++ = *from; while (*from++);  // sic: the whole string, whatever the block holds

    Size size{global(0x007a1968), global(0x007a1930)};
    if (pick(size)) {
        global(0x007a1968) = size.width;
        global(0x007a1930) = size.height;
    }
    const float half = 0.5f;  // 0x6fe230
    const float cx = static_cast<float>(static_cast<double>(global(0x007a1968)) * half);
    const float cy = static_cast<float>(static_cast<double>(global(0x007a1930)) * half);
    global(0x007a1a14) |= 8;
    global<float>(0x007a0c9c) = cx;
    global<float>(0x007a162c) = cy;
    global<float>(0x007a192c) = cx;
    global<float>(0x007a194c) = cy;
    Frame_ResetBounds();
    Quickload_Init();
}

void __cdecl Screen_Shutdown() {
    SinTable_Build(0x1000);
    Quickload_Free();
    if (global(0x0079d25c)) {
        GlobalMem_Free(global<void *>(0x0079d25c));
        global(0x0079d25c) = 0;
    }
    if (global(0x0079cb4c)) {
        GlobalMem_Free(global<void *>(0x0079cb4c));
        global(0x0079cb4c) = 0;
    }
}
}
