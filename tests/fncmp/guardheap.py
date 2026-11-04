# The heap with guard words (src/game/core/guardheap: 0x5d4ba0, 0x5d4c20) against the originals: sizes at and around 0, malloc (a recorder) answering a block, 0 or -1; blocks with both
# tags, a bad head, a freed head, a bad tail, sizes at the limits; pointers near null (within 0x800: a message; just outside: a fault in both). The tags are the case's, and the message
# pointer at 0x7748f4 a cdecl recorder.
# usage: guardheap.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Alloc,Free)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, recorder, set_return  # noqa: E402

MALLOC, FREE, MESSAGE = 1, 2, 3
B = Bench([(0x5d4ba0, 0x5d4c11), (0x5d4c20, 0x5d4d03)], hooks={0x67dcd0: _recorder(MALLOC, 1, cdecl=True), 0x67ad90: _recorder(FREE, 1, cdecl=True)})
OPS = [k for k in os.environ.get('OPS', 'Alloc,Free').split(',') if k]
BLOCK = RAM + 0x1000
LIMIT = 0x6400000


def tags(r):
    head, tail, freed = r.bytes(4), r.bytes(4), r.bytes(4)
    return head, tail, freed, {0x774934: head, 0x77492c: tail, 0x7749c0: freed, 0xf827f4: struct.pack('<I', r.choice([0, 1, 5, r.getrandbits(32)]))}


def alloc(r):
    log = log_region()
    regions = tags(r)[3]
    regions.update({LOG: log, 0x7748f4: struct.pack('<I', recorder(MESSAGE, 2, cdecl=True)), BLOCK: r.bytes(0x180)})
    set_return(log, MALLOC, r.choice([BLOCK, BLOCK, BLOCK, BLOCK, 0, 0xffffffff]))
    return Call(0x5d4ba0, [r.choice([0, 1, 2, 3, 4, 7, 8, 0x18, 0x100, -1, -0x80000000]) & 0xffffffff], regions, conv='s', ret='a')


def free(r):
    log = log_region()
    head, tail, freed, regions = tags(r)
    regions.update({LOG: log, 0x7748f4: struct.pack('<I', recorder(MESSAGE, 1, cdecl=True))})
    block = bytearray(r.bytes(0x180))
    size = r.choice([1, 2, 3, 4, 0x18, 0x100])
    kind = r.choice(['good', 'good', 'good', 'head', 'freed', 'tail', 'size', 'near'])
    user = BLOCK + 8
    near_tail = tail
    if kind == 'tail':  # (one byte of four differs: the compare stops there)
        at = r.randrange(4)
        near_tail = tail[:at] + bytes([tail[at] ^ 1]) + tail[at + 1:]
    block[0:4] = {'head': bytes([head[0] ^ r.choice([1, 0x80])]) + head[1:], 'freed': freed}.get(kind, head)
    struct.pack_into('<I', block, 4, size)
    block[8 + size:12 + size] = near_tail
    if kind == 'size':
        struct.pack_into('<i', block, 4, r.choice([0, -1, -0x80000000, LIMIT, LIMIT + 1]))
    if kind == 'near':
        user = r.choice([0, 1, 0x7ff, -0x7ff, 0x400, -1]) & 0xffffffff
    regions[BLOCK] = block
    return Call(0x5d4c20, [user], regions, conv='s', ret='a')


def case(r):
    return alloc(r) if r.deal(OPS) == 'Alloc' else free(r)


if __name__ == '__main__':
    B.main(case)
