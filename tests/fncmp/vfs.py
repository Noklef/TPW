# The virtual file system's paths (src/game/files/vfs: 0x41cff0) against the originals. StripBase: a base folder (now and then ending in
# the archive pattern) and a path under it or not (the base's case varied, cut short or other text; leading ".", "\" runs), out a
# string of any text; the globals "\", ".", ".wad*\" (0x78bf40), ".wad*" (0x78c810), "" (0x78be50) and the folder code's "\" (0xf80be0),
# now and then other texts, so that the archive branch, dead with the game's own pair, runs too ("." and the tail not empty, an archive in
# a folder: as the game's, else both fault). The strings as gamestring.py's, their blocks from the heap region.
# usage: vfs.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=StripBase)
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import Bench, Call, HEAP, RAM, heap  # noqa: E402
import gamestring  # noqa: E402
from gamestring import BACKSLASH, DOT, LOCALE, SLASH2, string  # noqa: E402

B = Bench(gamestring.B_RANGES + [(0x41cff0, 0x41d2d0)])
TAIL, WAD, EMPTY = 0x78bf40, 0x78c810, 0x78be50
PATH, BASE, OUT = RAM, RAM + 0x40, RAM + 0x80
WORDS = [b'data', b'Data', b'ui', b'x.wct', b'a', b'TP', b'levels']


def flip(r, text, p=0.3):
    return bytes(c ^ 0x20 if (0x61 <= c <= 0x7a or 0x41 <= c <= 0x5a) and r.random() < p else c for c in text)


def folder(r):
    return b'\\'.join(r.choice(WORDS) for _ in range(r.choice([0, 1, 1, 2, 3])))


def strip_base(r):
    regions = {HEAP: heap(), LOCALE: bytes(4)}
    base = folder(r)
    archive = r.random() < 0.3
    if archive:
        base = (base or b'data') + r.choice([b'.wad*', b'.WAD*', b'.wad*\\', b'.wad'])
    elif r.random() < 0.4:
        base += b'\\'
    k = r.random()
    if k < 0.6:   # under the base (as the archive's folder: its pattern dropped, a "\\" after it)
        head = base[:-len(b'.wad*')] + b'\\' if archive and base.endswith(b'.wad*') else base
        path = flip(r, head) + r.choice([b'', b'\\', b'\\\\', b'/']) + folder(r)
    elif k < 0.8:
        path = flip(r, base[:r.randrange(len(base) + 1)])
    else:
        path = folder(r)
    if r.random() < 0.3:
        path = r.choice([b'.', b'.\\', b'..\\', b'\\', b'\\\\', b'.\\\\']) + path
    if r.random() < 0.2:
        base = r.choice([b'.\\', b'\\', b'.']) + base
    string(r, 0, PATH, RAM + 0x1000, regions, path)
    string(r, 0, BASE, RAM + 0x1400, regions, base)
    string(r, 0, OUT, RAM + 0x1800, regions)
    for at, data, usual, other in ((BACKSLASH, RAM + 0x2000, b'\\', [b'/', b'\\\\', b'a']), (DOT, RAM + 0x2100, b'.', [b'.\\', b'..']),
                                    (TAIL, RAM + 0x2200, b'.wad*\\', [b'.wad*', b'.wad*', b'.wad']),
                                    (WAD, RAM + 0x2300, b'.wad*', [b'.wad*\\', b'.wad', b'']), (EMPTY, RAM + 0x2400, b'', [b'x', b'\\a']),
                                    (SLASH2, RAM + 0x2500, b'\\', [b'/', b'\\\\'])):
        string(r, 0, at, data, regions, usual if r.random() < 0.7 else r.choice(other))
    return Call(0x41cff0, [PATH, BASE, OUT], regions, ret='a')


OPS_ALL = {'StripBase': strip_base}
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


def case(r):
    return OPS_ALL[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)
