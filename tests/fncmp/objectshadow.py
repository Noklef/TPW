# An object's shadow kept up (src/game/core/vobj2: 0x57b8a0) against the original, Box_Extend and __ftol-free x87 real, the rest recorders, the transformer's slot 6 logging one of its matrices a case and filling the product: a block new or there, no sun or one overhead the wrong way, the box in any order, due or not, each failure after.
# usage: objectshadow.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fb, fill, fill_this, log_region, put, set_return, strings, vtable  # noqa: E402

MULTIPLY, TREE, RENDER, UNLINK, BEGIN, ALLOC, ATEXIT, MAKE = 6, 7, 8, 9, 10, 11, 12, 13
B = Bench([(0x57b8a0, 0x57be81), (0x57b410, 0x57b490)], hooks={0x57b610: _recorder(TREE, 4), 0x57b6e0: _recorder(RENDER, 1), 0x58dc50: _recorder(UNLINK, 1), 0x565240: _recorder(BEGIN, 5), 0x582640: _recorder(ALLOC, 1), 0x67a7d0: _recorder(ATEXIT, 1, cdecl=True), 0x57b490: _recorder(MAKE, 0)})
SELF, OBJECT, BLOCK, TRANSFORMER, VT, BATCH, VERTICES = RAM, RAM + 0x200, RAM + 0x300, RAM + 0x380, RAM + 0x400, RAM + 0x500, RAM + 0x600
OPS = ['drawn', 'drawn', 'drawn', 'new block', 'new block', 'cannot make', 'no sun', 'sun upward', 'sun nan', 'due later', 'no batch', 'no vertices', 'tree fails', 'render fails', 'flat box', 'huge box', 'no static']   # huge box: the grown ends pass the float range, where the registers keep them


def floats(r, n, a, b):
    return [fb(r.uniform(a, b)) for _ in range(n)]


def case(r):
    op = r.deal(OPS)
    log = log_region()
    me = bytearray(r.bytes(0x140))
    put(me, 8, 'I', OBJECT)
    new = op in ('new block', 'cannot make')
    put(me, 0x13c, 'I', 0 if new else BLOCK)
    if new:
        fill_this(log, MAKE, struct.pack('<I', BLOCK), 0x13c, which=2)
    set_return(log, MAKE, 0 if op == 'cannot make' else r.choice([1, 1, 0x100]))
    box = floats(r, 6, -60, 60)
    if op == 'flat box':
        box[3], box[5] = box[0], box[2]
        box[4] = box[1]
    if op == 'huge box':
        box = floats(r, 3, -3.4e38, -3.3e38) + floats(r, 3, 3.3e38, 3.4e38)
    obj = bytearray(r.bytes(0x98))
    put(obj, 0x78, 'I', r.getrandbits(32))
    put(obj, 0x80, '6I', *box)
    frame = r.randrange(0, 1 << 20)
    block = bytearray(r.bytes(0x38))
    put(block, 0x1c, 'i', frame + r.randrange(1, 500) if op == 'due later' else frame - r.deal([0, 1, 2, 300]))
    sun = floats(r, 3, -3, 3)
    sun[1] = fb(r.uniform(0.0, 2.0)) if op == 'sun upward' else 0x7fc00000 if op == 'sun nan' else fb(-r.uniform(0.05, 4))
    product = floats(r, 16, -2, 2)
    fill(log, MULTIPLY, 3, struct.pack('<16I', *product), which=0)
    logged = r.deal([1, 2])
    blob(log, MULTIPLY, logged, 0xffff)
    strings(log, MULTIPLY, 0, 6 ^ (1 << logged))
    logged = r.deal([0, 1])
    blob(log, TREE, logged, 3 if logged == 0 else 0xffff)
    strings(log, TREE, 0, 1 << (1 - logged))
    set_return(log, BEGIN, 0 if op == 'no batch' else BATCH)
    set_return(log, ALLOC, 0 if op == 'no vertices' else VERTICES)
    set_return(log, TREE, 0 if op == 'tree fails' else r.choice([1, 1, 0x100]))
    set_return(log, RENDER, 0 if op == 'render fails' else r.choice([1, 1, 0x100]))
    regions = {LOG: log, SELF: me, OBJECT: obj, BLOCK: block, TRANSFORMER: struct.pack('<I', VT), VT: vtable({}, 8, cdecl={MULTIPLY: 4}),
               0x87b1d8: struct.pack('<I', TRANSFORMER), 0x8c1818: struct.pack('<I', 0 if op == 'no sun' else r.choice([1, 0x100, r.getrandbits(32) | 1])), 0x8c17c8: struct.pack('<3I', *sun),
               0xfb33e0: bytes([r.getrandbits(8) & 0xfe if op == 'no static' else r.getrandbits(8) | 1]), 0xfb3400: r.bytes(0x40), 0x879ab0: r.bytes(4), 0x8bcbc4: r.bytes(4), 0x8c4b44: r.bytes(4), 0x8bcc40: struct.pack('<i', frame),
               0x8bd4b4: struct.pack('<II', 1 << r.randrange(0, 8), r.getrandbits(16))}
    return Call(0x57b8a0, [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
