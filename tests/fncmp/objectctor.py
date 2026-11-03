# The placed object's constructor (src/game/world/objectctor: Object_Construct 0x4db090) against the original. The definitions (gObjectDefs at 0x7890a0, 1-3 of
# them) as placeobject.py's, ObjectDefs_Find, IntPair_Rotated, Thing_GetHandle, ThingMsg06_Ctor and the text copy real; the message centre there or made (the
# heap failing now and then); every other callee a recorder as driver.cpp's kObjectCtorEnv. The 0x450-byte object over random bytes, compared whole.
# usage: objectctor.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, LOG_BYTES, RAM, Bench, Call, _recorder, blob_bytes, heap, log_fold, log_region, put, set_return, strings  # noqa: E402

GROW, CONSTRUCT, ADD, ADDTHING, POST, SPREAD, HASKEY, UNLOCK, MONEY, DEBIT, STATE, CLAIM, F4DCF90, FINDRIDE, SETWORD, CAPACITY, ELEMENT, NOW, CONTROLS, NAME, SUFFIX, ANALYSER, STAMP = range(9, 32)
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x4db090, 0x4db7c0), (0x40f330, 0x40f360), (0x50b350, 0x50b370), (0x412e90, 0x412f00), (0x4d9cc0, 0x4d9d80), (0x67ae00, 0x67af10), (0x5da3c0, 0x5da3c1),
           (0x67acf0, 0x67ada0), (0x405460, 0x405530), (0x40f970, 0x40f9e0), (0x4053c0, 0x40545d), (0x40f6d0, 0x40f6e9), (0x40f950, 0x40f961), (0x67acb0, 0x67acb1), (0x67afa0, 0x67b0d0)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None,
                 0x50afe0: _recorder(CONSTRUCT, 2), 0x519d80: _recorder(ADD, 1), 0x40f9e0: _recorder(ADDTHING, 2), 0x40fb10: _recorder(POST, 1), 0x4d8440: _recorder(SPREAD, 2),
                 0x4d4b70: _recorder(HASKEY, 1), 0x4d4ad0: _recorder(UNLOCK, 1), 0x5195d0: _recorder(MONEY, 0), 0x4d01f0: _recorder(DEBIT, 1), 0x4e0e60: _recorder(STATE, 1),
                 0x4dd500: _recorder(CLAIM, 1), 0x4dcf90: _recorder(F4DCF90, 0), 0x55a070: C(FINDRIDE, 1), 0x55a300: C(SETWORD, 2), 0x4dd7f0: _recorder(CAPACITY, 1),
                 0x55a0b0: C(ELEMENT, 3), 0x4f8690: _recorder(NOW, 9), 0x4d3d10: _recorder(CONTROLS, 2), 0x413020: _recorder(NAME, 3), 0x4130b0: _recorder(SUFFIX, 3),
                 0x519510: _recorder(ANALYSER, 0), 0x4c7680: _recorder(STAMP, 0), 0x454550: C(GROW, 2)})
DEFS, ITEMS, ITEM, OBJECT, WORLD, TEXT, TIME, CONTROL, RIDE = 0x7890a0, RAM + 0x1000, 0x500, RAM + 0x3000, RAM + 0x100000, RAM + 0x4000, RAM + 0x4100, RAM + 0x4200, RAM + 0x4300
NIL, COUNTER, CENTRE = RAM + 0x4400, 0x785de0, 0x788d3c
BLACK = 1


def small(r):
    return r.choice([0, 1, 2, 3, -1, -2, -3, 5, -5, r.getrandbits(32) - (1 << 31)])


