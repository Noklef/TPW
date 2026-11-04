#include "crtstream.h"

#include <cstdint>
#include <cstring>

#include "crt.h"

namespace {

template <typename F> F import_slot(u32 slot) { return *reinterpret_cast<F *>((uintptr_t)slot); }

bool read_file(u32 handle, void *data, u32 count, u32 *got) {
    return import_slot<s32(__stdcall *)(u32, void *, u32, u32 *, void *)>(0x006fd0dc)(handle, data, count, got, nullptr) != 0;
}

u32 last_error() { return import_slot<u32(__stdcall *)()>(0x006fd138)(); }

bool text_file(const CrtFile *file) { return !(file->flags & 0x40) && crt_handle_or_dummy(file->fd)[4] & 0x80; }

// _filbuf's and _filwbuf's start: the buffer read again, `least` bytes of it taken (fewer: flag 0x10 at the end of the file, otherwise 0x20, and false).
bool refill(CrtFile *file, u32 least) {
    if (!(file->flags & 0x83) || file->flags & 0x40) return false;
    if (file->flags & 2) {
        file->flags |= 0x20;
        return false;
    }
    file->flags |= 1;
    if (file->flags & 0x10c)
        file->ptr = file->base;
    else
        CRT_getbuf(file);
    const s32 got = CRT_read(static_cast<u32>(file->fd), file->base, static_cast<u32>(file->size));
    file->count = got;
    if (got == -1 || static_cast<u32>(got) < least) {
        file->count = 0;
        file->flags |= got ? 0x20 : 0x10;
        return false;
    }
    if (!(file->flags & 0x82) && (crt_handle_or_dummy(file->fd)[4] & 0x82) == 0x82) file->flags |= 0x2000;
    if (file->size == 0x200 && file->flags & 8 && !(file->flags & 0x400)) file->size = 0x1000;
    file->count = got - static_cast<s32>(least);
    return true;
}

s32 next_byte(CrtFile *file) {
    if (--file->count < 0) return CRT_filbuf(file);
    return static_cast<u8>(*file->ptr++);
}

s32 put_byte(char c, CrtFile *file) {
    if (--file->count < 0) return CRT_flsbuf(c, file);
    *file->ptr++ = c;
    return static_cast<u8>(c);
}

bool may_unget(const CrtFile *file) { return file->flags & 1 || (file->flags & 0x80 && !(file->flags & 2)); }

void ungot(CrtFile *file) { file->flags = (file->flags & ~0x10u) | 1; }

}  // namespace

