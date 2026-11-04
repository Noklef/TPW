// The FTP client (object of 0x2780 bytes; the control socket +4, its sockaddr +8, bytes sent +0x18, the reply buffer +0x20 and its length
// +0x420, the data buffer +0x424; the LIST command's "LIST %s\r\n" at 0x5d2468; every Winsock call through the import table, by way of the
// thunks at 0x657cc0): reimplementation of the socket layer and of the listing line's size field 0x5d2720. All names are unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../core/semaphore.h"

// The client's calls outside Winsock (the bench's recorders; each has a bench of its own).
struct FtpEnv {
    void *(__cdecl *fopen)(const char *name, const char *mode);                 // CRT_fopen (0x67b8a0)
    u32(__cdecl *fwrite)(const void *data, u32 size, u32 count, void *file);    // CRT_fwrite (0x67b6d0)
    s32(__cdecl *fclose)(void *file);                                           // CRT_fclose (0x67b620)
    u32(__cdecl *threadBegin)(void *proc, void *arg);                           // Thread_Begin (0x5c8b70)
    u32(__fastcall *eventSet)(const u32 *event);                                // SyncEvent_Set (0x5c8c20)
    bool(__fastcall *eventWait)(const u32 *event, void *, s32 timeout);         // SyncEvent_Wait (0x5c8bf0)
    u32 *(__fastcall *eventCtor)(u32 *event);                                   // SyncEvent_Ctor (0x5c8bd0)
    s32(__fastcall *semaphoreDtor)(Semaphore *semaphore);                       // Semaphore_Dtor (0x6307e0)
    u8 *(__fastcall *baseCtor)(u8 *object);                                     // Class702958_Ctor (0x5c8d80)
};
void Ftp_SetEnv(const FtpEnv *env);

