#include "crt.h"

#include <cstdint>
#include <cstring>

namespace {

template <typename F> F game(u32 address) { return reinterpret_cast<F>(address); }

const CrtEnv kGameCrt = {
    game<s32(__cdecl *)(const char *, const char *)>(0x0067df40),
    game<char *(__cdecl *)(const char *, s32)>(0x0067b5f0),
    game<char *(__cdecl *)(const char *, s32)>(0x0067bfc0),
    game<s32(__cdecl *)(const char *, const char *, u32)>(0x0067c250),
    game<char *(__cdecl *)(const char *, const char *)>(0x0067c080),
    game<s32(__cdecl *)(s32)>(0x0067f1a0),
    game<s32(__cdecl *)(s32)>(0x0067fa60),
    game<s32(__cdecl *)(const char *)>(0x0067a520),
    game<s32(__cdecl *)(char *, const char *, ...)>(0x0067a650),
    game<u32(__cdecl *)(const u16 *)>(0x0067acd0),
    game<char *(__cdecl *)(char *, const char *, u32)>(0x0067ae00),
    game<u32(__cdecl *)(u32)>(0x0067a730),
    game<u16 *(__cdecl *)(u16 *, const u16 *)>(0x0067bf10),
    game<s32(__cdecl *)(s32)>(0x0067a530),
    game<s32(__cdecl *)(s32)>(0x0067a560),
    game<s32(__cdecl *)(s32)>(0x0067a590),
    game<s32(__cdecl *)(s32)>(0x0067a5c0),
    game<double(__cdecl *)(const char *)>(0x0067a6c0),
    game<s32(__cdecl *)(u16 *, const u16 *, ...)>(0x0067af00),
    game<u16 *(__cdecl *)(u16 *, const u16 *, u32)>(0x0067b0d0),
    game<s32(__cdecl *)(const u16 *, const u16 *)>(0x0067b140),
    game<u16 *(__cdecl *)(u16 *, const u16 *)>(0x0067bed0),
    game<s32(__cdecl *)(const u16 *, const u16 *)>(0x0067c290),
    game<s32(__cdecl *)(const char *, const char *)>(0x00680060),
    game<s32(__cdecl *)(s32 *)>(0x0067d3f0),
    game<s32(__cdecl *)()>(0x0067b5c0),
    game<void(__cdecl *)(void *)>(0x0067ad90),
    game<s32(__cdecl *)(const u16 *, const u16 *, u32)>(0x0067c8d0),
};
const CrtEnv *g_crt = &kGameCrt;

}  // namespace

const CrtEnv &Crt() { return *g_crt; }

void Crt_SetEnv(const CrtEnv *env) { g_crt = env ? env : &kGameCrt; }

