# The slot table functions and the counted list erase of src/game/core/small11 against the originals: slot selections around the limits,
# matching and mismatching ids, entries naming object definitions or control records, nodes with counted buffers.
# usage: small11.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Add,Get,Erase)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from thingenv import park  # noqa: E402

CALLS = {'Add': (0x4d27a0, 0xa0), 'Get': (0x503a20, 0xa0), 'Erase': (0x4f3400, 0xa0)}
B = Bench([(a, a + n) for (a, n) in CALLS.values()] + [(0x5da3c0, 0x5da3c1), (0x412e90, 0x412eea), (0x4d3d80, 0x4d3df0)])
LIST, BUFS, TABLE = RAM + 0x1000, RAM + 0x2000, RAM + 0x20000


def case(r):
    name = r.choice(OPS)
    address, _n = CALLS[name]
    obj = bytearray(r.bytes(0x1500))
    regions = {RAM: obj, HEAP: heap()}
    if name == 'Add':
        put(obj, 0x3cc, 'i', r.choice([-1, 0, 1, 5, 19, 20, 21, 0x7fffffff]))
        obj[0x3d9] = r.choice([0, 1, 1, 7])
        put(obj, 0x3e0, 'H', r.choice([0, 0, 5, 0x1234]))
        put(obj, 0x3dc, 'I', r.getrandbits(32))
        ident, kind, extra = r.choice([1, 5, 0x1234, 0xffffffff]), r.choice([0, 5, 0x1234, 0x11234]), r.choice([0, 5, 0x1234, 0x21234])
        slot = r.randrange(20)
        for n in range(20):
            put(obj, 0xc + 48 * n, 'I', ident if r.random() < 0.7 else r.getrandbits(32))
            put(obj, 0xc + 48 * n + 0xc, 'H', kind & 0xffff if r.random() < 0.7 else r.getrandbits(16))
        return Call(address, [RAM, ident, r.choice([0, 1, 100, 0xffffffff]), kind, extra], regions, conv='t', ret='v')
    if name == 'Get':
        for k in range(5):
            put(obj, 0x1414 + 16 * k, 'i', r.choice([-1, 0, 1, 2, 3]))
        dtab, _h, _a = None, None, None
        defs, _h, _a = park(r, count=0, defs=True)
        regions.update({k: v for k, v in defs.items() if k in (0x7890a0, 0x30008000)})
        items = defs[0x30008000]
        for n in range(4):  # the entries: word +0xc names a definition (slot 4) or a control record, byte +0x15 a part
            o = 0xc + 20 * n
            put(obj, o, 'H', struct.unpack_from('<H', items, 0x500 * r.randrange(16))[0] if r.random() < 0.7 else r.choice([1, 2, 3, 9]))
            obj[20 * n + 0x15] = r.choice([0, 1, 2, 5, 7])
        table = bytearray(r.bytes(0x1400))
        have = r.choice([0, 1, 3, 6])
        put(table, 0x12c0, 'i', have)
        for k in range(have):
            put(table, 0x20 * k, 'H', r.choice([1, 2, 3, 9, struct.unpack_from('<H', obj, 0xc)[0]]))
        put(table, 0x12c4, 'H', r.choice([0, 1, 2, 3, 9]))
        put(table, 0x12c8, 'I', r.choice([0, TABLE + 0x20 * r.randrange(6)]))
        regions[TABLE] = table
        regions[0x80239c] = struct.pack('<I', TABLE - 0x1d943c)
        return Call(address, [RAM, r.choice([0, 1, 2, 3, 4, 4, 4])], regions, conv='t', ret='a')
    n = r.choice([1, 2, 3, 4])
    nodes = [LIST + 0x20 * (i + 1) for i in range(n)]
    order = [LIST] + nodes
    data = bytearray(r.bytes(0x20 * (n + 1)))
    bufs = bytearray(r.bytes(0x100))
    for i, a in enumerate(order):
        put(data, a - LIST, 'II', order[(i + 1) % len(order)], order[i - 1])
        use = r.choice([0, 1, 2, 5, 0xfe, 0xff])
        bufs[0x20 * i + 2] = use
        put(data, a - LIST + 0xc, 'I', r.choice([0, BUFS + 0x20 * i + 3, BUFS + 0x20 * i + 3]))
    put(obj, 8, 'I', r.choice([n, n + 1, 0, 100]))
    regions.update({LIST: data, BUFS: bufs})
    return Call(address, [RAM, RAM + 0x300, r.choice(order), r.choice(order)], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
