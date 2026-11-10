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

#include "../core/gamedata.h"

#pragma pack(push, 1)
struct Postcardmail_773d80_t {   // 0x773d80-0x773e98: the base64 alphabet (indexed from 0x773d80), then the mail's texts
    char base64[0x40];
    char f40[0x14];
    char f54[0xc];
    char f60[0x10];
    char f70[0x18];
    char f88[0x34];
    char fbc[0x18];
    char fd4[0x24];
    char ff8[0x20];
};
#pragma pack(pop)

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[0x4], gText_Postcardmail_773cdc, 0x00773cdc, 0x4);
GAME_DATA(Postcardmail_773d80_t, gPostcardmail_773d80, 0x00773d80, 0x118);
GAME_DATA(char[12], gPostcardmail_773e98, 0x00773e98, 0xc);
GAME_DATA(char[32], gPostcardMail_Build_773ea4, 0x00773ea4, 0x20);
GAME_DATA(char[48], gPostcardMail_Build_773ec4, 0x00773ec4, 0x30);
GAME_DATA(char[12], gPostcardMail_Build_773ef4, 0x00773ef4, 0xc);
GAME_DATA(char[140], gPostcardMail_Build_773f00, 0x00773f00, 0x8c);
GAME_DATA(char[44], gPostcardMail_Build_773f8c, 0x00773f8c, 0x2c);
GAME_DATA(char[52], gPostcardMail_Build_773fb8, 0x00773fb8, 0x34);
GAME_DATA(char[20], gPostcardMail_Build_773fec, 0x00773fec, 0x14);
GAME_DATA(char[20], gPostcardMail_Build_774000, 0x00774000, 0x14);
GAME_DATA(char[20], gPostcardMail_Build_774014, 0x00774014, 0x14);
GAME_DATA(char[8], gPostcardmail_774028, 0x00774028, 0x8);
GAME_DATA(char[20], gPostcardMail_Build_774030, 0x00774030, 0x14);
GAME_DATA(char[24], gPostcardMail_Build_774044, 0x00774044, 0x18);
GAME_DATA(char[24], gPostcardMail_Build_77405c, 0x0077405c, 0x18);
GAME_DATA(char[16], gPostcardMail_Build_774074, 0x00774074, 0x10);
GAME_DATA(char[4], gPostcardMail_Build_774084, 0x00774084, 0x4);
GAME_DATA(char[8], gPostcardMail_Build_774088, 0x00774088, 0x8);
GAME_DATA(char[12], gPostcardMail_Build_774090, 0x00774090, 0xc);
GAME_DATA(char[24], gPostcardMail_Build_77409c, 0x0077409c, 0x18);

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[0x80], gText_PostcardMail_Build_773d00, 0x00773d00, 0x80);   // the mail's HTML head
