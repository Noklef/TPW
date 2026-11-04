#include "crt.h"

#include <intrin.h>

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
    game<void(__cdecl *)()>(0x00687d80),
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

const CrtInnerEnv &CrtInner() { return *g_inner; }

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

static u32 global_u32(u32 address) { return *reinterpret_cast<const u32 *>((uintptr_t)address); }

static s32 conversion_failed() {
    *CRT_errno() = 0x2a;
    return -1;
}

static s32 conversion_rejected(u8 *state) {
    *reinterpret_cast<u32 *>(state) = 0;
    return conversion_failed();
}

// MultiByteToWideChar(page, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, text, count, unit, unit given).
static s32 multi_to_wide(u32 page, const void *text, u32 count, u16 *unit) {
    return import_slot<s32(__stdcall *)(u32, u32, const void *, u32, u16 *, u32)>(0x006fd174)(page, 9, text, count, unit, unit ? 1 : 0);
}

unsigned long long __cdecl CRT_Getcvt() {
    const bool locked = locale_enter();
    const u32 hand = global_u32(0x00fb1440);
    const u32 page = global_u32(0x00fb1450);
    locale_leave(locked);
    return static_cast<unsigned long long>(page) << 32 | hand;
}

u32 *__cdecl CRT_Getctype(u32 *out) {
    const bool locked = locale_enter();
    const u32 hand = global_u32(0x00fb143c);
    const u32 page = global_u32(0x00fb1450);
    u32 *const copy = static_cast<u32 *>(CRT_malloc(0x200));
    u32 *const pctype = reinterpret_cast<u32 *>((uintptr_t)global_u32(0x0077ea90));
    if (copy) {
        for (u32 i = 0; i < 0x80; ++i) copy[i] = pctype[i];
    }
    locale_leave(locked);
    out[0] = hand;
    out[1] = page;
    out[2] = static_cast<u32>(reinterpret_cast<uintptr_t>(copy ? copy : pctype));
    out[3] = copy ? 1 : 0;
    return out;
}

s32 __cdecl CRT_WcrtombLocked(char *to, u32 unit, void *, const u32 *conversion) {
    const u32 hand = conversion ? conversion[0] : global_u32(0x00fb1440);
    const u32 page = conversion ? conversion[1] : global_u32(0x00fb1450);
    if (!hand) {
        if (static_cast<u16>(unit) > 0xff) return conversion_failed();
        *to = static_cast<char>(unit);
        return 1;
    }
    u32 used = 0;
    const s32 written = import_slot<s32(__stdcall *)(u32, u32, const u32 *, u32, char *, u32, const char *, u32 *)>(0x006fd1dc)(page, 0x220, &unit, 1, to, global_u32(0x0077e7dc), nullptr, &used);    // WideCharToMultiByte
    if (written && !used) return written;
    return conversion_failed();
}

s32 __cdecl CRT_Wcrtomb(char *to, u32 unit, void *, const u32 *conversion) {
    const bool locked = locale_enter();
    const s32 result = CRT_WcrtombLocked(to, unit, nullptr, conversion);
    locale_leave(locked);
    return result;
}

s32 __cdecl CRT_MbrtowcLocked(u16 *unit, const char *text, u32 n, u8 *state, const u32 *conversion) {
    if (!text || !n) return 0;
    const u8 first = static_cast<u8>(*text);
    if (!first) {
        if (unit) *unit = 0;
        return 0;
    }
    const u32 hand = conversion ? conversion[0] : global_u32(0x00fb1440);
    const u32 page = conversion ? conversion[1] : global_u32(0x00fb1450);
    if (!hand) {
        if (unit) *unit = first;
        return 1;
    }
    if (*reinterpret_cast<const u32 *>(state)) {
        state[1] = first;
        if (static_cast<s32>(global_u32(0x0077e7dc)) <= 1) return conversion_rejected(state);
        const s32 got = multi_to_wide(page, state, 2, unit);
        if (!got) return conversion_rejected(state);
        *reinterpret_cast<u32 *>(state) = 0;
        return got;
    }
    if (reinterpret_cast<const u16 *>((uintptr_t)global_u32(0x0077ea90))[first] & 0x8000) {
        const u32 most = global_u32(0x0077e7dc);
        if (n < most) {
            state[0] = first;
            return -2;
        }
        if (static_cast<s32>(most) > 1 && multi_to_wide(page, text, most, unit)) return most;
        if (!text[1]) return conversion_rejected(state);
        return most;
    }
    if (multi_to_wide(page, text, 1, unit)) return 1;
    return conversion_failed();
}

