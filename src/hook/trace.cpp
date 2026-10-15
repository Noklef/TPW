#include "trace.h"

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <intrin.h>
#include <windows.h>

#include "log.h"
#include "patch.h"

#pragma intrinsic(_ReturnAddress)

namespace tpw::hook {

namespace {

#define CALLER (unsigned)(uintptr_t)_ReturnAddress()

// Bounded counts for the chatty ones.
LONG g_file_failures, g_exceptions, g_libraries;

const char *str(const char *s) { return s ? s : "(null)"; }

// The originals, filled in as the import slots are redirected.
decltype(&MessageBoxA) o_MessageBoxA;
decltype(&MessageBoxW) o_MessageBoxW;
decltype(&PostQuitMessage) o_PostQuitMessage;
decltype(&ExitProcess) o_ExitProcess;
decltype(&CreateWindowExA) o_CreateWindowExA;
decltype(&CreateMutexA) o_CreateMutexA;
decltype(&CreateFileA) o_CreateFileA;
decltype(&LoadLibraryA) o_LoadLibraryA;
decltype(&GetDriveTypeA) o_GetDriveTypeA;
decltype(&GetVolumeInformationA) o_GetVolumeInformationA;
decltype(&RegOpenKeyA) o_RegOpenKeyA;
decltype(&RegCreateKeyExA) o_RegCreateKeyExA;
decltype(&RegQueryValueExA) o_RegQueryValueExA;
using DirectDrawCreate_t = HRESULT(WINAPI *)(GUID *, void **, IUnknown *);
using DirectDrawEnumerateA_t = HRESULT(WINAPI *)(void *, void *);
using DirectSoundCreate_t = HRESULT(WINAPI *)(const GUID *, void **, IUnknown *);
DirectDrawCreate_t o_DirectDrawCreate;
DirectDrawEnumerateA_t o_DirectDrawEnumerateA;
DirectSoundCreate_t o_DirectSoundCreate;

int WINAPI t_MessageBoxA(HWND w, LPCSTR text, LPCSTR caption, UINT type) {
    log_line("[%6.1fs] MessageBoxA from %08X: \"%s\" / \"%s\" (type %X)", log_seconds(), CALLER, str(caption),
             str(text), type);
    const int r = o_MessageBoxA(w, text, caption, type);
    log_line("[%6.1fs]   -> button %d", log_seconds(), r);
    return r;
}

int WINAPI t_MessageBoxW(HWND w, LPCWSTR text, LPCWSTR caption, UINT type) {
    char t[512] = "(null)", c[256] = "(null)";
    if (text) WideCharToMultiByte(CP_ACP, 0, text, -1, t, sizeof t, nullptr, nullptr);
    if (caption) WideCharToMultiByte(CP_ACP, 0, caption, -1, c, sizeof c, nullptr, nullptr);
    log_line("[%6.1fs] MessageBoxW from %08X: \"%s\" / \"%s\" (type %X)", log_seconds(), CALLER, c, t, type);
    const int r = o_MessageBoxW(w, text, caption, type);
    log_line("[%6.1fs]   -> button %d", log_seconds(), r);
    return r;
}

void WINAPI t_PostQuitMessage(int code) {
    log_line("[%6.1fs] PostQuitMessage(%d) from %08X", log_seconds(), code, CALLER);
    o_PostQuitMessage(code);
}

void WINAPI t_ExitProcess(UINT code) {
    log_line("[%6.1fs] ExitProcess(%u) from %08X", log_seconds(), code, CALLER);
    o_ExitProcess(code);
}

HWND WINAPI t_CreateWindowExA(DWORD ex, LPCSTR cls, LPCSTR name, DWORD style, int x, int y, int w, int h,
                              HWND parent, HMENU menu, HINSTANCE inst, LPVOID param) {
    const HWND r = o_CreateWindowExA(ex, cls, name, style, x, y, w, h, parent, menu, inst, param);
    const DWORD e = GetLastError();
    log_line("[%6.1fs] CreateWindowExA(\"%s\", %dx%d, style %X) from %08X -> %p (error %u)", log_seconds(),
             HIWORD(cls) ? cls : "#atom", w, h, style, CALLER, r, r ? 0 : e);
    SetLastError(e);
    return r;
}

HANDLE WINAPI t_CreateMutexA(LPSECURITY_ATTRIBUTES sa, BOOL owner, LPCSTR name) {
    const HANDLE r = o_CreateMutexA(sa, owner, name);
    const DWORD e = GetLastError();
    log_line("[%6.1fs] CreateMutexA(\"%s\") from %08X -> %p (last error %u)", log_seconds(), str(name), CALLER, r, e);
    SetLastError(e);
    return r;
}

HANDLE WINAPI t_CreateFileA(LPCSTR name, DWORD access, DWORD share, LPSECURITY_ATTRIBUTES sa, DWORD disposition,
                            DWORD flags, HANDLE templ) {
    const HANDLE r = o_CreateFileA(name, access, share, sa, disposition, flags, templ);
    if (r == INVALID_HANDLE_VALUE) {
        const DWORD e = GetLastError();
        const LONG n = InterlockedIncrement(&g_file_failures);
        if (n <= 300)
            log_line("[%6.1fs] CreateFileA(\"%s\", access %X, disposition %u) from %08X FAILED (error %u)%s",
                     log_seconds(), str(name), access, disposition, CALLER, e,
                     n == 300 ? " -- no more file failures logged" : "");
        SetLastError(e);
    }
    return r;
}

HMODULE WINAPI t_LoadLibraryA(LPCSTR name) {
    const HMODULE r = o_LoadLibraryA(name);
    const DWORD e = GetLastError();
    if (InterlockedIncrement(&g_libraries) <= 100)
        log_line("[%6.1fs] LoadLibraryA(\"%s\") from %08X -> %p (error %u)", log_seconds(), str(name), CALLER, r,
                 r ? 0 : e);
    SetLastError(e);
    return r;
}

UINT WINAPI t_GetDriveTypeA(LPCSTR root) {
    const UINT r = o_GetDriveTypeA(root);
    log_line("[%6.1fs] GetDriveTypeA(\"%s\") from %08X -> %u", log_seconds(), str(root), CALLER, r);
    return r;
}

BOOL WINAPI t_GetVolumeInformationA(LPCSTR root, LPSTR label, DWORD label_size, LPDWORD serial, LPDWORD max_len,
                                    LPDWORD flags, LPSTR fs, DWORD fs_size) {
    const BOOL r = o_GetVolumeInformationA(root, label, label_size, serial, max_len, flags, fs, fs_size);
    const DWORD e = GetLastError();
    log_line("[%6.1fs] GetVolumeInformationA(\"%s\") from %08X -> %d, label \"%s\" (error %u)", log_seconds(),
             str(root), CALLER, r, r && label ? label : "", r ? 0 : e);
    SetLastError(e);
    return r;
}

LSTATUS WINAPI t_RegOpenKeyA(HKEY key, LPCSTR name, PHKEY out) {
    const LSTATUS r = o_RegOpenKeyA(key, name, out);
    log_line("[%6.1fs] RegOpenKeyA(%p, \"%s\") from %08X -> %ld", log_seconds(), key, str(name), CALLER, r);
    return r;
}

LSTATUS WINAPI t_RegCreateKeyExA(HKEY key, LPCSTR name, DWORD reserved, LPSTR cls, DWORD options, REGSAM sam,
                                 const LPSECURITY_ATTRIBUTES sa, PHKEY out, LPDWORD disposition) {
    const LSTATUS r = o_RegCreateKeyExA(key, name, reserved, cls, options, sam, sa, out, disposition);
    log_line("[%6.1fs] RegCreateKeyExA(%p, \"%s\", access %X) from %08X -> %ld (disposition %u)", log_seconds(), key,
             str(name), sam, CALLER, r, r == 0 && disposition ? *disposition : 0);
    return r;
}

LSTATUS WINAPI t_RegQueryValueExA(HKEY key, LPCSTR name, LPDWORD reserved, LPDWORD type, LPBYTE data, LPDWORD size) {
    const LSTATUS r = o_RegQueryValueExA(key, name, reserved, type, data, size);
    char value[80] = "";
    if (r == 0 && data && size && type) {
        if ((*type == REG_DWORD) && *size >= 4)
            wsprintfA(value, " = 0x%X", *reinterpret_cast<DWORD *>(data));
        else if (*type == REG_SZ)
            wsprintfA(value, " = \"%.60s\"", reinterpret_cast<const char *>(data));
    }
    log_line("[%6.1fs] RegQueryValueExA(%p, \"%s\") from %08X -> %ld (type %d)%s", log_seconds(), key, str(name),
             CALLER, r, type ? (int)*type : -1, value);
    return r;
}

HRESULT WINAPI t_DirectDrawEnumerateA(void *callback, void *context) {
    const HRESULT r = o_DirectDrawEnumerateA(callback, context);
    log_line("[%6.1fs] DirectDrawEnumerateA from %08X -> %08X", log_seconds(), CALLER, (unsigned)r);
    return r;
}

HRESULT WINAPI t_DirectDrawCreate(GUID *guid, void **out, IUnknown *outer) {
    const HRESULT r = o_DirectDrawCreate(guid, out, outer);
    log_line("[%6.1fs] DirectDrawCreate(%s) from %08X -> %08X", log_seconds(), guid ? "device" : "primary", CALLER,
             (unsigned)r);
    return r;
}

HRESULT WINAPI t_DirectSoundCreate(const GUID *guid, void **out, IUnknown *outer) {
    const HRESULT r = o_DirectSoundCreate(guid, out, outer);
    log_line("[%6.1fs] DirectSoundCreate from %08X -> %08X", log_seconds(), CALLER, (unsigned)r);
    return r;
}

LONG CALLBACK on_exception(EXCEPTION_POINTERS *info) {
    const EXCEPTION_RECORD *e = info->ExceptionRecord;
    // C++ throws (E06D7363) and thread naming (406D1388) are routine.
    if (e->ExceptionCode == 0xE06D7363 || e->ExceptionCode == 0x406D1388) return EXCEPTION_CONTINUE_SEARCH;
    const LONG n = InterlockedIncrement(&g_exceptions);
    if (n <= 30) {
        const CONTEXT *c = info->ContextRecord;
        char detail[64] = "";
        if (e->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && e->NumberParameters >= 2)
            wsprintfA(detail, " (%s %08X)", e->ExceptionInformation[0] == 0 ? "reading" :
                                            e->ExceptionInformation[0] == 1 ? "writing" : "executing",
                      (unsigned)e->ExceptionInformation[1]);
        log_line("[%6.1fs] exception %08X at %08X%s, thread %u; eax %08X ecx %08X edx %08X esi %08X edi %08X "
                 "esp %08X [esp] %08X",
                 log_seconds(), e->ExceptionCode, (unsigned)(uintptr_t)e->ExceptionAddress, detail,
                 GetCurrentThreadId(), c->Eax, c->Ecx, c->Edx, c->Esi, c->Edi, c->Esp,
                 IsBadReadPtr((void *)(uintptr_t)c->Esp, 4) ? 0 : *(DWORD *)(uintptr_t)c->Esp);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

// The game's debug print (0x5da3c0) is a bare RET in the retail build, but its 1688 call sites
// still pass their messages ("Error: Unable to open text resource at %s"). The stub below
// takes its place: it saves every register and the flags, logs, and returns like the RET did,
// so callers that use it as an empty function pointer (vtables, zfree, a sprite opcode) are
// unaffected. Its argument 0 is sometimes a condition or channel number, so the format is
// whichever of the first two arguments points at text inside tp.exe.
const uintptr_t kDebugPrint = 0x5da3c0;
const std::uint8_t kDebugPrintBytes[5] = {0xc3, 0x90, 0x90, 0x90, 0x90};
LONG g_debug_lines;
uintptr_t g_image_end;

bool is_image_text(DWORD p) {
    if (p < 0x400000 || p >= g_image_end) return false;
    const auto *s = reinterpret_cast<const unsigned char *>(p);
    int n = 0;
    for (; n < 300 && s[n]; ++n)
        if ((s[n] < 0x20 && s[n] != '\n' && s[n] != '\r' && s[n] != '\t') || s[n] >= 0x7f) return false;
    return n >= 3 && n < 300;
}

void ignore_invalid_parameter(const wchar_t *, const wchar_t *, const wchar_t *, unsigned, uintptr_t) {}

extern "C" void __cdecl debug_print_log(const DWORD *stack) {  // stack[0]: return address
    __try {
        int format = 0;
        if (is_image_text(stack[1])) format = 1;
        else if (is_image_text(stack[2])) format = 2;
        if (!format) return;
        const LONG n = InterlockedIncrement(&g_debug_lines);
        if (n > 2000) return;
        char text[512];
        _vsnprintf_s(text, sizeof text, _TRUNCATE, reinterpret_cast<const char *>(stack[format]),
                     reinterpret_cast<va_list>(const_cast<DWORD *>(&stack[format + 1])));
        for (size_t len = std::strlen(text); len && (text[len - 1] == '\n' || text[len - 1] == '\r');)
            text[--len] = 0;
        log_line("[%6.1fs] game debug from %08X: %s%s", log_seconds(), stack[0], text,
                 n == 2000 ? " -- no more game debug lines logged" : "");
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        log_line("[%6.1fs] game debug from %08X: (unreadable arguments)", log_seconds(), stack[0]);
    }
}

__declspec(naked) void debug_print_stub() {
    __asm {
        pushfd
        pushad
        lea eax, [esp + 36]  // past pushad (32) and pushfd (4): the return address
        push eax
        call debug_print_log
        add esp, 4
        popad
        popfd
        ret
    }
}

struct Redirect {
    const char *dll;
    const char *name;  // nullptr: by ordinal
    WORD ordinal;
    void *replacement;
    void **original;
};

#define BY_NAME(dll, fn) {dll, #fn, 0, (void *)&t_##fn, (void **)&o_##fn}

const Redirect kRedirects[] = {
    BY_NAME("user32.dll", MessageBoxA),
    BY_NAME("user32.dll", MessageBoxW),
    BY_NAME("user32.dll", PostQuitMessage),
    BY_NAME("user32.dll", CreateWindowExA),
    BY_NAME("kernel32.dll", ExitProcess),
    BY_NAME("kernel32.dll", CreateMutexA),
    BY_NAME("kernel32.dll", CreateFileA),
    BY_NAME("kernel32.dll", LoadLibraryA),
    BY_NAME("kernel32.dll", GetDriveTypeA),
    BY_NAME("kernel32.dll", GetVolumeInformationA),
    BY_NAME("advapi32.dll", RegOpenKeyA),
    BY_NAME("advapi32.dll", RegCreateKeyExA),
    BY_NAME("advapi32.dll", RegQueryValueExA),
    BY_NAME("ddraw.dll", DirectDrawEnumerateA),
    BY_NAME("ddraw.dll", DirectDrawCreate),
    {"dsound.dll", nullptr, 1, (void *)&t_DirectSoundCreate, (void **)&o_DirectSoundCreate},
};

void write_slot(void **slot, void *value) {
    DWORD old;
    if (VirtualProtect(slot, sizeof *slot, PAGE_READWRITE, &old)) {
        *slot = value;
        VirtualProtect(slot, sizeof *slot, old, &old);
    }
}

}  // namespace

void install_trace() {
    auto *base = reinterpret_cast<BYTE *>(GetModuleHandleA(nullptr));
    auto *nt = reinterpret_cast<IMAGE_NT_HEADERS *>(base + reinterpret_cast<IMAGE_DOS_HEADER *>(base)->e_lfanew);
    const IMAGE_DATA_DIRECTORY &dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    int redirected = 0, unbound = 0;
    for (auto *d = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR *>(base + dir.VirtualAddress); d->Name; ++d) {
        const char *dll = reinterpret_cast<const char *>(base + d->Name);
        auto *names = reinterpret_cast<IMAGE_THUNK_DATA *>(base + (d->OriginalFirstThunk ? d->OriginalFirstThunk
                                                                                         : d->FirstThunk));
        auto *slots = reinterpret_cast<void **>(base + d->FirstThunk);
        for (; names->u1.AddressOfData; ++names, ++slots) {
            const bool ordinal = IMAGE_SNAP_BY_ORDINAL(names->u1.Ordinal);
            const char *name =
                ordinal ? nullptr
                        : reinterpret_cast<IMAGE_IMPORT_BY_NAME *>(base + names->u1.AddressOfData)->Name;
            for (const Redirect &r : kRedirects) {
                if (lstrcmpiA(r.dll, dll) != 0) continue;
                if (r.name ? (!name || std::strcmp(r.name, name) != 0)
                           : (!ordinal || IMAGE_ORDINAL(names->u1.Ordinal) != r.ordinal))
                    continue;
                // A bound slot is kept as the original (it may be a compatibility shim's); an
                // unbound one still holds an offset into the exe, so ask the module instead.
                void *original = *slots;
                const uintptr_t value = reinterpret_cast<uintptr_t>(original);
                if (value < reinterpret_cast<uintptr_t>(base) + nt->OptionalHeader.SizeOfImage) {
                    HMODULE module = GetModuleHandleA(dll);
                    original = module ? (void *)GetProcAddress(module, r.name ? r.name : MAKEINTRESOURCEA(r.ordinal))
                                      : nullptr;
                    ++unbound;
                }
                if (!original) {
                    log_line("startup trace: %s!%s not found, left alone", dll, r.name ? r.name : "#ordinal");
                    continue;
                }
                *r.original = original;
                write_slot(slots, r.replacement);
                ++redirected;
            }
        }
    }
    AddVectoredExceptionHandler(1, on_exception);

    g_image_end = reinterpret_cast<uintptr_t>(base) + nt->OptionalHeader.SizeOfImage;
    _set_invalid_parameter_handler(ignore_invalid_parameter);  // a bad game format must not end the process
    const bool debug = kDebugPrint + 5 <= g_image_end &&
                       patch_jump(reinterpret_cast<void *>(kDebugPrint), (void *)&debug_print_stub,
                                  kDebugPrintBytes) == PatchResult::Ok;
    log_line("startup trace: game debug messages (0x5da3c0) %s", debug ? "logged" : "NOT logged (bytes differ)");
    log_line("startup trace: %d imports redirected, %d of them not yet bound (tpw.ini [tpw] trace=0 turns this off)",
             redirected, unbound);
}

}  // namespace tpw::hook
