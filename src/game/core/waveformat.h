// A wave format holder (a caller's local; the names are invented): +0 the channels (a byte), +1 the bits per sample (a byte), +2 the
// sample rate (a word), +4 a WAVEFORMATEX (0x12 bytes) built from those or from a sound format record, +0x18 that record (or null; the
// record's word +0x18 the rate, byte +0x1a the bits, byte +0x1b bit 0 stereo). Reimplementation of 0x6d3a80.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6d3a80 (thiscall; returns the WAVEFORMATEX at +4): once (its format tag, +4, still 0), from the record (+0x18) when there is one,
// else from +0 / +1 / +2: the channels (+6, 2 for the record's bit 0, else 1), the rate (+8), the bits (+0x12); the block align (+0x10,
// channels * bits >> 3), the bytes per second (+0xc, the rate times the block align), the tag 1 (PCM) and cbSize (+0x14) 0.
u8 *__fastcall WaveFormat_Build(u8 *holder);

}  // extern "C"