s32 __cdecl CRT_Mbrtowc(u16 *unit, const char *text, u32 n, u8 *state, const u32 *conversion) {
    const bool locked = locale_enter();
    const s32 result = CRT_MbrtowcLocked(unit, text, n, state, conversion);
    locale_leave(locked);
    return result;
}

s32 __cdecl CRT_wctomb_lk(char *out, u32 c) {
    if (!out) return 0;
    const u16 unit = static_cast<u16>(c);
    s32 result = 0;
    s32 defaulted = 0;
    if (!locale_set()) {
        if (unit <= 0xff) {
            *out = static_cast<char>(unit);
            return 1;
        }
    } else {
        result = import_slot<s32(__stdcall *)(u32, u32, const u16 *, s32, char *, s32, const char *, s32 *)>(0x006fd1dc)(global_u32(0x00fb1450), 0x220, &unit, 1, out, static_cast<s32>(global_u32(0x0077e7dc)), nullptr, &defaulted);
        if (result && !defaulted) return result;
    }
    *CRT_errno() = 0x2a;
    return -1;
}

s32 __cdecl CRT_wctomb(char *out, u32 c) {
    const bool locked = locale_enter();
    const s32 result = CRT_wctomb_lk(out, c);
    locale_leave(locked);
    return result;
}

s32 __cdecl CRT_mbtowc_lk(u16 *out, const char *text, u32 size) {
    if (!text || !size) return 0;
    const u8 first = static_cast<u8>(*text);
    if (!first) {
        if (out) *out = 0;
        return 0;
    }
    if (!locale_set()) {
        if (out) *out = first;
        return 1;
    }
    const auto convert = import_slot<s32(__stdcall *)(u32, u32, const char *, s32, u16 *, s32)>(0x006fd174);
    const u32 page = global_u32(0x00fb1450);
    if (reinterpret_cast<const u16 *>((uintptr_t)global_u32(0x0077ea90))[first] & 0x8000) {
        const s32 most = static_cast<s32>(global_u32(0x0077e7dc));
        if (most > 1 && static_cast<s32>(size) >= most && convert(page, 9, text, most, out, out != nullptr)) return most;
        if (size >= static_cast<u32>(most) && text[1]) return most;
    } else if (convert(page, 9, text, 1, out, out != nullptr)) {
        return 1;
    }
    *CRT_errno() = 0x2a;
    return -1;
}

s32 __cdecl CRT_mbtowc(u16 *out, const char *text, u32 size) {
    const bool locked = locale_enter();
    const s32 result = CRT_mbtowc_lk(out, text, size);
    locale_leave(locked);
    return result;
}

char *__cdecl CRT_strrchr(const char *text, s32 c) {
    const char *at = text + std::strlen(text);
    for (;;) {
        if (*at == static_cast<char>(c)) return const_cast<char *>(at);
        if (at == text) return nullptr;
        --at;
    }
}

char *__cdecl CRT_strpbrk(const char *text, const char *set) {
    u32 bits[8] = {};
    for (; *set; ++set) {
        const u8 c = static_cast<u8>(*set);
        bits[c >> 5] |= 1u << (c & 31);
    }
    for (; *text; ++text) {
        const u8 c = static_cast<u8>(*text);
        if (bits[c >> 5] >> (c & 31) & 1) return const_cast<char *>(text);
    }
    return nullptr;
}

