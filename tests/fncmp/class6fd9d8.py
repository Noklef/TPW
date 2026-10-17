# The class 0x6fd9d8 destructors of src/game/core/class6fd9d8 against the originals: the destructor itself (0x41c8b0: an object with a member of its own deleted through slot 3, a list of
# up to three nodes, some without a held object, one with a bad count, whose objects are deleted through slot 11, and two game strings; every virtual call is a recorder), the derived destructor stubs (0x41e2c0 .. 0x41e5c0) and their scalar deleting destructors
# (0x41d880 .. 0x41e250), and the complete destructors (0x41e270, 0x41e2d0, 0x41e330, 0x41e470, 0x41e4f0, 0x41e560). The objects are all of that shape; the handles at +0x40 / +0x48 hold nothing (the release of a held object goes through an import the emulator does not give); the wrappers get flags either
# way (the free needs the heap region).
# Class6fda98's destructors (0x41e3a0, 0x41dc40: a second list at +0x4c, held objects deleted through slot 0) and the base's Find (0x41d710: the current object's slot 2, then
# the list's objects after it through slot 1, their results written through the first argument; none found deletes the member at +0x3c).
# The constructors (0x41d7c0 .. 0x41e190): a name and a folder (wide strings, now and then empty, the folder ending in a backslash or not), the global backslash String at
# 0xf80be0, and a heap whose list allocation (or an earlier one) now and then fails.
# usage: class6fd9d8.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import gamestring  # noqa: E402
from fncmp import Bench, Call, HEAP, LOG, RAM, echo, fill, heap, log_region, put, strings, vtable  # noqa: E402

# name: (address, flags argument?, kind: s a stub or wrapper, h a complete destructor with the handle at +0x40 / +0x48, g with the handle at +0x2c)
CALLS = {'Base': (0x41c8b0, 0, 'b'), 'S48': (0x41e2c0, 0, 's'), 'S68': (0x41e320, 0, 's'), 'Sa8': (0x41e4d0, 0, 's'), 'Sb8': (0x41e4e0, 0, 's'), 'Se8': (0x41e5b0, 0, 's'), 'Sf8': (0x41e5c0, 0, 's'),
         'W48': (0x41d880, 1, 's'), 'W68': (0x41da50, 1, 's'), 'Wa8': (0x41ddc0, 1, 's'), 'Wb8': (0x41dea0, 1, 's'), 'We8': (0x41e170, 1, 's'), 'Wf8': (0x41e250, 1, 's'),
         'A': (0x41e270, 0, 'h'), 'B': (0x41e2d0, 0, 'h'), 'C': (0x41e560, 0, 'h'), 'D88': (0x41e330, 0, 'h'), 'Dc8': (0x41e4f0, 0, 'h'), 'Str': (0x41e470, 0, 'g'),
         'WB': (0x41d970, 1, 'h'), 'WD88': (0x41db50, 1, 'h'), 'WStr': (0x41dc60, 1, 'g'), 'WDc8': (0x41dfa0, 1, 'h'), 'WC': (0x41e090, 1, 'h'),
         'S98': (0x41e3a0, 0, 'l'), 'W98': (0x41dc40, 1, 'l'), 'Find': (0x41d710, 0, 'f')}
CTORS = {'C48': 0x41d7c0, 'C58': 0x41d8a0, 'C68': 0x41d990, 'C88': 0x41da70, 'C98': 0x41db70, 'Ca8': 0x41dd00, 'Cb8': 0x41dde0, 'Cc8': 0x41dec0, 'Cd8': 0x41dfc0,
         'Ce8': 0x41e0b0, 'Cf8': 0x41e190}
RANGES = [(0x41c8b0, 0x41c990), (0x41e3a0, 0x41e470), (0x41d710, 0x41d7c0), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000), (0x5f61e0, 0x5f6210), (0x5f82a0, 0x5f8310), (0x5f8420, 0x5f8460), (0x619800, 0x619870)]
RANGES += [(a, a + 0x80) for (a, _f, _k) in CALLS.values()]
CTOR_ENDS = [0x41d880, 0x41d970, 0x41da50, 0x41db50, 0x41dc40, 0x41ddc0, 0x41dea0, 0x41dfa0, 0x41e090, 0x41e170, 0x41e250]
RANGES += list(zip(CTORS.values(), CTOR_ENDS)) + [(0x5c5480, 0x5c5510), (0x424cf0, 0x424d30), (0x5f8240, 0x5f8250)]
B = Bench(RANGES)
OBJ, LST, NODES, OBJS, VT, VTL, VTM, MEMBER = RAM, RAM + 0x1000, RAM + 0x1100, RAM + 0x2000, RAM + 0x3000, RAM + 0x3100, RAM + 0x3200, RAM + 0x3300
LST2, NODES2 = RAM + 0x1080, RAM + 0x1180   # Class6fda98's list at +0x4c and its nodes


