#include "ftp.h"

#include <cstdint>
#include <cstring>

#include "../core/crt.h"
#include "../core/crtfile.h"
#include "../core/crtopen.h"
#include "../core/gameheap.h"
#include "../core/stdstring.h"
#include "../core/sync.h"
#include "../core/thunks.h"
#include "../core/vcall.h"
#include "../core/vdtor.h"

namespace {
template <typename F> F import(std::uintptr_t slot) { return *reinterpret_cast<const F *>(slot); }
template <typename T> T &field(void *self, u32 offset) { return *reinterpret_cast<T *>(static_cast<u8 *>(self) + offset); }

using SocketFn = u32(__stdcall *)(s32, s32, s32);
const auto Connect = [](u32 s, const void *address, u32 size) { return import<s32(__stdcall *)(u32, const void *, u32)>(0x006fd404)(s, address, size); };
const auto Htons = [](u32 value) { return import<u16(__stdcall *)(u32)>(0x006fd408)(value); };
const auto GetHostByName = [](const char *name) { return import<u8 *(__stdcall *)(const char *)>(0x006fd40c)(name); };
const auto InetAddr = [](const char *text) { return import<u32(__stdcall *)(const char *)>(0x006fd410)(text); };
const auto Socket = [](s32 family, s32 type, s32 protocol) { return import<SocketFn>(0x006fd414)(family, type, protocol); };
const auto Recv = [](u32 s, void *buffer, s32 size, s32 flags) { return import<s32(__stdcall *)(u32, void *, s32, s32)>(0x006fd418)(s, buffer, size, flags); };
const auto CloseSocket = [](u32 s) { return import<s32(__stdcall *)(u32)>(0x006fd41c)(s); };
const auto GetSockName = [](u32 s, void *address, s32 *size) { return import<s32(__stdcall *)(u32, void *, s32 *)>(0x006fd420)(s, address, size); };
const auto GetHostName = [](char *name, s32 size) { return import<s32(__stdcall *)(char *, s32)>(0x006fd3f8)(name, size); };
const auto SendTo = [](u32 s, const void *buffer, s32 size, s32 flags) { return import<s32(__stdcall *)(u32, const void *, s32, s32)>(0x006fd428)(s, buffer, size, flags); };
const auto Shutdown = [](u32 s, s32 how) { return import<s32(__stdcall *)(u32, s32)>(0x006fd444)(s, how); };
const auto Accept = [](u32 s, void *address, u32 *size) { return import<u32(__stdcall *)(u32, void *, u32 *)>(0x006fd434)(s, address, size); };
const auto Bind = [](u32 s, const void *address, u32 size) { return import<s32(__stdcall *)(u32, const void *, u32)>(0x006fd440)(s, address, size); };
const auto Listen = [](u32 s, s32 backlog) { return import<s32(__stdcall *)(u32, s32)>(0x006fd43c)(s, backlog); };
const auto Ntohs = [](u32 value) { return import<u16(__stdcall *)(u32)>(0x006fd438)(value); };
const auto WsaStartup = [](u32 version, void *data) { return import<s32(__stdcall *)(u32, void *)>(0x006fd42c)(version, data); };
const auto WsaCleanup = []() { return import<s32(__stdcall *)()>(0x006fd430)(); };
const auto WsaGetLastError = []() { return import<s32(__stdcall *)()>(0x006fd424)(); };
const auto SleepMs = [](u32 ms) { import<void(__stdcall *)(u32)>(0x006fd1d4)(ms); };
const auto MessageBoxText = [](u32 owner, const char *text, const char *caption, u32 type) { return import<s32(__stdcall *)(u32, const char *, const char *, u32)>(0x006fd37c)(owner, text, caption, type); };

const FtpEnv kPorts = {reinterpret_cast<void *(__cdecl *)(const char *, const char *)>(&CRT_fopen), reinterpret_cast<u32(__cdecl *)(const void *, u32, u32, void *)>(&CRT_fwrite), &CRT_fclose, &Thread_Begin,
                       &SyncEvent_Set, &SyncEvent_Wait, &SyncEvent_Ctor, &Semaphore_Dtor, &Class702958_Ctor};
const FtpEnv *g_env = &kPorts;
const char *text_at(u32 address) { return reinterpret_cast<const char *>(static_cast<std::uintptr_t>(address)); }

const char *bind_text(void *self) {
    const char *const text = field<const char *>(self, 0x2764);
    return text ? text : text_at(0x006fe5b0);
}

// A socket's end: shutdown, the rest read into the reply buffer, closesocket, -1. Nothing for one that is -1.
void close_socket_at(void *self, u32 offset) {
    u32 &s = field<u32>(self, offset);
    if (s == 0xffffffffu) return;
    Shutdown(s, 1);
    while (Recv(s, &field<u8>(self, 0x20), 0x400, 0) > 0) {
    }
    CloseSocket(s);
    s = 0xffffffffu;
}

// The thread started if it was not, its readiness waited for, the command given.
void post_command(void *self, u32 command) {
    if (!field<u8>(self, 0x277d) && g_env->threadBegin(reinterpret_cast<void *>(static_cast<std::uintptr_t>(0x005d2850)), self) != 0xffffffffu) field<u8>(self, 0x277d) = 1;
    while (!field<u8>(self, 0x277c))
        SleepMs(1);
    field<u8>(self, 0x277c) = 0;
    field<u32>(self, 0x2778) = command;
    g_env->eventSet(&field<u32>(self, 0x2770));
}

// The end of the quit: the thread's command 2 too.
s32 quit_now(void *self) {
    if (field<u32>(self, 4) == 0xffffffffu) return 0x38;
    Ftp_SendLine(self, nullptr, text_at(0x00774158));
    const s32 code = Ftp_ReadReply(self, nullptr, nullptr, 0, 1);
    if (code != 0 && code != 0xdd) return 0x43;
    Ftp_Close(self);
    close_socket_at(self, 0x2428);
    return 0;
}

bool no_data_socket(void *self) { return field<u32>(self, 0x2428) == 0xffffffffu && field<u32>(self, 0x2424) == 0xffffffffu; }

// Ftp_Retrieve's body, up to the closing of the data socket: keepData after a final 250.
s32 retrieve_steps(void *self, bool &keepData) {
    const CrtEnv &crt = Crt();
    s32 size = -1;
    char line[0x100];
    if (const s32 failed = Ftp_DataSetup(self)) return failed;
    if (no_data_socket(self)) {
        if (const s32 failed = Ftp_DataConnect(self)) return failed;
    }
    field<u32>(self, 0x1c) = 0;
    field<u32>(self, 0x18) = 0;
    crt.sprintf(line, text_at(0x0077416c), &field<char>(self, 0x265c));
    Ftp_SendLine(self, nullptr, line);
    s32 code = Ftp_ReadReply(self, nullptr, nullptr, 0, 1);
    if (code == 0) return 0x38;
    if (code == 0x7d || code == 0x96) {
        bool listed = true;
        if (Ftp_Receive(self, nullptr, 0, 0xffffffffu) == 0) size = Ftp_ListLineSize(self, nullptr, &field<char>(self, 0x424));
        else listed = false;
        code = Ftp_ReadReply(self, nullptr, nullptr, 0, 1);
        if (code == 0) return 0x38;
        if (code == 0xe2) {
            if (!listed) return 0x44;
            close_socket_at(self, 0x2424);
            if (const s32 failed = Ftp_DataSetup(self)) return failed;
            if (no_data_socket(self)) {
                if (const s32 failed = Ftp_DataConnect(self)) return failed;
            }
        } else if (code != 0xfa) {
            return 0x44;
        }
    }
    field<s32>(self, 0x18) += size;
    crt.sprintf(line, text_at(0x00774160), &field<char>(self, 0x265c));
    Ftp_SendLine(self, nullptr, line);
    code = Ftp_ReadReply(self, nullptr, nullptr, 0, 1);
    if (code == 0) return 0x38;
    if (code != 0x96 && code != 0x7d) return 0x43;
    Ftp_Receive(self, nullptr, 1, static_cast<u32>(size));
    code = Ftp_ReadReply(self, nullptr, nullptr, 0, 1);
    if (code == 0) return 0x38;
    if (code == 0xe2) return 0;
    if (code == 0xfa) {
        keepData = true;
        return 0;
    }
    return 0x44;
}

// Ftp_Login's body, up to the closing of its sockets on a failure (not 0).
s32 login_steps(void *self) {
    const CrtEnv &crt = Crt();
    char line[0x100];
    field<u32>(self, 4) = Socket(2, 1, 0);
    if (field<u32>(self, 4) == 0xffffffffu) return 0x34;
    if (field<u32>(self, 0x2768)) {
        std::memset(&field<u8>(self, 8), 0, 0x10);
        field<u16>(self, 8) = 2;
        field<u32>(self, 0xc) = InetAddr(bind_text(self));
        Bind(field<u32>(self, 4), &field<u8>(self, 8), 0x10);
    }
    const char *const host = &field<char>(self, 0x243c);
    u32 address = InetAddr(host);
    if (address == 0xffffffffu && std::strcmp(host, text_at(0x00773ce0)) != 0) {
        const u8 *const found = GetHostByName(host);
        if (found) address = ***reinterpret_cast<const u32 *const *const *>(found + 0xc);
    }
    field<u32>(self, 0xc) = address;
    field<u16>(self, 8) = 2;
    field<u16>(self, 0xa) = Htons(0x15);
    if (Connect(field<u32>(self, 4), &field<u8>(self, 8), 0x10) != 0) return 0x40;
    s32 code = Ftp_ReadReply(self, nullptr, nullptr, 0, 1);
    if (code == 0) return 0x38;
    if (code != 0xdc) return 0x41;
    crt.sprintf(line, text_at(0x0077414c), &field<char>(self, 0x249c));
    Ftp_SendLine(self, nullptr, line);
    code = Ftp_ReadReply(self, nullptr, nullptr, 0, 1);
    if (code == 0) return 0x38;
    if (code == 0x14b || code == 0x14c) {
        crt.sprintf(line, text_at(0x00774140), &field<char>(self, 0x24fc));
        Ftp_SendLine(self, nullptr, line);
        code = Ftp_ReadReply(self, nullptr, nullptr, 0, 1);
        if (code == 0) return 0x38;
        if (code != 0xe6) return 0x42;
    }
    Ftp_SendLine(self, nullptr, text_at(0x00774134));
    code = Ftp_ReadReply(self, nullptr, nullptr, 0, 1);
    if (code == 0) return 0x38;
    return code == 0xc8 ? 0 : 0x43;
}

void cut_at_line_end(char *text) {
    for (; *text; text++) {
        if (*text == '\r' || *text == '\n') {
            *text = 0;
            return;
        }
    }
}

#pragma pack(push, 1)
struct SockAddrIn {
    u16 family;
    u16 port;
    u32 address;
    u8 zero[8];
};
#pragma pack(pop)
}  // namespace

