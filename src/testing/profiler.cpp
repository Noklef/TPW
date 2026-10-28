#include "profiler.h"

#include "log.h"

namespace tpw::testing {

namespace {

constexpr DWORD kGameText = 0x401000, kGameTextEnd = 0x6e7000;
constexpr DWORD kDllBuckets = 0x40000;   // 4 MB of the DLL's image in 16-byte buckets
constexpr int kModules = 32, kTop = 40;

DWORD g_main_thread;
DWORD g_dll_base, g_dll_size;
volatile LONG g_samples;
DWORD g_game[(kGameTextEnd - kGameText) / 16];
DWORD g_dll[kDllBuckets];
struct Module {
    HMODULE handle;
    DWORD count;
};
Module g_modules[kModules];
DWORD g_unknown;

void count_module(DWORD eip) {
    HMODULE module = nullptr;
    if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            reinterpret_cast<LPCSTR>(static_cast<ULONG_PTR>(eip)), &module)) {
        ++g_unknown;
        return;
    }
    for (Module &m : g_modules) {
        if (m.handle == module || !m.handle) {
            m.handle = module;
            ++m.count;
            return;
        }
    }
    ++g_unknown;
}

DWORD WINAPI sample(LPVOID) {
    HANDLE thread = OpenThread(THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME, FALSE, g_main_thread);
    if (!thread) return 0;
    for (;;) {
        CONTEXT context = {};
        context.ContextFlags = CONTEXT_CONTROL;
        if (SuspendThread(thread) == (DWORD)-1) break;
        const BOOL ok = GetThreadContext(thread, &context);
        ResumeThread(thread);
        if (!ok) break;
        const DWORD eip = context.Eip;
        if (eip >= kGameText && eip < kGameTextEnd) {
            ++g_game[(eip - kGameText) / 16];
        } else if (eip - g_dll_base < g_dll_size && (eip - g_dll_base) / 16 < kDllBuckets) {
            ++g_dll[(eip - g_dll_base) / 16];
        } else {
            count_module(eip);
        }
        InterlockedIncrement(&g_samples);
        Sleep(1);
    }
    CloseHandle(thread);
    return 0;
}

// The kTop busiest of `counts` (n buckets of 16 bytes from `origin`), busiest first.
void log_top(const char *label, const DWORD *counts, DWORD n, DWORD origin, DWORD total) {
    DWORD top[kTop] = {}, at[kTop] = {};
    for (DWORD i = 0; i < n; ++i) {
        if (counts[i] <= top[kTop - 1]) continue;
        int k = kTop - 1;
        for (; k > 0 && counts[i] > top[k - 1]; --k) {
            top[k] = top[k - 1];
            at[k] = at[k - 1];
        }
        top[k] = counts[i];
        at[k] = i;
    }
    for (int k = 0; k < kTop && top[k]; ++k)
        log_line("  %s %08X  %6u  %5.2f%%", label, (unsigned)(origin + 16 * at[k]), (unsigned)top[k], 100.0 * top[k] / total);
}

}  // namespace

void start_profiler(HINSTANCE dll) {
    g_main_thread = GetCurrentThreadId();
    const auto *self = reinterpret_cast<const BYTE *>(dll);
    const auto *nt = reinterpret_cast<const IMAGE_NT_HEADERS *>(
        self + reinterpret_cast<const IMAGE_DOS_HEADER *>(self)->e_lfanew);
    g_dll_base = static_cast<DWORD>(reinterpret_cast<ULONG_PTR>(dll));
    g_dll_size = nt->OptionalHeader.SizeOfImage;
    if (HANDLE sampler = CreateThread(nullptr, 0, sample, nullptr, 0, nullptr)) CloseHandle(sampler);
}

void log_profile() {
    const DWORD total = static_cast<DWORD>(g_samples);
    if (!total) return;
    DWORD game = 0, dll = 0;
    for (DWORD c : g_game) game += c;
    for (DWORD c : g_dll) dll += c;
    log_line("[%6.1fs] profile: %u samples of the main thread; game code %.1f%%, hook DLL (base %08X) %.1f%%, other modules:",
             log_seconds(), (unsigned)total, 100.0 * game / total, (unsigned)g_dll_base, 100.0 * dll / total);
    for (const Module &m : g_modules) {
        if (!m.handle) break;
        char name[MAX_PATH] = "?";
        GetModuleFileNameA(m.handle, name, MAX_PATH);
        const char *base = name;
        for (const char *p = name; *p; ++p)
            if (*p == '\\') base = p + 1;
        log_line("  module %-24s %6u  %5.2f%%", base, (unsigned)m.count, 100.0 * m.count / total);
    }
    if (g_unknown) log_line("  module (none)                   %6u  %5.2f%%", (unsigned)g_unknown, 100.0 * g_unknown / total);
    log_line("  busiest 16-byte buckets in the game's code (address) and the hook DLL (RVA):");
    log_top("game", g_game, (kGameTextEnd - kGameText) / 16, kGameText, total);
    log_top("dll ", g_dll, kDllBuckets, 0, total);
}

}  // namespace tpw::testing
