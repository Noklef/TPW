# The action replay's two actions behind ArReplay_Step (src/game/coaster/arreplayaction: 0x404d80, 0x404f70) against the originals: each action of the first's switch, others; the second's two with a text of 0-40 characters (the wide
# string's code real, the save singleton had or made) and others. The thing found through a cell's chain or a table entry's id. Its calls recorders (driver.cpp's kArReplayActionCalls, from CALLS below: arreplayaction.py --table prints them).
# usage: arreplayaction.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Do,Text)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, blob, fill, heap, log_region, put, set_return, strings  # noqa: E402

SET, ONLY, ECAC, B0CC, D7A4, FOCUS, NEXT, PREV, BY_ID, F447D60, AT, TEXT_CALL, SAVES_CTOR, CHECK, A950, A920, AA50, A860, SELECT = range(1, 20)
CALLS = {0x52f200: ('C', SET, 2), 0x52f580: ('C', ONLY, 2), 0x52fad0: ('C', ECAC, 1), 0x52fa90: ('C', B0CC, 1), 0x52f1b0: ('C', D7A4, 2), 0x5276d0: ('C', FOCUS, 1), 0x447ae0: ('C', NEXT, 1), 0x447b20: ('C', PREV, 1), 0x447da0: ('C', BY_ID, 1),
         0x447d60: ('C', F447D60, 1), 0x447dd0: ('C', AT, 1), 0x4682b0: ('C', TEXT_CALL, 3), 0x5aef50: ('T', SAVES_CTOR, 0), 0x5d3230: ('T', CHECK, 2), 0x4e2950: ('T', A950, 1), 0x4e2920: ('T', A920, 2), 0x4e2a50: ('T', AA50, 1), 0x4e2860: ('T', A860, 2),
         0x4815b0: ('T', SELECT, 1)}
HOOKS = {addr: _recorder(k, n, cdecl=kind == 'C') for addr, (kind, k, n) in CALLS.items()}
B = Bench([(0x404d80, 0x404ef4), (0x404f70, 0x4051c0), (0x4051d0, 0x405390), (0x479150, 0x4791b0), (0x479230, 0x479256), (0x47b750, 0x47bcf0), (0x4f23f0, 0x4f2450), (0x4f6080, 0x4f60a0), (0x67acd0, 0x67ad00), (0x5da3c0, 0x5da3c1)], hooks=HOOKS)
OPS = os.environ.get('OPS', 'Do,Text').split(',')
SELF, WORDS, THINGS, WORLD, SAVES, MADE = RAM, RAM + 0x100, RAM + 0x1000, RAM + 0x2000, RAM + 0x4000, RAM + 0x4100
TABLE = 0x7cfb90
CELLS, IDS, HIGH = 8, 12, 0x101
ACTIONS = [0, 0xa, 0x19, 0x1e, 0x23, 0x32, 0x32, 0x32, 0x3d, 0x3e, 0x42, 0x43, 0x44, 1, 0x45, 0x31, 0xffffffff]
TEXT_ACTIONS = [0x50, 0x50, 0x55, 0x55, 0x51, 0x54, 0, 0x150]


def world(r, regions):
    """The cells' chains over the things of the table's ids 1-11: ending in nothing, or at a thing of the kind 3."""
    things = bytearray(r.bytes(0x20 * (IDS + 1)))
    table = bytearray(r.bytes(20 * (HIGH + 1)))
    for h in range(IDS):
        put(table, 20 * h, 'I', THINGS + 0x20 * h)
        put(things, 0x20 * h + 2, 'B', r.choice([3, 1, 2, 4, 0, 0x83]))
        following = h + 1 if h + 1 < IDS else 0
        put(things, 0x20 * h + 0xa, 'H', r.choice([0, following, following, following, HIGH]))
    put(table, 20 * HIGH, 'I', THINGS + 0x20 * IDS)   # (a handle past a byte: of the same low byte as handle 1)
    put(things, 0x20 * IDS + 2, 'B', r.choice([3, 3, 1]))
    put(things, 0x20 * IDS + 0xa, 'H', 0)
    regions[THINGS] = things
    regions[TABLE] = table
    cells = bytearray(r.bytes(0x2b8 + 0x44 * CELLS))
    for c in range(CELLS):
        put(cells, 0x2b8 + 0x44 * c, 'H', r.choice([0, r.randrange(1, IDS), r.randrange(1, IDS), HIGH]))
    regions[WORLD] = cells
    regions[0x7cf83c] = struct.pack('<I', WORLD)
    regions[WORLD + 0x1da730] = struct.pack('<HH', r.getrandbits(16), r.choice([r.randrange(0, IDS), r.randrange(0, IDS), HIGH]))
    return r.randrange(CELLS) | r.getrandbits(16) << 16


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, SELF: r.bytes(0x10)}
    word = world(r, regions)
    if op == 'Do':
        return Call(0x404d80, [SELF, r.deal(ACTIONS), word], regions, conv='t')
    if r.random() < 0.3:
        word = 0xea60 | r.getrandbits(16) << 16
    n = r.choice([0, 1, 2, 31, 32, 40])
    regions[WORDS] = b''.join(struct.pack('<H', r.randrange(1, 0x10000)) for _ in range(40)) + bytes(2)
    saves = r.choice([0, SAVES, SAVES])
    regions[0xf7c58c] = struct.pack('<I', saves)
    set_return(log, SAVES_CTOR, MADE)
    blob(log, CHECK, 0, 0xe)   # (the string: its allocator is stack garbage)
    strings(log, TEXT_CALL, 6)
    strings(log, A920, 0, 1)
    strings(log, A860, 0, 1)
    fill(log, A920, 0, b'one text\x01\x01\0\0', which=0)
    fill(log, A860, 0, b'another text\x01\x01\0\0', which=1)
    # (the singleton's allocation failing, after the text's when it has one; or the text's, which faults in both)
    regions[HEAP] = heap(0x400, fail=(1 if n == 0 else 2) if not saves and r.random() < 0.4 else 1 if r.random() < 0.02 else 0)
    return Call(0x404f70, [SELF, r.deal(TEXT_ACTIONS), word, n | r.getrandbits(24) << 8, WORDS], regions, conv='t')


if __name__ == '__main__':
    if '--table' in sys.argv:
        for addr, (kind, k, n) in sorted(CALLS.items()):
            print('    {0x%06x, %s(%d, %d)},' % (addr, {'T': 'recorder', 'C': 'jpeg_recorder'}[kind], k, n))
        sys.exit(0)
    B.main(case)
