#include "crtopen.h"

#include <cstdint>
#include <cstring>

#include "crt.h"
#include "crtenv.h"
#include "crtmbstring.h"
#include "crtstream.h"
#include "crttime.h"
#include "crtstartup.h"
#include "crtmessage.h"

namespace {

template <typename F> F import_slot(u32 slot) { return *reinterpret_cast<F *>((uintptr_t)slot); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

CrtFile *&stream(s32 index) { return CRT_piob[index]; }

u8 *&handle_block(u32 index) { return CRT_pioinfo[index]; }

u32 &handle_count() { return CRT_nhandle; }

void init_section(void *section) { import_slot<void(__stdcall *)(void *)>(0x006fd144)(section); }
void enter_section(void *section) { import_slot<void(__stdcall *)(void *)>(0x006fd140)(section); }
void leave_section(void *section) { import_slot<void(__stdcall *)(void *)>(0x006fd130)(section); }

u32 last_error() { return import_slot<u32(__stdcall *)()>(0x006fd138)(); }
s32 close_handle(u32 handle) { return import_slot<s32(__stdcall *)(u32)>(0x006fd13c)(handle); }

void set_std_handle(u32 fd, u32 handle) {
    if (CRT_app_type == 1 && fd <= 2) import_slot<void(__stdcall *)(s32, u32)>(0x006fd268)(-10 - static_cast<s32>(fd), handle);
}

s32 invalid_open() {
    *CRT_errno() = 0x16;
    *CRT_doserrno() = 0;
    return -1;
}

s32 failed_open(s32 fd) {
    CRT_dosmaperr(last_error());
    CRT_unlock_fhandle(fd);
    return -1;
}

struct FindData {   // WIN32_FIND_DATAA
    u32 attributes, created[2], accessed[2], written[2], size_high, size_low, reserved[2];
    char name[0x104], short_name[0xe];
};

// _stat's ENOENT: errno and __doserrno 2.
s32 no_entry() {
    *CRT_errno() = 2;
    *CRT_doserrno() = 2;
    return -1;
}

s32 failed_find(u32 handle) {
    CRT_dosmaperr(last_error());
    import_slot<s32(__stdcall *)(u32)>(0x006fd104)(handle);   // FindClose
    return -1;
}

// A FILETIME by FileTimeToLocalFileTime, FileTimeToSystemTime and _loctotime_t; false when a conversion fails.
bool file_time(const u32 *time, u32 *out) {
    u32 local[2];
    u16 when[8];
    if (!import_slot<u32(__stdcall *)(const u32 *, u32 *)>(0x006fd15c)(time, local) || !import_slot<u32(__stdcall *)(const u32 *, u16 *)>(0x006fd0bc)(local, when)) return false;
    *out = CRT_loctotime_t(when[0], when[1], when[3], when[4], when[5], when[6], -1);
    return true;
}

}  // namespace

extern "C" {

CrtFile *__cdecl CRT_getstream() {
    CrtFile *found = nullptr;
    CrtInner().lock(2);
    for (s32 i = 0; i < CRT_nstream; ++i) {
        if (!stream(i)) {
            stream(i) = static_cast<CrtFile *>(CRT_malloc(0x38));
            if (stream(i)) {
                u8 *const section = reinterpret_cast<u8 *>(stream(i)) + 0x20;
                init_section(section);
                enter_section(section);
                found = stream(i);
            }
            break;
        }
        if (!(stream(i)->flags & 0x83)) {
            CrtInner().lockFile2(i, stream(i));
            if (!(stream(i)->flags & 0x83)) {
                found = stream(i);
                break;
            }
            CrtInner().unlockFile2(i, stream(i));
        }
    }
    if (found) {
        found->count = 0;
        found->flags = 0;
        found->base = nullptr;
        found->ptr = nullptr;
        found->name = nullptr;
        found->fd = -1;
    }
    CrtInner().unlock(2);
    return found;
}

CrtFile *__cdecl CRT_openfile(const char *name, const char *mode, u32 share, CrtFile *file) {
    u32 oflag;
    u32 flags = CRT_commode;
    switch (*mode) {
    case 'a':
        oflag = 0x109;
        flags |= 2;
        break;
    case 'r':
        oflag = 0;
        flags |= 1;
        break;
    case 'w':
        oflag = 0x301;
        flags |= 2;
        break;
    default:
        return nullptr;
    }
    bool commit = false, access = false, more = true;
    for (++mode; *mode && more; ++mode) {
        switch (*mode) {
        case '+':
            if (oflag & 2) {
                more = false;
            } else {
                oflag = (oflag & ~1u) | 2;
                flags = (flags & ~3u) | 0x80;
            }
            break;
        case 'b':
            if (oflag & 0xc000) more = false;
            else oflag |= 0x8000;
            break;
        case 't':
            if (oflag & 0xc000) more = false;
            else oflag |= 0x4000;
            break;
        case 'c':
            if (commit) {
                more = false;
            } else {
                commit = true;
                flags |= 0x4000;
            }
            break;
        case 'n':
            if (commit) {
                more = false;
            } else {
                commit = true;
                flags &= 0xffffbfff;
            }
            break;
        case 'S':
            if (access) {
                more = false;
            } else {
                access = true;
                oflag |= 0x20;
            }
            break;
        case 'R':
            if (access) {
                more = false;
            } else {
                access = true;
                oflag |= 0x10;
            }
            break;
        case 'T':
            if (oflag & 0x1000) more = false;
            else oflag |= 0x1000;
            break;
        case 'D':
            if (oflag & 0x40) more = false;
            else oflag |= 0x40;
            break;
        default:
            more = false;
            break;
        }
    }
    const s32 fd = CRT_sopen(name, oflag, share, 0x1a4);
    if (fd < 0) return nullptr;
    ++CRT_cflush;
    file->flags = flags;
    file->count = 0;
    file->ptr = nullptr;
    file->base = nullptr;
    file->name = nullptr;
    file->fd = fd;
    return file;
}

CrtFile *__cdecl CRT_fsopen(const char *name, const char *mode, u32 share) {
    CrtFile *const file = CRT_getstream();
    if (!file) return nullptr;
    CrtFile *const opened = CRT_openfile(name, mode, share, file);
    CrtInner().unlockFile(file);
    return opened;
}

CrtFile *__cdecl CRT_fopen(const char *name, const char *mode) { return CRT_fsopen(name, mode, 0x40); }

s32 __cdecl CRT_sopen(const char *path, u32 oflag, u32 shflag, u32 pmode) {
    struct Security {
        u32 length;
        void *descriptor;
        u32 inherit;
    } security = {0xc, nullptr, oflag & 0x80 ? 0u : 1u};
    u8 flags = oflag & 0x80 ? 0x10 : 0;
    if (!(oflag & 0x8000) && (oflag & 0x4000 || CRT_fmode != 0x8000)) flags |= 0x80;
    u32 desired;
    switch (oflag & 3) {
    case 0: desired = 0x80000000; break;
    case 1: desired = 0x40000000; break;
    case 2: desired = 0xc0000000; break;
    default: return invalid_open();
    }
    u32 sharing;
    switch (shflag) {
    case 0x10: sharing = 0; break;
    case 0x20: sharing = 1; break;
    case 0x30: sharing = 2; break;
    case 0x40: sharing = 3; break;
    default: return invalid_open();
    }
    static const u32 kCreation[8] = {3, 4, 5, 2, 3, 1, 5, 1};
    const u32 creation = kCreation[oflag >> 8 & 7];
    u32 attributes = 0x80;
    if (oflag & 0x100 && !(pmode & ~CRT_umaskval & 0x80)) attributes = 1;
    if (oflag & 0x40) {
        attributes |= 0x4000000;
        desired |= 0x10000;
    }
    if (oflag & 0x1000) attributes |= 0x100;
    if (oflag & 0x20) attributes |= 0x8000000;
    else if (oflag & 0x10) attributes |= 0x10000000;
    const s32 fd = CRT_alloc_osfhnd();
    if (fd == -1) {
        *CRT_errno() = 0x18;
        *CRT_doserrno() = 0;
        return -1;
    }
    const u32 handle = import_slot<u32(__stdcall *)(const char *, u32, u32, Security *, u32, u32, u32)>(0x006fd0d8)(path, desired, sharing, &security, creation, attributes, 0);
    if (handle == 0xffffffff) return failed_open(fd);
    const u32 type = import_slot<u32(__stdcall *)(u32)>(0x006fd1a8)(handle);
    if (!type) {
        close_handle(handle);
        return failed_open(fd);
    }
    if (type == 2) flags |= 0x40;
    else if (type == 3) flags |= 8;
    CRT_set_osfhnd(fd, handle);
    flags |= 1;
    crt_handle(fd)[4] = flags;
    const bool device = flags & 0x48;
    if (!device && flags & 0x80 && oflag & 2) {
        const s32 end = CRT_lseek_lk(fd, -1, 2);
        bool failed = false;
        if (end == -1) {
            failed = *CRT_doserrno() != 0x83;
        } else {
            char last = 0;
            if (CRT_read_lk(fd, &last, 1) == 0 && last == 0x1a && CRT_chsize_lk(fd, end) == -1) failed = true;
            else failed = CRT_lseek_lk(fd, 0, 0) == -1;
        }
        if (failed) {
            CRT_close(fd);
            CRT_unlock_fhandle(fd);
            return -1;
        }
    }
    if (!device && oflag & 8) crt_handle(fd)[4] |= 0x20;
    CRT_unlock_fhandle(fd);
    return fd;
}

s32 __cdecl CRT_alloc_osfhnd() {
    s32 fd = -1;
    CrtInner().lock(0x12);
    for (u32 i = 0; i < 0x40 && fd == -1; ++i) {
        if (!handle_block(i)) {
            u8 *const block = static_cast<u8 *>(CRT_malloc(0x480));
            if (block) {
                handle_block(i) = block;
                handle_count() += 0x20;
                for (u8 *slot = block; slot < block + 0x480; slot += 0x24) {
                    slot[4] = 0;
                    *reinterpret_cast<u32 *>(slot) = 0xffffffff;
                    slot[5] = 0xa;
                    *reinterpret_cast<u32 *>(slot + 8) = 0;
                }
                fd = static_cast<s32>(i << 5);
                CRT_lock_fhandle(fd);
            }
            break;
        }
        for (u8 *slot = handle_block(i); slot < handle_block(i) + 0x480; slot += 0x24) {
            if (slot[4] & 1) continue;
            u32 &made = *reinterpret_cast<u32 *>(slot + 8);
            if (!made) {
                CrtInner().lock(0x11);
                if (!made) {
                    init_section(slot + 0xc);
                    ++made;
                }
                CrtInner().unlock(0x11);
            }
            enter_section(slot + 0xc);
            if (!(slot[4] & 1)) {
                *reinterpret_cast<u32 *>(slot) = 0xffffffff;
                fd = static_cast<s32>((slot - handle_block(i)) / 0x24 + (i << 5));
                break;
            }
            leave_section(slot + 0xc);
        }
    }
    CrtInner().unlock(0x12);
    return fd;
}

s32 __cdecl CRT_set_osfhnd(u32 fd, u32 handle) {
    if (fd < handle_count()) {
        u8 *const slot = crt_handle(static_cast<s32>(fd));
        if (*reinterpret_cast<u32 *>(slot) == 0xffffffff) {
            set_std_handle(fd, handle);
            *reinterpret_cast<u32 *>(slot) = handle;
            return 0;
        }
    }
    return crt_bad_handle();
}

s32 __cdecl CRT_free_osfhnd(u32 fd) {
    if (crt_handle_open(fd)) {
        u8 *const slot = crt_handle(static_cast<s32>(fd));
        if (*reinterpret_cast<u32 *>(slot) != 0xffffffff) {
            set_std_handle(fd, 0);
            *reinterpret_cast<u32 *>(slot) = 0xffffffff;
            return 0;
        }
    }
    return crt_bad_handle();
}

s32 __cdecl CRT_close(u32 fd) {
    if (!crt_handle_open(fd)) return crt_bad_handle();
    CRT_lock_fhandle(static_cast<s32>(fd));
    const s32 result = CRT_close_lk(fd);
    CRT_unlock_fhandle(static_cast<s32>(fd));
    return result;
}

s32 __cdecl CRT_close_lk(u32 fd) {
    u32 error = 0;
    if (CRT_get_osfhandle(fd) != 0xffffffff && !((fd == 1 || fd == 2) && CRT_get_osfhandle(1) == CRT_get_osfhandle(2))) {
        if (!close_handle(CRT_get_osfhandle(fd))) error = last_error();
    }
    CRT_free_osfhnd(fd);
    crt_handle(static_cast<s32>(fd))[4] = 0;
    if (!error) return 0;
    CRT_dosmaperr(error);
    return -1;
}

s32 __cdecl CRT_chsize_lk(s32 fd, s32 size) {
    const u32 file = static_cast<u32>(fd);
    const s32 position = CRT_lseek_lk(file, 0, 1);
    if (position == -1) return -1;
    const s32 end = CRT_lseek_lk(file, 0, 2);
    if (end == -1) return -1;
    s32 result = 0;
    s32 missing = size - end;
    if (missing > 0) {
        const char zeros[0x1000] = {};
        const s32 mode = CRT_setmode_lk(fd, 0x8000);
        do {
            const s32 written = CRT_write_lk(fd, zeros, static_cast<u32>(missing < 0x1000 ? missing : 0x1000));
            if (written == -1) {
                if (*CRT_doserrno() == 5) *CRT_errno() = 0xd;
                result = -1;
                break;
            }
            missing -= written;
        } while (missing > 0);
        CRT_setmode_lk(fd, static_cast<u32>(mode));
        CRT_lseek_lk(file, position, 0);
        return result;
    }
    if (missing < 0) {
        CRT_lseek_lk(file, size, 0);
        if (!import_slot<s32(__stdcall *)(u32)>(0x006fd27c)(CRT_get_osfhandle(file))) {
            result = -1;
            *CRT_errno() = 0xd;
            *CRT_doserrno() = last_error();
        }
    }
    CRT_lseek_lk(file, position, 0);
    return result;
}

s32 __cdecl CRT_setmode_lk(s32 fd, u32 mode) {
    u8 &flags = crt_handle(fd)[4];
    const bool text = flags & 0x80;
    if (mode == 0x8000) {
        flags &= 0x7f;
    } else if (mode == 0x4000) {
        flags |= 0x80;
    } else {
        *CRT_errno() = 0x16;
        return -1;
    }
    return text ? 0x4000 : 0x8000;
}

void __cdecl CRT_freebuf(CrtFile *file) {
    if (file->flags & 0x83 && file->flags & 8) {
        Crt().free(file->base);
        file->flags &= 0xfffffbf7;
        file->ptr = nullptr;
        file->base = nullptr;
        file->count = 0;
    }
}

s32 __cdecl CRT_setvbuf(CrtFile *file, char *buffer, u32 type, u32 size) {
    if (type != 4 && (size < 2 || size > 0x7fffffff || (type != 0 && type != 0x40))) return -1;
    size &= ~1u;
    CrtInner().lockFile(file);
    CRT_flush(file);
    CRT_freebuf(file);
    file->flags &= 0xffffc2f3;
    char *base = buffer;
    if (type & 4) {
        file->flags |= 4;
        base = reinterpret_cast<char *>(&file->charBuffer);
        size = 2;
    } else if (base) {
        file->flags |= 0x500;
    } else {
        base = static_cast<char *>(CRT_malloc(size));
        if (!base) {
            ++CRT_cflush;
            CrtInner().unlockFile(file);
            return -1;
        }
        file->flags |= 0x408;
    }
    file->size = static_cast<s32>(size);
    file->base = base;
    file->ptr = base;
    file->count = 0;
    CrtInner().unlockFile(file);
    return 0;
}

s32 __cdecl CRT_fflush_lk(CrtFile *file) {
    if (CRT_flush(file)) return -1;
    if (file->flags & 0x4000) return CRT_commit(static_cast<u32>(file->fd)) ? -1 : 0;
    return 0;
}

s32 __cdecl CRT_fflush(CrtFile *file) {
    if (!file) return CRT_flsall(0);
    CrtInner().lockFile(file);
    const s32 result = CRT_fflush_lk(file);
    CrtInner().unlockFile(file);
    return result;
}

s32 __cdecl CRT_flsall(s32 mode) {
    s32 flushed = 0, error = 0;
    CrtInner().lock(2);
    for (s32 i = 0; i < CRT_nstream; ++i) {
        if (!stream(i) || !(stream(i)->flags & 0x83)) continue;
        CrtInner().lockFile2(i, stream(i));
        CrtFile *const file = stream(i);
        const u32 flags = file->flags;
        if (flags & 0x83) {
            if (mode == 1) {
                if (CRT_fflush_lk(file) != -1) ++flushed;
            } else if (mode == 0 && flags & 2) {
                if (CRT_fflush_lk(file) == -1) error = -1;
            }
        }
        CrtInner().unlockFile2(i, stream(i));
    }
    CrtInner().unlock(2);
    return mode == 1 ? flushed : error;
}

s32 __cdecl CRT_flushall() { return CRT_flsall(1); }

u32 __cdecl CRT_dtoxmode(u32 attributes, const char *path) {
    const char *const root = path[1] == ':' ? path + 2 : path;
    const bool slash = root[0] == '\\' || root[0] == '/';
    u32 mode = (slash && !root[1]) || attributes & 0x10 || !root[0] ? 0x4040 : 0x8000;
    mode |= (2 | (~attributes & 1)) << 7;
    const char *const extension = CRT_mbsrchr(path, 0x2e);
    if (extension) {
        const char *const texts[] = {CRT_text_exe_708a3c, CRT_text_cmd_708a34, CRT_text_bat_708a2c, CRT_text_com_708a24};
        for (const char *const text : texts) {
            if (!CRT_mbsicmp(extension, text)) {
                mode |= 0x40;
                break;
            }
        }
    }
    mode |= (mode >> 3) & 0x38;
    return (mode >> 6) & 7 | mode;
}

u32 __cdecl CRT_IsRootUNCName(const char *path) {
    const auto is_slash = [](char c) { return c == '\\' || c == '/'; };
    const auto past_name = [&](const char *at) {
        while (*at && !is_slash(*at)) ++at;
        return at;
    };
    u32 length = 0;
    while (path[length]) ++length;
    if (length < 5 || !is_slash(path[0]) || !is_slash(path[1])) return 0;
    const char *at = past_name(path + 3);
    if (!*at || !at[1]) return 0;
    at = past_name(at + 1);
    return *at && at[1] ? 0 : 1;
}

s32 __cdecl CRT_fclose_lk(CrtFile *file) {
    s32 result = -1;
    if (file->flags & 0x83) {
        result = CRT_flush(file);
        CRT_freebuf(file);
        if (CRT_close(static_cast<u32>(file->fd)) < 0) {
            file->flags = 0;
            return -1;
        }
        if (file->name) {
            Crt().free(file->name);
            file->name = nullptr;
        }
    }
    file->flags = 0;
    return result;
}

s32 __cdecl CRT_fcloseall() {
    s32 count = 0;
    CrtInner().lock(2);
    for (s32 i = 3; i < CRT_nstream; ++i) {
        if (!stream(i)) continue;
        if (stream(i)->flags & 0x83 && CRT_fclose(stream(i)) != -1) ++count;
        if (i >= 0x14) {
            import_slot<void(__stdcall *)(void *)>(0x006fd0d0)(reinterpret_cast<u8 *>(stream(i)) + 0x20);
            Crt().free(stream(i));
            stream(i) = nullptr;
        }
    }
    CrtInner().unlock(2);
    return count;
}

s32 __cdecl CRT_commit(u32 fd) {
    if (crt_handle_open(fd)) {
        CRT_lock_fhandle(static_cast<s32>(fd));
        s32 result = -1;
        bool bad = true;
        if (crt_handle(static_cast<s32>(fd))[4] & 1) {
            const u32 error = import_slot<s32(__stdcall *)(u32)>(0x006fd0fc)(CRT_get_osfhandle(fd)) ? 0 : last_error();
            if (error) *CRT_doserrno() = error;
            else bad = false;
        }
        if (bad) *CRT_errno() = 9;
        else result = 0;
        CRT_unlock_fhandle(static_cast<s32>(fd));
        return result;
    }
    *CRT_errno() = 9;
    return -1;
}

s32 __cdecl CRT_remove(const char *path) {
    const u32 error = import_slot<s32(__stdcall *)(const char *)>(0x006fd0c4)(path) ? 0 : last_error();    // DeleteFileA
    if (!error) return 0;
    CRT_dosmaperr(error);
    return -1;
}

s32 __cdecl CRT_unlink(const char *path) { return CRT_remove(path); }

s32 __cdecl CRT_rename(const char *from, const char *to) {
    const u32 error = import_slot<s32(__stdcall *)(const char *, const char *)>(0x006fd11c)(from, to) ? 0 : last_error();    // MoveFileA
    if (!error) return 0;
    CRT_dosmaperr(error);
    return -1;
}

void __cdecl CRT_ioinit() {
    const auto clear_block = [](u32 index) {
        for (u8 *slot = handle_block(index); slot < handle_block(index) + 0x480; slot += 0x24) {
            slot[4] = 0;
            *reinterpret_cast<u32 *>(slot) = 0xffffffff;
            slot[5] = 0xa;
            *reinterpret_cast<u32 *>(slot + 8) = 0;
        }
    };
    const auto file_type = [](u32 handle) { return import_slot<u32(__stdcall *)(u32)>(0x006fd1a8)(handle); };
    u8 *const block = static_cast<u8 *>(CRT_malloc(0x480));
    if (!block) CrtInner().amsgExit(0x1b);
    handle_block(0) = block;
    handle_count() = 0x20;
    clear_block(0);
    u8 info[0x44];
    import_slot<void(__stdcall *)(void *)>(0x006fd23c)(info);    // GetStartupInfoA
    const u8 *const passed = *reinterpret_cast<u8 *const *>(info + 0x34);
    if (*reinterpret_cast<const u16 *>(info + 0x32) && passed) {
        s32 count = *reinterpret_cast<const s32 *>(passed);
        const u8 *flags = passed + 4;
        const u32 *handles = reinterpret_cast<const u32 *>(flags + count);
        if (count >= 0x800) count = 0x800;
        if (static_cast<s32>(handle_count()) < count) {
            for (u32 i = 1;; ++i) {
                u8 *const more = static_cast<u8 *>(CRT_malloc(0x480));
                if (!more) {
                    count = static_cast<s32>(handle_count());
                    break;
                }
                handle_block(i) = more;
                handle_count() += 0x20;
                clear_block(i);
                if (static_cast<s32>(handle_count()) >= count) break;
            }
        }
        for (s32 i = 0; i < count; ++i, ++flags, ++handles) {
            if (*handles == 0xffffffff || !(*flags & 1)) continue;
            if (!(*flags & 8) && !file_type(*handles)) continue;
            u8 *const slot = crt_handle(i);
            *reinterpret_cast<u32 *>(slot) = *handles;
            slot[4] = *flags;
        }
    }
    for (s32 i = 0; i < 3; ++i) {
        u8 *const slot = handle_block(0) + i * 0x24;
        if (*reinterpret_cast<u32 *>(slot) != 0xffffffff) {
            slot[4] |= 0x80;
            continue;
        }
        slot[4] = 0x81;
        const u32 handle = import_slot<u32(__stdcall *)(s32)>(0x006fd1ac)(i == 0 ? -10 : i == 1 ? -11 : -12);    // GetStdHandle
        u32 type;
        if (handle != 0xffffffff && (type = file_type(handle)) != 0) {
            *reinterpret_cast<u32 *>(slot) = handle;
            if ((type & 0xff) == 2)
                slot[4] |= 0x40;
            else if ((type & 0xff) == 3)
                slot[4] |= 8;
        } else {
            slot[4] |= 0x40;
        }
    }
    import_slot<u32(__stdcall *)(u32)>(0x006fd1b0)(handle_count());    // SetHandleCount
}

void __cdecl CRT_initstdio() {
    s32 &count = CRT_nstream;
    u32 *&table = reinterpret_cast<u32 *&>(CRT_piob);   // (the _iob records by address)
    if (!count)
        count = 0x200;
    else if (count < 0x14)
        count = 0x14;
    table = static_cast<u32 *>(CrtInner().calloc(static_cast<u32>(count), 4));
    if (!table) {
        count = 0x14;
        table = static_cast<u32 *>(CrtInner().calloc(0x14, 4));
        if (!table) CrtInner().amsgExit(0x1a);
    }
    for (u32 i = 0, file = 0x0077e800; file < 0x0077ea80; file += 0x20, ++i) table[i] = file;
    for (s32 i = 0; i < 3; ++i) {
        const u32 handle = *reinterpret_cast<const u32 *>(crt_handle(i));
        if (handle == 0xffffffff || !handle) global<u32>(0x0077e810 + i * 0x20) = 0xffffffff;
    }
}

void __cdecl CRT_endstdio() {
    CRT_flushall();
    if (CRT_exitflag) CRT_fcloseall();
}

void __cdecl CRT_init_namebuf(s32 flag) {
    char *const name = flag ? CRT_tmpfile_buffer : CRT_tmpnam_buffer;
    *reinterpret_cast<u16 *>(name) = global<u16>(0x007477fc);
    char *at = name + 1;
    if (name[0] != '\\' && name[0] != '/') *at++ = '\\';
    *at++ = flag ? 't' : 's';
    CRT_ultoa(import_slot<u32(__stdcall *)()>(0x006fd1e8)(), at, 32);   // (_getpid 0x68b9e0: JMP [0x6fd1e8])
    std::strcat(name, reinterpret_cast<const char *>((uintptr_t)0x00747800));
}

CrtFile *__cdecl CRT_tmpfile() {
    char *const name = CRT_tmpfile_buffer;
    CrtInner().getptd();   // (its answer unused, sic)
    CrtInner().lock(3);
    if (!name[0]) {
        CRT_init_namebuf(1);
    } else if (CRT_genfname(name)) {
        CrtInner().unlock(3);
        return nullptr;
    }
    CrtFile *const file = CRT_getstream();
    if (!file) {
        CrtInner().unlock(3);
        return nullptr;
    }
    s32 fd = CRT_sopen(name, 0x8542, 0x40, 0x180);
    while (fd == -1 && *CRT_errno() == 0x11 && !CRT_genfname(name)) fd = CRT_sopen(name, 0x8542, 0x40, 0x180);
    if (fd != -1) {
        file->name = CRT_strdup(name);
        if (file->name) {
            file->count = 0;
            file->ptr = nullptr;
            file->base = nullptr;
            file->flags = CRT_commode | 0x80;
            file->fd = fd;
            CrtInner().unlockFile(file);
            CrtInner().unlock(3);
            return file;
        }
        CRT_close(fd);
    }
    CrtInner().unlockFile(file);
    CrtInner().unlock(3);
    return nullptr;
}

s32 __cdecl CRT_rmtmp() {
    s32 count = 0;
    CrtInner().lock(2);
    for (s32 i = 0; i < CRT_nstream; ++i) {
        if (!stream(i)) continue;
        CrtInner().lockFile2(i, stream(i));
        CrtFile *const file = stream(i);
        if (file->flags & 0x83 && file->name) {
            CrtInner().fcloseLocked(file);
            ++count;
        }
        CrtInner().unlockFile2(i, stream(i));
    }
    CrtInner().unlock(2);
    return count;
}

s32 __cdecl CRT_validdrive(u32 drive) {
    if (!drive) return 1;
    const char root[4] = {static_cast<char>(drive + 0x40), ':', '\\', 0};
    const u32 type = import_slot<u32(__stdcall *)(const char *)>(0x006fd09c)(root);
    return type != 0 && type != 1;
}

char *__cdecl CRT_getdcwd_lk(u32 drive, char *buffer, s32 size) {
    char path[0x104];
    u32 length;
    if (drive) {
        if (!CRT_validdrive(drive)) {
            *CRT_doserrno() = 0xf;
            *CRT_errno() = 0xd;
            return nullptr;
        }
        const char name[4] = {static_cast<char>(drive + 0x40), ':', '.', 0};
        char *part;
        length = import_slot<u32(__stdcall *)(const char *, u32, char *, char **)>(0x006fd214)(name, 0x104, path, &part);
    } else {
        length = import_slot<u32(__stdcall *)(u32, char *)>(0x006fd118)(0x104, path);
    }
    if (!length || ++length > 0x104) return nullptr;
    if (!buffer) {
        buffer = static_cast<char *>(CRT_malloc(static_cast<s32>(length) > size ? length : static_cast<u32>(size)));
        if (!buffer) {
            *CRT_errno() = 0xc;
            return nullptr;
        }
    } else if (static_cast<s32>(length) > size) {
        *CRT_errno() = 0x22;
        return nullptr;
    }
    return std::strcpy(buffer, path);
}

char *__cdecl CRT_getcwd(char *buffer, s32 size) {
    CrtInner().lock(0xc);
    char *const result = CRT_getdcwd_lk(0, buffer, size);
    CrtInner().unlock(0xc);
    return result;
}

char *__cdecl CRT_fullpath(char *buffer, const char *path, u32 size) {
    if (!path || !*path) return CRT_getcwd(buffer, static_cast<s32>(size));
    char *out = buffer;
    if (!buffer) {
        out = static_cast<char *>(CRT_malloc(0x104));
        if (!out) {
            *CRT_errno() = 0xc;
            return nullptr;
        }
        size = 0x104;
    }
    char *part;   // (the original's: its own size argument's slot)
    const u32 got = import_slot<u32(__stdcall *)(const char *, u32, char *, char **)>(0x006fd214)(path, size, out, &part);   // GetFullPathNameA
    if (got >= size) {
        if (!buffer) Crt().free(out);
        *CRT_errno() = 0x22;
        return nullptr;
    }
    if (!got) {
        if (!buffer) Crt().free(out);
        CRT_dosmaperr(last_error());
        return nullptr;
    }
    return out;
}

s32 __cdecl CRT_getdrive() {
    char path[0x104];
    if (!import_slot<u32(__stdcall *)(u32, char *)>(0x006fd118)(0x104, path) || path[1] != ':') return 0;   // GetCurrentDirectoryA
    return CRT_toupper(static_cast<u8>(path[0])) - 0x40;
}

s32 __cdecl CRT_stat(const char *path, CrtStat *out) {
    if (CRT_mbspbrk(path, CRT_text_wildcards_708a48)) return no_entry();
    s32 drive;
    if (path[1] == ':') {
        if (path[0] && !path[2]) return no_entry();
        drive = static_cast<s32>(CRT_mbctolower(static_cast<u32>(static_cast<s32>(static_cast<signed char>(path[0]))))) - 0x60;
    } else {
        drive = CRT_getdrive();
    }
    FindData found;
    const u32 handle = import_slot<u32(__stdcall *)(const char *, FindData *)>(0x006fd100)(path, &found);   // FindFirstFileA
    if (handle == 0xffffffff) {
        if (!CRT_mbspbrk(path, reinterpret_cast<const char *>((u32)(uintptr_t)&gCrtopen_708a44))) return no_entry();
        char full[0x104];
        const char *const root = CRT_fullpath(full, path, 0x104);
        if (!root) return no_entry();
        if (std::strlen(root) != 3 && !CRT_IsRootUNCName(root)) return no_entry();
        if (import_slot<u32(__stdcall *)(const char *)>(0x006fd09c)(root) <= 1) return no_entry();   // GetDriveTypeA
        found.attributes = 0x10;
        found.size_high = 0;
        found.size_low = 0;
        found.name[0] = 0;
        out->ctime = out->atime = out->mtime = CRT_loctotime_t(0x7bc, 1, 1, 0, 0, 0, -1);
    } else {
        u32 time = 0;
        if (!file_time(found.written, &time)) return failed_find(handle);
        out->mtime = time;
        if ((found.accessed[0] || found.accessed[1]) && !file_time(found.accessed, &time)) return failed_find(handle);
        out->atime = time;
        if (!found.created[0] && !found.created[1]) {
            out->ctime = out->mtime;
        } else {
            if (!file_time(found.created, &time)) return failed_find(handle);
            out->ctime = time;
        }
        import_slot<s32(__stdcall *)(u32)>(0x006fd104)(handle);   // FindClose
    }
    out->mode = static_cast<u16>(CRT_dtoxmode(found.attributes, path));
    out->nlink = 1;
    out->dev = static_cast<u32>(drive - 1);
    out->rdev = static_cast<u32>(drive - 1);
    out->size = static_cast<s32>(found.size_low);
    out->ino = 0;
    out->gid = 0;
    out->uid = 0;
    return 0;
}

}
