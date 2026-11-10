#include "crtstartup.h"

#include <excpt.h>

#include <cstdint>

#include "addrcalls.h"
#include "crt.h"
#include "crtmbstring.h"
#include "imports.h"

namespace {

bool lead_byte(char c) { return (&CRT_mbctype[1])[static_cast<u8>(c)] & 4; }

template <typename T> T &at(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename F> F import_slot(u32 slot) { return at<F>(slot); }

AddrCalls g_startCalls;

using Init = void(__cdecl *)();
void initterm(u32 begin, u32 end) { CRT_initterm(reinterpret_cast<Init *>((uintptr_t)begin), reinterpret_cast<Init *>((uintptr_t)end)); }

}  // namespace

void CRT_SetStartupCalls(const AddrCall *table, u32 count) { g_startCalls.Set(table, count); }

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

void __cdecl CRT_setargv() {
    char *const name = CRT_pgmname;
    import_slot<u32(__stdcall *)(void *, char *, u32)>(IMPORT(Kernel32_GetModuleFileNameA))(nullptr, name, 0x104);    // GetModuleFileNameA
    CRT_pgmptr = name;
    const char *line = at<const char *>(0x00fb3cb8);
    if (!*line) line = name;
    s32 count, size;
    CRT_parse_cmdline(line, nullptr, nullptr, &count, &size);
    char **const argv = static_cast<char **>(CRT_malloc(static_cast<u32>(size + count * 4)));
    if (!argv) CrtInner().amsgExit(8);
    CRT_parse_cmdline(line, argv, reinterpret_cast<char *>(argv + count), &count, &size);
    CRT_argv = argv;
    CRT_argc = count - 1;
}

void __cdecl CRT_cinit() {
    if (const Init fpinit = at<Init>(0x0077e718)) fpinit();
    initterm(0x0072e0ec, 0x0072e0fc);
    initterm(0x00727000, 0x0072e0e8);
}

void __cdecl CRT_doexit(s32 code, s32 quick, s32 retcaller) {
    CRT_lockexit();
    if (CRT_C_Exit_Done == 1) import_slot<s32(__stdcall *)(u32, s32)>(IMPORT(Kernel32_TerminateProcess))(import_slot<u32(__stdcall *)()>(IMPORT(Kernel32_GetCurrentProcess))(), code);
    CRT_C_Termination_Done = 1;
    CRT_exitflag = static_cast<u8>(retcaller);
    if (!quick) {
        if (CRT_onexitbegin) {
            for (u32 entry = CRT_onexitend - 4; entry >= CRT_onexitbegin; entry -= 4)   // (unsigned: an end below 4 wraps, as the original's)
                if (const Init f = at<Init>(entry)) f();
        }
        initterm(0x0072e100, 0x0072e10c);
    }
    initterm(0x0072e110, 0x0072e118);
    if (retcaller) {
        CRT_unlockexit();
        return;
    }
    CRT_C_Exit_Done = 1;
    import_slot<void(__stdcall *)(s32)>(IMPORT(Kernel32_ExitProcess))(code);   // (ExitProcess: the original returns if it does)
}

void __cdecl CRT_exit(s32 code) { CRT_doexit(code, 0, 0); }
void __cdecl CRT__exit(s32 code) { CRT_doexit(code, 1, 0); }
void __cdecl CRT_lockexit() { CrtInner().lock(0xd); }
void __cdecl CRT_unlockexit() { CrtInner().unlock(0xd); }

void __cdecl CRT_WinMainCRTStartup() {
    const u32 version = import_slot<u32(__stdcall *)()>(IMPORT(Kernel32_GetVersion))();   // GetVersion
    const u32 minor = version >> 8 & 0xff, major = version & 0xff;
    at<u32>(0x00fb1370) = minor;
    at<u32>(0x00fb136c) = major;
    at<u32>(0x00fb1368) = (major << 8) + minor;
    at<u32>(0x00fb1364) = version >> 16;
    if (!g_startCalls.Cd(0x006826b0)) g_startCalls.Cd(0x0067eff0, 0x1c);   // _heap_init, fast_error_exit
    if (!g_startCalls.Cd(0x006823e0)) g_startCalls.Cd(0x0067eff0, 0x10);   // _mtinit
    u32 code = 0;
    __try {
        g_startCalls.Cd(0x00687230);   // _ioinit
        g_startCalls.Cd(0x0068a160);   // the multibyte table
        at<u32>(0x00fb3cb8) = import_slot<u32(__stdcall *)()>(IMPORT(Kernel32_GetCommandLineA))();   // GetCommandLineA
        const u32 environment = g_startCalls.Cd(0x00689b00);
        at<u32>(0x00fb1354) = environment;
        if (!environment || !at<u32>(0x00fb3cb8)) g_startCalls.Cd(0x0067f920, -1);   // exit
        g_startCalls.Cd(0x00689850);   // _setargv
        g_startCalls.Cd(0x00689760);   // _setenvp
        g_startCalls.Cd(0x0067f8f0);   // _cinit
        const u8 *line = at<const u8 *>(0x00fb3cb8);
        if (*line == '"') {
            for (;;) {
                const u8 c = *++line;
                if (c == '"' || !c) break;
                if (g_startCalls.Cd(0x00689700, static_cast<u32>(c))) ++line;   // _ismbblead
            }
            if (*line == '"') ++line;
        } else {
            while (*line > ' ') ++line;
        }
        while (*line && *line <= ' ') ++line;
        u8 info[0x44];   // STARTUPINFOA: only its dwFlags cleared
        *reinterpret_cast<u32 *>(info + 0x2c) = 0;
        import_slot<void(__stdcall *)(u8 *)>(IMPORT(Kernel32_GetStartupInfoA))(info);   // GetStartupInfoA
        const u32 show = info[0x2c] & 1 ? *reinterpret_cast<const u16 *>(info + 0x30) : 10u;   // STARTF_USESHOWWINDOW: wShowWindow, else SW_SHOWDEFAULT
        const u32 module = import_slot<u32(__stdcall *)(u32)>(IMPORT(Kernel32_GetModuleHandleA))(0);   // GetModuleHandleA
        g_startCalls.Cd(0x0067f920, g_startCalls.St(0x0045a960, module, 0u, reinterpret_cast<uintptr_t>(line), show));   // exit(WinMain(...))
    } __except (code = GetExceptionCode(), static_cast<s32>(g_startCalls.Cd(0x00688800, code, GetExceptionInformation()))) {   // _XcptFilter
        g_startCalls.Cd(0x0067f940, code);   // _exit
    }
}

}  // extern "C"
