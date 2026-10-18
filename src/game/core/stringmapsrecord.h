// A record of two wide strings, three string-valued maps and a list of counted strings, built at 0x4f0010 (not ported: it reads its
// allocator bytes from uninitialised stack) and held at +0xe60 of the object whose unwinds are 0x6dc4xx / 0x6dc5xx: { +0 a std::wstring,
// +0x10 a std::wstring, +0x20 a map of stdtree's kind D (core/wstringmap's), +0x30 one of kind C, +0x40 one of kind E, +0x54 a list
// {allocator, head +0x58, count +0x5c} of 0x18-byte nodes with a counted wide buffer at +0xc }. What it is for is not known yet.
// Reimplementation of its destructor 0x4e54c0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4e54c0 (thiscall; name unconfirmed; run only by exception unwinds, 0x6dc4f4 / 0x6dc5a5): the list erased node by node
// (List_EraseNodeCounted2), its sentinel freed and its head and count cleared; the maps destroyed E, C, then D (StdTree_DtorE / C / D,
// inlined there); then the strings at +0x10 and +0 tidied (CountedBuf_Reset(true), inlined).
void __fastcall StringMapsRecord_Dtor(u8 *record);
}