void __stdcall CRT_CxxThrowException(void *object, const void *info) {
    u32 record[8];
    std::memcpy(record, reinterpret_cast<const void *>((uintptr_t)0x007089e8), sizeof record);
    record[6] = static_cast<u32>(reinterpret_cast<uintptr_t>(object));
    record[7] = static_cast<u32>(reinterpret_cast<uintptr_t>(info));
    import_slot<void(__stdcall *)(u32, u32, u32, const u32 *)>(0x006fd218)(record[0], record[1], record[4], &record[5]);
}

u32 __cdecl CRT_strncnt(const char *text, u32 n) {
    const char *at = text;
    u32 left = n;
    if (left) {
        while (*at) {
            ++at;
            if (!--left) break;
        }
    }
    return *at ? n : static_cast<u32>(at - text);
}

s32 __cdecl CRT_GetStringTypeA(u32 info, const char *text, s32 count, u16 *types, u32 page, u32 id, s32 strict) {
    using TypesW = s32(__stdcall *)(u32, const void *, s32, u16 *);
    using TypesA = s32(__stdcall *)(u32, u32, const void *, s32, u16 *);
    using Widen = s32(__stdcall *)(u32, u32, const char *, s32, u16 *, s32);
    u32 &mode = *reinterpret_cast<u32 *>((uintptr_t)0x00fb1890);
    u16 probe[2];
    if (!mode) {
        if (import_slot<TypesW>(0x006fd1ec)(1, reinterpret_cast<const void *>((uintptr_t)0x00708da8), 1, probe))
            mode = 1;
        else if (import_slot<TypesA>(0x006fd1f0)(0, 1, reinterpret_cast<const void *>((uintptr_t)0x00708da4), 1, probe))
            mode = 2;
        else
            return 0;
    }
    if (mode == 2) return import_slot<TypesA>(0x006fd1f0)(id ? id : global_u32(0x00fb1440), info, text, count, types);
    if (mode != 1) return static_cast<s32>(mode);
    const u32 code = page ? page : global_u32(0x00fb1450);
    u16 *block = nullptr;
    s32 result = 0;
    const s32 units = import_slot<Widen>(0x006fd174)(code, strict ? 9 : 1, text, count, nullptr, 0);
    if (units) {
        block = static_cast<u16 *>(g_inner->calloc(2, units));
        if (block) {
            const s32 made = import_slot<Widen>(0x006fd174)(code, 1, text, count, block, units);
            if (made) {
                result = import_slot<TypesW>(0x006fd1ec)(info, block, made, types);
            }
        }
    }
    Crt().free(block);
    return result;
}

