#include "mailbox.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/crtoutput.h"
#include "../core/initializers.h"
#include "../core/iosbase.h"
#include "../core/narrowfilebuf.h"
#include "../core/nstream.h"
#include "../core/ptrlist.h"
#include "../core/stdlist.h"
#include "../core/stdstring.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const MailboxEnv kGameEnv = {&Init_5cfd80, &NOstream_Ctor, &NIstream_Ctor, &NFileBuf_Ctor, reinterpret_cast<u8 *(__fastcall *)(u8 *, void *, const char *, u32)>((uintptr_t)0x005d08e0), &BasicIosW_SetState,
                             &NOfstream_DtorOnIos, &NIfstream_DtorOnIos, &NIos_Widen, reinterpret_cast<u8 *(__fastcall *)(u8 *, void *, char *, u32, u32)>((uintptr_t)0x005cfe40),
                             reinterpret_cast<u8 *(__cdecl *)(u8 *, const char *)>((uintptr_t)0x005d0e70), &CRT_fclose, &NFileBuf_Init, &NFileBuf_Close, &NFileBuf_Dtor, &IosBase_Dtor,
                             reinterpret_cast<s32(__cdecl *)(const char *)>((uintptr_t)0x0067a520), &List_EraseNodeCounted, &StdListString_Insert};
const MailboxEnv *g_env = &kGameEnv;

// The ios of a stream, by the offset in its vbtable.
u8 *IosOf(u8 *stream) { return stream + at<u32>(at<u8 *>(stream, 0), 4); }

u32 npos() { return at<u32>(nullptr, 0x006fe5ac); }

const char *TextOf(const StdString &s) { return s.ptr ? s.ptr : reinterpret_cast<const char *>((uintptr_t)0x006fe5b0); }

void WriteNumbers(u8 *self, u32 listAt, u8 *stream) {
    char line[0x100];
    for (u8 *node = at<u8 *>(at<u8 *>(self, listAt), 0); node != at<u8 *>(self, listAt); node = at<u8 *>(node, 0)) {
        CRT_sprintf(line, reinterpret_cast<const char *>((uintptr_t)0x007740c8), at<u32>(node, 8));
        g_env->write(stream, line);
    }
}

u8 *ReadLine(u8 *stream, char *line) { return g_env->getline(stream, nullptr, line, 0x100, g_env->widen(IosOf(stream), nullptr, 10)); }

}  // namespace

void Mailbox_SetEnv(const MailboxEnv *env) { g_env = env ? env : &kGameEnv; }

void __fastcall NIos_Dtor(u8 *self) {
    at<u32>(self, 0) = 0x00709954;
    g_env->iosDtor(self);
}

u8 __fastcall Mailbox_SaveOutbox(u8 *self) {
    StdString path;
    StdString_Tidy(&path, nullptr, 0);
    u8 stream[0x8c];   // (+0 the vbtable, +4 the file buffer, its file at +0x54, +0x58 the ios: the original's frame has this much)
    u8 *const buffer = stream + 4;
    u8 *const ios = stream + 0x58;
    at<u32>(stream, 0) = 0x00702978;
    g_env->iosInit(ios);
    g_env->ostreamCtor(stream, nullptr, buffer, 0, 1, 0);
    g_env->fileCtor(buffer, nullptr, nullptr);
    at<u32>(IosOf(stream), 0) = 0x00702970;
    StdString_AssignSub(&path, nullptr, reinterpret_cast<const StdString *>(self), 0, npos());
    const char *const name = reinterpret_cast<const char *>((uintptr_t)0x007740b4);
    StdString_AppendChars(&path, nullptr, name, CRT_strlen(name));
    if (!g_env->open(buffer, nullptr, TextOf(path), 0x12)) g_env->setState(IosOf(stream), nullptr, 2, 0);
    if (!at<void *>(stream, 0x54)) {
        g_env->ofstreamDtor(ios);
        NIos_Dtor(ios);
        StdString_Tidy(&path, nullptr, 1);
        return 0;
    }
    const char *const lineEnd = reinterpret_cast<const char *>((uintptr_t)0x0074da30);
    WriteNumbers(self, 0x5c, stream);
    g_env->write(stream, lineEnd);
    WriteNumbers(self, 0x74, stream);
    g_env->write(stream, lineEnd);
    WriteNumbers(self, 0x68, stream);
    void *const file = at<void *>(stream, 0x54);
    if (file && g_env->fclose(file) == 0)
        g_env->fileInit(buffer, nullptr, nullptr, 2);
    else
        g_env->setState(IosOf(stream), nullptr, 2, 0);
    at<u32>(IosOf(stream), 0) = 0x00702970;
    g_env->fileDtor(buffer);
    at<u32>(IosOf(stream), 0) = 0x0070995c;
    at<u32>(ios, 0) = 0x00709954;
    g_env->iosDtor(ios);
    StdString_Tidy(&path, nullptr, 1);
    return 1;
}

