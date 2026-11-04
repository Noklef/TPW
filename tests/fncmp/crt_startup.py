# The C library's command line parsing (src/game/core/crtstartup: 0x6898f0) against the original: no calls, the buffers and counts compared.
# usage: crt_startup.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

B = Bench([(0x6898f0, 0x689aff)])
COMMAND, WORDS, TEXT, COUNTS = RAM, RAM + 0x400, RAM + 0x800, RAM + 0xc00
PIECES = [b'a', b'bc', b' ', b'  ', b'\t', b'"', b'""', b'\\', b'\\\\', b'\\\\\\', b'\\"', b'\\\\"', b'\\\\\\"', b'\x81x', b'\x81"', b'\x81 ', b'\x81\\', b'\xe9', b'word']


def case(r):
    command = b''.join(r.choice(PIECES) for _ in range(r.randrange(14))) + r.choice([b'', b'', b'\x81'])
    regions = {COMMAND: command + bytes(1) + r.choice([bytes(0x10), b'after end\0'.ljust(0x10, b'\0')]), WORDS: r.bytes(0x100), TEXT: r.bytes(0x100), COUNTS: r.bytes(8)}
    regions[0xfb1669] = bytes(4 if 0x81 <= i <= 0x9f else r.choice([0, 1, 2, 8]) for i in range(256))
    return Call(0x6898f0, [COMMAND, r.choice([WORDS, WORDS, 0]), r.choice([TEXT, TEXT, 0]), COUNTS, COUNTS + 4], regions, conv='c')


if __name__ == '__main__':
    B.main(case)
