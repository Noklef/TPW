# The registry owner's index selected (src/game/core/namedobjects: 0x4093d0, 0x409480, 0x409790) against the originals: the index the same or new, past the count or negative; the vtable's +0x18 a recorder that writes the next
# name over the object's own (its value is the name's block); GlobalSave_ThemeSelect a recorder; the save info set, or null with the heap's block failing. By name: the wanted name first, later, in another case, or absent.
# usage: namedobjectsselect.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import gamestring  # noqa: E402
from fncmp import HEAP, LOG, RAM, VSTUBS, Bench, Call, _recorder, fill, heap, log_region, recorder, set_return  # noqa: E402

SLOT, SELECT = 1, 9
B = Bench([(0x4093d0, 0x40960f), (0x409790, 0x4098e8), (0x409410, 0x409480), (0x409ce0, 0x409cf0)] + gamestring.B_RANGES, hooks={0x5b08a0: _recorder(SELECT, 1)})
SELF, VT, DATA, NODES, NAME = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400
OWNER = 0x786b68
OPS = ['Index', 'Index', 'ByName', 'ByName', 'ByName', 'Mode']
NAMES = [b'aaaa', b'bbbb', b'CCCC', b'dddd', b'eeee', b'ffff']


def object_bytes(r, index, count):
    return struct.pack('<IIIII', VT, 4, DATA, 8, 8) + struct.pack('<i', index) + r.bytes(0xc) + struct.pack('<III', r.getrandbits(32), NODES, r.getrandbits(32)) + struct.pack('<I', count) + r.bytes(0x14)


def case(r):
    op = r.deal(OPS)
    log = log_region()
    set_return(log, SELECT, r.getrandbits(32))
    fill(log, SLOT, 0, b''.join(n + b'\0' for n in NAMES), records=len(NAMES))
    nodes = b''.join(struct.pack('<II', DATA, NODES + 8 * ((i + 1) % 8)) for i in range(8))
    null = r.random() < 0.15
    index = r.choice([0, 1, 2, 3, 4, -1, -2])
    regions = {LOG: log, VT: bytes(0x18) + struct.pack('<I', recorder(SLOT, 1)), DATA: r.choice(NAMES) + b'\0' + r.bytes(4), NODES: nodes, 0xf7c58c: struct.pack('<I', 0 if null else r.getrandbits(32) | 1), gamestring.LOCALE: bytes(4)}
    regions[HEAP] = heap(fail=1) if null else heap()
    if op == 'Mode':
        regions[OWNER] = object_bytes(r, r.choice([0, 1, 2, 3, 4]), 4)
        return Call(0x409790, [r.choice([0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 0xffffffff])], regions, ret='a')
    regions[SELF] = object_bytes(r, 0 if null and op == 'ByName' else index, 2 if null and op == 'ByName' else r.choice([0, 1, 2, 3, 5]))
    if op == 'Index':
        return Call(0x4093d0, [SELF, r.choice([0, 1, 2, 3, 4, -1 & 0xffffffff, index & 0xffffffff, index & 0xffffffff])], regions, conv='t', ret='a')
    want = r.choice(NAMES + [n.swapcase() for n in NAMES] + [b'zzzz', b'aaa', b'aaaaa'])
    if null:   # (the info made only once: found at once, so the failed block is the only try)
        want = bytes(regions[DATA][:4])
    regions[NAME] = struct.pack('<IIIII', r.getrandbits(32), len(want), NAME + 0x20, 8, 8) + bytes(0xc) + want + b'\0'
    return Call(0x409480, [SELF, NAME], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
