# The action recorder (src/game/core/actionrec) against the originals: its constructor, destructor, Start / Restart and record writers.
# Each stream is the game's (vtable 0x6fd6a0, resolved to the ports in the driver; no source, so starting opens a real memory source on the bench heap
# and the start record goes through StreamBase_Write into it), a recorder stream (vtable slot 1, the close, a recorder; a RefObject source with
# recorder slots or none), or, for the first, a record stream (the driver's write stub: at most `left` bytes to the cursor, a short `left` cutting a
# record off; its `left` is the next stream's vtable pointer when that matters). CreateSemaphoreA, CloseHandle and the Interlocked imports are
# recorders (InterlockedDecrement now and then 0: the source torn down); the heap now and then fails an allocation. The recording and playback flags
# and the mode are 0, 1 or random.
# usage: actionrec.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Write,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, STUBS, Bench, Call, heap, import_table, log_region, set_return, strings, vtable  # noqa: E402

B = Bench([(0x403400, 0x403f00), (0x5f7000, 0x5f7400), (0x5f8000, 0x5f9000), (0x5fa900, 0x5fa9a0), (0x618c00, 0x619260), (0x6307c0, 0x630800),
           (0x5da3c0, 0x5da3d0), (0x405000, 0x406000), (0x67a400, 0x690000)])
THIS, VT, BUF, SRC, VT_SRC = RAM, RAM + 0x400, RAM + 0x500, RAM + 0x600, RAM + 0x680
GAME_STREAM = 0x6fd6a0
IMPORTS = {'CreateSemaphoreA': (40, 0x6fd16c, 4, 8, 0), 'CloseHandle': (41, 0x6fd13c, 1, 0, 0), 'InterlockedIncrement': (42, 0x6fd0c0, 1, 0, 0),
           'InterlockedDecrement': (43, 0x6fd12c, 1, 0, 0)}
# name: (address, operands: n random dwords, 'm' the mode, '' none; this: w a writer's, s streams to start / stop, c room for the constructor)
OPS = {'Write': (0x403be0, 1, 'w'), 'WriteBB': (0x403c20, 3, 'w'), 'WriteBBW': (0x403c80, 4, 'w'), 'WriteW': (0x403d00, 2, 'w'),
       'WriteWW': (0x403d60, 3, 'w'), 'WriteWWW': (0x403dd0, 4, 'w'), 'WriteWB': (0x403e60, 3, 'w'),
       'Ctor': (0x403480, 0, 'c'), 'Dtor': (0x4036e0, 0, 's'), 'Start': (0x403a40, 'm', 's'), 'Restart': (0x403b00, 0, 's')}


def flags(r, o):
    struct.pack_into('<2I', o, 0, r.choice([0, 1, 1, 1, r.getrandbits(32)]), r.choice([0, 0, 1, 2, r.getrandbits(32)]))
    struct.pack_into('<I', o, 0x24, r.choice([0, 0, 0, 0, 1, r.getrandbits(32)]))


def writer(r):
    """A writer's recorder: the first stream a record stream (vtable VT, cursor BUF, bytes left)."""
    o = bytearray(r.bytes(0x30))
    flags(r, o)
    struct.pack_into('<3I', o, 0xc, VT, BUF, r.choice([0, 1, 2, 3, 4, 5, 6, 7, 0x20, 0x20, 0x20, 0x20]))
    vt = bytearray(r.bytes(0x10))
    struct.pack_into('<I', vt, 0xc, STUBS + 0x10)
    return {THIS: o, VT: vt, BUF: r.bytes(0x20)}


def streams(r, regions, dtor):
    """A recorder whose streams are the game's, recorder streams or (the first, not for the destructor) a record stream."""
    o = bytearray(r.bytes(0x30))
    flags(r, o)
    vts = bytearray(r.bytes(0x60))
    for i in range(3):
        kind = r.choice(['game', 'game', 'rec'] + ([] if dtor or i else ['stub', 'stub', 'stub']))
        if kind == 'game':
            struct.pack_into('<2I', o, 0xc + 8 * i, GAME_STREAM, SRC + 0x10 * i if dtor and r.random() < 0.5 else 0)
        else:
            vt = vtable({0: 1, 3: 2}, 8)   # (+0xc the write: its buffer, on the stack, logged as 0)
            if kind == 'stub':
                struct.pack_into('<I', vt, 0xc, STUBS + 0x10)
            vts[0x20 * i:0x20 * i + 0x20] = vt
            struct.pack_into('<2I', o, 0xc + 8 * i, VT + 0x20 * i, BUF if kind == 'stub' else r.choice([0, SRC + 0x10 * i]))
    regions[THIS], regions[VT] = o, vts
    regions[BUF] = r.bytes(0x100)
    regions[SRC] = b''.join(struct.pack('<3I', VT_SRC, 0, r.choice([1, 2, 3])) + r.bytes(4) for _ in range(3))
    regions[VT_SRC] = vtable({0: 1}, 12)


def case(r):
    address, n, kind = OPS[r.choice(PICK)]
    if kind == 'w':
        return Call(address, [THIS] + [r.getrandbits(32) for _ in range(n)], writer(r), conv='t', ret='v')
    log = log_region()
    set_return(log, 40, r.choice([0, 0x44, 0x44]))
    set_return(log, 43, r.choice([0, 1, 1, 2]))
    strings(log, 3, 0, 1)
    regions = {LOG: log, IMPORT_TABLE: import_table(log, IMPORTS), HEAP: heap(0x800, fail=r.choice([1, 2, 3, 4]) if r.random() < 0.1 else 0)}
    if kind == 'c':
        regions[THIS] = r.bytes(0x3e0)
        return Call(address, [THIS], regions, conv='t', ret='a')
    streams(r, regions, address == 0x4036e0)
    args = [r.choice([0, 0, 1, 2, r.getrandbits(32)])] if n == 'm' else []
    return Call(address, [THIS] + args, regions, conv='t', ret='v')


PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