namespace {

const CrtInnerEnv kGameInner = {
    game<u8 *(__cdecl *)()>(0x00682460),
    game<void(__cdecl *)()>(0x0067fa20),
    game<void(__cdecl *)()>(0x0067fa30),
    game<u32(__cdecl *)(void *)>(0x006818f0),
    game<void *(__cdecl *)(void *, u32)>(0x00681740),
    game<void(__cdecl *)(s32)>(0x00682720),
    game<void(__cdecl *)(s32)>(0x006827a0),
    game<void *(__cdecl *)(void *, void **, void **)>(0x00682b40),
    game<void(__cdecl *)(void *, void *, void *)>(0x00682ba0),
    game<void(__cdecl *)(void *)>(0x006827c0),
    game<void(__cdecl *)(void *)>(0x00682830),
    game<s32(__cdecl *)(void *)>(0x0067b660),
    game<void(__cdecl *)(void *)>(0x0067acb0),
    game<void *(__cdecl *)(u32)>(0x0067acc0),
    game<void *(__cdecl *)(u32, u32)>(0x0067dcf0),
    game<u16(__cdecl *)(u32)>(0x0067e670),
    game<s32(__cdecl *)(void *, const u16 *, void *)>(0x00683090),
    game<s32(__cdecl *)(s32, void *)>(0x00680ad0),
    game<s32(__cdecl *)(u32 *, const u32 *, u32, u32 *, void *)>(0x00693bc0),
    game<void *(__cdecl *)(u32, u32)>(0x0067fcb0),
    game<void(__cdecl *)(void *)>(0x00682440),
    game<void(__cdecl *)(s32)>(0x0067efc0),
    game<void(__cdecl *)(u32)>(0x00686980),
};
const CrtInnerEnv *g_inner = &kGameInner;

u32 &exit_table_end() { return *reinterpret_cast<u32 *>((uintptr_t)0x00fb3cb0); }
u32 &exit_table() { return *reinterpret_cast<u32 *>((uintptr_t)0x00fb3cb4); }

// The imports that a routine below reads from their slots.
template <typename F> F import_slot(u32 slot) { return *reinterpret_cast<F *>((uintptr_t)slot); }
u32 last_error() { return import_slot<u32(__stdcall *)()>(0x006fd138)(); }

// The string file of swprintf and _snwprintf: its pointer, count, base and flags (the rest is never set).
struct StringFile {
    char *ptr;
    s32 count;
    char *base;
    u32 flags;
    u32 unset[4];
};

// Past _woutput: two zero bytes put.
void put_end(StringFile &file) {
    if (--file.count < 0) {
        g_inner->flsbuf(0, &file);
    } else {
        *file.ptr = 0;
        ++file.ptr;
    }
    if (--file.count < 0)
        g_inner->flsbuf(0, &file);
    else
        *file.ptr = 0;
}

s32 format_wide(u16 *to, s32 size, const u16 *format, void *args) {
    StringFile file;
    file.ptr = file.base = reinterpret_cast<char *>(to);
    file.count = size;
    file.flags = 0x42;
    const s32 written = g_inner->woutput(&file, format, args);
    put_end(file);
    return written;
}

u16 fold_c(u16 unit) { return unit >= 0x41 && unit <= 0x5a ? (u16)(unit + 0x20) : unit; }

// The locale's lock for a reader: true when it took lock 0x13 instead of the count.
bool locale_enter() {
    u32 *const readers = reinterpret_cast<u32 *>((uintptr_t)0x00fb3ca8);
    import_slot<s32(__stdcall *)(u32 *)>(0x006fd0c0)(readers);    // InterlockedIncrement
    if (!*reinterpret_cast<u32 *>((uintptr_t)0x00fb3ca4)) return false;
    import_slot<s32(__stdcall *)(u32 *)>(0x006fd12c)(readers);    // InterlockedDecrement
    g_inner->lock(0x13);
    return true;
}

void locale_leave(bool locked) {
    if (locked)
        g_inner->unlock(0x13);
    else
        import_slot<s32(__stdcall *)(u32 *)>(0x006fd12c)(reinterpret_cast<u32 *>((uintptr_t)0x00fb3ca8));
}

bool locale_set() { return *reinterpret_cast<u32 *>((uintptr_t)0x00fb1440) != 0; }

}  // namespace

void CrtInner_SetEnv(const CrtInnerEnv *env) { g_inner = env ? env : &kGameInner; }

