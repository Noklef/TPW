#include "data_dump.h"

#include <windows.h>

#include <cstring>

#include "log.h"

namespace tpw::testing {

namespace {

char g_path[MAX_PATH];
const BYTE *g_data;
DWORD g_size;

DWORD WINAPI dump_thread(void *) {
    bool down = false;
    for (;;) {
        Sleep(50);
        const bool now = (GetAsyncKeyState(VK_F11) & 0x8000) != 0;
        if (now && !down) {
            const HANDLE file = CreateFileA(g_path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            DWORD written = 0;
            if (file != INVALID_HANDLE_VALUE) {
                WriteFile(file, g_data, g_size, &written, nullptr);
                CloseHandle(file);
            }
            log_line("[%6.1fs] data dump: %lu of %lu bytes from %p", log_seconds(), written, g_size, g_data);
        }
        down = now;
    }
}

}  // namespace

void start_data_dump(const char *ini) {
    lstrcpynA(g_path, ini, MAX_PATH);
    char *const slash = strrchr(g_path, '\\');
    if (!slash) return;
    lstrcpyA(slash + 1, "tpwdump.bin");
    const auto *self = reinterpret_cast<const BYTE *>(GetModuleHandleA(nullptr));
    const auto *nt = reinterpret_cast<const IMAGE_NT_HEADERS *>(self + reinterpret_cast<const IMAGE_DOS_HEADER *>(self)->e_lfanew);
    const IMAGE_SECTION_HEADER *section = IMAGE_FIRST_SECTION(nt), *largest = nullptr;
    for (unsigned i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++section)
        if ((section->Characteristics & IMAGE_SCN_MEM_WRITE) && (!largest || section->Misc.VirtualSize > largest->Misc.VirtualSize)) largest = section;
    if (!largest) return;
    g_data = self + largest->VirtualAddress;
    g_size = largest->Misc.VirtualSize;
    if (const HANDLE thread = CreateThread(nullptr, 0, dump_thread, nullptr, 0, nullptr)) CloseHandle(thread);
}

}  // namespace tpw::testing
