# The global save info's byte accessors (src/game/core/globalsave: 0x5af7f0-0x5b0cc0) against
# the originals: indices across the arrays, values wider than a byte, with and without a
# current theme. The sign names' setters (0x5b0940, 0x5b0970): names of 0 to 0x24 units with a NUL or not. Construct and Instance
# (0x5aef50, 0x409410): the trees' shared nils (0xf7c970, 0xf7c968) made or not, their user counts any, the instance (0xf7c58c) made or
# not, the heap failing now and then (the instance's block: it stays 0; the list's node, which is not checked: a crash in both). The
# constructor writes uninitialised stack bytes to +0, +0xc, +0x28, +0x29, +0x38 and +0x39 (the lists' and trees' allocators and the trees'
# comparators, empty classes): masked; neither function reads them.
# The set-byte counts (0x5af4a0, 0x5af530, 0x5af5d0, 0x5af680): the info's bytes any (mostly 0 or 1), its theme map a valid red-black
# tree (stdtree.py's build) of 0-6 nodes, their records' six bytes likewise and their global.sam loaded already (+0x1c set), or one
# not and the heap failing its block (ThemeRecord_LoadGlobalSam: 0, nothing read). SamField (0x5b11a0, src/game/core/balancefile): a
# theme record's global.sam loaded already (+0x1c set: its dword +0x14 returned) or not and the heap failing (-1).
# The marks (0x5af810, 0x5afb00, 0x5af940): count()'s info and map, every record's global.sam loaded (see mark()). The cheat keys'
# marks (src/game/core/keyactions: KeyTheme 0x40c6c0-0x40c760, MarkTheme 0-5; KeyScore 0x40c780-0x40c7e0, MarkScore 0-3 of 100): mark()'s
# world, the instance (0xf7c58c) its info.
# usage: globalsave.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=ThemeByte,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
import struct

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, Bench, Call, Mask, RAM, heap, put  # noqa: E402
import stdtree  # noqa: E402


def lock(e):  # the STL lock (0x691de0, thiscall, returns its argument; 0x691ea0 releases): the port leaves them out
    e.r['EAX'] = e.r['ECX']


B = Bench([(0x5af7f0, 0x5af80b), (0x5afc30, 0x5afc5a), (0x5b09a0, 0x5b09b4), (0x5b0ca0, 0x5b0cc5), (0x5b0940, 0x5b0996), (0x67b0d0, 0x67b10f),
           (0x5aef50, 0x5af0dc), (0x409410, 0x40947a), (0x5b3110, 0x5b3130), (0x5b2fd0, 0x5b2ff0), (0x5b1680, 0x5b16a6),
           (0x5af4a0, 0x5af52a), (0x5af530, 0x5af5cf), (0x5af5d0, 0x5af674), (0x5af680, 0x5af734), (0x5b11c0, 0x5b12d3), (0x5b2480, 0x5b2530), (0x5b11a0, 0x5b11b9),
           (0x5af810, 0x5af940), (0x5afb00, 0x5afc30), (0x5af940, 0x5afb00), (0x40c6c0, 0x40c7f6)],
          hooks={0x691de0: lock, 0x691ea0: lambda e: None})
INFO, THEME = RAM + 0x10, RAM + 0x100
CALLS = {'ThemeByte': (0x5af7f0, 'i', 'b'), 'Byte26': (0x5af800, 'i', 'b'), 'Inc20': (0x5afc30, '', 'v'),
         'Byte24': (0x5afc40, '', 'b'), 'SetByte24': (0x5afc50, 'v', 'v'), 'Byte25': (0x5b09b0, '', 'b'),
         'SetByte25': (0x5b09a0, 'v', 'v'), 'AllResearch': (0x5b0ca0, '', 'v'), 'Byte4c': (0x5b0cb0, '', 'b'),
         'ClearByte4c': (0x5b0cc0, '', 'v'), 'SignNameA': (0x5b0940, 'w', 'v'), 'SignNameB': (0x5b0970, 'w', 'v')}
