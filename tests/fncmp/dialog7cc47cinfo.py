# What the window [0x7cc47c] shows of an item (src/game/ui/dialog7cc47cinfo: 0x4b8850) against the original. Window_FindChild among decoys, _Grow, wcslen and the global save info
# (chatcommand.py's: its byte +0x25 set or not, now and then made) real; Window_SetShown, swprintf, the children's slot 9 and the censor (0x5d3230) recorders as driver.cpp's
# kDialog7cc47cInfoEnv; the censor now and then puts another block (of any reference count, or none) in the string. The string's characters are seen in the heap.
# usage: dialog7cc47cinfo.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import chatcommand  # noqa: E402
from dialog7cb400 import VT, kids  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, Mask, _recorder, blob_bytes, fill, heap, log_region, put, recorder  # noqa: E402

SHOWN, FORMAT, SLOT, CENSOR = 30, 31, 29, 21
B = Bench(chatcommand.RANGES + [(0x4b8850, 0x4b8aa0), (0x65d7bc, 0x65d840), (0x47bad0, 0x47bc90)],
          hooks={**chatcommand.HOOKS, 0x65d9dd: _recorder(SHOWN, 1), 0x67af00: _recorder(FORMAT, 3, cdecl=True), 0x5d3230: _recorder(CENSOR, 2)})
DIALOG, KIDS, ITEM, HOLDER, CHARS, BLOCK = RAM + 0x8000, RAM + 0x9000, RAM + 0xa000, RAM + 0xa200, RAM + 0xa400, RAM + 0xa800


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x1000, r.deal([0] * 9 + [1, 2]))}
    masks = chatcommand.save(r, regions, instance=False)
    blob_bytes(log, CENSOR, 0, 0xfff0, inner=(4, 0x7fff))
    regions[VT] = put(bytearray(0x40), 0x24, 'I', recorder(SLOT, 1))
    first, _ = kids(r, regions, KIDS, (0x49da, 0x49d9))
    regions[DIALOG] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', first)
    regions[0x7cc47c] = struct.pack('<I', r.deal([DIALOG] * 19 + [0]))
    block = bytearray(r.bytes(0x20))
    block[1] = r.deal([0, 1, 2, 0xff, 0xfe])   # (the block's reference count)
    regions[BLOCK] = block
    swap = r.deal([None, None, None, 0, BLOCK + 2])
    if swap is not None:
        fill(log, CENSOR, 0, struct.pack('<I', swap), offset=4)
    item = bytearray(r.bytes(0x80))
    put(item, 0x70, 'I', r.deal([HOLDER] * 9 + [0]))
    regions[ITEM] = item
    regions[HOLDER] = put(bytearray(r.bytes(0x20)), 0x14, 'I', CHARS)
    count = r.deal([0, 1, 2, 7, 30, 31, 32, 33, 100])
    regions[CHARS] = b''.join(struct.pack('<H', r.randrange(1, 0x10000)) for _ in range(count)) + bytes(2) + r.bytes(8)
    if masks:   # (a global save made after the string's block: its place in the heap moved by that block)
        moved = (((count | 0x1f) + 2) * 2 + 15) & ~15 if count else 0
        masks = [Mask(HEAP, [0x10 + moved + k for k in chatcommand.GARBAGE], 'uninitialised stack bytes (the allocators and comparators) written by 0x5aef50; nothing reads them')]
    return Call(0x4b8850, [r.deal([ITEM] * 9 + [0])], regions, ret='v', masks=masks)


if __name__ == '__main__':
    B.main(case)
