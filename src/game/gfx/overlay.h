// The base of the model-backed screen overlays (vtable 0x6fee10, built at 0x476a90; the bar
// mesh 0x6fee88 and others derive from it): reimplementation of 0x476e80. +4 its holder (whose
// +4 model's +0x78 starts the node list, linked through +0xc), +0xa / +0xc (u16) the shown
// node's index as base + frame, +0x7c the shown node.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x476e80: with flag 0x80000 takes the base from desc +0xa, with flag 1 the frame from desc
// +4 (else, with neither, nothing); then shows node base + frame (u16; fewer when the list ends
// first, null past it): the old one gets 0x8010 set in its flags (+0), the new one cleared.
// Returns the shown node (+0x7c).
const u8 *__fastcall Overlay_ShowFrame(u8 *overlay, void *, u32 flags, const u8 *desc);
}