extern "C" {

s32 __cdecl CRT_read_lk(s32 fd, char *data, u32 count) {
    if (!count) return 0;
    u8 *const record = crt_handle(fd);
    if (record[4] & 2) return 0;
    const u32 handle = *reinterpret_cast<u32 *>(record);
    s32 total = 0;
    char *to = data;
    if (record[4] & 0x48 && record[5] != 0xa) {
        *to++ = static_cast<char>(record[5]);
        --count;
        total = 1;
        record[5] = 0xa;
    }
    u32 got;
    if (!read_file(handle, to, count, &got)) {
        const u32 error = last_error();
        if (error == 5) {
            *CRT_errno() = 9;
            *CRT_doserrno() = 5;
            return -1;
        }
        if (error == 0x6d) return 0;
        CRT_dosmaperr(error);
        return -1;
    }
    total += static_cast<s32>(got);
    if (!(record[4] & 0x80)) return total;
    if (got && *data == '\n')
        record[4] |= 4;
    else
        record[4] &= 0xfb;
    const char *const end = data + total;
    const char *from = data;
    to = data;
    while (from < end) {
        const char c = *from;
        if (c == 0x1a) {
            if (!(record[4] & 0x40)) record[4] |= 2;
            break;
        }
        if (c != '\r') {
            *to++ = c;
            ++from;
        } else if (from < end - 1) {
            *to++ = from[1] == '\n' ? '\n' : '\r';
            from += from[1] == '\n' ? 2 : 1;
        } else {
            ++from;
            char next;
            const u32 error = read_file(handle, &next, 1, &got) ? 0 : last_error();
            if (error || !got) {
                *to++ = '\r';
            } else if (record[4] & 0x48) {
                if (next == '\n') {
                    *to++ = '\n';
                } else {
                    *to++ = '\r';
                    record[5] = static_cast<u8>(next);
                }
            } else if (to == data && next == '\n') {
                *to++ = '\n';
            } else {
                CRT_lseek_lk(static_cast<u32>(fd), -1, 1);
                if (next != '\n') *to++ = '\r';
            }
        }
    }
    return static_cast<s32>(to - data);
}

s32 __cdecl CRT_read(u32 fd, char *data, u32 count) {
    if (!crt_handle_open(fd)) return crt_bad_handle();
    CRT_lock_fhandle(static_cast<s32>(fd));
    const s32 got = CRT_read_lk(static_cast<s32>(fd), data, count);
    CRT_unlock_fhandle(static_cast<s32>(fd));
    return got;
}

s32 __cdecl CRT_filbuf(CrtFile *file) {
    if (!refill(file, 1)) return -1;
    return static_cast<u8>(*file->ptr++);
}

u32 __cdecl CRT_filwbuf(CrtFile *file) {
    if (!refill(file, 2)) return 0xffff;
    const u32 c = *reinterpret_cast<u16 *>(file->ptr);
    file->ptr += 2;
    return c;
}

s32 __cdecl CRT_fgetc(CrtFile *file) {
    CrtInner().lockFile(file);
    const s32 c = next_byte(file);
    CrtInner().unlockFile(file);
    return c;
}

u32 __cdecl CRT_fread_lk(char *data, u32 size, u32 count, CrtFile *file) {
    u32 left = count * size;
    const u32 total = left;
    if (!left) return 0;
    u32 chunk = file->flags & 0x10c ? static_cast<u32>(file->size) : 0x1000;
    do {
        if (file->flags & 0x10c && file->count) {
            const u32 take = left < static_cast<u32>(file->count) ? left : static_cast<u32>(file->count);
            std::memcpy(data, file->ptr, take);
            left -= take;
            file->count -= static_cast<s32>(take);
            file->ptr += take;
            data += take;
        } else if (left >= chunk) {
            const s32 got = CRT_read(static_cast<u32>(file->fd), data, chunk ? left - left % chunk : left);
            if (!got) {
                file->flags |= 0x10;
                return (total - left) / size;
            }
            if (got == -1) {
                file->flags |= 0x20;
                return (total - left) / size;
            }
            left -= static_cast<u32>(got);
            data += got;
        } else {
            const s32 c = CRT_filbuf(file);
            if (c == -1) return (total - left) / size;
            *data++ = static_cast<char>(c);
            --left;
            chunk = static_cast<u32>(file->size);
        }
    } while (left);
    return count;
}

u32 __cdecl CRT_fread(char *data, u32 size, u32 count, CrtFile *file) {
    CrtInner().lockFile(file);
    const u32 result = CRT_fread_lk(data, size, count, file);
    CrtInner().unlockFile(file);
    return result;
}

s32 __cdecl CRT_ftell_lk(CrtFile *file) {
    const s32 fd = file->fd;
    if (file->count < 0) file->count = 0;
    s32 place = CRT_lseek(static_cast<u32>(fd), 0, 1);
    if (place < 0) return -1;
    const u32 flags = file->flags;
    if (!(flags & 0x108)) return place - file->count;
    char *const ptr = file->ptr;
    char *const base = file->base;
    s32 offset = static_cast<s32>(ptr - base);
    if (flags & 3) {
        if (crt_handle(fd)[4] & 0x80) {
            for (const char *at = base; at < ptr; ++at)
                if (*at == '\n') ++offset;
        }
    } else if (!(flags & 0x80)) {
        *CRT_errno() = 0x16;
        return -1;
    }
    if (!place) return offset;
    if (!(file->flags & 1)) return place + offset;
    if (!file->count) return place;
    u32 read = static_cast<u32>(ptr - base) + static_cast<u32>(file->count);
    if (crt_handle(fd)[4] & 0x80) {
        if (CRT_lseek(static_cast<u32>(fd), 0, 2) == place) {
            const char *const start = file->base;
            const char *const end = start + read;
            for (const char *at = start; at < end; ++at)
                if (*at == '\n') ++read;
            if (file->flags & 0x2000) ++read;
        } else {
            CRT_lseek(static_cast<u32>(fd), place, 0);
            if (read <= 0x200 && file->flags & 8 && !(file->flags & 0x400))
                read = 0x200;
            else
                read = static_cast<u32>(file->size);
            if (crt_handle(fd)[4] & 4) ++read;
        }
    }
    return place - static_cast<s32>(read) + offset;
}

s32 __cdecl CRT_ftell(CrtFile *file) {
    CrtInner().lockFile(file);
    const s32 result = CRT_ftell_lk(file);
    CrtInner().unlockFile(file);
    return result;
}

s32 __cdecl CRT_fseek_lk(CrtFile *file, s32 offset, s32 origin) {
    if (!(file->flags & 0x83) || (origin != 0 && origin != 1 && origin != 2)) {
        *CRT_errno() = 0x16;
        return -1;
    }
    file->flags &= ~0x10;
    if (origin == 1) {
        offset += CRT_ftell_lk(file);
        origin = 0;
    }
    CRT_flush(file);
    if (file->flags & 0x80) file->flags &= ~3;
    else if (file->flags & 1 && file->flags & 8 && !(file->flags & 0x400)) file->size = 0x200;
    return CRT_lseek(static_cast<u32>(file->fd), offset, static_cast<u32>(origin)) == -1 ? -1 : 0;
}

s32 __cdecl CRT_fseek(CrtFile *file, s32 offset, s32 origin) {
    CrtInner().lockFile(file);
    const s32 result = CRT_fseek_lk(file, offset, origin);
    CrtInner().unlockFile(file);
    return result;
}

s32 __cdecl CRT_stbuf(CrtFile *file) {
    if (!CRT_isatty(static_cast<u32>(file->fd))) return 0;
    u32 slot;
    if (reinterpret_cast<uintptr_t>(file) == 0x0077e820) slot = 0;
    else if (reinterpret_cast<uintptr_t>(file) == 0x0077e840) slot = 1;
    else return 0;
    ++*reinterpret_cast<u32 *>((uintptr_t)0x00fb1350);
    if (file->flags & 0x10c) return 0;
    char *&kept = reinterpret_cast<char **>((uintptr_t)0x00fb1540)[slot];
    if (!kept) kept = static_cast<char *>(CRT_malloc(0x1000));
    if (kept) {
        file->size = 0x1000;
        file->base = file->ptr = kept;
        file->count = 0x1000;
    } else {
        file->base = file->ptr = reinterpret_cast<char *>(&file->charBuffer);
        file->size = file->count = 2;
    }
    file->flags |= 0x1102;
    return 1;
}

void __cdecl CRT_ftbuf(s32 flag, CrtFile *file) {
    if (!flag || !(file->flags & 0x1000)) return;
    CRT_flush(file);
    file->flags &= ~0x1100;
    file->size = 0;
    file->ptr = nullptr;
    file->base = nullptr;
}

s32 __cdecl CRT_ungetc_lk(s32 c, CrtFile *file) {
    if (c == -1 || !may_unget(file)) return -1;
    if (!file->base) CRT_getbuf(file);
    if (file->ptr == file->base) {
        if (file->count) return -1;
        ++file->ptr;
    }
    --file->ptr;
    if (!(file->flags & 0x40)) {
        *file->ptr = static_cast<char>(c);
    } else if (*file->ptr != static_cast<char>(c)) {
        ++file->ptr;
        return -1;
    }
    ungot(file);
    ++file->count;
    return c & 0xff;
}

s32 __cdecl CRT_ungetc(s32 c, CrtFile *file) {
    CrtInner().lockFile(file);
    const s32 result = CRT_ungetc_lk(c, file);
    CrtInner().unlockFile(file);
    return result;
}

u16 __cdecl CRT_fgetwc_lk(CrtFile *file) {
    if (!text_file(file)) {
        file->count -= 2;
        if (file->count < 0) return static_cast<u16>(CRT_filwbuf(file));
        const u16 c = *reinterpret_cast<u16 *>(file->ptr);
        file->ptr += 2;
        return c;
    }
    char bytes[2];
    u32 size = 1;
    const s32 c = next_byte(file);
    if (c == -1) return 0xffff;
    bytes[0] = static_cast<char>(c);
    if ((*reinterpret_cast<const u16 *const *>((uintptr_t)0x0077ea90))[c & 0xff] & 0x8000) {
        const s32 second = next_byte(file);
        if (second == -1) {
            CRT_ungetc(bytes[0], file);
            return 0xffff;
        }
        bytes[1] = static_cast<char>(second);
        size = 2;
    }
    // The original's answer is the file argument's own slot: its low word stands when mbtowc writes nothing.
    u16 wide = static_cast<u16>(reinterpret_cast<uintptr_t>(file));
    if (CRT_mbtowc(&wide, bytes, size) == -1) {
        *CRT_errno() = 0x2a;
        return 0xffff;
    }
    return wide;
}

u16 __cdecl CRT_fgetwc(CrtFile *file) {
    CrtInner().lockFile(file);
    const u16 c = CRT_fgetwc_lk(file);
    CrtInner().unlockFile(file);
    return c;
}

u16 __cdecl CRT_fputwc_lk(u32 c, CrtFile *file) {
    if (!text_file(file)) {
        file->count -= 2;
        if (file->count < 0) return static_cast<u16>(CRT_flswbuf(c & 0xffff, file));
        *reinterpret_cast<u16 *>(file->ptr) = static_cast<u16>(c);
        file->ptr += 2;
        return static_cast<u16>(c);
    }
    char bytes[8];
    const s32 size = CRT_wctomb(bytes, c);
    if (size == -1) {
        *CRT_errno() = 0x2a;
        return 0xffff;
    }
    if (put_byte(bytes[0], file) == -1) return 0xffff;
    if (size != 1 && put_byte(bytes[1], file) == -1) return 0xffff;
    return static_cast<u16>(c);
}

u16 __cdecl CRT_fputwc(u32 c, CrtFile *file) {
    CrtInner().lockFile(file);
    const u16 result = CRT_fputwc_lk(c, file);
    CrtInner().unlockFile(file);
    return result;
}

u16 __cdecl CRT_ungetwc_lk(u32 c, CrtFile *file) {
    if (static_cast<u16>(c) == 0xffff || !may_unget(file)) return 0xffff;
    if (!file->base) CRT_getbuf(file);
    if (!text_file(file)) {
        if (file->ptr == file->base) {
            if (file->count) return 0xffff;
            file->ptr += 2;
        }
        file->count += 2;
        file->ptr -= 2;
        *reinterpret_cast<u16 *>(file->ptr) = static_cast<u16>(c);
        ungot(file);
        return static_cast<u16>(c);
    }
    // The original's buffer is the character's own argument slot.
    u32 slot = c;
    char *const bytes = reinterpret_cast<char *>(&slot);
    const s32 size = CRT_wctomb(bytes, c);
    if (size == -1) {
        *CRT_errno() = 0x2a;
        return 0xffff;
    }
    if (file->ptr == file->base) {
        if (file->count) return 0xffff;
        file->ptr += size;
    }
    if (size != 1) *--file->ptr = bytes[1];
    *--file->ptr = bytes[0];
    file->count += size;
    ungot(file);
    return static_cast<u16>(c);
}

u16 __cdecl CRT_ungetwc(u32 c, CrtFile *file) {
    CrtInner().lockFile(file);
    const u16 result = CRT_ungetwc_lk(c, file);
    CrtInner().unlockFile(file);
    return result;
}

}  // extern "C"