def objects(r, nulls=0.2):
    obj = bytearray(r.bytes(0x100))
    regions = {OBJ: obj, LOG: log_region(), HEAP: heap()}
    n = r.choice([0, 1, 2, 3])
    lst = bytearray(r.bytes(0x20))
    put(lst, 0, 'I', VTL)
    put(lst, 4, 'II', NODES if n else 0, NODES + 0x10 * (n - 1) if n else 0)
    put(lst, 0xc, 'I', n)
    nodes = bytearray(r.bytes(0x60))
    for k in range(n):
        held = OBJS + 0x40 * k
        put(nodes, 0x10 * k, 'III', held if r.random() >= nulls else 0, NODES + 0x10 * (k + 1) if k + 1 < n else 0, NODES + 0x10 * (k - 1) if k else 0)
        regions[held] = struct.pack('<I', VT) + r.bytes(0x20)
    regions.update({LST: lst, NODES: nodes, VT: vtable({1: 3, 2: 2, 11: 1}), VTL: vtable({0: 1}), VTM: vtable({3: 1})})
    put(obj, 0x2c, 'I', LST if r.random() < 0.9 else 0)
    put(obj, 0x3c, 'I', MEMBER if r.random() < 0.8 else 0)
    regions[MEMBER] = struct.pack('<I', VTM) + r.bytes(0x20)
    wide = r.choice([0, 1])
    for k, off in enumerate((4, 0x18)):
        gamestring.string(r, wide, RAM + 0x4000 + 0x40 * k, RAM + 0x6000 + 0x80 * k, regions)
        obj[off:off + 0x14] = regions.pop(RAM + 0x4000 + 0x40 * k)[:0x14]
    return Call(0x41c8b0, [OBJ], regions, conv='t', ret='v')


SLASH = 0xf80be0     # the global backslash String
LOCALE = 0xfb1440    # the CRT's LC_CTYPE handle: 0, the C locale (_stricmp's)


def construct(r, address):
    """A constructor (name, folder) on an object of random bytes."""
    regions = {OBJ: r.bytes(0x60), HEAP: heap(0x4000, r.choice(range(1, 8)) if r.random() < 0.3 else 0), LOCALE: bytes(4)}
    args = []
    for k in range(2):
        n = r.choice([0, 1, 3, 6, 9])
        text = struct.pack('<%dH' % n, *[r.choice(b'aB.\\_') for _ in range(n)])
        if k and n and r.random() < 0.5:
            text = text[:-2] + struct.pack('<H', 0x5c)
        gamestring.string(r, 1, RAM + 0x4000 + 0x40 * k, RAM + 0x6000 + 0x80 * k, regions, text)
        args.append(RAM + 0x4000 + 0x40 * k)
    gamestring.string(r, 0, SLASH, RAM + 0x6200, regions, b'\\' if r.random() < 0.9 else r.choice([b'', b'/']))
    return Call(address, [OBJ] + args, regions, conv='t', ret='a')


def case(r):
    name = OPS[r.deal(range(len(OPS)))]
    if name in CTORS:
        return construct(r, CTORS[name])
    address, flags, kind = CALLS[name]
    c = objects(r, 0.05 if kind == 'f' else 0.2)
    c.address, c.ret = address, 'a' if flags else 'v'
    c.regions.setdefault(HEAP, heap())
    obj = c.regions[OBJ]
    if kind == 'l':   # Class6fda98: the handle at +0x40 holds nothing; a second list at +0x4c (objects deleted through slot 0)
        put(obj, 0x44, 'I', 0)
        n = r.choice([0, 1, 2, 3])
        lst = bytearray(r.bytes(0x20))
        put(lst, 0, 'IIII', VTL, NODES2 if n else 0, NODES2 + 0x10 * (n - 1) if n else 0, n if r.random() < 0.9 else max(n - 1, 0))
        nodes = bytearray(r.bytes(0x40))
        for k in range(n):
            held = OBJS + 0x100 + 0x40 * k
            put(nodes, 0x10 * k, 'III', held if r.random() >= 0.2 else 0, NODES2 + 0x10 * (k + 1) if k + 1 < n else 0, NODES2 + 0x10 * (k - 1) if k else 0)
            c.regions[held] = struct.pack('<I', VTL) + r.bytes(0x20)
        c.regions[LST2], c.regions[NODES2] = lst, nodes
        put(obj, 0x4c, 'I', LST2 if r.random() < 0.9 else 0)
    elif kind == 'f':   # Find (key): the current object (+0x34) one of the list's, none, or now and then one not in it; the opens' results written through their first argument
        log = c.regions[LOG]
        for k in (1, 2):
            strings(log, k, 0, 1)
            fill(log, k, 0, struct.pack('<i', r.choice([0, 1, -1, -0x80000000])), k - 1)
            echo(log, k, 0)
        lst = c.regions[LST]
        n = struct.unpack_from('<I', lst, 0xc)[0]
        held = [struct.unpack_from('<I', c.regions[NODES], 0x10 * k)[0] for k in range(n)]
        put(obj, 0x34, 'I', r.choice(held + held + [0]) if held and r.random() < 0.95 else (0 if r.random() < 0.5 else OBJS + 0x100))
        c.regions[OBJS + 0x100] = struct.pack('<I', VT) + r.bytes(0x20)
        put(obj, 0x2c, 'I', LST)
        put(obj, 0x3c, 'I', MEMBER if r.random() < 0.95 else 0)
        c.args.append(r.choice([0, 1, 0x100, r.getrandbits(32)]))
        c.ret = 'a'
    if kind == 'g':
        put(obj, 0x30, 'I', 0)  # (the handle's object)
    elif kind == 'h':
        put(obj, 0x44, 'I', 0)
        put(obj, 0x4c, 'I', 0)
    if flags:
        c.args.append(r.choice([0, 1, 2, 3, 0xffffff00, 0xffffffff]))
    return c


OPS = [k for k in os.environ.get('OPS', ','.join(list(CALLS) + list(CTORS))).split(',') if k]

if __name__ == '__main__':
    B.main(case)