u8 __fastcall Mailbox_LoadMessage(u8 *self, void *, u32 number) {
    StdString path;
    StdString_Tidy(&path, nullptr, 0);
    StdString unused;
    StdString_Tidy(&unused, nullptr, 0);
    u8 stream[0x90];   // (+0 the vbtable, +8 the file buffer, its file at +0x58, +0x5c the ios)
    u8 *const buffer = stream + 8;
    u8 *const ios = stream + 0x5c;
    at<u32>(stream, 0) = 0x00702968;
    g_env->iosInit(ios);
    g_env->istreamCtor(stream, nullptr, buffer, 0, 0);
    g_env->fileCtor(buffer, nullptr, nullptr);
    at<u32>(IosOf(stream), 0) = 0x00702960;
    StdString_AssignSub(&path, nullptr, reinterpret_cast<const StdString *>(self), 0, npos());
    char line[0x100];
    CRT_sprintf(line, reinterpret_cast<const char *>((uintptr_t)0x007740d8), number);
    StdString_AppendChars(&path, nullptr, line, CRT_strlen(line));
    if (!g_env->open(buffer, nullptr, TextOf(path), 1)) g_env->setState(IosOf(stream), nullptr, 2, 0);
    if (!at<void *>(stream, 0x58)) {
        g_env->ifstreamDtor(ios);
        NIos_Dtor(ios);
        StdString_Tidy(&unused, nullptr, 1);
        StdString_Tidy(&path, nullptr, 1);
        return 0;
    }
    ReadLine(stream, line);
    at<s32>(self, 0x4c) = g_env->atoi(line);
    ReadLine(stream, line);
    at<s32>(self, 0x50) = g_env->atoi(line);
    ReadLine(stream, line);
    at<s32>(self, 0x54) = g_env->atoi(line);
    ReadLine(stream, line);
    StdString_AssignChars(reinterpret_cast<StdString *>(self + 0x10), nullptr, line, CRT_strlen(line));
    ReadLine(stream, line);
    StdString_AssignChars(reinterpret_cast<StdString *>(self + 0x20), nullptr, line, CRT_strlen(line));
    u8 *const list = self + 0x30;
    u8 *const head = at<u8 *>(self, 0x34);
    u32 cursor = at<u32>(head, 0);
    while (cursor != static_cast<u32>(reinterpret_cast<uintptr_t>(head))) {
        u32 old, out;
        const u32 node = *ListCursor_Next(&cursor, nullptr, &old, 0);
        g_env->erase(list, nullptr, &out, reinterpret_cast<u8 *>((uintptr_t)node));
    }
    for (;;) {
        u8 *const got = ReadLine(stream, line);
        // (a null stream: the state read at 4, a fault, as in the original)
        const u8 *const state = got ? IosOf(got) : nullptr;
        if ((at<u8>(state, 4) & 6) != 0) break;
        StdString text;
        StdString_Tidy(&text, nullptr, 0);
        StdString_AssignChars(&text, nullptr, line, CRT_strlen(line));
        u8 *out;
        g_env->insert(list, nullptr, &out, at<u8 *>(self, 0x34), &text);
        StdString_Tidy(&text, nullptr, 1);
    }
    if (!g_env->fileClose(buffer)) g_env->setState(IosOf(stream), nullptr, 2, 0);
    StdString_AssignSub(reinterpret_cast<StdString *>(self + 0x3c), nullptr, reinterpret_cast<const StdString *>(self), 0, npos());
    CRT_sprintf(line, reinterpret_cast<const char *>((uintptr_t)0x007740cc), number);
    StdString_AppendChars(reinterpret_cast<StdString *>(self + 0x3c), nullptr, line, CRT_strlen(line));
    g_env->ifstreamDtor(ios);
    NIos_Dtor(ios);
    StdString_Tidy(&unused, nullptr, 1);
    StdString_Tidy(&path, nullptr, 1);
    return 1;
}
