#include "guardfile.h"

#include <cstdint>

#include "crt.h"
#include "crtfile.h"
#include "crtlocale.h"
#include "crtopen.h"
#include "crtstream.h"
#include "guardheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
const char *text(u32 address) { return reinterpret_cast<const char *>((uintptr_t)address); }

const u32 kBuffer = 0x424, kBlockSize = 0x2424, kPage = 0x2000;

const GuardFileEnv kGameEnv = {reinterpret_cast<s32(__cdecl *)(const char *, void *)>((uintptr_t)0x0067e900),
                               reinterpret_cast<void *(__cdecl *)(const char *, const char *)>(&CRT_fopen),
                               reinterpret_cast<s32(__cdecl *)(void *, s32, s32)>(&CRT_fseek),
                               reinterpret_cast<s32(__cdecl *)(const void *, u32, s32, void *)>(&CRT_fwrite),
                               reinterpret_cast<s32(__cdecl *)(void *, u32, s32, void *)>(&CRT_fread),
                               reinterpret_cast<s32(__cdecl *)(void *)>(&CRT_fflush),
                               reinterpret_cast<s32(__cdecl *)(void *)>(&CRT_fclose),
                               reinterpret_cast<s32(__cdecl *)(const char *)>((uintptr_t)0x0067e530),
                               reinterpret_cast<s32(__cdecl *)(const char *, const char *)>((uintptr_t)0x0067ecc0)};
const GuardFileEnv *g_env = &kGameEnv;

using Message = void(__cdecl *)(const char *, ...);
Message message() { return *reinterpret_cast<Message *>((uintptr_t)0x007748f4); }

void Copy(char *to, const char *from) {
    while ((*to++ = *from++) != 0) {
    }
}

// The name's extension, from a '.' after any ':', '/' or '\\', replaced by the text.
void ChangeExtension(char *name, const char *extension) {
    char *p = name + CRT_strlen(name) - 1;
    while (p > name && *p != '.' && *p != ':' && *p != '/' && *p != '\\') --p;
    if (*p != '.') p = name + CRT_strlen(name);
    Copy(p, extension);
}

// "gwriteflush": a changed buffer written at its place, the buffer emptied, the FILE flushed.
void Flush(u8 *file) {
    if (at<u32>(file, 0x1c) && at<u32>(file, 0x18)) {
        g_env->fseek(at<void *>(file, 0), at<s32>(file, 0xc), 0);
        const s32 wrote = g_env->fwrite(file + kBuffer, 1, static_cast<s32>(at<u8 *>(file, 0x14) - file - kBuffer), at<void *>(file, 0));
        if (wrote < static_cast<s32>(at<u8 *>(file, 0x14) - file - kBuffer)) message()(text((u32)(uintptr_t)&gGuardfile_774af0));
        at<u32>(file, 0x1c) = 0;
    }
    at<u8 *>(file, 0x14) = file + kBuffer;
    at<u8 *>(file, 0x10) = file + kBuffer;
    g_env->fflush(at<void *>(file, 0));
}

}  // namespace

