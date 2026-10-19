# A list's new row window (src/game/ui/listrows: 0x480d30 / 0x4a8380) against the originals. HilightList_NewRow / List7cc188_NewRow:
# window.py's tree and queues (Window_Ctor's surroundings), the heap failing now and then, font 7 (Language_Font, of group 0-2) a recorder
# whose slot 1 gives its height (edges too), the divisor word [0xfaa5c0] any but 0.
# usage: listrows.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=HilightList_NewRow,List7cc188_NewRow)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, Bench, Call, heap, log_region, put, set_return, vtable  # noqa: E402
from window import RANGES, VTABLE, queues, tree  # noqa: E402

CALLS = {'HilightList_NewRow': 0x480d30, 'List7cc188_NewRow': 0x4a8380}
B = Bench(RANGES)


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log}
    wins = tree(r, regions)
    queues(r, regions)
    regions[HEAP] = heap(0x400, r.choice([0] * 5 + [1]))
    regions[0xfaa540] = struct.pack('<II', r.randrange(-10, 120) & 0xffffffff, r.randrange(-10, 40) & 0xffffffff)
    regions[0xfaa5d8] = struct.pack('<5I', r.choice([wins[0], 0]), r.getrandbits(32), r.choice(wins + [0, 0]), r.getrandbits(32),
                                    r.choice([0] * 6 + [1]))
    regions[0xfaa5c0] = struct.pack('<H', r.choice([1, 2, 3, 0x600, 0x7fff, 0x8000, 0xffff, 0xfffd, r.randrange(1, 0x10000)]))
    lang = bytearray(r.bytes(0x400))
    group = r.choice([0, 1, 2])
    put(lang, 4, 'I', group)
    put(lang, 0x1c + (7 + 13 * group) * 0x18, 'I', VTABLE)
    regions[0x78b530] = lang
    regions[VTABLE] = vtable({})
    set_return(log, 1, r.choice([r.randrange(0, 40), 0x7fff, 0x8000, 0xffff, r.getrandbits(16)]) | (r.getrandbits(16) << 16 if r.random() < 0.5 else 0))
    return Call(CALLS[op], [], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
