# The class 0x6fd9d8 destructors of src/game/core/class6fd9d8 against the originals: the destructor itself (0x41c8b0: an object with a member of its own deleted through slot 3, a list of
# up to three nodes, some without a held object, one with a bad count, whose objects are deleted through slot 11, and two game strings; every virtual call is a recorder), the derived destructor stubs (0x41e2c0 .. 0x41e5c0) and their scalar deleting destructors
# (0x41d880 .. 0x41e250), and the complete destructors (0x41e270, 0x41e2d0, 0x41e330, 0x41e470, 0x41e4f0, 0x41e560). The objects are all of that shape; the handles at +0x40 / +0x48 hold nothing (the release of a held object goes through an import the emulator does not give); the wrappers get flags either
# way (the free needs the heap region).
# usage: class6fd9d8.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import gamestring  # noqa: E402
from fncmp import Bench, Call, HEAP, LOG, RAM, heap, log_region, put, vtable  # noqa: E402

# name: (address, flags argument?, kind: s a stub or wrapper, h a complete destructor with the handle at +0x40 / +0x48, g with the handle at +0x2c)
CALLS = {'Base': (0x41c8b0, 0, 'b'), 'S48': (0x41e2c0, 0, 's'), 'S68': (0x41e320, 0, 's'), 'Sa8': (0x41e4d0, 0, 's'), 'Sb8': (0x41e4e0, 0, 's'), 'Se8': (0x41e5b0, 0, 's'), 'Sf8': (0x41e5c0, 0, 's'),
         'W48': (0x41d880, 1, 's'), 'W68': (0x41da50, 1, 's'), 'Wa8': (0x41ddc0, 1, 's'), 'Wb8': (0x41dea0, 1, 's'), 'We8': (0x41e170, 1, 's'), 'Wf8': (0x41e250, 1, 's'),
         'A': (0x41e270, 0, 'h'), 'B': (0x41e2d0, 0, 'h'), 'C': (0x41e560, 0, 'h'), 'D88': (0x41e330, 0, 'h'), 'Dc8': (0x41e4f0, 0, 'h'), 'Str': (0x41e470, 0, 'g'),
         'WB': (0x41d970, 1, 'h'), 'WD88': (0x41db50, 1, 'h'), 'WStr': (0x41dc60, 1, 'g'), 'WDc8': (0x41dfa0, 1, 'h'), 'WC': (0x41e090, 1, 'h')}
RANGES = [(0x41c8b0, 0x41c990), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000), (0x5f61e0, 0x5f6210), (0x5f82a0, 0x5f8310), (0x5f8420, 0x5f8460), (0x619800, 0x619870)]
RANGES += [(a, a + 0x80) for (a, _f, _k) in CALLS.values()]
B = Bench(RANGES)
OBJ, LST, NODES, OBJS, VT, VTL, VTM, MEMBER = RAM, RAM + 0x1000, RAM + 0x1100, RAM + 0x2000, RAM + 0x3000, RAM + 0x3100, RAM + 0x3200, RAM + 0x3300


def objects(r):
    obj = bytearray(r.bytes(0x100))
    regions = {OBJ: obj, LOG: log_region(), HEAP: heap()}
    n = r.choice([0, 1, 2, 3])
    nulls = 0.2
    lst = bytearray(r.bytes(0x20))
    put(lst, 0, 'I', VTL)
    put(lst, 4, 'II', NODES if n else 0, NODES + 0x10 * (n - 1) if n else 0)
    put(lst, 0xc, 'I', n)
    nodes = bytearray(r.bytes(0x60))
    for k in range(n):
        held = OBJS + 0x40 * k
        put(nodes, 0x10 * k, 'III', held if r.random() >= nulls else 0, NODES + 0x10 * (k + 1) if k + 1 < n else 0, NODES + 0x10 * (k - 1) if k else 0)
        regions[held] = struct.pack('<I', VT) + r.bytes(0x20)
    regions.update({LST: lst, NODES: nodes, VT: vtable({11: 1}), VTL: vtable({0: 1}), VTM: vtable({3: 1})})
    put(obj, 0x2c, 'I', LST if r.random() < 0.9 else 0)
    put(obj, 0x3c, 'I', MEMBER if r.random() < 0.8 else 0)
    regions[MEMBER] = struct.pack('<I', VTM) + r.bytes(0x20)
    wide = r.choice([0, 1])
    for k, off in enumerate((4, 0x18)):
        gamestring.string(r, wide, RAM + 0x4000 + 0x40 * k, RAM + 0x6000 + 0x80 * k, regions)
        obj[off:off + 0x14] = regions.pop(RAM + 0x4000 + 0x40 * k)[:0x14]
    return Call(0x41c8b0, [OBJ], regions, conv='t', ret='v')


def case(r):
    name = r.choice(OPS)
    address, flags, kind = CALLS[name]
    c = objects(r)
    c.address, c.ret = address, 'a' if flags else 'v'
    c.regions.setdefault(HEAP, heap())
    obj = c.regions[OBJ]
    if kind == 'g':
        put(obj, 0x30, 'I', 0)  # (the handle's object)
    elif kind == 'h':
        put(obj, 0x44, 'I', 0)
        put(obj, 0x4c, 'I', 0)
    if flags:
        c.args.append(r.choice([0, 1, 2, 3, 0xffffff00, 0xffffffff]))
    return c


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
