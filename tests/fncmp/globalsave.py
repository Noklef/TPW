# The global save info's byte accessors (src/game/core/globalsave: 0x5af7f0-0x5b0cc0) against
# the originals: indices across the arrays, values wider than a byte, with and without a
# current theme. The sign names' setters (0x5b0940, 0x5b0970): names of 0 to 0x24 units with a NUL or not.
# usage: globalsave.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=ThemeByte,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x5af7f0, 0x5af80b), (0x5afc30, 0x5afc5a), (0x5b09a0, 0x5b09b4), (0x5b0ca0, 0x5b0cc5), (0x5b0940, 0x5b0996), (0x67b0d0, 0x67b10f)])
INFO, THEME = RAM + 0x10, RAM + 0x100
CALLS = {'ThemeByte': (0x5af7f0, 'i', 'b'), 'Byte26': (0x5af800, 'i', 'b'), 'Inc20': (0x5afc30, '', 'v'),
         'Byte24': (0x5afc40, '', 'b'), 'SetByte24': (0x5afc50, 'v', 'v'), 'Byte25': (0x5b09b0, '', 'b'),
         'SetByte25': (0x5b09a0, 'v', 'v'), 'AllResearch': (0x5b0ca0, '', 'v'), 'Byte4c': (0x5b0cb0, '', 'b'),
         'ClearByte4c': (0x5b0cc0, '', 'v'), 'SignNameA': (0x5b0940, 'w', 'v'), 'SignNameB': (0x5b0970, 'w', 'v')}
NAME = RAM + 0x200


def case(r):
    fn, arg, ret = CALLS[r.choice(OPS)]
    info = bytearray(r.bytes(0x60))
    put(info, 0x48, 'I', r.choice([THEME, THEME, 0]))
    put(info, 0x20, 'I', r.choice([0, 0xffffffff, 0xff, r.getrandbits(32)]))
    args = [INFO]
    if arg == 'i':
        args.append(r.choice([0, 1, 0x20, r.randrange(0xc0)]))
    elif arg == 'w':   # a name of 0-0x24 units (none 0), its NUL or not (the units after it any)
        n = r.choice([0, 1, 0x1f, 0x20, 0x21, 0x22, r.randrange(0x25)])
        name = bytearray(r.bytes(0x60))
        for i in range(n):
            put(name, 2 * i, 'H', r.randrange(1, 0x10000))
        if r.random() < 0.85:
            put(name, 2 * n, 'H', 0)
        args.append(NAME)
        return Call(fn, args, {RAM: r.bytes(0x10), INFO: info, THEME: r.bytes(0xc0), NAME: name}, conv='t', ret=ret)
    elif arg == 'v':
        args.append(r.choice([0, 1, 0x100, 0x1ff, r.getrandbits(32)]))
    return Call(fn, args, {RAM: r.bytes(0x10), INFO: info, THEME: r.bytes(0xc0)}, conv='t', ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
