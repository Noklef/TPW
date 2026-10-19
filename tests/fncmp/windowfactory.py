# The GUI window kinds' constructors (each over Window_Ctor, 0x65bcad) against the originals. Each: windowlife.py's Ctor set-up (a random
# window built with a parent from window.py's tree (its +0x60 chain and word +0xe0 near 0x100) or none, flags 0x400 and 1 set or not, words
# edges or near (upper halves random); the root the new window, the tree's or none, the queues as window.py's), the window's bytes past
# Window_Ctor's random too.
# usage: windowfactory.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor705a50,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, Bench, Call, log_region, put, vtable  # noqa: E402
from window import RANGES, VTABLE, WINDOW2, point, queues, tree  # noqa: E402

CALLS = {'Ctor705a50': 0x6688b3, 'RadioGroupCtor': 0x66a1e8, 'Ctor705c58': 0x66d9e0, 'Ctor705b98': 0x66c7e0,
         'Ctor705c28': 0x66d750, 'Ctor705bf8': 0x66d0c0, 'Ctor705bc8': 0x66c970, 'Ctor705af0': 0x66a4c0}
B = Bench(RANGES + [(0x6688b3, 0x6689bc), (0x66a1e8, 0x66a255), (0x66d9e0, 0x66da21), (0x66c7e0, 0x66c82e), (0x65e526, 0x65e549),
                       (0x66d750, 0x66d82e), (0x66d0c0, 0x66d1b6), (0x66c970, 0x66cab7), (0x66a4c0, 0x66a5a8)])


def ctor_args(r, regions):
    """windowlife.py's Ctor set-up for a window at WINDOW2 of 0x180 random bytes; returns the arguments after `this`"""
    wins = tree(r, regions)
    for i, w in enumerate(wins):
        put(regions[w], 0x60, 'I', r.choice([wins[i - 1], 0]) if i else 0)
        put(regions[w], 0xe0, 'H', r.choice([0, 1, 0xfe, 0xff, 0x100, 0x7fff, 0x8000, 0xffff, r.getrandbits(16)]))
    regions[WINDOW2] = r.bytes(0x180)
    queues(r, regions)
    words = []
    for _ in range(4):
        w = r.choice([r.randrange(-40, 60) & 0xffff] * 4 + [0, 0x7fff, 0x8000, 0xffff, r.getrandbits(16)])
        words.append(w | (r.getrandbits(16) << 16 if r.random() < 0.5 else 0))
    x, y = point(r, regions, wins)
    if r.random() < 0.4:
        x, y = words[0] + r.randrange(-2, 30), words[1] + r.randrange(-2, 30)
    regions[0xfaa540] = struct.pack('<II', x & 0xffffffff, y & 0xffffffff)
    regions[0xfaa5d8] = struct.pack('<5I', r.choice([wins[0], WINDOW2, WINDOW2, 0]), r.getrandbits(32), r.choice(wins + [0, 0]),
                                    r.getrandbits(32), r.choice([0] * 6 + [1]))
    flags = r.choice([1, 0x401, 0x400, 0, 0x10000001, r.getrandbits(32)]) | (r.getrandbits(32) & 0x0ffff800 if r.random() < 0.3 else 0)
    return [r.choice(wins + [0, 0]), flags, r.getrandbits(32)] + words


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, VTABLE: vtable({5: 1})}
    return Call(CALLS[op], [WINDOW2] + ctor_args(r, regions), regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
