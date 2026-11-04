#include "crtopen.h"

#include <cstdint>

#include "crt.h"
#include "crtstream.h"

namespace {

template <typename F> F import_slot(u32 slot) { return *reinterpret_cast<F *>((uintptr_t)slot); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

CrtFile *&stream(s32 index) { return global<CrtFile **>(0x00fb3cbc)[index]; }

u8 *&handle_block(u32 index) { return reinterpret_cast<u8 **>((uintptr_t)0x00fb3ba0)[index]; }

u32 &handle_count() { return global<u32>(0x00fb3ca0); }

void init_section(void *section) { import_slot<void(__stdcall *)(void *)>(0x006fd144)(section); }
void enter_section(void *section) { import_slot<void(__stdcall *)(void *)>(0x006fd140)(section); }
void leave_section(void *section) { import_slot<void(__stdcall *)(void *)>(0x006fd130)(section); }

u32 last_error() { return import_slot<u32(__stdcall *)()>(0x006fd138)(); }
s32 close_handle(u32 handle) { return import_slot<s32(__stdcall *)(u32)>(0x006fd13c)(handle); }

void set_std_handle(u32 fd, u32 handle) {
    if (global<u32>(0x0077ea84) == 1 && fd <= 2) import_slot<void(__stdcall *)(s32, u32)>(0x006fd268)(-10 - static_cast<s32>(fd), handle);
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

}  // namespace

extern "C" {

CrtFile *__cdecl CRT_getstream() {
    CrtFile *found = nullptr;
    CrtInner().lock(2);
    for (s32 i = 0; i < global<s32>(0x00fb4cc0); ++i) {
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
    u32 flags = global<u32>(0x00fb1894);
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
    ++global<u32>(0x00fb1350);
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
    if (!(oflag & 0x8000) && (oflag & 0x4000 || global<u32>(0x00fb1914) != 0x8000)) flags |= 0x80;
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
    if (oflag & 0x100 && !(pmode & ~global<u32>(0x00fb1360) & 0x80)) attributes = 1;
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
            ++global<u32>(0x00fb1350);
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
    for (s32 i = 0; i < global<s32>(0x00fb4cc0); ++i) {
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

}
