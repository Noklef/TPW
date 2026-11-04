#include "crtmessage.h"

#include <cstdint>
#include <cstring>

#include "crt.h"

namespace {

template <typename F> F import_slot(u32 slot) { return *reinterpret_cast<F *>((uintptr_t)slot); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

bool lead_byte(char c) { return reinterpret_cast<const u8 *>((uintptr_t)0x00fb1669)[static_cast<u8>(c)] & 4; }

u32 up_to_ff(const char *from, const char *to) {
    const u32 n = static_cast<u32>(to - from);
    return n < 0xff ? n : 0xff;
}

void part(char *out, const char *from, u32 n) {
    CRT_mbsnbcpy(out, from, n);
    out[n] = 0;
}

}  // namespace

extern "C" {

void __cdecl CRT_amsg_exit(s32 code) {
    if (global<u32>(0x00fb135c) == 1) CRT_FF_MSGBANNER();
    CRT_NMSG_WRITE(code);
    global<void(__cdecl *)(s32)>(0x0077ea80)(0xff);
}

void __cdecl CRT_fast_error_exit(s32 code) {
    if (global<u32>(0x00fb135c) == 1) CRT_FF_MSGBANNER();
    CRT_NMSG_WRITE(code);
    import_slot<void(__stdcall *)(u32)>(0x006fd244)(0xff);
}

void __cdecl CRT_FF_MSGBANNER() {
    const u32 mode = global<u32>(0x00fb135c);
    if (mode != 1 && (mode != 0 || global<u32>(0x0077ea84) != 1)) return;
    CRT_NMSG_WRITE(0xfc);
    if (const auto hook = global<void(__cdecl *)()>(0x00fb1888)) hook();
    CRT_NMSG_WRITE(0xff);
}

void __cdecl CRT_NMSG_WRITE(s32 code) {
    const s32 *const table = reinterpret_cast<const s32 *>((uintptr_t)0x00781680);
    s32 entry = 0;
    while (entry < 18 && code != table[2 * entry]) ++entry;
    if (code != table[2 * entry]) return;
    const char *const text = reinterpret_cast<const char *>((uintptr_t)static_cast<u32>(table[2 * entry + 1]));
    const u32 mode = global<u32>(0x00fb135c);
    if (mode == 1 || (mode == 0 && global<u32>(0x0077ea84) == 1)) {
        const u8 *const block = global<const u8 *>(0x00fb3ba0);
        u32 handle = block ? *reinterpret_cast<const u32 *>(block + 0x48) : 0xffffffff;
        if (handle == 0xffffffff) handle = import_slot<u32(__stdcall *)(s32)>(0x006fd1ac)(-12);
        u32 written;
        import_slot<s32(__stdcall *)(u32, const char *, u32, u32 *, void *)>(0x006fd0e0)(handle, text, static_cast<u32>(std::strlen(text)), &written, nullptr);
        return;
    }
    if (code == 0xfc) return;
    char path[0x104];
    if (!import_slot<u32(__stdcall *)(void *, char *, u32)>(0x006fd194)(nullptr, path, 0x104)) std::strcpy(path, "<program name unknown>");
    char *name = path;
    if (std::strlen(path) + 1 > 0x3c) {
        name = path + std::strlen(path) - 0x3b;
        std::memcpy(name, "...", 3);
    }
    char message[0x200];
    std::strcpy(message, "Runtime Error!\n\nProgram: ");
    std::strcat(message, name);
    std::strcat(message, "\n\n");
    std::strcat(message, text);
    CRT_crtMessageBoxA(message, reinterpret_cast<const char *>((uintptr_t)0x00709034), 0x12010);
}

s32 __cdecl CRT_crtMessageBoxA(const char *text, const char *title, u32 type) {
    using Box = s32(__stdcall *)(u32, const char *, const char *, u32);
    using Active = u32(__stdcall *)();
    using Popup = u32(__stdcall *)(u32);
    if (!global<Box>(0x00fb1900)) {
        const u32 library = import_slot<u32(__stdcall *)(u32)>(0x006fd0b4)(0x00709724);
        if (!library) return 0;
        const auto find = import_slot<void *(__stdcall *)(u32, u32)>(0x006fd0b0);
        global<void *>(0x00fb1900) = find(library, 0x00709718);
        if (!global<void *>(0x00fb1900)) return 0;
        global<void *>(0x00fb1904) = find(library, 0x00709708);
        global<void *>(0x00fb1908) = find(library, 0x007096f4);
    }
    u32 owner = 0;
    if (const Active active = global<Active>(0x00fb1904)) owner = active();
    if (owner) {
        if (const Popup popup = global<Popup>(0x00fb1908)) owner = popup(owner);
    }
    return global<Box>(0x00fb1900)(owner, text, title, type);
}

char *__cdecl CRT_mbsnbcpy(char *to, const char *from, u32 n) {
    if (!global<u32>(0x00fb1870)) return Crt().strncpy(to, from, n);
    CrtInner().lock(0x19);
    char *out = to;
    u32 left = n;
    while (left) {
        char c = *from++;
        --left;
        *out++ = c;
        if (lead_byte(c)) {
            if (!left) {
                out[-1] = 0;
                break;
            }
            c = *from++;
            --left;
            *out++ = c;
            if (!c) {
                out[-2] = 0;
                break;
            }
        } else if (!c) {
            break;
        }
    }
    std::memset(out, 0, left);
    CrtInner().unlock(0x19);
    return to;
}

void __cdecl CRT__splitpath(const char *path, char *drive, char *dir, char *name, char *ext) {
    const char *at = path;
    if (*at && at[1] == ':') {
        if (drive) part(drive, at, 2);
        at += 2;
    } else if (drive) {
        *drive = 0;
    }
    const char *slash = nullptr, *dot = nullptr, *end = at;
    for (; *end; ++end) {
        if (lead_byte(*end)) ++end;
        else if (*end == '/' || *end == '\\') slash = end + 1;
        else if (*end == '.') dot = end;
    }
    if (slash) {
        if (dir) part(dir, at, up_to_ff(at, slash));
        at = slash;
    } else if (dir) {
        *dir = 0;
    }
    if (dot && dot >= at) {
        if (name) part(name, at, up_to_ff(at, dot));
        if (ext) part(ext, dot, up_to_ff(dot, end));
    } else {
        if (name) part(name, at, up_to_ff(at, end));
        if (ext) *ext = 0;
    }
}

}
