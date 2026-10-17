# The sprite files' sort (src/game/sprite/loading: 0x541210) against the original: tables of 0 to
# 16 entries (and negative counts) whose files' names differ in case only, share a prefix, are
# equal or empty, files shared between entries, random words and bytes beside them.
# usage: spritefiles.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

B = Bench([(0x541210, 0x541302), (0x424cc0, 0x424cf0), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000)])
NAMES = [b'guest', b'Guest', b'GUESTS', b'guest2', b'clown', b'Clown01', b'bin', b'', b'a', b'z', b'_x', b'shop - a', b'shop - b']
FILES, TEXTS = RAM, RAM + 0x1000


def case(r):
    n = r.choice([0, 1, 2, 3, 5, 8, 12, 16, r.randrange(17)])
    count = r.choice([n] * 8 + [-1, 0x80000000])
    nfiles = r.randrange(1, 17)
    files, texts = bytearray(r.bytes(0x40 * nfiles)), bytearray()
    for k in range(nfiles):
        name = bytearray(r.choice(NAMES))
        for i in range(len(name)):
            if r.random() < 0.2:
                name[i] ^= 0x20
        text = bytes(name) + b'\0'
        put(files, 0x40 * k + 0x20, 'I', TEXTS + len(texts))
        texts += text + bytes(-len(text) % 4)
    table = bytearray(r.bytes(12 * max(n, 1)))
    for i in range(n):
        put(table, 12 * i, 'I', FILES + 0x40 * r.randrange(nfiles))
    regions = {0x875970: table, 0x875430: put(bytearray(4), 0, 'I', count & 0xffffffff), FILES: files,
               TEXTS: texts + bytes(0x10), HEAP: heap(0x10000)}
    return Call(0x541210, [], regions)


if __name__ == '__main__':
    B.main(case)
