#include "crtlock.h"

#include <cstdint>

#include "crt.h"

namespace {

template <typename F> F import_slot(u32 slot) { return *reinterpret_cast<F *>((uintptr_t)slot); }

void init_section(void *section) { import_slot<void(__stdcall *)(void *)>(0x006fd144)(section); }
void enter_section(void *section) { import_slot<void(__stdcall *)(void *)>(0x006fd140)(section); }
s32 leave_section(void *section) { return import_slot<s32(__stdcall *)(void *)>(0x006fd130)(section); }

void *&section(s32 which) { return reinterpret_cast<void **>((uintptr_t)0x0077ece0)[which]; }

u8 *section_of(void *file) { return static_cast<u8 *>(file) + 0x20; }

bool in_table(void *file) {
    const u32 at = static_cast<u32>(reinterpret_cast<uintptr_t>(file));
    return at >= 0x0077e800 && at <= 0x0077ea60;
}

s32 table_lock(void *file) { return (static_cast<s32>(static_cast<u32>(reinterpret_cast<uintptr_t>(file)) - 0x0077e800) >> 5) + 0x1c; }

}  // namespace

extern "C" {

void __cdecl CRT_lock(s32 which) {
    if (!section(which)) {
        void *const fresh = CRT_malloc(0x18);
        if (!fresh) CrtInner().amsgExit(0x11);
        CRT_lock(0x11);
        if (!section(which)) {
            init_section(fresh);
            section(which) = fresh;
        } else {
            Crt().free(fresh);
        }
        CRT_unlock(0x11);
    }
    enter_section(section(which));
}

s32 __cdecl CRT_unlock(s32 which) { return leave_section(section(which)); }

void __cdecl CRT_lock_file(void *file) {
    if (in_table(file))
        CRT_lock(table_lock(file));
    else
        enter_section(section_of(file));
}

void __cdecl CRT_unlock_file(void *file) {
    if (in_table(file))
        CRT_unlock(table_lock(file));
    else
        leave_section(section_of(file));
}

void __cdecl CRT_lock_file2(s32 index, void *file) {
    if (index < 0x14)
        CRT_lock(index + 0x1c);
    else
        enter_section(section_of(file));
}

void __cdecl CRT_unlock_file2(s32 index, void *file) {
    if (index < 0x14)
        CRT_unlock(index + 0x1c);
    else
        leave_section(section_of(file));
}

}
