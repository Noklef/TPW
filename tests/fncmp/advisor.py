# The advisor (src/game/world/advisor: 0x5997b0, 0x5989c0) against the originals. Lookup: tables of up to eight entries ended by the id
# 0x270f anywhere (first included), ids repeated, the wanted id among them, past the end, the end id itself or missing. Init: model slots 0
# and 1 with a holder or none; a holder's parts (0-6, named for the mouth parts and the bug head, case varied, or otherwise) and its motion
# bank 5 (loaded or not, 0-24 entries); the advisor's block, the particle pools' block and the sine table random.
# usage: advisor.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Lookup,Init)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, put  # noqa: E402

B = Bench([(0x5997b0, 0x5997f8), (0x429fe0, 0x42a010), (0x44b2e0, 0x44b3d0), (0x474070, 0x4740e0), (0x5442e0, 0x544340), (0x5989c0, 0x598ae0),
           (0x5ed580, 0x5ed5b0), (0x67a400, 0x6902c0)])
OPS = [k for k in os.environ.get('OPS', 'Lookup,Init').split(',') if k]
NAMES = [b'mouth - normal', b'MOUTH - aah', b'mouth - eee', b'Mouth - Ooh', b'mouth - sss', b'bug head', b'mouth', b'head', b'']


def lookup(r):
    table = bytearray(r.bytes(0x20 * 8))
    end = r.randrange(0, 8)
    ids = [r.choice([1, 2, 3, 100, 0x270e]) for _ in range(8)]
    for i in range(8):
        put(table, 0x20 * i, 'I', 0x270f if i == end else ids[i])
    wanted = r.choice(ids + [0x270f, 0, 4])
    return Call(0x5997b0, [wanted], {0x768fb8: table}, ret='a')


def holder(r, at, regions):
    """A holder at at: +8 -> P, P + 4 -> data (the parts: count word +0x42, elements of 0xa0 from [+0x70], names [+0x54]); motion bank 5:
    flags byte +0x54, count +0x58, entries from [+0x5c] + 4, 8 bytes apart, each a block whose end frame is at +8."""
    h = bytearray(r.bytes(0x100))
    n = r.choice([0, 1, 3, 6])
    data = bytearray(r.bytes(0x80))
    put(h, 8, 'I', at + 0x100)
    regions[at + 0x100] = put(bytearray(r.bytes(8)), 4, 'I', at + 0x200)
    put(data, 0x42, 'H', n)
    put(data, 0x70, 'I', at + 0x400)
    parts = bytearray(r.bytes(0xa0 * n))
    for i in range(n):
        put(parts, 0xa0 * i + 0x54, 'I', at + 0x800 + 0x20 * i)
        regions[at + 0x800 + 0x20 * i] = r.choice(NAMES) + bytes(1)
    regions[at + 0x200] = data
    if n:
        regions[at + 0x400] = parts
    h[0x54] = r.choice([0, 1, 2, 3, 4, r.getrandbits(8)])
    put(h, 0x58, 'I', r.choice([0, 5, 13, 14, 20, 24]))
    put(h, 0x5c, 'I', at + 0xc00)
    table = bytearray(r.bytes(4 + 8 * 24))
    for k in range(24):
        put(table, 4 + 8 * k, 'I', at + 0x1000 + 0x10 * k)
    regions[at + 0xc00] = table
    regions[at + 0x1000] = r.bytes(0x10 * 24)
    regions[at] = h
    return at


def init(r):
    regions = {0xf79660: r.bytes(0x170), 0xf868f0: r.bytes(0xf0), 0x877350: r.bytes(0x810)}
    slots = bytearray(r.bytes(0x160))
    for slot in range(2):
        put(slots, 0xb0 * slot + 4, 'I', holder(r, RAM + 0x2000 * slot, regions) if r.random() < 0.7 else 0)
    regions[0x790398] = slots
    return Call(0x5989c0, [], regions, ret='v')


def case(r):
    return init(r) if r.deal(OPS) == 'Init' else lookup(r)


if __name__ == '__main__':
    B.main(case)
