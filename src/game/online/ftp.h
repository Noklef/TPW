// The FTP client (its object's reply line at +0x424, the LIST command's "LIST %s\r\n" at 0x5d2468): reimplementation of the listing
// line's size field 0x5d2720.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5d2720 (thiscall, this unused; line, pops 4; name unconfirmed; the LIST reply at 0x5d24cc reads its +0x424 line with it): the number
// (atoi) in the fifth field of a directory listing line, an "ls -l" line's size: four times a run of non-spaces, then a run of spaces
// (isspace, the char sign-extended), skipped. A NUL is not a space, so a line of fewer fields is read on past its end (sic).
s32 __fastcall Ftp_ListLineSize(void *self, void *, const char *line);
}
