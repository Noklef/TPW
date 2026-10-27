# The GUI library's start and end (src/game/ui/guisystem: 0x6584df, 0x658708) against the originals.
# Init (0x6584df): the capacity 1, 2, 4 or 8, the flag byte any (upper bytes random); the heap failing now and then (any of the first
# eight allocations: the root, the queues and their entries, the timers' pool, the timed table); the root window built as windowlife.py's
# Ctor (window.py's tree as the old root and hover, the mouse [0xfaa540], the old queues as window.py's), the clock [0x77c490] a recorder
# object (the pool's 0x100 nodes each read it), [0xfaa634] / [0xfaa62c] random. AX compared.
# Shutdown (0x658708): the root [0xfaa5d8] a recorder object (slot 2) or 0, the queues [0xfaa5f0] / [0xfaa5f4] and the timed table
# [0xfaa5f8] each set (their entries any or 0: only freed) or 0, the timers' pool [0xfaa630] an array (windowtimers.py's Shutdown) or 0,
# the registered windows [0xfaa610] and the named objects [0xfaa624] 0 or one node (its object a recorder object or 0). AX compared.
# usage: guisystem.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Init,Shutdown)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, log_region, put, set_return, vtable  # noqa: E402
from window import RANGES, VTABLE, point, queues, tree  # noqa: E402

CALLS = {'Init': 0x6584df, 'Shutdown': 0x658708}
B = Bench(RANGES + [(0x6584df, 0x658708), (0x658708, 0x658840),
                    (0x669700, 0x669735), (0x669735, 0x669767), (0x669767, 0x6697e3), (0x6596b0, 0x6596de),
                    (0x669c41, 0x669c62), (0x669c62, 0x669c8d), (0x669efb, 0x669f79), (0x6596e0, 0x65970e),
                    (0x662154, 0x66220d), (0x661e70, 0x661ece), (0x661f9b, 0x661fe5), (0x67afa0, 0x67b0d0), (0x67acf0, 0x67ada0),
                    (0x66245c, 0x66249a), (0x6624a0, 0x662500), (0x659a58, 0x659cc0), (0x661b3c, 0x661bab), (0x65968e, 0x6596a2)])
CLOCK, ROOT, OBJECTS, TABLES, ARRAY, NODES = RAM + 0x9000, RAM + 0x9100, RAM + 0x9200, RAM + 0x9300, RAM + 0x9400, RAM + 0x9600


def init(r):
    log = log_region()
    set_return(log, 0, r.getrandbits(32))
    regions = {LOG: log, VTABLE: vtable({5: 1}), CLOCK: struct.pack('<I', VTABLE), 0x77c490: struct.pack('<I', CLOCK)}
    wins = tree(r, regions)
    for i, w in enumerate(wins):
        put(regions[w], 0x60, 'I', r.choice([wins[i - 1], 0]) if i else 0)
        put(regions[w], 0xe0, 'H', r.choice([0, 1, 0xff, 0x100, 0xffff, r.getrandbits(16)]))
    queues(r, regions)
    x, y = point(r, regions, wins)
    regions[0xfaa540] = struct.pack('<II', x & 0xffffffff, y & 0xffffffff)
    regions[0xfaa5d8] = struct.pack('<5I', r.choice([wins[0], 0]), r.getrandbits(32), r.choice(wins + [0, 0]), r.getrandbits(32),
                                    r.choice([0] * 6 + [1]))
    regions[0xfaa568] = r.bytes(0x18)
    regions[0xfaa5bc] = r.bytes(4)
    regions[0xfaa5f8] = r.bytes(4)
    regions[0xfaa608] = r.bytes(4)
    regions[0xfaa62c] = r.bytes(0x10)
    regions[HEAP] = heap(0x3800, r.choice([0] * 8 + list(range(1, 9))))
    flag = r.choice([0, 1, 0xff, r.getrandbits(8)]) | (r.getrandbits(24) << 8 if r.random() < 0.5 else 0)
    return Call(CALLS['Init'], [r.getrandbits(32), r.choice([1, 2, 4, 8]), flag], regions, ret='w')


def shutdown(r):
    log = log_region()
    regions = {LOG: log, VTABLE: vtable({0: 1, 2: 1}), ROOT: struct.pack('<I', VTABLE), HEAP: heap(),
               OBJECTS: struct.pack('<I', VTABLE), OBJECTS + 0x20: struct.pack('<I', VTABLE)}
    set_return(log, 2, r.getrandbits(32))
    regions[0xfaa5d8] = struct.pack('<3I', r.choice([ROOT] * 9 + [0]), r.getrandbits(32), r.getrandbits(32))
    for k, g in enumerate((0xfaa5f0, 0xfaa5f4, 0xfaa5f8)):
        regions[TABLES + 0x20 * k] = struct.pack('<2I', *[r.choice([RAM + 0xb000 + 0x100 * k, 0, r.getrandbits(32)])] * 2)
        regions[g] = struct.pack('<I', r.choice([TABLES + 0x20 * k] * 4 + [0]))
    n = r.choice([1, 2, 3])
    nodes = [ARRAY + 4 + 0x20 * i for i in range(n)]
    buf = bytearray(r.bytes(4 + 0x20 * n))
    put(buf, 0, 'i', r.choice([n, n, n - 1, 0]))
    for i in range(n):
        put(buf, 4 + 0x20 * i + 0x18, 'I', nodes[i + 1] if i + 1 < n else 0)
        put(buf, 4 + 0x20 * i + 0x1c, 'I', nodes[i - 1] if i else 0)
    regions[ARRAY] = buf
    regions[0xfaa62c] = r.bytes(4) + struct.pack('<I', r.choice([nodes[0], 0, 0])) + r.bytes(8)
    for k, g in enumerate((0xfaa610, 0xfaa624)):
        node = bytearray(r.bytes(0x20))
        put(node, 0, 'I', r.choice([OBJECTS + 0x20 * k, OBJECTS + 0x20 * k, 0]))
        put(node, 0x14 if g == 0xfaa610 else 0x10, 'I', 0)
        regions[NODES + 0x40 * k] = node
        regions[g] = struct.pack('<I', r.choice([NODES + 0x40 * k, 0, 0]))
    return Call(CALLS['Shutdown'], [], regions, ret='w')


def case(r):
    op = r.deal(OPS)
    return init(r) if op == 'Init' else shutdown(r)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
