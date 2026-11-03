# The vehicle that brings the visitors (Game_ArrivalVehicle 0x51a2f0, src/game/world/arrivalvehicle) against the original. The game object a region at its fields +0x1da700: the random state, the handle and the
# three kinds' words 0 or small. The three names WStrings; the definitions (gObjectDefs at 0x7890a0, 1-6 of them, as endthing.py) named by them or not. What it calls is recorders: the object constructor 0x4db090 (the
# cell and the id logged as words: stack bytes in their high halves), Rides_FindById and Rides_SetElement. The heap failing now and then, which of 1-3 mostly (0 the random number; others a fault in both).
# usage: arrivalvehicle.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, LOG_BYTES, RAM, Bench, Call, _recorder, heap, log_region, put, set_return  # noqa: E402

B = Bench([(0x51a2f0, 0x51a68a), (0x412e60, 0x412f00), (0x50b350, 0x50b370), (0x67a400, 0x690000), (0x5da3c0, 0x5da3c1), (0x5f5fc0, 0x5f7f00)],
          hooks={0x4db090: _recorder(11, 4), 0x55a070: _recorder(2, 1, cdecl=True), 0x55a0b0: _recorder(3, 3, cdecl=True)})
DEFS, ITEMS, ITEM, OBJECT, THING, TEXTS, OUT = 0x7890a0, RAM + 0x1000, 0x400, RAM + 0x3000, RAM + 0x3800, RAM + 0x4000, RAM + 0x5000
GAME = RAM + 0x10000 - 0x1da700
NAMES = {0x802370: 'Bus', 0x801d58: 'Plane', 0x7cf3f0: 'Boat'}
ROWS = 8


def wide(s):
    return s.encode('utf-16-le') + b'\0\0'


def case(r):
    log = log_region()
    regions = {HEAP: heap(fail=r.choice([0] * 12 + [1])), LOG: log, OUT: r.bytes(4), OBJECT: r.bytes(0x40), THING: r.bytes(0x40)}
    game = bytearray(r.bytes(0x40))
    put(game, 0x2a, 'H', r.choice([0, 0, 0, r.randrange(1, ROWS)]))
    for offset in (0x2c, 0x2e, 0x30):
        put(game, offset, 'H', r.choice([0, 0, r.randrange(1, ROWS)]))
    regions[RAM + 0x10000] = game
    regions[0x7cfb90] = b''.join(struct.pack('<I', THING) + r.bytes(16) for _ in range(ROWS))
    for i, (address, text) in enumerate(NAMES.items()):
        name = r.choice([text, text, text, text.upper(), text + 's'])
        at = TEXTS + 0x40 * i
        regions[at] = wide(name)
        wstr = bytearray(r.bytes(0x14))
        put(wstr, 4, 'I', len(name))
        put(wstr, 8, 'I', at)
        put(wstr, 0xc, 'I', len(name) + 1)
        regions[address] = wstr
    n = r.randrange(1, 7)
    dtab = bytearray(r.bytes(0x740))
    items = bytearray(r.bytes(ITEM * n))
    for i in range(n):
        put(dtab, 4 * i, 'I', ITEMS + ITEM * i)
        s = r.choice(list(NAMES.values()) + ['bus', 'PLANE', 'Other', ''])
        items[ITEM * i + 0x390:ITEM * i + 0x390 + 2 * len(s) + 2] = wide(s)
    put(dtab, 0x528, 'i', r.choice([n, n, n, n, n - 1, 0]))
    put(dtab, 0x72c, 'I', r.getrandbits(32))
    regions.update({DEFS: dtab, ITEMS: items})
    set_return(log, 11, r.choice([OBJECT] * 12 + [0]))
    set_return(log, 2, r.getrandbits(32))
    struct.pack_into('<I', log, LOG_BYTES + 4 * 11, 3 << 16)   # (the cell and the id: words)
    which = r.choice([0, 0, 1, 2, 3, 1, 2, 3, 4, r.getrandbits(32)])
    return Call(0x51a2f0, [GAME, OUT, which], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