def case(r):
    log = log_region()
    log_fold(log)
    nil = bytearray(0x20)
    struct.pack_into('<I', nil, 0x10, BLACK)
    regions = {LOG: log, HEAP: heap(0x1000, r.choice([0] * 12 + [1, 1, 2, 3])), NIL: nil,
               COUNTER: struct.pack('<II', r.choice([0, 1, 2, 0x7fffffff, 0xffffffff]), r.choice([0, NIL]))}
    regions[CENTRE] = struct.pack('<I', r.choice([0, RAM + 0x4500, RAM + 0x4500, RAM + 0x4500, r.getrandbits(32) | 1]))
    n = r.randrange(1, 4)
    ids = r.sample(range(1, 0x10000), n)
    k = r.randrange(n)
    dtab = bytearray(r.bytes(0x740))
    items = bytearray(r.bytes(ITEM * n))
    for i in range(n):
        put(dtab, 4 * i, 'I', ITEMS + ITEM * i)
        put(items, ITEM * i, 'H', ids[i])
    put(dtab, 0x528, 'i', n)
    put(dtab, 0x730, 'HxxI', ids[k] if r.random() < 0.3 else ids[k] ^ r.randrange(1, 0x10000), ITEMS + ITEM * k)
    d = ITEM * k
    for off in (0xf4, 0xf8, 0xfc, 0x3c, 0x104, 0x40, 0xf0, 0x110, 0x48):
        put(items, d + off, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    for off in (0xc4, 0x1a8, 0x198, 0x1a0):
        put(items, d + off, 'i', r.choice([0, -1, 1, 0x100, 0x7f, r.getrandbits(32) - (1 << 31), r.getrandbits(32) & 0x7fffffff]))
    value = struct.unpack_from('<B', items, d + 0x1a0)[0]
    put(items, d + 0x12c, 'i', r.choice([value, value + 1, value - 1, -1, 0x100 + value, r.getrandbits(32) - (1 << 31)]))
    put(items, d + 0x130, 'i', r.choice([value, value + 1, value - 1, -1, 0x100 + value, r.getrandbits(32) - (1 << 31)]))
    for off in (0x4b0, 0x494, 0x4a0):
        put(items, d + off, 'ii', small(r), small(r))
    put(items, d + 0x14, 'I', TEXT)
    regions.update({DEFS: dtab, ITEMS: items, TEXT: bytes(r.choice(b'abcXYZ') for _ in range(r.randrange(0, 0x28))) + bytes(0x30),
                    TIME: r.bytes(8), CONTROL: put(bytearray(r.bytes(0x20)), 0x1c, 'I', r.choice([0, 0, 1, r.getrandbits(32)])), OBJECT: r.bytes(0x450)})
    regions[0x80239c] = struct.pack('<I', WORLD)
    regions[0x8023a0] = r.bytes(4)
    regions[0x7a4610] = r.bytes(4)
    regions[WORLD + 0x1da70c] = r.bytes(4)
    set_return(log, CONSTRUCT, r.getrandbits(32))
    set_return(log, HASKEY, r.choice([0, 0, 1, 0x100, r.getrandbits(32) & 0xffffff00, r.getrandbits(32)]))
    set_return(log, MONEY, RAM + 0x5000)
    set_return(log, ANALYSER, RAM + 0x5100)
    set_return(log, FINDRIDE, RIDE)
    set_return(log, NOW, TIME)
    set_return(log, CONTROLS, CONTROL)
    strings(log, NOW, 0, out=1)
    blob_bytes(log, POST, 0, 0x3ff)
    struct.pack_into('<I', log, LOG_BYTES + 4 * HASKEY, 1 << 16)
    struct.pack_into('<I', log, LOG_BYTES + 4 * UNLOCK, 1 << 16)
    struct.pack_into('<I', log, LOG_BYTES + 4 * CAPACITY, 1)
    angle = r.choice([0, 0x5a, 0xb4, 0x10e, 0, 0x5a, 0xb4, 0x10e, 0x2d, 360, -90, r.getrandbits(32)])
    return Call(0x4db090, [OBJECT, r.getrandbits(32), (r.getrandbits(16) << 16) | ids[k], angle & 0xffffffff, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
