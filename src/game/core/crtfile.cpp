#include "crtfile.h"

#include <cstdint>
#include <cstring>

#include "crt.h"
#include "crtopen.h"
#include "imports.h"

namespace {

template <typename F> F import_slot(u32 slot) { return *reinterpret_cast<F *>((uintptr_t)slot); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

void *lock_of(s32 fd) { return crt_handle(fd) + 0xc; }

u32 last_error() { return import_slot<u32(__stdcall *)()>(IMPORT(Kernel32_GetLastError))(); }

bool write_file(u32 handle, const void *data, u32 count, u32 *written) {
    return import_slot<s32(__stdcall *)(u32, const void *, u32, u32 *, void *)>(IMPORT(Kernel32_WriteFile))(handle, data, count, written, nullptr) != 0;
}

// _flsbuf's and _flswbuf's work on a unit of `width` bytes at `unit`: false when it cannot go (flag 0x20).
bool flush_unit(CrtFile *file, const char *unit, s32 width) {
    const s32 fd = file->fd;
    if (!(file->flags & 0x82) || file->flags & 0x40) {
        file->flags |= 0x20;
        return false;
    }
    if (file->flags & 1) {
        file->count = 0;
        if (!(file->flags & 0x10)) {
            file->flags |= 0x20;
            return false;
        }
        file->ptr = file->base;
        file->flags &= ~1u;
    }
    file->count = 0;
    file->flags = (file->flags & ~0x10u) | 2;
    if (!(file->flags & 0x10c)) {
        const bool standard = file == reinterpret_cast<CrtFile *>(0x0077e820) || file == reinterpret_cast<CrtFile *>(0x0077e840);
        if (!standard || !CRT_isatty(static_cast<u32>(fd))) CRT_getbuf(file);
    }
    s32 wanted;
    s32 written = 0;
    if (file->flags & 0x108) {
        char *const base = file->base;
        wanted = static_cast<s32>(file->ptr - base);
        file->ptr = base + width;
        file->count = file->size - width;
        if (wanted > 0) {
            written = CRT_write(static_cast<u32>(fd), base, static_cast<u32>(wanted));
        } else if (crt_handle_or_dummy(fd)[4] & 0x20) {
            CRT_lseek(static_cast<u32>(fd), 0, 2);
        }
        std::memcpy(file->base, unit, static_cast<u32>(width));
    } else {
        wanted = width;
        written = CRT_write(static_cast<u32>(fd), unit, static_cast<u32>(width));
    }
    if (written != wanted) {
        file->flags |= 0x20;
        return false;
    }
    return true;
}

}  // namespace

s32 crt_bad_handle() {
    *CRT_errno() = 9;
    *CRT_doserrno() = 0;
    return -1;
}

extern "C" {

u32 *__cdecl CRT_doserrno() { return reinterpret_cast<u32 *>(CrtInner().getptd() + 0xc); }

void __cdecl CRT_dosmaperr(u32 error) {
    *CRT_doserrno() = error;
    for (u32 at = 0x007811e0; at < 0x00781348; at += 8) {
        if (error == global<u32>(at)) {
            *CRT_errno() = global<s32>(at + 4);
            return;
        }
    }
    *CRT_errno() = error >= 0x13 && error <= 0x24 ? 0xd : error >= 0xbc && error <= 0xca ? 8 : 0x16;
}

u32 __cdecl CRT_get_osfhandle(u32 fd) {
    if (crt_handle_open(fd)) return *reinterpret_cast<u32 *>(crt_handle(static_cast<s32>(fd)));
    return static_cast<u32>(crt_bad_handle());
}

void __cdecl CRT_lock_fhandle(s32 fd) {
    u8 *const record = crt_handle(fd);
    if (!*reinterpret_cast<u32 *>(record + 8)) {
        CrtInner().lock(0x11);
        if (!*reinterpret_cast<u32 *>(record + 8)) {
            import_slot<void(__stdcall *)(void *)>(IMPORT(Kernel32_InitializeCriticalSection))(record + 0xc);
            ++*reinterpret_cast<u32 *>(record + 8);
        }
        CrtInner().unlock(0x11);
    }
    import_slot<void(__stdcall *)(void *)>(IMPORT(Kernel32_EnterCriticalSection))(lock_of(fd));
}

void __cdecl CRT_unlock_fhandle(s32 fd) { import_slot<void(__stdcall *)(void *)>(IMPORT(Kernel32_LeaveCriticalSection))(lock_of(fd)); }

s32 __cdecl CRT_lseek_lk(u32 fd, s32 offset, u32 origin) {
    const u32 handle = CRT_get_osfhandle(fd);
    if (handle == 0xffffffffu) {
        *CRT_errno() = 9;
        return -1;
    }
    const s32 at = import_slot<s32(__stdcall *)(u32, s32, void *, u32)>(IMPORT(Kernel32_SetFilePointer))(handle, offset, nullptr, origin);
    const u32 error = at == -1 ? last_error() : 0;
    if (error) {
        CRT_dosmaperr(error);
        return -1;
    }
    crt_handle(static_cast<s32>(fd))[4] &= 0xfd;
    return at;
}

s32 __cdecl CRT_lseek(u32 fd, s32 offset, u32 origin) {
    if (!crt_handle_open(fd)) return crt_bad_handle();
    CRT_lock_fhandle(static_cast<s32>(fd));
    const s32 at = CRT_lseek_lk(fd, offset, origin);
    CRT_unlock_fhandle(static_cast<s32>(fd));
    return at;
}

int64_t __cdecl CRT_lseeki64_lk(u32 fd, int64_t offset, u32 origin) {
    const u32 handle = CRT_get_osfhandle(fd);
    if (handle == 0xffffffffu) {
        *CRT_errno() = 9;
        return -1;
    }
    s32 high = static_cast<s32>(offset >> 32);
    const u32 low = import_slot<u32(__stdcall *)(u32, s32, s32 *, u32)>(IMPORT(Kernel32_SetFilePointer))(handle, static_cast<s32>(offset), &high, origin);
    if (low == 0xffffffffu) {
        const u32 error = last_error();
        if (error) {
            CRT_dosmaperr(error);
            return -1;
        }
    }
    crt_handle(static_cast<s32>(fd))[4] &= 0xfd;
    return static_cast<int64_t>(static_cast<uint64_t>(static_cast<u32>(high)) << 32 | low);
}

int64_t __cdecl CRT_lseeki64(u32 fd, int64_t offset, u32 origin) {
    if (!crt_handle_open(fd)) return crt_bad_handle();
    CRT_lock_fhandle(static_cast<s32>(fd));
    const int64_t at = CRT_lseeki64_lk(fd, offset, origin);
    CRT_unlock_fhandle(static_cast<s32>(fd));
    return at;
}

s32 __cdecl CRT_isatty(u32 fd) {
    if (fd >= CRT_nhandle) return 0;
    return crt_handle(static_cast<s32>(fd))[4] & 0x40;
}

s32 __cdecl CRT_write_lk(s32 fd, const char *data, u32 count) {
    if (!count) return 0;
    if (crt_handle(fd)[4] & 0x20) CRT_lseek_lk(static_cast<u32>(fd), 0, 2);
    u32 total = 0;
    u32 returns = 0;
    u32 error = 0;
    u32 written;
    const u32 handle = *reinterpret_cast<u32 *>(crt_handle(fd));
    if (crt_handle(fd)[4] & 0x80) {
        const char *from = data;
        for (;;) {
            char buffer[0x404];
            char *to = buffer;
            while (static_cast<u32>(from - data) < count) {
                const char c = *from++;
                if (c == '\n') {
                    ++returns;
                    *to++ = '\r';
                }
                *to++ = c;
                if (to - buffer >= 0x400) break;
            }
            const s32 size = static_cast<s32>(to - buffer);
            if (!write_file(handle, buffer, static_cast<u32>(size), &written)) {
                error = last_error();
                break;
            }
            total += written;
            if (static_cast<s32>(written) < size || static_cast<u32>(from - data) >= count) break;
        }
    } else if (write_file(handle, data, count, &written)) {
        total = written;
    } else {
        error = last_error();
    }
    if (total) return static_cast<s32>(total - returns);
    if (error) {
        if (error == 5) {
            *CRT_errno() = 9;
            *CRT_doserrno() = 5;
        } else {
            CRT_dosmaperr(error);
        }
        return -1;
    }
    if (crt_handle(fd)[4] & 0x40 && *data == 0x1a) return 0;
    *CRT_errno() = 0x1c;
    *CRT_doserrno() = 0;
    return -1;
}

s32 __cdecl CRT_write(u32 fd, const char *data, u32 count) {
    if (!crt_handle_open(fd)) return crt_bad_handle();
    CRT_lock_fhandle(static_cast<s32>(fd));
    const s32 written = CRT_write_lk(static_cast<s32>(fd), data, count);
    CRT_unlock_fhandle(static_cast<s32>(fd));
    return written;
}

void __cdecl CRT_getbuf(CrtFile *file) {
    ++CRT_cflush;
    file->base = static_cast<char *>(CRT_malloc(0x1000));
    if (file->base) {
        file->flags |= 8;
        file->size = 0x1000;
    } else {
        file->flags |= 4;
        file->size = 2;
        file->base = reinterpret_cast<char *>(&file->charBuffer);
    }
    file->ptr = file->base;
    file->count = 0;
}

s32 __cdecl CRT_flush(CrtFile *file) {
    s32 result = 0;
    if ((file->flags & 3) == 2 && file->flags & 0x108) {
        const s32 waiting = static_cast<s32>(file->ptr - file->base);
        if (waiting > 0) {
            if (CRT_write(static_cast<u32>(file->fd), file->base, static_cast<u32>(waiting)) == waiting) {
                if (file->flags & 0x80) file->flags &= ~2u;
            } else {
                file->flags |= 0x20;
                result = -1;
            }
        }
    }
    file->ptr = file->base;
    file->count = 0;
    return result;
}

s32 __cdecl CRT_flsbuf(s32 c, CrtFile *file) {
    const char byte = static_cast<char>(c);
    return flush_unit(file, &byte, 1) ? c & 0xff : -1;
}

u32 __cdecl CRT_flswbuf(u32 c, CrtFile *file) {
    const u16 word = static_cast<u16>(c);
    return flush_unit(file, reinterpret_cast<const char *>(&word), 2) ? word : 0xffffu;
}

u32 __cdecl CRT_fwrite_lk(const void *data, u32 size, u32 items, CrtFile *file) {
    const u32 total = size * items;
    if (!total) return 0;
    const char *from = static_cast<const char *>(data);
    u32 left = total;
    u32 unit = file->flags & 0x10c ? static_cast<u32>(file->size) : 0x1000;
    do {
        const u32 buffered = file->flags & 0x108;
        if (buffered && file->count) {
            const u32 room = static_cast<u32>(file->count);
            const u32 taken = left < room ? left : room;
            std::memcpy(file->ptr, from, taken);
            left -= taken;
            file->count -= static_cast<s32>(taken);
            file->ptr += taken;
            from += taken;
        } else if (left >= unit) {
            if (buffered && CRT_flush(file)) return (total - left) / size;
            const u32 chunk = unit ? left - left % unit : left;
            const s32 written = CRT_write(static_cast<u32>(file->fd), from, chunk);
            if (written == -1) {
                file->flags |= 0x20;
                return (total - left) / size;
            }
            left -= static_cast<u32>(written);
            from += written;
            if (static_cast<u32>(written) < chunk) {
                file->flags |= 0x20;
                return (total - left) / size;
            }
        } else {
            if (CRT_flsbuf(*from, file) == -1) return (total - left) / size;
            ++from;
            --left;
            unit = file->size > 0 ? static_cast<u32>(file->size) : 1;
        }
    } while (left);
    return items;
}

u32 __cdecl CRT_fwrite(const void *data, u32 size, u32 items, CrtFile *file) {
    CrtInner().lockFile(file);
    const u32 written = CRT_fwrite_lk(data, size, items, file);
    CrtInner().unlockFile(file);
    return written;
}

}  // extern "C"
