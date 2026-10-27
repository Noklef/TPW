// A visitor's score of an option, an object it might go to ("Calculating option score"): reimplementation of 0x4fcc30. The name is
// medium-confidence (the debug print's), the weights' meanings are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4fcc30 (thiscall on the visitor; object, pops 4): 0 when the visitor's last ride (+0x1e0, a handle; its thing slot at 0x7cfb90) is
// a thing of the object's id (+0xe). Else the weighted mean (unsigned; the products and sums wrap) of seven scores, each 0-100:
//   distance: 100 - min(100, d2 * 100 / 450 (signed)) for d2 the squared cells from the visitor's map id to the object's back of the queue
//     (Object_BackOfQueueTo, as column (id - 1) & 0x7f and row (id - 1) >> 7 taken as an s16), over the queue back's RE cell's word +8
//     (Map_RECell; signed, but divided unsigned) when that is not 0; weight the low word of 0x7850b0;
//   queue (d2 at most 8, else 0 with weight 0): 100 - Object_QueuePositionOfNone * 100 / (the object's +0x40 (0: 1) * 4), unsigned;
//     weight the dword 0x7850b4;
//   rating: 100 - 2 * min(50, |the visitor's wanted rating (the byte of 0x7850e4's 12-byte row +0x1f0) - Object_Rating(0)|); weight the
//     low word of 0x7850b8 when Object_Rating(1) is not 0, else 0;
//   two stats (the low bytes of the __ftol of the floats +0x1a4, +0x1a8) against the definition's bytes +0x144, +0x148: the byte of the
//     11 by 11 table 0x75d0f8 at [definition / 10][stat / 10]; weights the low words of 0x7850bc, 0x7850c0;
//   two more (+0x1ac, +0x1b0) when the object's +0x32 has bit 0: the byte of 0x75d178 at (stat + 4) / 5, else 0; weights the low words
//     of 0x7850c4, 0x7850c8.
// (A zero sum of weights faults, as in the game.) Then times the dword 0x785094 when Object_AgeDays is at most the dword 0x785090
// (unsigned); times the dword 0x785098 when the world's GameTable7cfb90_Get1da724_519590 record's +0x30 is positive and the definition's
// +0x114 is set; with the definition's +0xc4 positive, (1 - (c4 + 1) * -0.1) times it (x87 at 24 bits, __ftol's low dword), else with its
// +0x1b8 set and b8 * 1/3000 (the float 0x700750) above 1, (1 - that * -0.1f) times it. Then signed divisions: by 5, 4, 3 or 2 for the
// first of the visitor's +0x1e0, +0x1e2, +0x1e4, +0x1e6 that is the object's handle, and by 5, 4, 3, 2 for each of +0x1e8-+0x1ee that
// is. The original's debug prints are left out (their calls to Object_GetDef, Object_BackOfQueueTo and Thing_GetHandle kept).
u32 __fastcall Visitor_ScoreOption(u8 *visitor, void *, u8 *object);

// 0x4fcb10 (thiscall on the visitor; name invented): the major destination +0x1dc chosen: cleared, then the world's objects ([0x80239c]
// +0x1da746 the first's handle, each one's next at +0xc) with flag 4 of +0x32 and a queue back (Object_BackOfQueueTo) scored
// (Visitor_ScoreOption); one scoring 10 or more (unsigned) above the best so far (signed), or equal to it with bit 0 of the world's tick
// +0x1da70c set, that takes people into its queue (Object_QueueOpen) and that the visitor can be sent to (Sprite_SetDestinationCell of the
// queue back) becomes the destination and its score the best. (The debug prints are left out: their reads of [0x7cfb90] only.)
void __fastcall Visitor_ChooseMajorDest(u8 *visitor);

// 0x4fd570 (thiscall on the visitor; name invented, after its debug print "Minor Decision"): with a major destination (+0x1dc, a handle), the
// cells (column x0 - 2 to x0 + 1, row y0 - 2 to y0 + 1, on the map) round the visitor's map id (Cell_MakeHandle of the bytes +5, +7) that are
// nearer (squared, signed) to the destination's back of the queue (Object_BackOfQueueTo; none: nothing) than the visitor is: their things (the
// first handle at [0x8023a0] + id * 0x44 - 0x20, each one's next at +0xa) of model 3 (+2), other than the destination, that take people into
// their queue (Object_QueueOpen), scored (Visitor_ScoreOption): one above the best so far (signed, from 0), or equal to it with bit 0 of the
// world's tick ([0x80239c] +0x1da70c) set, is the best. Then, with a best, the route lengths (Route_CellDistance in the mode +0x188) from the
// destination's cell (+0x36; sic, the low word of a dword whose high word is the radius's) to the visitor's map id and to the best's cell
// (+0x36): when both are found and the best is nearer, the pair (0x17, the best's handle word +0) goes on the ring at +0x30 (Ring_PushUnique),
// the destination moves to +0x1de, the best becomes it, and the visitor is sent to the best's cell (Sprite_SetDestinationCell). (The debug
// prints are left out: their reads only.)
void __fastcall Visitor_ChooseMinorDest(u8 *visitor);
}
