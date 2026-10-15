# The text scanner (src/game/files/textscan: 0x4273c0-0x427614) against the originals: the
# character classes on every kind of byte (high bytes compare signed) and the token reader over
# mixed text, its position near the end and sizes from none up.
# usage: textscan.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x4273c0, 0x427615)])
SCAN, OUT, TEXT = RAM, RAM + 0x100, RAM + 0x200
EDGES = b' !/09:@AZ[`az{~#;\t\n\x00\x7f\x80\xff' + bytes(range(0x1f, 0x22))


def byte(r):
    return r.choice([r.choice(EDGES), r.randrange(256), r.choice(b'abcXYZ0123.,-_#;')])


def case(r):
    op = r.choice(OPS)
    ram = bytearray(r.bytes(0x400))
    text = bytes(byte(r) for _ in range(0x100))
    put(ram, TEXT - RAM, '256s', text)
    if op in ('Step', 'Skip'):  # runs of blanks, often up to the end
        if op == 'Skip' and r.random() < 0.5:
            put(ram, TEXT - RAM + r.randrange(0x40), '%ds' % 0x30, bytes(r.choice(b' \t\n\x00\x80\xff\x7f') for _ in range(0x30)))
        length = r.choice([0, 1, 5, 0x40, r.randrange(0x40)])
        put(ram, 0, '3I', TEXT + r.randrange(0x80), r.choice([0, length, max(length - 1, 0), r.randrange(0x40)]), length)
        return Call(0x4273c0 if op == 'Step' else 0x4273e0, [SCAN], {RAM: ram}, ret='a')
    if op != 'Read':
        at = {'Alpha': 0x427450, 'Digit': 0x427480, 'Punct': 0x4274a0, 'Blank': 0x4274e0}[op]
        put(ram, 0, 'I', TEXT + r.randrange(0x100))
        return Call(at, [SCAN], {RAM: ram}, ret='a')
    length = r.choice([0, 1, 5, 0x40, r.randrange(0x40)])
    pos = r.choice([0, length, max(length - 1, 0), max(length - 2, 0), r.randrange(0x40), length + 1])
    put(ram, 0, '3I', TEXT + r.randrange(0x80), pos, length)
    size = r.choice([0, 1, 2, 3, 0x10, 0x40, r.randrange(-3, 0x60) & 0xffffffff, r.getrandbits(32)])
    if size >= 0x80 and size < 0x80000000:
        size = 0x80  # the out buffer
    flag = lambda: r.choice([0, 1, r.getrandbits(32)])  # noqa: E731
    return Call(0x427540, [SCAN, OUT, size, flag(), flag(), flag(), flag()], {RAM: ram}, ret='a')


OPS_ALL = ['Alpha', 'Digit', 'Punct', 'Blank', 'Read', 'Read', 'Read', 'Read', 'Step', 'Skip', 'Skip']
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
