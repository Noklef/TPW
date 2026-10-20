# The text button set-up (src/game/ui/buttontext: 0x486660) against the original: listrows.py's init_case surroundings (the text part's
# construction with the heap failing now and then, font 7's table, the window's old impl +0x9c and object +0x98, shown or not), the
# window grown to the 0x705a50 kind's size, the objects +0x98 released through their slot 5 as well; the GUI's registered windows [0xfaa610]: a chain of 0-3 nodes, one of them "!cbut" (Gui_HashName
# 0x2f) or none, its window a recorder (slot 8 giving an object of the +0x98 kind, or 0); the text part's slot 9 (its text; 0x48f830, a
# known skip behind 0x6ae0f0) a recorder of one argument in both (the vtable 0x6ff320's dword +0x24 a case region); the text any, 0 now
# and then; the font 0-12 or past them; the colour bytes any; the value any.
# usage: buttontext.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import listrows  # noqa: E402
from listrows import register  # noqa: E402
from fncmp import RAM, Bench, Call, recorder  # noqa: E402
from window import RANGES  # noqa: E402

B = Bench(sorted(set(RANGES + listrows.PART_RANGES + [
    (0x486660, 0x48677e), (0x659bc9, 0x659c42), (0x6588b6, 0x6588ef), (0x65d070, 0x65d1c8), (0x65f5b7, 0x65f5fe), (0x668f5a, 0x668f9c),
    (0x668820, 0x6688b3), (0x668ad3, 0x668bd4)])))


def case(r):
    regions = dict(listrows.init_case(r, 'HilightList_InitRow').regions)
    row = regions[listrows.ROW] + bytearray(r.bytes(0x30))   # (the 0x705a50 kind's +0x14c-+0x15a)
    regions[listrows.ROW] = row
    register(r, regions, '!cbut')
    regions[0x6ff344] = struct.pack('<I', recorder(9, 1))   # (the text part's slot 9)
    text = r.choice([0, RAM + 0xa400, r.getrandbits(32)])
    font = r.choice(list(range(13)) + [13, 0xff, r.getrandbits(32)])
    args = [listrows.ROW, text, font, r.getrandbits(8), r.getrandbits(8), r.getrandbits(8), r.getrandbits(32)]
    if r.random() < 0.3:   # (the colour arguments' upper bytes set: only the low bytes count)
        args[3:6] = [a | r.getrandbits(32) & 0xffffff00 for a in args[3:6]]
    return Call(0x486660, args, regions, ret='a')


if __name__ == '__main__':
    B.main(case)
