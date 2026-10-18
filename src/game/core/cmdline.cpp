// The game's command line: reimplementation of 0x40f000.
#include "cmdline.h"

#include <cstdint>

#include "crt.h"
#include "gameerror.h"

namespace {

template <typename T> T &global(uintptr_t address) { return *reinterpret_cast<T *>(address); }

s32 length(const char *s) {   // (inline repne scasb)
    s32 n = 0;
    while (s[n]) ++n;
    return n;
}

// Whether arg is a switch ('/' or '-') named name (any case).
bool is_switch(const char *arg, uintptr_t name) {
    return (arg[0] == '/' || arg[0] == '-') && Crt().stricmp(arg + 1, reinterpret_cast<const char *>(name)) == 0;
}

u32 no_error() { return global<u32>(0x00785de8) == 0 ? 1 : 0; }

}  // namespace

extern "C" {

u32 __cdecl CommandLine_Parse(s32 argc, char **argv) {
    for (s32 i = 1; i < argc; ++i) {
        const char *const arg = argv[i];
        if (is_switch(arg, 0x0074874c) || is_switch(arg, 0x00748744)) return 0;   // "v", "version"
        if (is_switch(arg, 0x00748738) || is_switch(arg, 0x00748730) || is_switch(arg, 0x00748728) || is_switch(arg, 0x0074871c))
            continue;   // "quickload", "nodebug", "noload", "SAVEDEBUG"
        if (is_switch(arg, 0x00748714)) {   // "flmouse"
            global<u32>(0x0078826c) = 1;
            continue;
        }
        if (is_switch(arg, 0x00748708)) {   // "bwcursor"
            global<u32>(0x00788270) = 1;
            continue;
        }
        bool named = false;
        if (!global<u32>(0x00788160)) {
            const char *const extension = global<const char *>(0x007888c8);
            const s32 n = length(arg), k = length(extension);
            named = n >= k && Crt().stricmp(arg + n - k, extension) == 0;
        }
        if (!named) {
            GameError_Set(reinterpret_cast<u8 *>(0x00785de8), nullptr, 2, 0, argv[i]);
            return no_error();
        }
        global<u32>(0x00788160) = 1;
        char *const to = reinterpret_cast<char *>(0x00788164);
        for (u32 c = 0; (to[c] = arg[c]) != 0; ++c) {
        }
    }
    return no_error();
}
}
