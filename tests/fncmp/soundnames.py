# The sound names' table (src/game/core/soundnames) against the original: Save (0x6c0dc0) writes to a memory stream (fncmp's write stub
# and seek; the buffer BUF of random bytes, room for all of it or less) whose tell (slot 6) is a recorder answering 0 or its hash per call
# (fail_mask) or one small position for both calls. The table at SELF: a count of 0-5 (now and then any, its zero dwords cut short by the
# stream's room), a deque of pointers from block BLOCK1 (its end set a few entries on, so the walk crosses into BLOCK2 and BLOCK3 through
# the map MAP) or ending within it; elements null now and then, else objects at ELEMS whose name is a length byte (small, 0xff now and
# then: length + 1 is 0) and a text at TEXT.
# usage: soundnames.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, SEEK, STUBS, Bench, Call, fail_mask, log_region, recorder, set_return  # noqa: E402

B = Bench([(0x6c0dc0, 0x6c0f44)])
SELF, OUT, STREAM, BUF, MAP, ELEMS, TEXT = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x500, RAM + 0x600, RAM + 0x800
BLOCK1, BLOCK3, BLOCK2 = RAM + 0x900, RAM + 0xa00, RAM + 0x1000   # (BLOCK2 a whole block of 0x400 entries)


def element(r):
    return r.choice([0] + [ELEMS + 0x18 * i for i in range(8)] * 4)


def case(r):
    log = log_region()
    if r.random() < 0.6:
        fail_mask(log, 6, r.getrandbits(2))
    else:
        set_return(log, 6, r.choice([0, 4, 8, 0x10, r.randrange(0x100), 0x200]))
    size = 0x180
    left = r.choice([size, size, size, r.randrange(0, 0x40)])
    stream = struct.pack('<8I', STREAM + 0x20, BUF, left, BUF, size, 0, 0, 0) + struct.pack('<8I', 0, 0, STUBS, STUBS + 0x10, SEEK, 0,
                                                                                        recorder(6, 0), 0)
    count = r.choice([0, 1, 2, 3, 5, r.randrange(0x100)])
    where = r.randrange(5)
    full = where == 4 and r.random() < 0.5   # (a walk through all of BLOCK2: mostly null elements, the stream full early)
    blocks = [[element(r) for _ in range(0x20)], [element(r) if not full or r.random() < 0.03 else 0 for _ in range(0x400)],
              [element(r) for _ in range(0x20)]]
    m = r.randrange(1, 9)   # the first block's end (in entries)
    j = r.randrange(m)      # the walk's start
    if where == 0:          # the end in the first block
        end = BLOCK1 + 4 * r.randrange(j, m)
    elif where < 4 or not full:   # in the second
        end = BLOCK2 + 4 * r.randrange(6)
    else:                   # in the third, past all of the second
        end = BLOCK3 + 4 * r.randrange(6)
    me = bytearray(r.bytes(0x68))
    struct.pack_into('<4I', me, 0x3c, BLOCK1, BLOCK1 + 4 * m, BLOCK1 + 4 * j, MAP)
    struct.pack_into('<4I', me, 0x4c, r.getrandbits(32), r.getrandbits(32), end, r.getrandbits(32))
    struct.pack_into('<I', me, 0x64, count)
    elems = bytearray(r.bytes(0x18 * 8))
    for i in range(8):
        elems[0x18 * i + 0x10] = r.choice([0, 1, 3, 7, 12, 0x20, 0xff, r.getrandbits(8)])
        struct.pack_into('<I', elems, 0x18 * i + 0x14, TEXT + r.randrange(0x40))
    regions = {SELF: me, OUT: r.bytes(4), STREAM: stream, BUF: r.bytes(size), MAP: struct.pack('<4I', BLOCK1, BLOCK2, BLOCK3, 0), ELEMS: elems,
               TEXT: r.bytes(0x140), LOG: log}
    for at, b in zip((BLOCK1, BLOCK2, BLOCK3), blocks):
        regions[at] = struct.pack('<%dI' % len(b), *b)
    return Call(0x6c0dc0, [SELF, OUT, STREAM], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