NAME = RAM + 0x200
NILS = RAM + 0x400
COUNTS = {'Count4': 0x5af4a0, 'Count': 0x5af530, 'CountLeft': 0x5af5d0, 'CountLevel': 0x5af680}
HEAD, NIL, NODES, RECORDS = RAM + 0x800, RAM + 0x900, RAM + 0x1000, RAM + 0x2000
GARBAGE = [0, 0xc, 0x28, 0x29, 0x38, 0x39]
MARKS = {'MarkTheme': 0x5af810, 'MarkExtra': 0x5afb00, 'MarkScore': 0x5af940}
KEYS = {'KeyTheme': [0x40c6c0 + 0x20 * i for i in range(6)], 'KeyScore': [0x40c780 + 0x20 * i for i in range(4)]}
SAMS = RAM + 0x3000


def construct(r, op):
    regions = {HEAP: heap(fail=r.choice([0] * 8 + [1, 2, 3, 4]))}
    for nil, users, k in ((0xf7c970, 0xf7c96c, 0), (0xf7c968, 0xf7c964, 1)):
        regions[nil] = struct.pack('<I', r.choice([0, 0, NILS + 0x40 * k]))
        regions[users] = struct.pack('<I', r.choice([0, 1, 5, 0xffffffff, r.getrandbits(32)]))
    regions[NILS] = r.bytes(0x80)
    if op == 'Construct':
        regions[INFO] = r.bytes(0x50)
        mask = Mask(INFO, GARBAGE, 'uninitialised stack bytes (the allocators and comparators) written by 0x5aef50; nothing reads them')
        return Call(0x5aef50, [INFO], regions, conv='t', ret='a', masks=[mask])
    regions[0xf7c58c] = struct.pack('<I', r.choice([0, 0, 0, INFO]))
    mask = Mask(HEAP, [0x10 + k for k in GARBAGE], 'uninitialised stack bytes (the allocators and comparators) written by 0x5aef50; nothing reads them')
    return Call(0x409410, [], regions, ret='a', masks=[mask])


def flags(r, n):
    return bytes(r.choice([0, 0, 1, 1, r.getrandbits(8)]) for _ in range(n))


def count(r, op):
    n = r.choice([0, 1, 2, 3, 4, 6])
    nodes, root = stdtree.build(r, n) if n else ({}, None)
    keys = sorted(nodes)
    addr = {k: NODES + 0x40 * i for i, k in enumerate(keys)}
    blob, recs = bytearray(r.bytes(0x40 * n)), bytearray(r.bytes(0x40 * n))
    unloaded = r.randrange(n) if n and r.random() < 0.2 else -1
    for i, k in enumerate(keys):
        left, parent, right, colour = nodes[k]
        put(blob, 0x40 * i, 'III', NIL if left is None else addr[left], HEAD if parent is None else addr[parent],
            NIL if right is None else addr[right])
        put(blob, 0x40 * i + 0x20, 'II', RECORDS + 0x40 * i, colour)
        recs[0x40 * i:0x40 * i + 6] = flags(r, 6)
        put(recs, 0x40 * i + 0x1c, 'I', 0 if i == unloaded else r.getrandbits(32) | 1)
    head = struct.pack('<III', addr[keys[0]], addr[root], addr[keys[-1]]) if n else struct.pack('<III', HEAD, NIL, HEAD)
    info = bytearray(r.bytes(0x60))
    info[0x18:0x1c] = flags(r, 4)
    info[0x26:0x28] = flags(r, 2)
    put(info, 0x1c, 'I', r.choice([0, 1, 5, r.getrandbits(32)]))
    put(info, 0x20, 'I', r.choice([0, 1, 5, r.getrandbits(32)]))
    put(info, 0x3c, 'I', HEAD)
    nil = put(bytearray(0x40), 0x24, 'I', stdtree.BLACK)
    regions = {INFO: info, HEAD: head + bytes(0x30), NIL: nil, 0xf7c968: struct.pack('<I', NIL), HEAP: heap(fail=1 if unloaded >= 0 else 0)}
    if n:
        regions[NODES], regions[RECORDS] = bytes(blob), bytes(recs)
    return Call(COUNTS[op], [INFO], regions, conv='t', ret='a')


