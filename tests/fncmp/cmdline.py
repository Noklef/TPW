# The command line parser (src/game/core/cmdline: 0x40f000) against the original: up to six arguments after the program's of the
# switches it knows (any case, '/' or '-'), near misses and other switches, and file names with the extension at [0x7888c8] or not
# (in any case, shorter than it, empty), with a park file already named or not and an error already set or not.
# usage: cmdline.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x40f000, 0x40f220), (0x405710, 0x405742), (0x67a400, 0x690000)])
ARGV, EXT, STRINGS = RAM, RAM + 0x40, RAM + 0x100
SWITCHES = [b'v', b'V', b'version', b'VERSION', b'quickload', b'nodebug', b'NoLoad', b'savedebug', b'SAVEDEBUG', b'flmouse', b'FLMouse',
            b'bwcursor', b'BWCURSOR', b'flmousex', b'ver', b'', b'x', b'park.sam']
NAMES = [b'park.sam', b'PARK.SAM', b'a.Sam', b'sam', b'.sam', b'am', b'', b'park', b'dir\\park.sam', b'park.sam.bak', b'x.tp2', b'-.sam']


def arg(r):
    if r.random() < 0.55:
        return bytes([r.choice(b'/-')]) + r.choice(SWITCHES)
    return r.choice(NAMES)


def case(r):
    n = r.choice([0, 1, 1, 2, 2, 3, 4, 6])
    args = [b'tp.exe'] + [arg(r) for _ in range(n)]
    strings, pointers = bytearray(), []
    for a in args:
        pointers.append(STRINGS + len(strings))
        strings += a + b'\0'
    ext = r.choice([b'.sam', b'.sam', b'.SAM', b'sam', b'', b'.tp2', b'k.sam'])
    regions = {ARGV: struct.pack('<%dI' % len(pointers), *pointers), EXT: ext + b'\0', STRINGS: bytes(strings),
               0x7888c8: struct.pack('<I', EXT), 0x788160: struct.pack('<I', r.choice([0, 0, 0, 1])) + r.bytes(0x60),
               0x78826c: r.bytes(8), 0x785de8: struct.pack('<I', r.choice([0, 0, 0, 1])) + r.bytes(0x5c)}
    argc = r.choice([n + 1] * 6 + [0, 1, -1])
    return Call(0x40f000, [argc & 0xffffffff, ARGV], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