s32 __cdecl CRT_LCMapStringA(u32 id, u32 flags, const char *from, s32 count, char *to, s32 size, u32 page, s32 strict) {
    using MapW = s32(__stdcall *)(u32, u32, const void *, s32, void *, s32);
    using MapA = s32(__stdcall *)(u32, u32, const void *, s32, void *, s32);
    using Widen = s32(__stdcall *)(u32, u32, const char *, s32, u16 *, s32);
    using Narrow = s32(__stdcall *)(u32, u32, const u16 *, s32, char *, s32, const char *, u32 *);
    u32 &mode = *reinterpret_cast<u32 *>((uintptr_t)0x00fb1554);
    if (!mode) {
        if (import_slot<MapW>(0x006fd198)(0, 0x100, reinterpret_cast<const void *>((uintptr_t)0x00708da8), 1, nullptr, 0))
            mode = 1;
        else if (import_slot<MapA>(0x006fd19c)(0, 0x100, reinterpret_cast<const void *>((uintptr_t)0x00708da4), 1, nullptr, 0))
            mode = 2;
        else
            return 0;
    }
    if (count > 0) count = static_cast<s32>(CRT_strncnt(from, static_cast<u32>(count)));
    if (mode == 2) return import_slot<MapA>(0x006fd19c)(id, flags, from, count, to, size);
    if (mode != 1) return static_cast<s32>(mode);
    const u32 code = page ? page : global_u32(0x00fb1450);
    const s32 units = import_slot<Widen>(0x006fd174)(code, strict ? 9 : 1, from, count, nullptr, 0);
    if (!units) return 0;
    u16 *const wide = static_cast<u16 *>(CRT_malloc(static_cast<u32>(units) * 2));
    if (!wide) return 0;
    u16 *mapped = nullptr;
    s32 result = 0;
    if (import_slot<Widen>(0x006fd174)(code, 1, from, count, wide, units)) {
        const s32 needed = import_slot<MapW>(0x006fd198)(id, flags, wide, units, nullptr, 0);
        if (needed) {
            if (flags & 0x400) {
                if (!size || (needed <= size && import_slot<MapW>(0x006fd198)(id, flags, wide, units, to, size))) result = needed;
            } else {
                mapped = static_cast<u16 *>(CRT_malloc(static_cast<u32>(needed) * 2));
                if (mapped && import_slot<MapW>(0x006fd198)(id, flags, wide, units, mapped, needed))
                    result = size ? import_slot<Narrow>(0x006fd1dc)(code, 0x220, mapped, needed, to, size, nullptr, nullptr)
                                  : import_slot<Narrow>(0x006fd1dc)(code, 0x220, mapped, needed, nullptr, 0, nullptr, nullptr);
            }
        }
    }
    Crt().free(wide);
    Crt().free(mapped);
    return result;
}

// A character's one or two bytes as the two routines below hand them on: true for two.
static bool lead_and_trail(s32 c, char *bytes) {
    const u32 lead = static_cast<u32>(c >> 8) & 0xff;
    if (reinterpret_cast<const u16 *>((uintptr_t)global_u32(0x0077ea90))[lead] & 0x8000) {
        bytes[0] = static_cast<char>(lead);
        bytes[1] = static_cast<char>(c);
        bytes[2] = 0;
        return true;
    }
    bytes[0] = static_cast<char>(c);
    bytes[1] = 0;
    return false;
}

s32 __cdecl CRT_isctype(s32 c, s32 mask) {
    if (static_cast<u32>(c) + 1 <= 0x100) return reinterpret_cast<const u16 *>((uintptr_t)global_u32(0x0077ea90))[c] & mask;
    char bytes[4];
    const s32 count = lead_and_trail(c, bytes) ? 2 : 1;
    u16 types;
    if (!CRT_GetStringTypeA(1, bytes, count, &types, 0, 0, 1)) return 0;
    return types & mask;
}

// What _tolower_lk, _toupper_lk, _Tolower and _Toupper share: below 0x100 only a character of the `type` (by isctype when characters may be longer); then __crtLCMapStringA.
static s32 map_case(s32 c, s32 type, u32 flags, u32 id, u32 page, s32 strict) {
    if (c < 0x100) {
        const s32 is_type = static_cast<s32>(global_u32(0x0077e7dc)) > 1 ? CRT_isctype(c, type) : reinterpret_cast<const u16 *>((uintptr_t)global_u32(0x0077ea90))[c] & type;
        if (!is_type) return c;
    }
    char bytes[4];
    const s32 count = lead_and_trail(c, bytes) ? 2 : 1;
    u8 mapped[8];
    const s32 got = CRT_LCMapStringA(id, flags, bytes, count, reinterpret_cast<char *>(mapped), 3, page, strict);
    if (!got) return c;
    return got == 1 ? mapped[0] : mapped[1] << 8 | mapped[0];
}

s32 __cdecl CRT_toupperLocked(s32 c) {
    if (!locale_set()) return c >= 0x61 && c <= 0x7a ? c - 0x20 : c;
    return map_case(c, 2, 0x200, global_u32(0x00fb1440), 0, 1);
}

s32 __cdecl CRT_toupper(s32 c) {
    if (!locale_set()) return c >= 0x61 && c <= 0x7a ? c - 0x20 : c;
    const bool locked = locale_enter();
    const s32 result = CRT_toupperLocked(c);
    locale_leave(locked);
    return result;
}

