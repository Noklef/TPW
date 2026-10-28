# The text formatter's arguments (src/game/core/textarg) against the originals: the constructors (0x4a77a0, 0x4ea220, 0x50aef0; values
# about 0 and INT_MIN, signed or not) and the two arguments' methods (0x6add40 the number's, 0x6add70 the string's) writing to the game's
# wide-text writer (widewriter.py's, into its buffer), its narrow-text writer (narrowwriter.py's, through its range table) or a recorder
# (slot 5 logged; a stack buffer as 0). The number's L"%d" through the CRT's swprintf (crt.swprintf_hook; the driver's stand-in).
# usage: textarg.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,StringCtor,IntCtor,IntWrite,StringWrite)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import LOG, RAM, Bench, Call, log_region, strings, vtable  # noqa: E402
import crt  # noqa: E402
import narrowwriter as nw  # noqa: E402
import widewriter as ww  # noqa: E402

B = Bench([(0x4a77a0, 0x4a77b9), (0x4ea220, 0x4ea240), (0x50aef0, 0x50af32), (0x6add40, 0x6add86), (0x6affe0, 0x6b0095), (0x6b00a0, 0x6b00da),
           (0x6afd40, 0x6afd84), (0x6afd90, 0x6afe0b), (0x6ad8f0, 0x6ad946)], hooks={0x67af00: crt.swprintf_hook})
ARG, STRING, REC, VT_REC = RAM + 0x800, RAM + 0x900, RAM + 0xa00, RAM + 0xa80
OPS = ['Ctor', 'StringCtor', 'IntCtor', 'IntWrite', 'StringWrite']
PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]


def value(r):
    return r.choice([0, 1, 0xffffffff, 0x80000000, 0x7fffffff, 9, 10, 99999, r.getrandbits(32), r.getrandbits(32)])


def write_case(r, op):
    """(writer, state) on a number (+0xc a value) or a string (+0xc a wide string) argument."""
    u = r.random()
    if u < 0.4:   # the wide-text writer (no code table: slot 5 copies cells)
        regions = {LOG: log_region()}
        ww.writer(r, regions, 0)
        writer = ww.W
    elif u < 0.75:
        regions = nw.world(r)
        writer = nw.W
    else:
        regions = {LOG: log_region(), VT_REC: vtable({5: 2}), REC: struct.pack('<I', VT_REC) + r.bytes(0xc)}
        if op == 'IntWrite':
            strings(regions[LOG], 5, 0, out=1)   # (the stack buffer)
        writer = REC
    text = [r.choice([0x41, 0x7a, 0x20, 3, 10, 15, 22, r.randrange(1, 0x80), r.getrandbits(16) or 1]) for _ in range(r.choice([0, 1, 3, 8, 0x20]))]
    regions[STRING] = struct.pack('<%dH' % (len(text) + 1), *(text + [0])) + r.bytes(8)
    regions[ARG] = struct.pack('<3I', r.choice([0x6ff170, 0x6ff174]), 0x5641524d, r.randrange(4)) + \
        struct.pack('<2I', value(r) if op == 'IntWrite' else STRING, r.randrange(2)) + r.bytes(8)
    return Call(0x6add40 if op == 'IntWrite' else 0x6add70, [ARG, writer, r.getrandbits(32)], regions, conv='t', ret='a')


def case(r):
    op = r.deal(PICK)
    if op in ('IntWrite', 'StringWrite'):
        return write_case(r, op)
    arg = RAM + r.randrange(16)
    regions = {RAM: r.bytes(0x40)}
    n = r.choice([0, 1, 2, 4, r.getrandbits(32)])
    if op == 'Ctor':
        return Call(0x4a77a0, [arg, n], regions, conv='t', ret='a')
    if op == 'StringCtor':
        return Call(0x4ea220, [arg, n, r.getrandbits(32)], regions, conv='t', ret='a')
    v = r.choice([0, 1, 0xffffffff, 0x80000000, 0x7fffffff, r.getrandbits(32)])
    return Call(0x50aef0, [arg, n, v, r.choice([0, 1, 0x100, r.getrandbits(32)])], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