def mark(r, op):
    """0x5af810 / 0x5afb00 (thiscall (i)) and 0x5af940 (thiscall (i, score)): count()'s info and theme map, every record's global.sam
    loaded (+0x1c a file whose +0x14 is a level about the info's, or any), the records' bytes +6..+9 0 or 1 and their scores small or
    any; the current theme (+0x48) one of the records or another; i mostly in range; the score about the records' or any."""
    c = count(r, 'Count')
    regions = dict(c.regions)
    regions[HEAP] = heap()
    n = len(regions.get(RECORDS, b'')) // 0x40
    recs = bytearray(regions.get(RECORDS, b''))
    sams = bytearray(r.bytes(0x20 * max(n, 1)))
    for k in range(n):
        put(recs, 0x40 * k + 0x1c, 'I', SAMS + 0x20 * k)
        recs[0x40 * k + 6:0x40 * k + 10] = flags(r, 4)
        put(recs, 0x40 * k + 0xc, '4i', *(r.choice([0, 1, 5, 9, -1, r.getrandbits(31)]) for _ in range(4)))
        put(sams, 0x20 * k + 0x14, 'i', r.choice([0, 1, 2, 3, 5, -1, r.getrandbits(31)]))
    if n:
        regions[RECORDS], regions[SAMS] = bytes(recs), bytes(sams)
    info = bytearray(regions[INFO])
    put(info, 0x48, 'I', RECORDS + 0x40 * r.randrange(n) if n and r.random() < 0.6 else THEME)
    put(info, 0x20, 'i', r.choice([0, 0, 1, -1, -5, r.getrandbits(31)]))
    regions[INFO] = bytes(info)
    theme = bytearray(r.bytes(0x40))
    theme[0:10] = flags(r, 10)
    put(theme, 0xc, '4i', *(r.choice([0, 1, 5, 9, -1, r.getrandbits(31)]) for _ in range(4)))
    regions[THEME] = bytes(theme)
    i = r.choice([0, 1, 2, 3, 1]) if op == 'MarkScore' else r.choice([0, 1, 1, 2, 5]) if op == 'MarkTheme' else r.choice([0, 1, 1])
    args = [INFO, i]
    if op == 'MarkScore':
        args.append(r.choice([0, 1, 5, 9, 10, -1, r.getrandbits(32)]))
    return Call(MARKS[op], args, regions, conv='t', ret='a')


def case(r):
    op = r.choice(OPS)
    if op in COUNTS:
        return count(r, op)
    if op in MARKS:
        return mark(r, op)
    if op in KEYS:
        regions = dict(mark(r, 'MarkTheme' if op == 'KeyTheme' else 'MarkScore').regions)
        regions[0xf7c58c] = struct.pack('<I', INFO)
        return Call(r.choice(KEYS[op]), [], regions, ret='a')
    if op == 'SamField':
        rec = bytearray(r.bytes(0x40))
        loaded = r.random() < 0.7
        put(rec, 0x1c, 'I', RECORDS if loaded else 0)
        return Call(0x5b11a0, [INFO], {INFO: rec, RECORDS: r.bytes(0x40), HEAP: heap(fail=0 if loaded else 1)}, conv='t', ret='a')
    if op in ('Construct', 'Instance'):
        return construct(r, op)
    fn, arg, ret = CALLS[op]
    info = bytearray(r.bytes(0x60))
    put(info, 0x48, 'I', r.choice([THEME, THEME, 0]))
    put(info, 0x20, 'I', r.choice([0, 0xffffffff, 0xff, r.getrandbits(32)]))
    args = [INFO]
    if arg == 'i':
        args.append(r.choice([0, 1, 0x20, r.randrange(0xc0)]))
    elif arg == 'w':   # a name of 0-0x24 units (none 0), its NUL or not (the units after it any)
        n = r.choice([0, 1, 0x1f, 0x20, 0x21, 0x22, r.randrange(0x25)])
        name = bytearray(r.bytes(0x60))
        for i in range(n):
            put(name, 2 * i, 'H', r.randrange(1, 0x10000))
        if r.random() < 0.85:
            put(name, 2 * n, 'H', 0)
        args.append(NAME)
        return Call(fn, args, {RAM: r.bytes(0x10), INFO: info, THEME: r.bytes(0xc0), NAME: name}, conv='t', ret=ret)
    elif arg == 'v':
        args.append(r.choice([0, 1, 0x100, 0x1ff, r.getrandbits(32)]))
    return Call(fn, args, {RAM: r.bytes(0x10), INFO: info, THEME: r.bytes(0xc0)}, conv='t', ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(list(CALLS) + ['Construct', 'Instance'] + list(COUNTS) + ['SamField'] + list(MARKS) + list(KEYS) * 4)).split(',') if k]   # (the keys: ten functions)


if __name__ == '__main__':
    B.main(case)