void Ftp_SetEnv(const FtpEnv *env) { g_env = env ? env : &kPorts; }

extern "C" {

s32 __fastcall Ftp_ListLineSize(void *, void *, const char *line) {
    const CrtEnv &crt = Crt();
    for (int field = 0; field < 4; field++) {
        while (!crt.isspace(*line))
            line++;
        while (crt.isspace(*line))
            line++;
    }
    return crt.atoi(line);
}

s32 __fastcall Ftp_Send(void *self, void *, const char *buffer, s32 length) {
    if (!buffer) return 1;
    while (length > 0) {
        const s32 sent = SendTo(field<u32>(self, 4), buffer, length > 0x2000 ? 0x2000 : length, 0);
        if (sent < 0) return 1;
        length -= sent;
        buffer += sent;
    }
    return 0;
}

s32 __fastcall Ftp_SendLine(void *self, void *, const char *text) {
    const u32 length = static_cast<u32>(std::strlen(text));
    field<u32>(self, 0x18) += length;
    return Ftp_Send(self, nullptr, text, static_cast<s32>(length));
}

s32 __fastcall Ftp_ReadReply(void *self, void *, char *out, u32 size, u8 wanted) {
    const CrtEnv &crt = Crt();
    char *const buffer = &field<char>(self, 0x20);
    s32 &length = field<s32>(self, 0x420);
    for (;;) {
        s32 code = -1;
        char *line = buffer;
        char *at = buffer;
        for (s32 i = 0; i < length && code == -1;) {
            if (at[0] == '\r' && at[1] == '\n') {
                *at++ = 0;
                i += 2;
                *at++ = 0;
                code = crt.atoi(line);
                if (code < 100 || code >= 600) code = -1;
                if (out && (code != -1) == (wanted != 0)) crt.strncpy(out, line, size);
                if (line[3] == '-') code = -1;
                line = at;
            } else {
                at++;
                i++;
            }
        }
        if (line != buffer) {
            length += static_cast<s32>(buffer - line);
            if (length > 0) CRT_memmove(buffer, line, static_cast<u32>(length));
        }
        if (code != -1) return code;
        const s32 got = Recv(field<u32>(self, 4), buffer + length, 0x3ff - length, 0);
        if (got <= 0) return 0;
        length += got;
        buffer[length] = 0;
    }
}

u32 __cdecl Ftp_LocalAddress() {
    SockAddrIn local;
    s32 size;
    char name[0x80];
    local.address = 0;
    if (GetHostName(name, 0x80) == 0) {
        const u8 *const host = GetHostByName(name);
        if (host) local.address = ***reinterpret_cast<const u32 *const *const *>(host + 0xc);
    }
    if (local.address != 0) return local.address;
    size = 0x10;
    const u32 s = Socket(2, 2, 2);
    if (s == 0xffffffffu) return local.address;
    SockAddrIn remote;
    remote.family = 2;
    remote.port = Htons(7);
    remote.address = InetAddr(reinterpret_cast<const char *>(0x00773cf0));
    if (Connect(s, &remote, 0x10) == 0) GetSockName(s, &local, &size);
    CloseSocket(s);
    return local.address;
}

void __fastcall Ftp_Close(void *self) {
    u32 &s = field<u32>(self, 4);
    if (s == 0xffffffffu) return;
    u8 drain[0x100];
    Shutdown(s, 1);
    while (Recv(s, drain, 0x100, 0) > 0) {
    }
    CloseSocket(s);
    s = 0xffffffffu;
}

void __fastcall FtpSocket_Dtor(u32 *self) {
    *self = 0x00702994;
    Ftp_Close(self);
}

u32 *__fastcall FtpSocket_DeletingDtor(u32 *self, void *, u8 flags) {
    FtpSocket_Dtor(self);
    if (flags & 1) GameHeap_Free(self);
    return self;
}

void __fastcall FtpBase_Dtor(u32 *self) {
    *self = 0x0070295c;
    FtpSocket_Dtor(self);
}

u32 *__fastcall FtpBase_DeletingDtor(u32 *self, void *, u8 flags) {
    FtpBase_Dtor(self);
    if (flags & 1) GameHeap_Free(self);
    return self;
}

void __fastcall FtpMid_Dtor(u32 *self) { FtpBase_Dtor(self); }

u32 *__fastcall FtpMid_DeletingDtor(u32 *self, void *, u8 flags) {
    FtpMid_Dtor(self);
    if (flags & 1) GameHeap_Free(self);
    return self;
}

s32 __fastcall Ftp_Receive(void *self, void *, u8 toFile, u32 count) {
    SockAddrIn address;
    u32 addressSize = 0x10;
    void *file = nullptr;
    s32 result = 0;
    u32 total = 0;
    char *const data = &field<char>(self, 0x424);
    char *at = data;
    if (field<u32>(self, 0x2428) != 0xffffffffu && field<u32>(self, 0x2424) == 0xffffffffu) {
        field<u32>(self, 0x2424) = Accept(field<u32>(self, 0x2428), &address, &addressSize);
        if (field<u32>(self, 0x2424) == 0xffffffffu) return 3;
    }
    if (toFile == 1) {
        file = g_env->fopen(&field<char>(self, 0x255c), text_at(0x0074cfb4));
        if (!file) result = 1;
    }
    if (count != 0) {
        do {
            const s32 got = Recv(field<u32>(self, 0x2424), at, 0x1fff, 0);
            if (got <= 0) {
                result = 2;
                break;
            }
            if (file) g_env->fwrite(data, 1, static_cast<u32>(got), file);
            total += static_cast<u32>(got);
            field<s32>(self, 0x1c) += got;
            if (toFile == 0) {
                at += got;
                *at = 0;
                if (Crt().strchr(data, '\n')) total = count;
            }
        } while (total != count);
    }
    if (file) g_env->fclose(file);
    return result;
}

s32 __fastcall Ftp_DataSetup(void *self) {
    const CrtEnv &crt = Crt();
    s32 addressSize = 0x10;
    char line[0x100];
    Ftp_SendLine(self, nullptr, text_at(0x0077412c));
    const s32 code = Ftp_ReadReply(self, nullptr, line, 0x100, 1);
    if (code == 0) return 0x38;
    u16 &port = field<u16>(self, 0x242e);
    field<u16>(self, 0x242c) = 2;
    if (code == 0xe3) {
        const char *at = crt.strchr(line, '(');
        if (!at) return 0x43;
        at++;
        field<u8>(self, 0x2430) = static_cast<u8>(crt.atoi(at));
        at = crt.strchr(at, ',') + 1;
        field<u8>(self, 0x2431) = static_cast<u8>(crt.atoi(at));
        at = crt.strchr(at, ',') + 1;
        field<u8>(self, 0x2432) = static_cast<u8>(crt.atoi(at));
        at = crt.strchr(at, ',') + 1;
        field<u8>(self, 0x2433) = static_cast<u8>(crt.atoi(at));
        at = crt.strchr(at, ',') + 1;
        port = static_cast<u16>(static_cast<u8>(crt.atoi(at)) << 8);
        at = crt.strchr(at, ',') + 1;
        port = static_cast<u16>(port | static_cast<u16>(crt.atoi(at)));
        port = Htons(port);
        return 0;
    }
    field<u32>(self, 0x2428) = Socket(2, 1, 0);
    field<u32>(self, 0x2430) = field<u32>(self, 0x2768) ? InetAddr(bind_text(self)) : Ftp_LocalAddress();
    port = 0;
    if (Bind(field<u32>(self, 0x2428), &field<u8>(self, 0x242c), 0x10) != 0) return 0x34;
    if (Listen(field<u32>(self, 0x2428), 5) != 0) return 0x34;
    GetSockName(field<u32>(self, 0x2428), &field<u8>(self, 0x242c), &addressSize);
    port = Ntohs(port);
    crt.sprintf(line, text_at(0x00774110), field<u8>(self, 0x2430), field<u8>(self, 0x2431), field<u8>(self, 0x2432), field<u8>(self, 0x2433), port >> 8, port & 0xff);
    port = Htons(port);
    Ftp_SendLine(self, nullptr, line);
    const s32 reply = Ftp_ReadReply(self, nullptr, nullptr, 0, 1);
    if (reply == 0) return 0x38;
    return reply == 0xc8 ? 0 : 0x43;
}

s32 __fastcall Ftp_DataConnect(void *self) {
    SockAddrIn address;
    field<u32>(self, 0x2424) = Socket(2, 1, 0);
    if (field<u32>(self, 0x2768)) {
        std::memset(&address, 0, sizeof address);
        address.family = 2;
        address.address = InetAddr(bind_text(self));
        Bind(field<u32>(self, 0x2424), &address, 0x10);
    }
    address.address = field<u32>(self, 0x2430);
    address.family = 2;
    address.port = field<u16>(self, 0x242e);
    return Connect(field<u32>(self, 0x2424), &address, 0x10) != 0 ? 0x40 : 0;
}

s32 __fastcall Ftp_Login(void *self) {
    const s32 failed = login_steps(self);
    if (failed) {
        Ftp_Close(self);
        close_socket_at(self, 0x2428);
    }
    return failed;
}

s32 __fastcall Ftp_Quit(void *self) {
    if (field<u8>(self, 0x277e)) {
        post_command(self, 2);
        return 0;
    }
    return quit_now(self);
}

s32 __fastcall Ftp_Retrieve(void *self) {
    if (field<u32>(self, 4) == 0xffffffffu) return 0x38;
    bool keepData = false;
    const s32 result = retrieve_steps(self, keepData);
    if (!keepData) close_socket_at(self, 0x2424);
    return result;
}

s32 __fastcall Ftp_RetrieveNamed(void *self, void *, const char *remote, const char *local) {
    const CrtEnv &crt = Crt();
    crt.strncpy(&field<char>(self, 0x255c), local, 0x100);
    crt.strncpy(&field<char>(self, 0x265c), remote, 0x100);
    if (field<u8>(self, 0x277e)) {
        post_command(self, 3);
        return 0;
    }
    return Ftp_Retrieve(self);
}

s32 __fastcall Ftp_Connect(void *self, void *, const char *host, const char *user, const char *password, const char *bind) {
    const CrtEnv &crt = Crt();
    crt.strncpy(&field<char>(self, 0x243c), host, 0x60);
    crt.strncpy(&field<char>(self, 0x249c), user, 0x60);
    crt.strncpy(&field<char>(self, 0x24fc), password, 0x60);
    cut_at_line_end(&field<char>(self, 0x249c));
    cut_at_line_end(&field<char>(self, 0x24fc));
    if (bind) StdString_AssignChars(&field<StdString>(self, 0x2760), nullptr, bind, static_cast<u32>(std::strlen(bind)));
    if (field<u8>(self, 0x277e)) {
        post_command(self, 1);
        return 0;
    }
    return Ftp_Login(self);
}

void __cdecl Ftp_ThreadProc(void *self) {
    s32 result = static_cast<s32>(reinterpret_cast<std::uintptr_t>(self));
    bool done = false;
    while (!done) {
        g_env->eventWait(&field<u32>(self, 0x2770), nullptr, -1);
        const u32 command = field<u32>(self, 0x2778);
        field<u8>(self, 0x277c) = 1;
        switch (command) {
            case 0:
                done = true;
                result = 0;
                break;
            case 1: result = Ftp_Login(self); break;
            case 2: result = quit_now(self); break;
            case 3: result = Ftp_Retrieve(self); break;
            default: break;
        }
        vcall<void>(field<u32 *>(self, 0x275c), 0, command, result);
    }
    g_env->eventSet(&field<u32>(self, 0x2774));
}

u32 *__fastcall FtpClient_Ctor(u32 *self, void *, void *callback, u8 threaded) {
    u8 data[0x190];
    g_env->baseCtor(reinterpret_cast<u8 *>(self));
    field<u32>(self, 0x2424) = 0xffffffffu;
    field<u32>(self, 0x2428) = 0xffffffffu;
    field<void *>(self, 0x275c) = callback;
    field<u32>(self, 0x2764) = 0;
    field<u32>(self, 0x2768) = 0;
    field<u32>(self, 0x276c) = 0;
    g_env->eventCtor(&field<u32>(self, 0x2770));
    g_env->eventCtor(&field<u32>(self, 0x2774));
    field<u32>(self, 0x2778) = 0;
    field<u8>(self, 0x277c) = 1;
    field<u8>(self, 0x277d) = 0;
    field<u8>(self, 0x277e) = threaded;
    *self = 0x00702990;
    std::memset(&field<u8>(self, 0x242c), 0, 0x10);
    WsaStartup(0x101, data);
    return self;
}

void __fastcall FtpClient_Dtor(u32 *self) {
    *self = 0x00702990;
    while (!field<u8>(self, 0x277c))
        SleepMs(1);
    field<u8>(self, 0x277c) = 0;
    field<u32>(self, 0x2778) = 0;
    g_env->eventSet(&field<u32>(self, 0x2770));
    if (field<u8>(self, 0x277d)) g_env->eventWait(&field<u32>(self, 0x2774), nullptr, -1);
    WsaCleanup();
    g_env->semaphoreDtor(&field<Semaphore>(self, 0x2774));
    g_env->semaphoreDtor(&field<Semaphore>(self, 0x2770));
    StdString_Tidy(&field<StdString>(self, 0x2760), nullptr, 1);
    FtpBase_Dtor(self);
}

u32 *__fastcall FtpClient_DeletingDtor(u32 *self, void *, u8 flags) {
    FtpClient_Dtor(self);
    if (flags & 1) GameHeap_Free(self);
    return self;
}

void __fastcall Ftp_WsaErrorBox(void *) {
    const s32 error = WsaGetLastError();
    const s16 *code = *reinterpret_cast<const s16 *const *>(static_cast<std::uintptr_t>(0x00f827ec));
    u32 index = 0;
    while (*code != error && *code != 0) {
        code++;
        index++;
    }
    const char *const *const texts = *reinterpret_cast<const char *const *const *>(static_cast<std::uintptr_t>(0x00f827f0));
    MessageBoxText(0, texts[index], text_at(0x00774810), 0);
}

s32 __fastcall Ftp_WsaStartup(void *self) {
    u8 data[0x190];
    if (WsaStartup(0x101, data) == 0) return 0;
    Ftp_WsaErrorBox(self);
    return 1;
}

s32 __fastcall Ftp_WsaCleanup(void *self) {
    if (WsaCleanup() == 0) return 0;
    Ftp_WsaErrorBox(self);
    return 1;
}

}  // extern "C"