// A byte's class for strtoxl: _isctype under a multibyte locale, else the table.
static bool byte_is(u8 c, u32 mask) {
    if (static_cast<s32>(global_u32(0x0077e7dc)) > 1) return CRT_isctype(c, static_cast<s32>(mask)) != 0;
    return (reinterpret_cast<const u16 *>((uintptr_t)global_u32(0x0077ea90))[c] & mask) != 0;
}

u32 __cdecl CRT_strtoxl(const char *text, const char **end, s32 base, u32 flags) {
    const char *at = text;
    char c = *at++;
    while (byte_is(static_cast<u8>(c), 8)) c = *at++;
    if (c == '-') flags |= 2;
    if (c == '-' || c == '+') c = *at++;
    if (base < 0 || base == 1 || base > 36) {
        if (end) *end = text;
        return 0;
    }
    if (!base) base = c != '0' ? 10 : *at == 'x' || *at == 'X' ? 16 : 8;
    if (base == 16 && c == '0' && (*at == 'x' || *at == 'X')) {
        c = at[1];
        at += 2;
    }
    const u32 radix = static_cast<u32>(base);
    const u32 most = 0xffffffffu / radix;
    u32 number = 0;
    for (;; c = *at++) {
        u32 digit;
        if (byte_is(static_cast<u8>(c), 4))
            digit = static_cast<u32>(static_cast<s32>(c) - 0x30);
        else if (byte_is(static_cast<u8>(c), 0x103))
            digit = static_cast<u32>(CRT_toupper(c) - 0x37);
        else
            break;
        if (digit >= radix) break;
        flags |= 8;
        if (number < most || (number == most && digit <= 0xffffffffu % radix))
            number = number * radix + digit;
        else
            flags |= 4;
    }
    --at;
    if (!(flags & 8)) {
        at = text;
        number = 0;
    } else if (flags & 4 || (!(flags & 1) && (flags & 2 ? number > 0x80000000u : number > 0x7fffffffu))) {
        *CRT_errno() = 0x22;
        number = flags & 1 ? 0xffffffffu : flags & 2 ? 0x80000000u : 0x7fffffffu;
    }
    if (end) *end = at;
    return flags & 2 ? 0u - number : number;
}

s32 __cdecl CRT_strtol(const char *text, const char **end, s32 base) { return static_cast<s32>(CRT_strtoxl(text, end, base, 0)); }

s32 __cdecl CRT_tolowerLocked(s32 c) {
    if (!locale_set()) return c >= 0x41 && c <= 0x5a ? c + 0x20 : c;
    return map_case(c, 1, 0x100, global_u32(0x00fb1440), 0, 1);
}

s32 __cdecl CRT_stricmp(const char *a, const char *b) {
    const auto low = [](u8 c) -> u8 { return c >= 0x41 && c <= 0x5a ? c + 0x20 : c; };
    if (!locale_set()) {
        for (;;) {
            const u8 x = static_cast<u8>(*b++);
            const u8 y = static_cast<u8>(*a++);
            if (x == y) {
                if (!x) return 0;
                continue;
            }
            const u8 lx = low(x);
            const u8 ly = low(y);
            if (lx != ly) return ly < lx ? -1 : 1;
        }
    }
    long *const readers = reinterpret_cast<long *>((uintptr_t)0x00fb3ca8);
    _InterlockedIncrement(readers);
    const bool locked = *reinterpret_cast<const s32 *>((uintptr_t)0x00fb3ca4) > 0;
    if (locked) {
        _InterlockedDecrement(readers);
        g_inner->lock(0x13);
    }
    u32 x = 0xff;
    u32 y = 0;
    s32 result;
    for (;;) {
        if (!(x & 0xff)) {
            result = static_cast<s32>(x);
            break;
        }
        x = (x & 0xffffff00u) | static_cast<u8>(*b++);
        y = (y & 0xffffff00u) | static_cast<u8>(*a++);
        if ((x & 0xff) == (y & 0xff)) continue;
        y = static_cast<u32>(CRT_tolowerLocked(static_cast<s32>(y)));
        x = static_cast<u32>(CRT_tolowerLocked(static_cast<s32>(x)));
        if ((x & 0xff) == (y & 0xff)) continue;
        result = (y & 0xff) < (x & 0xff) ? -1 : 1;
        break;
    }
    if (locked)
        g_inner->unlock(0x13);
    else
        _InterlockedDecrement(readers);
    return result;
}

