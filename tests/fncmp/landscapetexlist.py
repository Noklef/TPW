# The landscape's texture list loaded (src/game/terrain/landscape: 0x418c30) against the original: the rows at 0x785728 (their count at 0x785724, 0-5 or negative), the real strings making each path
# (the base a recorder's text, the String at 0x78aa30 between it and the row's name, the backslash global odd now and then), Landscape_AddTexture a recorder given the path's text and answering
# not 0, 0 or not 0 and then 0, its out pointer's values in turn, the list written only for its yes.
# usage: landscapetexlist.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, fail_after, fill, heap, log_region, put, set_return, strings  # noqa: E402
from gamestring import LOCALE, string  # noqa: E402
import vfs  # noqa: E402

BASE, ADD = 3, 4
B = Bench(vfs.RANGES + [(0x418c30, 0x418d6b)], hooks={0x40ef40: _recorder(BASE, 0, cdecl=True), 0x5850c0: _recorder(ADD, 8)})
LIST, TEXT, NAMES, FOLDER, FOLDERDATA = RAM + 0x4000, RAM + 0x4040, RAM + 0x4100, RAM + 0x4400, RAM + 0x4440


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(fail=r.choice([0] * 15 + [1])), LOCALE: bytes(4)}
    vfs.globals_(r, regions)
    n = r.choice([1, 1, 2, 3, 4, 5, 0, -1])
    rows = bytearray(r.bytes(0x18 * 6))
    names = bytearray(0x200)
    for k in range(max(n, 0)):
        text = vfs.folder(r) + b'.wct'
        names[0x40 * k:0x40 * k + min(len(text), 0x3f)] = text[:0x3f]
        put(rows, 0x18 * k + 0x10, 'I', r.choice([0, 1, r.getrandbits(32)]))
        put(rows, 0x18 * k + 0x14, 'I', NAMES + 0x40 * k)
    regions.update({0x785724: struct.pack('<i', n), 0x785728: rows, NAMES: names, LIST: r.bytes(0x20), TEXT: (r.choice([b'c:\\game', b'c:\\game\\', b'', b'd']) + bytes(1)).ljust(0x20, b'\0')})
    string(r, 0, 0x78aa30, FOLDERDATA, regions, r.choice([b'landscape', b'land\\', b'']))
    set_return(log, BASE, TEXT)
    strings(log, ADD, 1 << 1, 1)
    fill(log, ADD, 0, struct.pack('<6I', *[r.getrandbits(32) for _ in range(6)]), records=6)
    how = r.random()
    if how < 0.2:
        set_return(log, ADD, 0)
    elif how < 0.5:
        fail_after(log, ADD, r.randrange(1, 4), 0)
    return Call(0x418c30, [LIST], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
