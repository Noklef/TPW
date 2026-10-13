// Proxy for dinput.dll. tp.exe imports a single function from it,
// DirectInputCreateA, which we forward to the real system DLL.
//
// Why dinput and not ddraw: DirectX wrappers needed on modern Windows
// (dgVoodoo2, DDrawCompat) are themselves ddraw.dll proxies, so that name is taken.
#include <windows.h>

namespace {

using DirectInputCreateA_t = HRESULT(WINAPI *)(HINSTANCE, DWORD, void **, void *);

FARPROC real_proc(const char *name) {
    static HMODULE real = [] {
        char path[MAX_PATH];
        GetSystemDirectoryA(path, MAX_PATH);  // SysWOW64 for a 32-bit process on 64-bit Windows
        lstrcatA(path, "\\dinput.dll");
        return LoadLibraryA(path);
    }();
    return real ? GetProcAddress(real, name) : nullptr;
}

}  // namespace

extern "C" HRESULT WINAPI Proxy_DirectInputCreateA(HINSTANCE inst, DWORD version, void **out, void *outer) {
    auto fn = reinterpret_cast<DirectInputCreateA_t>(real_proc("DirectInputCreateA"));
    if (!fn) return E_FAIL;
    return fn(inst, version, out, outer);
}
