# Dialog7cc2ec_Open (src/game/ui/dialog7cc2ec, 0x4b2750) against the original. crt_atexit fncmp._cdecl_log, HoverLabel_Attach 21 and Things_AverageOfKind 22 (stdcall) recorders: their own benches have them. dialog7ca308open.py's new
# window with the script at 0x756400: 0-2 plain children, then the dialog with its five bars (0x705af0, each with a child 0x2e8), the figures 0x2de (eight parts; missing now and then), the label 0x2da and 0x2dd, the script empty now and then
# (nothing built); dialog7ca308open.py's rings, for the three rings here at their own places; the guard and the mode setting; the dialog open now and then. dialogbuild.long_open(): its calls run past the log's 40.
# usage: dialog7cc2ecopen.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import advisor  # noqa: E402
import dialog7ca150  # noqa: E402
import dialogbuild  # noqa: E402
import panelparts  # noqa: E402
import scriptchain  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, _recorder, heap, put, recorder, set_return  # noqa: E402

OP = 'Dialog7cc2ec_Open'
dialog7ca150.TABS[OP] = (0x756400, 6, 0x752930, 0xac)
TEXTS = [0x93, 0x165, 0x166, 0x167, 0x169]
B = Bench(dialog7ca150.merged(dialog7ca150.ALL_RANGES + dialog7ca150.OPEN_RANGES + [
    (0x4b2750, 0x4b32fe), (0x65d7bc, 0x65d840), (0x519450, 0x519490), (0x519510, 0x519550), (0x66a5c4, 0x66abcf), (0x6697e3, 0x6698b4), (0x6693d0, 0x669407)]),
    hooks={**panelparts.animatormesh.HOOKS, 0x67a7d0: _cdecl_log, 0x485d20: _recorder(21, 3, cdecl=True), 0x4c9c00: _recorder(22, 1, True)})
RINGS, TABLE = 0x33100000, RAM + 0x61000
BARS = (0x2e7, 0x2ec, 0x2ea, 0x2eb, 0x2ed)
FIGURES = (0x2df, 0x2e0, 0x2e1, 0x2e2, 0x2e3, 0x2e4, 0x2e5, 0x2e6)


def script(r):
    top = []
    kids = list(BARS) + [0x2da, 0x2dd, 'figures'] + ['decoy'] * r.choice([0, 0, 1])
    if r.random() < 0.05:
        kids.remove('figures')
    r.shuffle(kids)
    for k in kids:
        if k == 'figures':
            inner = []
            for f in r.sample(FIGURES, len(FIGURES)):
                dialogbuild.child(r, inner, f, kind=1)
            dialogbuild.child(r, top, 0x2de, inner + [5], 160, 1)
        elif k == 'decoy':
            dialogbuild.child(r, top, r.choice([0, 1, r.getrandbits(32) | 0x1000]))
        elif k in BARS:
            inner = []
            dialogbuild.child(r, inner, 0x2e8, kind=1)
            dialogbuild.child(r, top, k, inner + [5], 160, 12)
        else:
            dialogbuild.child(r, top, k, kind=1)
    w = []
    for _ in range(r.choice([0, 0, 1, 2])):
        dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]))
    dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]), top + [5], 160, 1)
    return w + [5] * 4


def ring(r):
    """the ring's values (one in eight the first's, so differences of 0), position, count and flag"""
    count = r.choice([0, -1, 1, 2, 5, 32])
    position = r.choice([-1, -2, 0, 1, count - 1, -count]) if count > 0 else r.choice([0, 1, -1])
    data = bytearray(r.bytes(0x24c))
    put(data, 0x240, 'iiB', position, count, r.choice([0, 0, 1, r.getrandbits(8)]))
    return data


def case(r):
    regions = dialog7ca150.list_dialog(r, OP, lambda r, regions: TEXTS)
    w = script(r) if r.random() < 0.97 else [5] * 4
    regions[0x756400] = struct.pack('<%dH' % len(w), *w)
    log = regions[LOG]
    dialogbuild.long_open(log)
    regions[HEAP] = heap(0x28000, r.choice([0] * 40 + [r.randrange(1, 16)]))
    regions[0x7cc2ec] = struct.pack('<I', r.choice([0] * 12 + [r.getrandbits(32)]))
    regions[0x7cc2d4] = r.bytes(16 + 4)
    regions[0x7cb290] = r.bytes(4)   # (Global7cb28c_Select(3, ...)'s dword)
    regions[0xfb34c9] = bytes([r.choice([0, 1, 1, 1, r.getrandbits(8)])])
    regions[0xfb3b7c] = struct.pack('<I', r.choice([2, 2, 0, 1, 3, r.getrandbits(32)]))
    regions[0x7a1a8c] = struct.pack('<I', r.getrandbits(32) & ~(7 << 23) | r.choice([0, 1, 2, 4, 6]) << 23)
    regions[0x7cf83c] = struct.pack('<I', advisor.WORLD - 0x1da71c)
    world = bytearray(r.bytes(8))
    a, b = r.sample(range(0x100), 2)
    a, b = a + 0x100, b + 0x100
    put(world, 0, 'H', a)
    regions[advisor.WORLD - 4] = struct.pack('<H', b)
    regions[advisor.WORLD + 4] = world
    regions[0x7cfb90 + 20 * a] = struct.pack('<I', RINGS)
    regions[0x7cfb90 + 20 * b] = struct.pack('<I', TABLE)
    regions[TABLE] = r.bytes(0x40)
    for at in (0x1f5a4, 0x1f7f4, 0x1fc94):
        regions[RINGS + at] = ring(r)
    set_return(log, 22, r.choice([0, 1, 50, 99, 100, -1, -100, r.randrange(256), r.getrandbits(32)]))
    for vt in dialogbuild.KIND_VTABLES:   # (the label's slot 9)
        regions[vt + 0x24] = struct.pack('<I', recorder(30, 1))
    return Call(0x4b2750, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
