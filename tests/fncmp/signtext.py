# A sign's two lines of text (src/game/world/signtext: 0x4e2780, 0x4e2950, 0x4e2a50) against the original: the copying, the formatting, the handle, the save and the network recorders.
# usage: signtext.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, echo, fill, log_region, set_return, strings  # noqa: E402

COPY, MODE, ATEXIT, HANDLE, LINE_A, LINE_B, SAVE, SEND, FORMAT, FORMAT2 = range(1, 11)
FORMAT4 = _recorder(FORMAT, 4, cdecl=True)
FORMAT5 = _recorder(FORMAT2, 5, cdecl=True)


def format_hook(e):
    """The one routine, two shapes: its format argument tells them apart."""
    (FORMAT4 if e.u32(e.r['ESP'] + 8) == 0x75c0cc else FORMAT5)(e)


hooks = {0x67b0d0: _recorder(COPY, 3, cdecl=True), 0x550ca0: _recorder(MODE, 0), 0x67a7d0: _recorder(ATEXIT, 1, cdecl=True), 0x50b350: _recorder(HANDLE, 1), 0x5b0940: _recorder(LINE_A, 1), 0x5b0970: _recorder(LINE_B, 1), 0x409410: _recorder(SAVE, 0, cdecl=True), 0x403fd0: _recorder(SEND, 4), 0x67c830: format_hook}
B = Bench([(0x4e2780, 0x4e285e), (0x4e2950, 0x4e2a48), (0x4e2a50, 0x4e2b48), (0x67acd0, 0x67acee)], hooks=hooks)
SIGN = RAM + 0x1000
TO = RAM + 0x2000
TEXT = RAM + 0x2100
WORLD = RAM + 0x10000
SAVE_AT = RAM + 0x3000
OPS = ['format', 'first', 'second']


def wide(s):
    return (s + '\0').encode('utf-16-le')


def case(r):
    log = log_region()
    sign = bytearray(r.randbytes(0x500))
    first = r.choice(['', 'a', 'abc', 'abc-', '-', 'a-b', 'long name-'])
    second = r.choice(['', '', 'x', 'xyz'])
    sign[0x3c8:0x3c8 + 2 * len(first) + 2] = wide(first)
    sign[0x40a:0x40a + 2 * len(second) + 2] = wide(second)
    own = r.getrandbits(16)
    world = bytearray(r.randbytes(0x10))
    struct.pack_into('<H', world, 0x1da732 - 0x1da730, own)
    fill(log, HANDLE, 0, struct.pack('<H', r.choice([own, own, r.getrandbits(16)])))
    echo(log, HANDLE, 0)
    strings(log, HANDLE, 0, out=1)
    set_return(log, SAVE, SAVE_AT)
    regions = {LOG: log, SIGN: sign, TO: bytes(0x100), TEXT: wide('new'), SAVE_AT: bytes(4), WORLD + 0x1da730: world, 0x80239c: struct.pack('<I', WORLD), 0x7cdc10: r.randbytes(0x100), 0x785a08: bytes(4), 0xfb34b8: bytes([r.choice([0, 1, 1, r.getrandbits(8)])]), 0xfb3b7c: struct.pack('<I', r.choice([0, 1, 1, 2, r.getrandbits(32)]))}
    op = r.choice(OPS)
    if op == 'format':
        return Call(0x4e2780, [SIGN, TO, r.randrange(1, 0x80)], regions, conv='t', ret='v')
    return Call(0x4e2950 if op == 'first' else 0x4e2a50, [SIGN, TEXT], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
