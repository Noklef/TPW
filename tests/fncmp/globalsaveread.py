# The global save's file read (src/game/core/globalsave: 0x5afd70) against the original: the fields reset, the themes cleared (an empty tree's size goes to 0), the file's front ends and the serialise recorders: the open failing, the version's read failing or short, an old version, the serialise failing, then the swear lists' two paths and their load's answer.
# usage: globalsaveread.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, echo, fill, heap, log_region, set_return, strings  # noqa: E402

OPEN, CLOSE, SERIALIZE, READ, SPRINTF, FILTER = 1, 3, 8, 9, 10, 11
B = Bench([(0x5afd70, 0x5aff4b), (0x5da3c0, 0x5da3c1), (0x409410, 0x40947a), (0x41a820, 0x41a826), (0x619240, 0x619244), (0x5b08a0, 0x5b093a), (0x5b09c0, 0x5b0a6c), (0x5b0a70, 0x5b0bb2), (0x5b0bc0, 0x5b0c31), (0x5b0cd0, 0x5b0d70), (0x5b17c0, 0x5b1925), (0x5b1b80, 0x5b20c0), (0x5b20c0, 0x5b20e0), (0x5b20e0, 0x5b2169), (0x5b2f40, 0x5b2fd0), (0x5b2480, 0x5b2530),
           (0x5b16b0, 0x5b17c0), (0x5b2170, 0x5b2480), (0x5b2ff0, 0x5b30b0), (0x5b31f0, 0x5b3203), (0x5b3230, 0x5b324b), (0x5af1e0, 0x5af49b), (0x5b1925, 0x5b1b80), (0x5b2530, 0x5b2f40)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None, 0x5f5ad0: _recorder(OPEN, 4, cdecl=True), 0x5f5e20: _recorder(READ, 5, cdecl=True), 0x5f5b80: _recorder(CLOSE, 2, cdecl=True), 0x67a650: _recorder(SPRINTF, 4, cdecl=True), 0x5aff50: _recorder(SERIALIZE, 3), 0x5d3140: _recorder(FILTER, 3)})
SELF, INST, PATH, ROOT, HEAD, NIL = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x280, RAM + 0x300, RAM + 0x380
OPS = ['read', 'read', 'read', 'read', 'read', 'no open', 'bad read', 'short read', 'long read', 'old', 'bad serialize']


def case(r):
    op = r.deal(OPS)
    log = log_region()
    echo(log, OPEN, 0)
    echo(log, READ, 0)
    strings(log, OPEN, 4, 3)
    strings(log, READ, 0, 1 | 4 | 16)
    strings(log, CLOSE, 0, 1)
    strings(log, SPRINTF, 4 | 8, 1)
    strings(log, FILTER, 3)
    good = lambda: r.choice([0, 1, 0x7fffffff])  # noqa: E731
    fill(log, OPEN, 0, struct.pack('<i', -r.randrange(1, 1 << 31) if op == 'no open' else good()), which=0)
    fill(log, OPEN, 1, r.bytes(4), which=1)
    fill(log, READ, 0, struct.pack('<i', -r.randrange(1, 1 << 31) if op == 'bad read' else good()), which=2)
    fill(log, READ, 2, struct.pack('<I', r.randrange(0, 0xc) if op == 'old' else r.choice([0xc, 0xc, 0xd, 0x7fffffff, 0x80000000, 0xffffffff])), which=3)
    fill(log, READ, 4, struct.pack('<I', r.randrange(0, 4) if op == 'short read' else 5 if op == 'long read' else 4), which=4)
    fill(log, SPRINTF, 0, b'first path\0\0' + b'second path\0', which=5, records=2)
    set_return(log, SERIALIZE, r.choice([1, 0x100, 0xffffffff]) if op == 'bad serialize' else 0)
    set_return(log, FILTER, r.choice([0, 0, 1, 0x100, 0xffffff00, r.getrandbits(32)]))
    me = bytearray(r.bytes(0x50))
    struct.pack_into('<II', me, 0x3c, HEAD, 0)
    struct.pack_into('<I', me, 0x44, r.randrange(1, 1000))
    inst = bytearray(r.bytes(0x50))
    inst[0x25] = r.choice([0, 1, 1, 0x80])
    regions = {LOG: log, SELF: me, INST: inst, PATH: b'Save\\Global%d.sav\0' % r.randrange(100), ROOT: b'root%d\0' % r.randrange(100), HEAP: heap(),
               HEAD: struct.pack('<III', HEAD, NIL, HEAD) + bytes(0x20), NIL: bytes(0x40), 0xf7c968: struct.pack('<I', NIL), 0xf7c964: struct.pack('<I', r.choice([1, 2, 5])),
               0xf7c58c: struct.pack('<I', r.choice([INST, INST, INST, SELF])), 0xf816f0: struct.pack('<I', ROOT), 0xf7f230: b'lang%d\0' % r.randrange(100)}
    return Call(0x5afd70, [SELF, PATH], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
