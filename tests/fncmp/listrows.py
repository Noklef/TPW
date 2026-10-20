# A list's new row window (src/game/ui/listrows: 0x480d30 / 0x4a8380) against the originals. HilightList_NewRow / List7cc188_NewRow:
# window.py's tree and queues (Window_Ctor's surroundings), the heap failing now and then, font 7 (Language_Font, of group 0-2) a recorder
# whose slot 1 gives its height (edges too), the divisor word [0xfaa5c0] any but 0.
# HilightList_InitRow / List7cc188_InitRow (0x480dd0 / 0x4a8420): see init_case() below.
# usage: listrows.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=HilightList_NewRow,List7cc188_NewRow,HilightList_InitRow,List7cc188_InitRow)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import rendererpart  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, blob_bytes, heap, log_region, put, set_return, vtable  # noqa: E402
from window import RANGES, VTABLE, queues, tree  # noqa: E402

CALLS = {'HilightList_NewRow': 0x480d30, 'List7cc188_NewRow': 0x4a8380, 'HilightList_InitRow': 0x480dd0, 'List7cc188_InitRow': 0x4a8420}
# (the initialisers: the text part's construction (textparts.py's ranges), SetLayout (0x48fc40, 0x48f420), the pixels (trackedglobal.py's),
# the renderer's tag 0x30 list (0x48f5c0, Renderer_Add, Pool_AddSorted), Window_Notify40000)
PART_RANGES = [(0x48f150, 0x48f290), (0x5775d0, 0x577660), (0x579b40, 0x579c30), (0x588320, 0x588420), (0x588d70, 0x588e50), (0x58a4f0, 0x58a560),
               (0x58dc50, 0x58dd70), (0x590b90, 0x590c50), (0x590a70, 0x590a75), (0x588ca0, 0x588d6e), (0x6597b0, 0x6598ed), (0x659994, 0x6599bc),
               (0x659d00, 0x659d53), (0x65a9d0, 0x65aa00), (0x66bff7, 0x66c01b), (0x67afa0, 0x67b0d0), (0x48f6a0, 0x48f740),
               (0x5fa680, 0x5fa6b0), (0x6ad5d0, 0x6ad670), (0x6ade00, 0x6adf30), (0x6b0230, 0x6b0290), (0x48fc40, 0x48fdac), (0x48f420, 0x48f5b7),
               (0x48f3d0, 0x48f409), (0x588940, 0x5889a1), (0x56af20, 0x56af9c), (0x588e40, 0x588ee8), (0x48f5c0, 0x48f650), (0x576410, 0x576550),
               (0x576270, 0x576320), (0x65c5d5, 0x65c672), (0x480dd0, 0x480e62), (0x4a8420, 0x4a84b2)]
B = Bench(sorted(set(RANGES + PART_RANGES)))
ROW, OLD, OBJ98, VT98, VTOLD, OBJ4C, REND = RAM + 0x9000, RAM + 0x9200, RAM + 0x9300, RAM + 0x9380, RAM + 0x9400, RAM + 0x9480, RAM + 0x9500
PARENTS = RAM + 0x9600


def init_case(r, op):
    """HilightList_InitRow / List7cc188_InitRow: rendererpart.py's surroundings of a part's construction, the heap failing the part now
    and then, font 7 (Language_Font, of group 0-2); a row of random bytes, its rectangle record (+0x2c) small, its impl +0x9c none (a new
    row) or an old one (a recorder vtable, the words +0x10 / +0x12 about 1: released or deleted), +0x98 none or a recorder (slot 3's
    event pointer not compared), +0x4c none, an object or font 7 itself, shown (+0x48 bit 0, up its two parents or fewer) or not; the
    GUI's size words about 2048 x 1536 (the part's pixels small); the renderer's pool out of nodes (the tag 0x30 list left alone)."""
    rendererpart.OPS = ['Ctor']
    regions = dict(rendererpart.case(r).regions)
    regions.pop(rendererpart.PART)
    log = log_region()
    regions[LOG] = log
    regions[HEAP] = heap(0x4000, r.choice([0] * 9 + [1]))
    lang = bytearray(r.bytes(0x400))
    group = r.choice([0, 1, 2])
    put(lang, 4, 'I', group)
    regions[0x78b530] = lang
    font = 0x78b530 + 0x1c + (7 + 13 * group) * 0x18
    row = bytearray(r.bytes(0x138))
    left, top = r.randrange(-5, 30), r.randrange(-5, 30)
    put(row, 0x30, '4h', left, top, left + r.randrange(0, 40), top + r.randrange(0, 40))
    put(row, 0x9c, 'I', OLD if r.random() < 0.15 else 0)
    put(row, 0x98, 'I', r.choice([0, 0, OBJ98]))
    put(row, 0x4c, 'I', r.choice([0, 0, 0, OBJ4C, font]))
    put(row, 0x48, 'I', (r.getrandbits(32) & ~1) | r.choice([0, 1, 1]))
    put(row, 0x60, 'I', r.choice([0, PARENTS, PARENTS]))
    for k in range(2):   # (the parent and its parent: shown or not, the last without a parent)
        parent = put(bytearray(r.bytes(0x80)), 0x48, 'I', (r.getrandbits(32) & ~1) | r.choice([0, 1, 1, 1]))
        regions[PARENTS + 0x80 * k] = put(parent, 0x60, 'I', r.choice([0, PARENTS + 0x80]) if k == 0 else 0)
    regions[ROW] = row
    regions[OLD] = put(put(bytearray(r.bytes(0x20)), 0, 'I', VTOLD), 0x10, 'HH', r.choice([0, 1, 2]), r.choice([0, 1, 2]))
    regions[VTOLD] = vtable({5: 1})
    regions[OBJ98] = put(bytearray(r.bytes(0x20)), 0, 'I', VT98)
    regions[VT98] = vtable({0: 1, 3: 2})
    blob_bytes(log, 3, 1, 0)   # (its slot 3's event: a stack pointer, logged as a hash of none of its bytes; windownotify.py checks them)
    regions[OBJ4C] = r.bytes(0x20)
    regions[0xfaa5c0] = struct.pack('<hhh', r.choice([1536, 1536, 768, 0x600 + r.randrange(-3, 4)]), 0, r.choice([2048, 2048, 1024, 0x800 + r.randrange(-3, 4)]))
    regions[0x87a6d0] = struct.pack('<I', REND)
    regions[REND] = put(bytearray(r.bytes(0x40)), 4, 'I', 0)
    return Call(CALLS[op], [ROW], regions, ret='v')


def case(r):
    op = r.deal(OPS)
    if op.endswith('InitRow'):
        return init_case(r, op)

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
