# The park statistics graph of src/game/gfx/statsgraph against the originals: the greatest (0x49a7b0) and least (0x49a8f0) sample of the shown lines
# (the flags byte 0x751fa0) within the graph's window (kind [0x7ca2ec]), from 0: eight CircBufU32s of 144 in the game's record (the table 0x7cfb90 entry
# for the game object's +0x1da720; GameTable7cfb90_Get1da720_519510 run, its debug print a no-op) scanned by 0x4801d0 / 0x480380 (run).
# usage: statsgraph.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Max,Min)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

CALLS = {'Max': 0x49a7b0, 'Min': 0x49a8f0, 'Max2': 0x4a51d0}
B = Bench([(0x49a7b0, 0x49a8e6), (0x49a8f0, 0x49aa26), (0x4a51d0, 0x4a52b0), (0x519510, 0x519550), (0x4801d0, 0x480390), (0x480380, 0x480456),
           (0x5da3c0, 0x5da3c1)])
GAME, RECORD = RAM, RAM + 0x10000
LINES = [0x1f100, 0x1f350, 0x1fc90, 0x1fee0, 0x20130, 0x20380, 0x1f5a0, 0x1f7f0]
LINES2 = [0x20cc0, 0x20f10, 0x21160, 0x205d0, 0x213b0]  # the second graph's (0x4a51d0): +0x20f10 holds floats


def circbuf_f(r):
    buf = bytearray(circbuf(r))
    for i in range(0x90):
        put(buf, 4 + 4 * i, 'f', r.choice([0.0, 0.0, 1.0, -1.0, 0.5, 2.5, -2.5, 99.75, 3e9, -3e9, float('nan'), r.uniform(-50, 50)]))
    return bytes(buf)


def circbuf(r):
    buf = bytearray(r.bytes(0x250))
    for i in range(0x90):
        put(buf, 4 + 4 * i, 'i', r.choice([0, 0, 1, -1, 5, -5, 100, -100, 0x7fffffff, -0x80000000, r.randrange(-50, 50)]))
    count = r.choice([0, 1, 5, 12, 13, 36, 40, 144, r.randrange(0, 145)])
    put(buf, 0x244, 'i', r.choice([0, max(0, count - 1), r.randrange(0, 144)]))
    put(buf, 0x248, 'i', count)
    buf[0x24c] = r.choice([0, 1])
    return bytes(buf)


def case(r, op=None):
    index = r.randrange(4)
    table = bytearray(r.bytes(0x50))
    put(table, 20 * index, 'I', RECORD)
    regions = {GAME + 0x1da720: struct.pack('<H', index) + r.bytes(2), 0x7cf83c: struct.pack('<I', GAME), 0x7cfb90: bytes(table),
               0x751fa0: bytes([r.choice([0, 1, 0x80, 0xff, r.getrandbits(8), r.getrandbits(8)])]) + r.bytes(3),
               0x7ca2ec: struct.pack('<I', r.choice([0, 1, 2, 3]))}
    op = op or r.deal(OPS)
    if op == 'Max2':
        regions[0x753c48] = bytes([r.choice([0, 1, 2, 0x1f, 0xff, r.getrandbits(8), r.getrandbits(8)])]) + r.bytes(3)
        regions[0x7cb39c] = struct.pack('<I', r.choice([0, 1, 2, 3]))
        for offset in LINES2:
            regions[RECORD + offset] = circbuf_f(r) if offset == 0x20f10 else circbuf(r)
    else:
        for offset in LINES:
            regions[RECORD + offset] = circbuf(r)
    return Call(CALLS[op], [], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
