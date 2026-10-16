# The walks over the park's things (src/game/world/thingscan) against the originals: a consistent thing table (slots in use linked newest first,
# the things behind them with a kind, flags and the fields the scans read) and the object definitions they are found through, between none and
# six things; the finders start from the newest or after a thing.
# usage: thingscan.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=C1,C2,Max,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import Bench, Call, RAM  # noqa: E402
from thingenv import POOL, THING, park  # noqa: E402

# name: (address, convention, size, arguments: n none, p previous thing, vp value then previous thing, v value, o an output word)
CALLS = {'C1': (0x4c9f90, 'c', 0xa0, 'n'), 'C2': (0x4ca0c0, 'c', 0xa0, 'n'), 'Max': (0x4ca820, 's', 0xa0, 'v'),
         'F4ac': (0x4828f0, 'c', 0xd0, 'vp'), 'Fw': (0x4829c0, 'c', 0xc0, 'p'), 'Fu2': (0x482a80, 'c', 0xd0, 'p'),
         'Fs1': (0x482b50, 'c', 0xd0, 'p'), 'Fs0': (0x482c20, 'c', 0xd0, 'p'), 'Ff2': (0x482cf0, 'c', 0xb0, 'p'),
         'Last': (0x4831f0, 'c', 0x90, 'o'), 'C3': (0x4ca030, 'c', 0x90, 'n'), 'Staff': (0x4c9960, 's', 0xc0, 'v'), 'Avg': (0x4c9c00, 's', 0xb0, 'v'),
         'Re': (0x4c9ee0, 't', 0xb0, 'g'), 'D4': (0x4ca8e0, 's', 0xd0, 'vv'), 'D5': (0x4ca9b0, 's', 0xc0, 'v'),
         'B4ac': (0x483280, 'c', 0x140, 'vp'), 'Bw': (0x4833c0, 'c', 0x120, 'p'), 'Bu2': (0x4834e0, 'c', 0x140, 'p'), 'Bf2': (0x483620, 'c', 0x120, 'p'),
         'P1': (0x4c9130, 'c', 0xa0, 'n'), 'P2': (0x4c9530, 'c', 0xa0, 'n'), 'P3': (0x4c95d0, 'c', 0xa0, 'n'),
         'Sum': (0x4c9e60, 'c', 0x70, 'n'), 'Cs': (0x4c9a10, 'c', 0xa0, 'n'), 'U3c': (0x519690, 't', 0x130, 'u'), 'U3e': (0x519800, 't', 0x130, 'u'), 'U40': (0x519970, 't', 0x130, 'u'),
         'U42': (0x519c50, 't', 0x130, 'u'), 'U44': (0x519ae0, 't', 0x130, 'u'), 'U46': (0x519dc0, 't', 0x130, 'u'), 'S5': (0x59edd0, 't', 0x50, 's'), 'S6': (0x59ee20, 't', 0x50, 's'), 'S4': (0x59ee70, 't', 0x50, 's'), 'S7': (0x59eec0, 't', 0x50, 's')}
B = Bench([(a, a + n) for (a, _c, n, _s) in CALLS.values()] + [(0x516130, 0x516160), (0x516160, 0x5161b0), (0x50b350, 0x50b360), (0x4e0440, 0x4e0450),
                                                              (0x4e2710, 0x4e2740), (0x4dd4e0, 0x4dd500), (0x412e90, 0x412eea), (0x5da3c0, 0x5da3c1), (0x41a960, 0x41a970), (0x5161b0, 0x516200), (0x4fa990, 0x4fa9fa), (0x536300, 0x53644c), (0x519510, 0x519550)])
OUT = RAM + 0x1000


