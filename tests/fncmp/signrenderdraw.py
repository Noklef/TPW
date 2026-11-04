# A sign's two lines drawn and written out (src/game/gfx/signrender: 0x5ecb40) against the original: the text, the filters and the bump steps recorders, the image operations real.
# usage: signrenderdraw.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, fill, global_heap, heap, log_region, put, strings  # noqa: E402

TEXT, FILTER, BUILD = 8, 9, 10
B = Bench([(0x5ecb40, 0x5ed130), (0x5e7740, 0x5e78ac), (0x5e7d70, 0x5e80d0)] + gamestring.B_RANGES,
          hooks={0x5eab70: _recorder(TEXT, 3), 0x5ebf10: _recorder(FILTER, 2), 0x5eb820: _recorder(BUILD, 0)})
SELF, ARG, TEXTS, TO1, TO2, STAMPS = RAM, RAM + 0x800, RAM + 0x880, RAM + 0x1000, RAM + 0x1400, RAM + 0x2000


def case(r):
    log = log_region()
    table = bytearray(0x600)
    for at, stub in global_heap().items():
        table[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(0x8000)}
    width, height = r.choice([0, 1, 2, 4, 8]), r.choice([1, 2, 4, 8])
    arg = bytearray(0x20)
    struct.pack_into('<4I', arg, 0, *[(0 if r.random() < 0.02 else v) for v in (TEXTS, TEXTS + 0x40, TO1, TO2)])
    struct.pack_into('<3I', arg, 0x10, width, height, r.choice([1, 1, 2, 2, 3]))
    regions[ARG] = arg
    regions[TEXTS] = bytes(0x80)
    regions[TO1] = r.bytes(0x400)
    regions[TO2] = r.bytes(0x400)
    regions[STAMPS] = r.bytes(0x1000)
    self = bytearray(r.bytes(0x460))
    # The images the bump steps and the frame would have filled: no bigger than the destination.
    for at, block in ((0x18, STAMPS), (0x3b4, STAMPS + 0x400), (0x420, STAMPS + 0x800)):
        w, h = r.randrange(0, 2 * width + 1), r.randrange(0, 2 * height + 1)
        struct.pack_into('<4I', self, at, w, h, r.choice([1, 2, 4]), block if r.random() < 0.95 else 0)
    # The images remade: no old blocks.
    for at in (0x28, 0x384, 0x3f0):
        put(self, at + 0xc, 'I', 0)
    put(self, 4, 'I', r.choice([0, 0, 1, 2]))
    self[8] = r.choice([0, 1, 0x80])
    put(self, 0xc, 'I', r.choice([0, 1, 2, 3]))
    put(self, 0x10, 'I', r.choice([0, 1, 2, 3]))
    regions[SELF] = self
    strings(log, FILTER, 0, 1 << 1)
    if width:
        fill(log, FILTER, 1, r.bytes(2 * min(0x38, 4 * width * height)), records=2, through=0xc)
    return Call(0x5ecb40, [SELF, 0x2000, ARG], regions, conv='t', ret='b')


if __name__ == '__main__':
    B.main(case)