static s32 __cdecl tolower_with(s32 c, const u32 *locale, u32 strict) {
    const u32 id = locale ? locale[0] : global_u32(0x00fb1440);
    const u32 page = locale ? locale[1] : global_u32(0x00fb1450);
    if (!id) return c >= 0x41 && c <= 0x5a ? c + 0x20 : c;
    return map_case(c, 1, 0x100, id, page, static_cast<s32>(strict));
}

static s32 __cdecl toupper_with(s32 c, const u32 *locale, u32 strict) {
    const u32 id = locale ? locale[0] : global_u32(0x00fb1440);
    const u32 page = locale ? locale[1] : global_u32(0x00fb1450);
    if (!id) return c >= 0x61 && c <= 0x7a ? c - 0x20 : c;
    return map_case(c, 2, 0x200, id, page, static_cast<s32>(strict));
}

__declspec(naked) s32 __cdecl CRT_Tolower(s32, const u32 *) {
    __asm {
        push edi
        push dword ptr [esp + 0xc]
        push dword ptr [esp + 0xc]
        call tolower_with
        add esp, 0xc
        ret
    }
}

__declspec(naked) s32 __cdecl CRT_Toupper(s32, const u32 *) {
    __asm {
        push edi
        push dword ptr [esp + 0xc]
        push dword ptr [esp + 0xc]
        call toupper_with
        add esp, 0xc
        ret
    }
}

void *__cdecl CRT_memmove(void *dest, const void *from, u32 size) {
    std::memmove(dest, from, size);
    return dest;
}

char *__cdecl CRT_strncat(char *dest, const char *from, u32 size) {
    if (!size) return dest;
    char *to = dest + std::strlen(dest);
    for (; size; --size) {
        const char c = *from++;
        *to++ = c;
        if (!c) return dest;
    }
    *to = 0;
    return dest;
}

char *__cdecl CRT_strtok(char *text, const char *separators) {
    u8 *const thread = g_inner->getptd();
    char *&place = *reinterpret_cast<char **>(thread + 0x18);
    u8 map[0x20] = {};
    const u8 *at = reinterpret_cast<const u8 *>(separators);
    do {
        map[*at >> 3] |= static_cast<u8>(1 << (*at & 7));
    } while (*at++);
    const auto is_separator = [&](char c) { return (map[static_cast<u8>(c) >> 3] >> (static_cast<u8>(c) & 7)) & 1; };
    char *from = text ? text : place;
    while (*from && is_separator(*from)) ++from;
    char *const start = from;
    for (; *from; ++from) {
        if (is_separator(*from)) {
            *from++ = 0;
            break;
        }
    }
    place = from;
    return start != from ? start : nullptr;
}

void __cdecl CRT_xtoa(u32 value, char *out, u32 radix, s32 is_negative) {
    if (is_negative) {
        *out++ = '-';
        value = 0 - value;
    }
    char *first = out;
    do {
        const u32 digit = value % radix;
        value /= radix;
        *out++ = static_cast<char>(digit > 9 ? digit + 0x57 : digit + 0x30);
    } while (value);
    *out-- = 0;
    do {
        const char c = *first;
        *first = *out;
        *out = c;
        --out;
        ++first;
    } while (first < out);
}

u32 __cdecl CRT_strtoul(const char *text, const char **end, s32 base) { return CRT_strtoxl(text, end, base, 1); }

}  // extern "C"
