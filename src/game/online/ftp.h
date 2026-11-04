// The FTP client (object of 0x2780 bytes; the control socket +4, its sockaddr +8, bytes sent +0x18, the reply buffer +0x20 and its length
// +0x420, the data buffer +0x424; the LIST command's "LIST %s\r\n" at 0x5d2468; every Winsock call through the import table, by way of the
// thunks at 0x657cc0): reimplementation of the socket layer and of the listing line's size field 0x5d2720. All names are unconfirmed.
#pragma once

#include "../../../re/types.h"

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
}