void GuardFile_SetEnv(const GuardFileEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u8 *__stdcall GuardFile_Open(const char *name) {
    u32 mode = *reinterpret_cast<const u32 *>((uintptr_t)0x007748f0);
    void *handle = nullptr;
    char working[0x200];   // (the original's frame has this much: a longer name overruns it there too)
    if (!CRT_strncmp(name, text((u32)(uintptr_t)&gGuardfile_7749f0), 4)) {
        handle = reinterpret_cast<void *>((uintptr_t)0x0077e820);
        mode = 0;
        working[0] = 0;
    } else {
        u8 status[0x24];
        const bool readOnly = g_env->stat(name, status) != -1 && (at<u16>(status, 6) & 0x180) == 0x100;
        if (readOnly) {
            message()(text((u32)(uintptr_t)&gGuardfile_774aa4), name);
        } else if (g_env->stat(name, status) != -1 && (at<u16>(status, 6) & 0x4000)) {
            message()(text((u32)(uintptr_t)&gGuardfile_774a6c), name);
        } else {
            Copy(working, name);
            if (mode == 2 || mode == 3) ChangeExtension(working, text((u32)(uintptr_t)&gGuardfile_774a64));
            handle = g_env->fopen(working, text(0x00766128));
            if (!handle) {
                handle = g_env->fopen(working, text(0x0074cfb4));
                message()(text((u32)(uintptr_t)&gGuardfile_774a34), working);
            }
        }
    }
    if (!handle) return nullptr;
    u8 *const file = static_cast<u8 *>(GuardHeap_Alloc(kBlockSize));
    if (!file) {
        message()(text((u32)(uintptr_t)&gGuardfile_7749f8), name);
        return nullptr;
    }
    at<u32>(file, 4) = 0;
    at<u8 *>(file, 0x10) = file + kBuffer;
    at<u8 *>(file, 0x14) = file + kBuffer;
    at<u32>(file, 8) = 0;
    at<u32>(file, 0x1c) = 0;
    at<u32>(file, 0x20) = mode;
    at<u32>(file, 0xc) = 0;
    at<void *>(file, 0) = handle;
    at<u32>(file, 0x18) = 1;
    Copy(reinterpret_cast<char *>(file + 0x24), name);
    Copy(reinterpret_cast<char *>(file + 0x224), working);
    return file;
}

u32 __stdcall GuardFile_Close(u8 *file) {
    if (!file) return 1;
    Flush(file);
    u32 worked = 1;
    const u32 mode = at<u32>(file, 0x20);
    if (mode) {
        worked = g_env->fclose(at<void *>(file, 0)) == 0;
        if (worked && (mode == 2 || mode == 3)) {
            const char *const name = reinterpret_cast<const char *>(file + 0x24);
            char old[0x200], kept[0x200];   // (the original's frame has this much: a longer name overruns it there too)
            Copy(old, name);
            ChangeExtension(old, text((u32)(uintptr_t)&gGuardfile_774ae8));
            g_env->remove(old);
            g_env->rename(name, old);
            worked = g_env->rename(reinterpret_cast<const char *>(file + 0x224), name) == 0;
            if (worked) {
                if (mode == 2) {
                    Copy(kept, name);
                    ChangeExtension(kept, text((u32)(uintptr_t)&gGuardfile_774ae0));
                    g_env->remove(kept);
                    g_env->rename(old, kept);
                } else {
                    g_env->remove(old);
                }
            }
        }
    }
    return worked & GuardHeap_Free(file);
}

s32 __stdcall GuardFile_Write(u8 *file, const char *data, s32 count) {
    if (!file) return 0;
    if (!at<u32>(file, 0x18)) {
        message()(text(0x00774b30));
        return 0;
    }
    s32 total = 0;
    s32 fits = static_cast<s32>(file - at<u8 *>(file, 0x10)) + static_cast<s32>(kBlockSize);
    if (count < fits) fits = count;
    if (fits > 0) {
        u8 *const to = at<u8 *>(file, 0x10);
        for (s32 i = 0; i < fits; ++i) to[i] = static_cast<u8>(data[i]);
        at<u8 *>(file, 0x10) += fits;
        at<s32>(file, 4) += fits;
        if (at<u8 *>(file, 0x14) < at<u8 *>(file, 0x10)) at<u8 *>(file, 0x14) = at<u8 *>(file, 0x10);
        if (at<s32>(file, 4) > at<s32>(file, 8)) at<s32>(file, 8) = at<s32>(file, 4);
        at<u32>(file, 0x1c) = 1;
        data += fits;
        count -= fits;
        total = fits;
    }
    if (at<u8 *>(file, 0x14) != at<u8 *>(file, 0x10) || !count) return total;
    const s32 boundary = (count + at<s32>(file, 4)) / static_cast<s32>(kPage) * static_cast<s32>(kPage);
    Flush(file);
    if (at<s32>(file, 8) > boundary) {
        const s32 back = at<s32>(file, 8) - boundary > static_cast<s32>(kPage) ? static_cast<s32>(kPage) : at<s32>(file, 8) - boundary;
        g_env->fseek(at<void *>(file, 0), boundary, 0);
        g_env->fread(file + kBuffer, 1, back, at<void *>(file, 0));
        at<u8 *>(file, 0x14) += back;
    }
    g_env->fseek(at<void *>(file, 0), at<s32>(file, 4), 0);
    const s32 straight = g_env->fwrite(data, 1, boundary - at<s32>(file, 4), at<void *>(file, 0));
    if (straight < boundary - at<s32>(file, 4)) {
        message()(text((u32)(uintptr_t)&gGuardfile_774b14));
        return total + straight;
    }
    at<s32>(file, 4) += straight;
    total += count;
    const s32 rest = count - straight;
    u8 *const to = file + kBuffer;
    for (s32 i = 0; i < rest; ++i) to[i] = static_cast<u8>(data[straight + i]);
    at<s32>(file, 4) += rest;
    at<u8 *>(file, 0x10) += rest;
    at<s32>(file, 0xc) = boundary;
    if (at<s32>(file, 4) > at<s32>(file, 8)) at<s32>(file, 8) = at<s32>(file, 4);
    if (at<u8 *>(file, 0x10) > at<u8 *>(file, 0x14)) at<u8 *>(file, 0x14) = at<u8 *>(file, 0x10);
    at<u32>(file, 0x1c) = 1;
    return total;
}
}