extern "C" {

s32 __cdecl CRT_isnan(u32 low, u32 high) {
    const u32 top = (high >> 16) & 0x7ff8;
    if (top == 0x7ff0 && ((high & 0x7ffff) != 0 || low != 0)) return 1;
    return top == 0x7ff8;
}

long long __stdcall CRT_alldiv(u32 aLow, u32 aHigh, u32 bLow, u32 bHigh) {
    return (long long)((unsigned long long)aHigh << 32 | aLow) / (long long)((unsigned long long)bHigh << 32 | bLow);
}

s32 __cdecl CRT_rand() {
    u8 *const data = g_inner->getptd();
    u32 &seed = *reinterpret_cast<u32 *>(data + 0x14);
    seed = seed * 214013u + 2531011u;
    return (s32)((seed >> 16) & 0x7fff);
}

void *__cdecl CRT_onexit(void *fn) {
    const CrtInnerEnv &env = *g_inner;
    env.lockExit();
    const u32 size = env.msize(reinterpret_cast<void *>((uintptr_t)exit_table()));
    u32 end = exit_table_end();
    const u32 begin = exit_table();
    if (size < end - begin + 4) {
        const u32 now = env.msize(reinterpret_cast<void *>((uintptr_t)begin));
        const u32 grown = (u32)(uintptr_t)env.realloc(reinterpret_cast<void *>((uintptr_t)exit_table()), now + 0x10);
        if (!grown) {
            env.unlockExit();
            return nullptr;
        }
        const s32 used = (s32)(exit_table_end() - exit_table()) >> 2;
        exit_table() = grown;
        end = grown + (u32)used * 4;
        exit_table_end() = end;
    }
    *reinterpret_cast<void **>((uintptr_t)end) = fn;
    exit_table_end() += 4;
    env.unlockExit();
    return fn;
}

s32 __cdecl CRT_atexit(void *fn) { return CRT_onexit(fn) ? 0 : -1; }

void __cdecl CRT_free(void *block) {
    if (!block) return;
    const CrtInnerEnv &env = *g_inner;
    env.lock(9);
    void *region;     // (the callee fills it when it answers)
    void *page = block;   // (the original's argument slot)
    void *const entry = env.sbhFindBlock(block, &region, &page);
    if (entry) {
        env.sbhFreeBlock(region, page, entry);
        env.unlock(9);
        return;
    }
    env.unlock(9);
    (*reinterpret_cast<s32(__stdcall *const *)(u32, u32, void *)>((uintptr_t)0x006fd210))(*reinterpret_cast<u32 *>((uintptr_t)0x00fb3cac), 0, block);   // HeapFree
}

s32 __cdecl CRT_fclose(void *file) {
    u32 &flags = *reinterpret_cast<u32 *>(static_cast<u8 *>(file) + 0xc);
    if (flags & 0x40) {
        flags = 0;
        return -1;
    }
    const CrtInnerEnv &env = *g_inner;
    env.lockFile(file);
    const s32 result = env.fcloseLocked(file);
    env.unlockFile(file);
    return result;
}

void __fastcall CRT_exception_Dtor(u32 *self) {
    const u32 owned = self[2];
    self[0] = 0x007089c4;
    if (owned) g_inner->memFree(reinterpret_cast<void *>((uintptr_t)self[1]));
}

u32 *__fastcall CRT_exception_Ctor(u32 *self, void *, const char *const *text) {
    self[0] = 0x007089c4;
    const u32 size = (u32)strlen(*text) + 1;
    char *const copy = static_cast<char *>(g_inner->memAlloc(size));
    self[1] = (u32)(uintptr_t)copy;
    if (copy) memcpy(copy, *text, size);
    self[2] = 1;
    return self;
}

u32 *__fastcall CRT_exception_Copy(u32 *self, void *, const u32 *from) {
    self[0] = 0x007089c4;
    const u32 owned = from[2];
    self[2] = owned;
    if (!owned) {
        self[1] = from[1];
        return self;
    }
    const char *const text = reinterpret_cast<const char *>((uintptr_t)from[1]);
    const u32 size = (u32)strlen(text) + 1;
    char *const copy = static_cast<char *>(g_inner->memAlloc(size));
    self[1] = (u32)(uintptr_t)copy;
    if (copy) memcpy(copy, reinterpret_cast<const char *>((uintptr_t)from[1]), size);
    return self;
}

void *__cdecl CRT_malloc(u32 size) { return g_inner->nhMalloc(size, *reinterpret_cast<u32 *>((uintptr_t)0x00fb1548)); }

void *__cdecl CRT_getptd() {
    const u32 error = last_error();
    const u32 slot = *reinterpret_cast<u32 *>((uintptr_t)0x0077ecd0);
    void *data = import_slot<void *(__stdcall *)(u32)>(0x006fd1c4)(slot);    // TlsGetValue
    if (!data) {
        data = g_inner->calloc(1, 0x74);
        if (data && import_slot<s32(__stdcall *)(u32, void *)>(0x006fd230)(*reinterpret_cast<u32 *>((uintptr_t)0x0077ecd0), data)) {    // TlsSetValue
            g_inner->initptd(data);
            u32 *const fields = static_cast<u32 *>(data);
            fields[0] = import_slot<u32(__stdcall *)()>(0x006fd248)();    // GetCurrentThreadId
            fields[1] = 0xffffffffu;
        } else {
            g_inner->amsgExit(0x10);
        }
    }
    import_slot<void(__stdcall *)(u32)>(0x006fd1c8)(error);    // SetLastError
    return data;
}

s32 *__cdecl CRT_errno() { return reinterpret_cast<s32 *>(g_inner->getptd() + 8); }

u32 __cdecl CRT_beginthread(void *proc, u32 stack, void *arg) {
    u32 error = 0;
    u32 *const data = static_cast<u32 *>(g_inner->calloc(1, 0x74));
    if (data) {
        g_inner->initptd(data);
        data[0x48 / 4] = (u32)(uintptr_t)proc;
        data[0x4c / 4] = (u32)(uintptr_t)arg;
        const u32 handle = import_slot<u32(__stdcall *)(void *, u32, void *, void *, u32, void *)>(0x006fd22c)(nullptr, stack, reinterpret_cast<void *>((uintptr_t)0x0067e0d0), data, 4, data);    // CreateThread
        data[1] = handle;
        if (handle && import_slot<u32(__stdcall *)(u32)>(0x006fd228)(handle) != 0xffffffffu) return handle;    // ResumeThread
        error = last_error();
    }
    Crt().free(data);
    if (error) g_inner->dosmaperr(error);
    return 0xffffffffu;
}

s32 __cdecl CRT_swprintf(u16 *to, const u16 *format, ...) { return format_wide(to, 0x7fffffff, format, &format + 1); }

s32 __cdecl CRT_snwprintf(u16 *to, u32 count, const u16 *format, ...) { return format_wide(to, (s32)(count * 2), format, &format + 1); }

s32 __cdecl CRT_wcsicmp(const u16 *a, const u16 *b) {
    u16 x, y;
    if (!locale_set()) {
        do {
            x = fold_c(*a++);
            y = fold_c(*b++);
        } while (x && x == y);
    } else {
        const bool locked = locale_enter();
        do {
            x = g_inner->towlowerLocked(*a++);
            y = g_inner->towlowerLocked(*b++);
        } while (x && x == y);
        locale_leave(locked);
    }
    return (s32)x - (s32)y;
}

s32 __cdecl CRT_wcsnicmp(const u16 *a, const u16 *b, u32 n) {
    if (!n) return 0;
    u16 x, y;
    if (!locale_set()) {
        do {
            x = fold_c(*a++);
            y = fold_c(*b++);
        } while (--n && x && x == y);
    } else {
        const bool locked = locale_enter();
        do {
            x = g_inner->towlowerLocked(*a++);
            y = g_inner->towlowerLocked(*b++);
        } while (--n && x && x == y);
        locale_leave(locked);
    }
    return (s32)x - (s32)y;
}

u32 __cdecl CRT_btowc(u32 c) {
    if (c == 0xffffffffu) return c;
    u32 wc;
    u32 state = 0;
    if (g_inner->mbrtowc(&wc, &c, 1, &state, nullptr) < 0) return 0xffff;
    return wc & 0xffff;
}

void __stdcall CRT_VectorCtorIterator(u8 *array, u32 size, s32 count, void(__fastcall *ctor)(void *), void *) {
    u8 *element = array;
    for (s32 i = 0; i < count; ++i) {
        ctor(element);
        element += size;
    }
}

void __stdcall CRT_VectorCtorIteratorNoEh(u8 *array, u32 size, s32 count, void(__fastcall *ctor)(void *)) {
    if ((s32)((u32)count - 1) < 0) return;
    u32 left = (u32)count;  // (0x80000000 too, sic)
    do {
        ctor(array);
        array += size;
    } while (--left);
}

void __stdcall CRT_VectorVbaseCtorIterator(u8 *array, u32 size, s32 count, void(__fastcall *ctor)(void *, void *, u32), void *) {
    u8 *element = array;
    for (s32 i = 0; i < count; ++i) {
        ctor(element, nullptr, 1);
        element += size;
    }
}

void __stdcall CRT_VectorDtorIterator(u8 *array, u32 size, s32 count, void(__fastcall *dtor)(void *)) {
    u8 *element = array + count * size;
    for (s32 remaining = count - 1; remaining >= 0; --remaining) {
        element -= size;
        dtor(element);
    }
}

u16 *__cdecl CRT_wcsset(u16 *text, u16 unit) {
    for (u16 *p = text; *p; ++p) *p = unit;
    return text;
}

}  // extern "C"
