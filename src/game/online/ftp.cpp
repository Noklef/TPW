#include "ftp.h"

#include <cstdint>
#include <cstring>

#include "../core/crt.h"
#include "../core/gameheap.h"

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

#pragma pack(push, 1)
struct SockAddrIn {
    u16 family;
    u16 port;
    u32 address;
    u8 zero[8];
};
#pragma pack(pop)
}  // namespace

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

}  // extern "C"
