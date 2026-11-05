# The IME taken to a window and off it (src/game/core/imeinput: Ime_ResetIfClosed 0x479380, Ime_Detach 0x47a570, Ime_Attach 0x47a3a0) against the originals: valid map trees (stdtree.py's) real, the map made or not yet (its two unset bytes left out), the lists and strings real, the imports recorders, atexit recorder 1.
# usage: imeattach.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob, heap, import_table, log_region, put, set_return  # noqa: E402
import stdtree  # noqa: E402

ATEXIT = 1
GETCTX, RELEASE, NOTIFY, OPEN, SETOPEN, FORM, SETLONG, CREATE, LASTERROR = range(40, 49)
IMPORTS = {'ImmGetContext': (GETCTX, 0x6fd084, 1, 0, 0), 'ImmReleaseContext': (RELEASE, 0x6fd080, 2, 0, 0), 'ImmNotifyIME': (NOTIFY, 0x6fd068, 4, 0, 0), 'ImmGetOpenStatus': (OPEN, 0x6fd06c, 1, 0, 0), 'ImmSetOpenStatus': (SETOPEN, 0x6fd070, 2, 0, 0),
           'ImmSetCompositionWindow': (FORM, 0x6fd074, 2, 0, 0), 'SetWindowLongA': (SETLONG, 0x6fd388, 3, 0, 0), 'ImmCreateContext': (CREATE, 0x6fd090, 0, 0, 0), 'GetLastError': (LASTERROR, 0x6fd138, 0, 0, 0)}
B = Bench([(0x479150, 0x4796b0), (0x47a3a0, 0x47a564), (0x47a570, 0x47a6c8), (0x6b5570, 0x6b55be), (0x5fa650, 0x5fa680), (0x47ae40, 0x47dd50), (0x45b380, 0x45b9a0), (0x405000, 0x406000), (0x4f23f0, 0x4f2450), (0x4f6080, 0x4f60a0), (0x67a400, 0x690000), (0x6918f0, 0x691cd0)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None, 0x67a7d0: _recorder(ATEXIT, 1, cdecl=True)})
TREE, HEAD, NIL, NODES = 0x7b4a40, stdtree.HEAD, stdtree.NIL, stdtree.NODES
IME, OUT, ENTRIES, STRINGS, BLOCKS = RAM + 0x1000, RAM + 0x1200, RAM + 0x1400, RAM + 0x1800, RAM + 0x2000
COLOUR = 0x18


def map_regions(r, regions, value=None):
    """The map's tree, made (guard bit set) with n nodes (their values' first dword `value` when given), or not yet; returns its keys."""
    guard = r.random() < 0.85
    regions[0x7b4a38] = bytes([r.getrandbits(8) & ~1 | guard]) + r.bytes(7)
    regions[0x7b4a50] = struct.pack('<I', 1 if guard else r.choice([0, 1]))
    regions[0x7b4a54] = struct.pack('<I', NIL if guard or r.random() < 0.5 else 0)
    nil = bytearray(0x40)
    struct.pack_into('<I', nil, COLOUR, stdtree.BLACK)
    regions[NIL] = nil
    regions[HEAD] = bytes(0x40)
    if not guard:
        regions[TREE] = r.bytes(0x10)
        return []
    n = r.choice([0, 1, 2, 3, 5, 8])
    nodes, root = stdtree.build(r, n)
    keys = sorted(nodes)
    addr = {k: NODES + 0x40 * i for i, k in enumerate(keys)}
    ptr = lambda k: NIL if k is None else addr[k]  # noqa: E731
    blob_ = bytearray(0x4000)
    for k in keys:
        base = addr[k] - NODES
        struct.pack_into('<III', blob_, base, ptr(nodes[k][0]), HEAD if nodes[k][1] is None else addr[nodes[k][1]], ptr(nodes[k][2]))
        struct.pack_into('<3I', blob_, base + 0xc, k, r.getrandbits(32) if value is None else value, r.getrandbits(32))
        struct.pack_into('<I', blob_, base + COLOUR, nodes[k][3])
    regions[HEAD] = struct.pack('<III', addr[keys[0]] if keys else HEAD, addr[root] if keys else NIL, addr[keys[-1]] if keys else HEAD) + bytes(0x34)
    regions[NODES] = blob_
    tree = bytearray(0x10)
    struct.pack_into('<I', tree, 4, HEAD)
    struct.pack_into('<I', tree, 0xc, n)
    regions[TREE] = tree
    return keys


