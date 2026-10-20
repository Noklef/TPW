# The index converter and the 16-bit converter's destructors (src/game/gfx/pixconv: 0x63a7b0-0x63ae0d, 0x63ae60-0x63ae86, thiscall) against the
# originals: the index converter made, set up and destroyed (its table allocation failing, or a table already built from this palette, so
# the inverse map is never built here: tests/fncmp/invmap.py benches it), Convert / ConvertBack from a converter the original's Init left
# (table lookups through a random table, or the nearest-colour search), Nearest on random palettes; the destructors with flags any, freed
# into the heap region.
# usage: indexconv.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,DelDtor,Dtor,Init,FreeTable,Cv,Back,Nearest,R16Del,R16Dtor)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, global_heap, heap, put  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pixconv import fmt, pack  # noqa: E402

B = Bench([(0x63a7b0, 0x63ae0e), (0x63ae60, 0x63ae87), (0x63acb0, 0x63ad80), (0x642860, 0x6428a5), (0x5f5660, 0x5f568c), (0x67acb0, 0x67acb1),
           (0x67acc0, 0x67acc1)])
OBJ, FORMAT, PAL, SRC, DST, TABLE = RAM, RAM + 0x800, RAM + 0x1000, RAM + 0x1800, RAM + 0x1c00, RAM + 0x10000
COUNT = 0xfaa534   # (the counted allocations' count)
CALLS = {'Ctor': 0x63a7b0, 'DelDtor': 0x63a810, 'Dtor': 0x63a830, 'Init': 0x63a880, 'FreeTable': 0x63aa20, 'Cv': 0x63aa40, 'Back': 0x63ab70,
         'Nearest': 0x63ad80, 'R16Del': 0x63ae60, 'R16Dtor': 0x63ae80}
M = 0xffffffff


def palette(r):
    pal = bytearray(r.bytes(0x400))
    if r.random() < 0.3:   # (repeats: ties keep the first)
        for i in range(0, 0x400, 4):
            pal[i:i + 4] = pal[(i % 16):(i % 16) + 4]
    return pal


def state(r, obj, regions):
    """Init's preconditions: a table already built from this palette, or none and its allocation failing."""
    if r.random() < 0.4:
        put(obj, 0x404, '2I', PAL, TABLE)
        regions[TABLE] = r.bytes(0x8000)
    else:
        put(obj, 0x408, 'I', 0)
    regions[HEAP] = heap(fail=1)


def case(r):
    op = r.deal(OPS)
    address = CALLS[op]
    obj = bytearray(r.bytes(0x43c))
    regions = {OBJ: obj, PAL: palette(r), COUNT: r.bytes(4)}
    regions.update(global_heap())
    if op in ('R16Del', 'DelDtor') or op in ('R16Dtor', 'Dtor', 'FreeTable'):
        if op in ('DelDtor', 'Dtor', 'FreeTable'):
            put(obj, 0x408, 'I', r.choice([0, TABLE, TABLE]))
        regions[HEAP] = heap()
        if op in ('R16Del', 'DelDtor'):
            return Call(address, [OBJ, r.choice([0, 1, 1, 2, 3, M, r.getrandbits(32)])], regions, conv='t', ret='a')
        return Call(address, [OBJ], regions, conv='t', ret='v')
    if op == 'Nearest':
        return Call(address, [OBJ, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='b')
    f = fmt(r, r.choice([15, 16, 24, 32, 32]))
    regions[FORMAT] = pack(f)
    if op == 'Ctor':
        regions[HEAP] = heap(fail=1)
        return Call(address, [OBJ, FORMAT, PAL], regions, conv='t', ret='a')
    state(r, obj, regions)
    if op == 'Init':
        return Call(address, [OBJ, FORMAT, PAL], regions, conv='t', ret='v')
    # Convert / ConvertBack: the converter as the original's Init left it
    init = Call(0x63a880, [OBJ, FORMAT, PAL], regions, conv='t', ret='v')
    out = B.original(init)
    regions[OBJ] = bytes.fromhex(out[3 + list(init.regions).index(OBJ)])
    count = r.choice([0, 1, 2, 3, 5, 8, 16])
    regions[SRC] = r.bytes(4 * count + 8)
    regions[DST] = r.bytes(count + 8)
    return Call(address, [OBJ, DST, SRC, count], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