def case(r):
    name = r.choice(OPS)
    address, conv, _n, shape = CALLS[name]
    regions, handles, things = park(r, count=r.choice([1, 2, 3, 4, 6]) if shape in ('o', 'u') or name.startswith('B') else None,
                                    defs=name not in ('C1', 'C2', 'Max', 'C3', 'Staff', 'Avg', 'Re', 'P1', 'P2', 'P3', 'Sum', 'Cs') and shape != 'u',
                                    kinds=(4, 5, 6, 7, 8, 3, 1, 0) if name in ('Staff', 'S5', 'S6', 'S4', 'S7', 'Cs') else (1, 1, 1, 3, 0, 0xff) if name[0] == 'P' else (3, 3, 3, 1, 2, 0xff, 0),
                                    cells=name[0] == 'P')
    if shape == 'u':  # a singly linked list of some of the things (head word in the game object, links in each thing), one of them taken out
        head = {'U3c': 0x1da73c, 'U3e': 0x1da73e, 'U40': 0x1da740, 'U42': 0x1da742, 'U44': 0x1da744, 'U46': 0x1da746}[name]
        link = 0xc if name == 'U46' else 0x210
        order = r.sample(range(len(handles)), r.randrange(1, len(handles) + 1))
        pool = regions[POOL]
        for k, i in enumerate(order):
            struct.pack_into('<H', pool, THING * i + link, handles[order[k + 1]] if k + 1 < len(order) else 0)
        game = RAM + 0x20000 - head
        regions[RAM + 0x20000] = struct.pack('<H', handles[order[0]]) + r.bytes(0x10)
        return Call(address, [game, things[r.choice(order)]], regions, conv=conv, ret='v')
    prev = r.choice([0, 0] + things) if things else 0
    value = r.choice([0, 1, 2, 3, 5, 0x10001, r.getrandbits(32)])
    if name == 'Re':  # the game object: its fields far out, and the game time they are set from
        regions[0x80239c] = struct.pack('<I', RAM + 0x20000 - 0x1da70c)
        regions[RAM + 0x20000] = r.bytes(0x20)
        regions[RAM + 0x4000 + 0x1f0d0] = r.bytes(0x20)
    if name == 'Sum':  # the game object's handle, the things chained through the word at +0x210 (towards older ones), indexes into the 12-byte table
        regions[0x80239c] = struct.pack('<I', RAM + 0x20000 - 0x1da742)
        regions[RAM + 0x20000] = struct.pack('<H', r.choice(handles + [0]) if handles else 0)
        pool = regions[POOL]
        for i in range(len(handles)):
            struct.pack_into('<I', pool, THING * i + 0x1e4, r.randrange(4))
            struct.pack_into('<H', pool, THING * i + 0x210, handles[i + 1] if i + 1 < len(handles) and r.random() < 0.7 else 0)
        regions[0x78545c] = r.bytes(12 * 4)
    if name == 'Staff':
        value = r.choice([4, 5, 6, 7, 8, 3, 0x104, 0xff])
    if name[0] == 'S' and name[1:].isdigit():  # the object with the limits, and the game table window the 0x519510 getter reads
        regions[0x7cf83c] = struct.pack('<I', RAM + 0x20000 - 0x1da718)
        window = bytearray(r.bytes(0x20))
        window[8:10] = struct.pack('<H', r.choice([0, 1, 3, 5]))
        regions[RAM + 0x20000] = window
        obj = bytearray(r.bytes(0x20))
        for off in (0, 8, 0x10, 0x18):
            obj[off:off + 4] = struct.pack('<i', r.choice([-2, -1, 0, 1, 2, 5]))
            obj[off + 4:off + 8] = struct.pack('<I', r.choice([0, 7, 0xdead]))
        regions[RAM + 0x4000 + 0x360] = bytes(obj)
    if name == 'Avg':
        value = r.choice([3, 1, 2, 0xff, 0, 0x103])
    if name in ('D4', 'D5'):
        value = r.choice([0, 3, 3, 5])
    args = {'n': [], 'p': [prev], 'v': [value], 'vp': [r.choice([0, 3, 5, value]), prev], 'o': [OUT], 'g': [RAM + 0x4000], 's': [RAM + 0x4000],
            'vv': [value, r.choice([0, 1, 5, 0x7f, 0x80, 0xff, 0xffffffff, 0x80000000])]}[shape]
    regions[OUT] = r.bytes(4)
    return Call(address, args, regions, conv=conv, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
