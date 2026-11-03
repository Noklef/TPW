# The world's tick and a thing's (src/game/world/worldupdate: 0x50b360, 0x516380, 0x5166b0) against the originals. The thing table a chain of slots in use (handles their indices, now and then a null or deleted thing), the kinds any;
# the calls of Thing_Update, the rides' and the two last recorders; the delete queue real (0x51ad90, 0x516200), 0-3 handles. The tick a multiple of 30 mostly, the gates set.
# usage: worldupdate.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=thing,world,mode1)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, put, set_return  # noqa: E402

UPDATES = [0x501650, 0x5019f0, 0x4e0b90, 0x4e0e00, 0x4da5a0, 0x4da490, 0x4d7060, 0x4d73c0, 0x4d4660, 0x4d4810, 0x4d6360, 0x4d6410, 0x502960, 0x5029f0, 0x4f7c60, 0x4f7c90, 0x59a550, 0x512880, 0x4d1e90]
hooks = {at: _recorder(k, 0) for k, at in enumerate(UPDATES)}
SETTING, ATEXIT, FIND, GET, SET, COUNT, PLAIN, TICK, ACTION = 19, 20, 21, 22, 23, 24, 25, 26, 27
hooks.update({0x550ca0: _recorder(SETTING, 0), 0x67a7d0: _recorder(ATEXIT, 1, cdecl=True), 0x55a070: _recorder(FIND, 1, cdecl=True), 0x55a390: _recorder(GET, 2, cdecl=True), 0x55a0b0: _recorder(SET, 3, cdecl=True),
              0x4c9130: _recorder(COUNT, 0), 0x4fa990: _recorder(PLAIN, 0), 0x4d7b20: _recorder(TICK, 0), 0x4041d0: _recorder(ACTION, 0)})
B = Bench([(0x50b360, 0x50b500), (0x516380, 0x5166ab), (0x5166b0, 0x5166f9), (0x50b350, 0x50b35d), (0x51ad90, 0x51add1), (0x516200, 0x516270), (0x5da3c0, 0x5da3c1)], hooks=hooks)
WORLD = RAM + 0x10000 - 0x1da700
THINGS, NODES = RAM + 0x1000, RAM + 0x3000
TABLE, ROWS, THING = 0x7cfb90, 12, 0x40
OPS = [n for n in ('thing', 'world', 'mode1') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def setting_regions(r, regions):
    regions[0xfb34a8] = bytes([r.getrandbits(8), r.getrandbits(8) if r.random() < 0.5 else r.getrandbits(8) & 0xfe, r.getrandbits(8), r.getrandbits(8) if r.random() < 0.5 else r.getrandbits(8) & 0xfe])
    regions[0xfb3b7c] = struct.pack('<I', r.choice([1, 1, 1, 0, 2, r.getrandbits(32)]))


def table(r, regions):
    """Slots 1..ROWS-1: in use chained from [0x7cf56c] in a random order; a thing's handle its slot's index, its kind any, byte +3 deleted now and then."""
    rows = bytearray(r.bytes(20 * ROWS))
    things = bytearray(r.bytes(THING * ROWS))
    used = r.sample(range(1, ROWS), r.randrange(0, ROWS))
    null = r.choice(used) if used and r.random() < 0.05 else None
    for i, h in enumerate(used):
        at = 20 * h
        next_slot = TABLE + 20 * used[i + 1] if i + 1 < len(used) else 0
        put(rows, at + 4, 'I', h | r.getrandbits(16) << 16)
        put(rows, at + 8, 'I', next_slot)
        put(rows, at + 12, 'I', TABLE + 20 * used[i - 1] if i else 0)
    for h in range(ROWS):
        if h not in used:
            put(rows, 20 * h + 8, 'Q', 0)   # (a slot on the delete queue that was not in use: its links null)
        put(rows, 20 * h, 'I', 0 if h == null else THINGS + THING * h)
        t = THING * h
        put(things, t, 'H', h)
        put(things, t + 2, 'B', r.choice([1, 3, 4, 5, 6, 7, 8, 9, 11, 15, 18, 19, 0x20]))
        put(things, t + 3, 'B', r.choice([0, 0, 0, 0, 1, r.getrandbits(8)]))
    regions[0x7cf56c] = struct.pack('<I', TABLE + 20 * used[0] if used and r.random() < 0.95 else 0)
    regions[0x801bbc] = r.bytes(4)
    regions[TABLE] = rows
    regions[THINGS] = things
    return used


def world(r, log, regions):
    table(r, regions)
    setting_regions(r, regions)
    # The world's fields.
    fields = bytearray(r.bytes(0x40))
    put(fields, 0xc, 'I', r.choice([29, 59, 89, r.getrandbits(32)]))
    put(fields, 0x10, 'I', r.choice([1, 1, 1, 0]))
    put(fields, 0x20, 'H', r.randrange(ROWS))
    put(fields, 0x32, 'H', r.choice([0, 3, 5, 5, 7]))
    put(fields, 0x38, 'I', r.choice([1, 1, 1, 4]))
    regions[RAM + 0x10000] = fields
    set_return(log, FIND, r.getrandbits(32))
    set_return(log, GET, r.choice([1, 1, 1, 0, 2]))
    set_return(log, COUNT, r.choice([0, 0, 0, 1]))
    set_return(log, PLAIN, r.choice([1, 1, 1, 1, 0]))
    # The delete queue: 0-3 nodes, each a slot's handle.
    count = r.randrange(4)
    queue = bytearray(r.bytes(0x10))
    put(queue, 4, 'I', HEAP + 0x10 if count else 0)
    put(queue, 8, 'I', HEAP + 0x10 + 0x10 * (count - 1) if count else 0)
    put(queue, 0xc, 'I', count)
    block = heap(0x400)
    for i in range(count):
        at = 0x10 + 0x10 * i
        put(block, at, 'H', r.randrange(1, ROWS))
        put(block, at + 4, 'I', HEAP + at + 0x10 if i + 1 < count else 0)
    regions[0x801f38] = queue
    regions[HEAP] = block


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log}
    if op == 'thing':
        setting_regions(r, regions)
        thing = bytearray(r.bytes(0x20))
        put(thing, 2, 'B', r.choice(list(range(0, 21)) + [r.getrandbits(8)]))
        regions[THINGS] = thing
        return Call(0x50b360, [THINGS], regions, conv='t', ret='v')
    world(r, log, regions)
    return Call(0x516380 if op == 'world' else 0x5166b0, [WORLD], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