extern "C" {

// 0x5d2720 (thiscall, this unused; line, pops 4; name unconfirmed; the LIST reply at 0x5d24cc reads its +0x424 line with it): the number
// (atoi) in the fifth field of a directory listing line, an "ls -l" line's size: four times a run of non-spaces, then a run of spaces
// (isspace, the char sign-extended), skipped. A NUL is not a space, so a line of fewer fields is read on past its end (sic).
s32 __fastcall Ftp_ListLineSize(void *self, void *, const char *line);

// 0x5c90a0 (thiscall; buffer, length, pops 8): sends the buffer on the socket at +4 in pieces of at most 0x2000 bytes. Returns 1 for a null
// buffer or a negative send, else 0. A send of 0 bytes tries again, without end (sic).
s32 __fastcall Ftp_Send(void *self, void *, const char *buffer, s32 length);

// 0x5c8de0 (thiscall; text, pops 4): adds the text's length to +0x18 and sends it. Returns what Ftp_Send returns.
s32 __fastcall Ftp_SendLine(void *self, void *, const char *text);

// 0x5c8e20 (thiscall; out, size, wanted, pops 0xc): reads one reply from the buffer at +0x20 (the length at +0x420), asking the socket for up
// to 0x3ff less the length when no final line is there. For each CRLF ended line, cut there: its code by atoi, -1 outside 100-599 or when
// the fourth char is '-' (a continued line). Copies the line with strncpy to a non-null out when the atoi's code was valid and wanted is
// set, or was not valid and wanted is clear. Moves what follows the last line read to the buffer's start. Returns the code, or 0 when the
// socket gives no more.
s32 __fastcall Ftp_ReadReply(void *self, void *, char *out, u32 size, u8 wanted);

// 0x5c8f50: this machine's address: the first of gethostname's gethostbyname, or, when that fails or is 0, the local end of a UDP socket
// connected to the address string at 0x773cf0, port 7 (its getsockname's sockaddr shares its address dword with the result, which is 0
// until then).
u32 __cdecl Ftp_LocalAddress();

// 0x5d37c0 (thiscall): closes the socket at +4 unless it is -1: shutdown (1), recv to a stack buffer of 0x100 until that gives no more,
// closesocket, then +4 = -1.
void __fastcall Ftp_Close(void *self);

// 0x5d3750 (thiscall): the vtable pointer = 0x702994, then Ftp_Close's body.
void __fastcall FtpSocket_Dtor(u32 *self);

// 0x5d36d0 (thiscall; flags, pops 4): FtpSocket_Dtor's body, then GameHeap_Free with flag bit 0. Returns the object.
u32 *__fastcall FtpSocket_DeletingDtor(u32 *self, void *, u8 flags);

// 0x5c9090 (thiscall): the vtable pointer = 0x70295c, then FtpSocket_Dtor.
void __fastcall FtpBase_Dtor(u32 *self);

// 0x5d1980 (thiscall; toFile, count, pops 8): accepts on the listen socket +0x2428 when it is open and +0x2424 is not (a failure returns 3), then
// reads the data socket +0x2424 to the buffer at +0x424, at most 0x1fff at a time, until count bytes came (+0x1c counts them). toFile 1 opens the
// local name +0x255c (mode at 0x74cfb4; a failure is result 1, and the reading goes on) and writes each piece at the buffer's start (sic: any piece); toFile
// 0 appends and ends the reading at a '\n'. A recv that gives none is result 2. Returns the result.
s32 __fastcall Ftp_Receive(void *self, void *, u8 toFile, u32 count);

// 0x5d1c70 (thiscall): sends "PASV" (0x77412c). A reply 227 gives the address and port of its "(a,b,c,d,e,f)" (no '(' : 0x43; a missing comma crashes (sic)). Else
// it opens the listen socket +0x2428 (the bind string +0x2764 or Ftp_LocalAddress), binds, listens (a failure: 0x34), and sends its PORT line (0x774110): a reply of 200 only. No reply: 0x38.
s32 __fastcall Ftp_DataSetup(void *self);

// 0x5d2350 (thiscall): opens the data socket +0x2424 (bound to the bind string, if any, unchecked) and connects it to +0x2430 and +0x242e. Returns 0x40 on failure, else 0.
s32 __fastcall Ftp_DataConnect(void *self);

// 0x5d1f10 (thiscall): opens the control socket +4 and connects to the host +0x243c (inet_addr; if none and not the text at 0x773ce0, gethostbyname), port 21, then
// logs in with the user +0x249c and the password +0x24fc (asked by 331 or 332) and sends the type line (0x774134). Failures close both sockets and return 0x34 (socket),
// 0x40 (connect), 0x38 (no reply), 0x41 (no 220), 0x42 (no 230 to the password) or 0x43 (no 200).
s32 __fastcall Ftp_Login(void *self);

// 0x5d2180 (thiscall): threaded (+0x277e), posts command 2 and returns 0. Else no socket: 0x38; sends the quit line (0x774158); a reply neither none nor 221: 0x43; else closes both sockets, 0.
s32 __fastcall Ftp_Quit(void *self);

// 0x5d2400 (thiscall): no socket: 0x38. Data setup and connect; asks the remote name's size with the LIST line (0x77416c) and reads it (Ftp_ListLineSize of the first line); a 226 after it sets the data
// channel up again, 250 does not; then the RETR line (0x774160) and the file (Ftp_Receive to the local name). Closes the data socket at the end, except after a final 250 (sic). Returns 0 or the failure.
s32 __fastcall Ftp_Retrieve(void *self);

// 0x5d22a0 (thiscall; remote, local, pops 8): strncpy both names (0x100 each); threaded, posts command 3 and returns 0, else Ftp_Retrieve.
s32 __fastcall Ftp_RetrieveNamed(void *self, void *, const char *remote, const char *local);

// 0x5d1ac0 (thiscall; host, user, password, bind, pops 0x10): strncpy (0x60 each), the user and the password cut at the first CR or LF, a non-null bind assigned to the string at +0x2760;
// threaded, posts command 1 and returns 0, else Ftp_Login.
s32 __fastcall Ftp_Connect(void *self, void *, const char *host, const char *user, const char *password, const char *bind);

// 0x5d2850 (cdecl; the object): the thread: waits for the event +0x2770, sets ready, runs the command +0x2778 (0 the end, 1 login, 2 quit, 3 retrieve, others nothing: the result stays) and
// calls the callback +0x275c slot 0 (command, result; the first result before any is the object's address (sic)); after the end, sets the event +0x2774.
void __cdecl Ftp_ThreadProc(void *self);

// 0x5d1760 (thiscall; callback, threaded, pops 8): the base's constructor, both data sockets -1, the callback, the string empty (its allocator byte is stack garbage in the original), the two events, ready 1,
// vtable 0x702990, WSAStartup (1.1, unchecked). Returns the object.
u32 *__fastcall FtpClient_Ctor(u32 *self, void *, void *callback, u8 threaded);

// 0x5d1880 (thiscall): waits for ready with Sleep(1), posts command 0, waits for the thread's end if it was started, WSACleanup, the events' destructors, the string's, FtpBase_Dtor.
void __fastcall FtpClient_Dtor(u32 *self);

// 0x5d1860 (thiscall; flags, pops 4): FtpClient_Dtor, then GameHeap_Free with flag bit 0. Returns the object.
u32 *__fastcall FtpClient_DeletingDtor(u32 *self, void *, u8 flags);

// 0x5d29d0 (thiscall, this unused): WSAGetLastError's index in the 0-ended s16 table [0xf827ec] picks the text of [0xf827f0]: a message box of it, caption at 0x774810.
void __fastcall Ftp_WsaErrorBox(void *self);

// 0x5d2a20 (thiscall, this unused): WSAStartup (1.1); a failure: the error box and 1. Else 0.
s32 __fastcall Ftp_WsaStartup(void *self);

// 0x5d2a60 (thiscall, this unused): WSACleanup; a failure: the error box and 1. Else 0.
s32 __fastcall Ftp_WsaCleanup(void *self);
}
