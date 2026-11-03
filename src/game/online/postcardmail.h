// The virtual postcard's mail text (0x5ca320; name invented, unconfirmed). The mail: strings whose characters are at +4 the sender, +0x20 the subject, +0x48 (length +0x4c) the player's name and +0x58 (length +0x5c) the
// password; +0x14 the head of a list of receivers (each node: +0 the next, +0xc the address), +0x3c, +0x40 and +0x64 the player, customer and language numbers, +0x68 the attachment or null (+4 its name, +0x14 its type,
// +0x20 its text in base64), the text made at +0x74.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5ca320 (thiscall): the mail's headers and body into the string at +0x74: a boundary from GetTickCount; From:, To:, Subject: with each text cut at any CR or LF (in place, all of them blanked); the X-BOSS- numbers; a name
// and a password, when they have a length, as " %02x" of each byte pair (wide characters: it stops at a pair whose second byte is not 0, sic; a byte over 0x7f prints eight digits, of which the next item overwrites all but
// three characters, sic); the MIME header, the HTML part; with an attachment that has text, its part; the last boundary. Each text goes through one 0x80-byte buffer before the boundary, as a stack frame of 0xc0 bytes.
void __fastcall PostcardMail_Build(u8 *mail);
}