def composition(r, ime, regions):
    """The string +8: null, its own or frozen; the text 0x78a374 to assign."""
    regions[0x78a374] = struct.pack('<%dH' % (t := r.choice([0, 0, 1, 3])), *[r.randrange(1, 0x10000) for _ in range(t)]) + bytes(2)
    how = r.choice(['null', 'own', 'shared', 'frozen', 'long'])
    if how == 'null':
        put(ime, 0xc, '3I', 0, 0, 0)
        return
    res = 63 if how == 'long' else 31
    put(ime, 0xc, '3I', BLOCKS + 2, 4, res)
    block = bytearray(r.bytes(0x100))
    block[1] = {'own': 0, 'long': 0, 'shared': 2, 'frozen': 0xff}[how]
    regions[BLOCKS] = block


def lists(r, ime, regions):
    """The lists +0x38: 0 to 3 entries, each with 0 to 2 strings of no block."""
    n = r.choice([0, 1, 2, 3])
    data = bytearray(r.bytes(0x24 * 4))
    strings = bytearray(r.bytes(0x100))
    at = STRINGS
    for i in range(n):
        k = r.choice([0, 1, 2])
        put(data, i * 0x24 + 0x14, '3I', at if k else 0, at + 0x10 * k if k else 0, at + 0x10 * k if k else 0)
        for j in range(k):
            put(strings, at - STRINGS + 0x10 * j + 4, '3I', 0, 0, 0)
        at += 0x20
    regions[ENTRIES] = data
    regions[STRINGS] = strings
    put(ime, 0x3c, '3I', ENTRIES if n else 0, ENTRIES + n * 0x24 if n else 0, ENTRIES + n * 0x24 if n else 0)


def case(r):
    log = log_region()
    imports = import_table(log, IMPORTS)
    set_return(log, GETCTX, r.choice([0, 0x1000, 0x1000, 0x1000]))
    set_return(log, OPEN, r.choice([0, 0, 1, 0x100]))
    set_return(log, SETLONG, r.choice([0, 0x5000, 0x5000, 0x5000]))
    set_return(log, CREATE, r.choice([0, 0x6000, 0x6000]))
    set_return(log, LASTERROR, r.choice([0, 0, 5, 0x80000000 | r.getrandbits(16)]))
    set_return(log, RELEASE, r.getrandbits(32))
    blob(log, FORM, 1, 7)
    ime = bytearray(r.bytes(0x170))
    regions = {LOG: log, IMPORT_TABLE: imports, HEAP: heap(0x2000, r.choice([0] * 14 + [1, 2])), OUT: r.bytes(8)}
    keys = map_regions(r, regions)
    composition(r, ime, regions)
    lists(r, ime, regions)
    window = r.choice(keys) if keys and r.random() < 0.5 else r.randrange(0, 100)
    which = r.random()
    # (a map not yet made: its allocator and compare bytes are unset locals of the original's)
    ignore = [(TREE, 2)]
    if which < 0.2:
        regions[IME] = ime
        return Call(0x479380, [IME, window], regions, conv='t', ret='v', ignore=ignore)
    if which < 0.55:
        put(ime, 0x4c, '2I', r.choice([0, window, window, window]), r.getrandbits(32))
        regions[IME] = ime
        return Call(0x47a570, [IME], regions, conv='t', ret='v', ignore=ignore)
    put(ime, 0x4c, 'I', r.choice([0, 0, 0, r.getrandbits(32)]))
    put(ime, 0x5c, 'I', r.choice([0, r.getrandbits(32)]))
    regions[IME] = ime
    return Call(0x47a3a0, [IME, OUT, window], regions, conv='t', ret='a', ignore=ignore)


if __name__ == '__main__':
    B.main(case)
