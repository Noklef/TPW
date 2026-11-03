# Dialog7ca210_Open (src/game/ui/dialog7ca210, 0x498d80) and Thing_NameBuffer7ba058 (0x486280) against the originals. crt_atexit fncmp._cdecl_log, HoverLabel_Attach 21 and 0x4e2780 22 (the name's copy, thiscall: agent A's) recorders. dialog7ca308open.py's
# new window with the script at 0x751798: 0-2 plain children, then the dialog with 0x4f3ac, 0x4f3ae (0x705af0), 0x4f3af, 0x4f3b0, 0x4f3b1 (0x705a50) (none missing: not checked, so a fault in both); the dialog open now and then; the guard and the mode
# setting; the word +0x1da732's table entry, the money's +0x118, the flag +0x1da710; advisor.py's PostId for 0x12f. dialogbuild.long_open(): its calls run past the log's 40.
# usage: dialog7ca210open.py [CASES [SEED]] [--wild|--both] [--show N]
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
import window705a50  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, _recorder, heap, log_region, put, recorder, set_return  # noqa: E402

OP = 'Dialog7ca210_Open'
dialog7ca150.TABS[OP] = (0x751798, 6, 0x752930, 0xac)
TEXTS = [0xa0]
B = Bench(dialog7ca150.merged(dialog7ca150.ALL_RANGES + dialog7ca150.OPEN_RANGES + window705a50.RANGES + [
    (0x486280, 0x4862a0), (0x498d80, 0x499051), (0x65d7bc, 0x65d840), (0x5195d0, 0x519610), (0x51a280, 0x51a2c0), (0x66a5c4, 0x66abcf), (0x6697e3, 0x6698b4), (0x6693d0, 0x669407), (0x668bec, 0x668c99)]),
    hooks={**panelparts.animatormesh.HOOKS, 0x67a7d0: _cdecl_log, 0x485d20: _recorder(21, 3, cdecl=True), 0x4e2780: _recorder(22, 2)})
MONEY = RAM + 0x61000
THING = 0x33100000


def script(r):
    top = []
    kids = [(0x4f3ac, 1), (0x4f3ae, 12), (0x4f3af, 1), (0x4f3b0, 1), (0x4f3b1, 2)] + [(r.choice([0, 1, r.getrandbits(32) | 0x1000]), None)] * r.choice([0, 0, 1])
    r.shuffle(kids)
    for ident, kind in kids:
        dialogbuild.child(r, top, ident, kind=kind)
    w = []
    for _ in range(r.choice([0, 0, 1, 2])):
        dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]))
    dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]), top + [5], 160, 1)
    return w + [5] * 4


def open_case(r):
    regions = dialog7ca150.list_dialog(r, OP, lambda r, regions: TEXTS)
    w = script(r)
    regions[0x751798] = struct.pack('<%dH' % len(w), *w)
    log = regions[LOG]
    dialogbuild.long_open(log)
    regions[HEAP] = heap(0x28000, r.choice([0] * 40 + [r.randrange(1, 16)]))
    regions[0x7ca210] = struct.pack('<I', r.choice([0] * 12 + [r.getrandbits(32)]))
    regions[0x7ca1e8] = r.bytes(4)
    regions[0x7ca208] = r.bytes(4)
    regions[0xfb34c5] = bytes([r.choice([0, 1, 1, 1, r.getrandbits(8)])])
    regions[0xfb3b7c] = struct.pack('<I', r.choice([2, 2, 0, 1, 3, r.getrandbits(32)]))
    masks = scriptchain.advisor_env(r, regions, 0x12f)
    a, b = r.sample(range(0x100, 0x200), 2)
    regions[advisor.WORLD + 0x16] = struct.pack('<H', a)
    regions[advisor.WORLD + 0xa] = struct.pack('<H', b)
    regions[advisor.WORLD - 0xc] = struct.pack('<I', r.choice([0, 0, 1, r.getrandbits(32)]))
    regions[0x7cfb90 + 20 * a] = struct.pack('<I', THING + a)
    regions[0x7cfb90 + 20 * b] = struct.pack('<I', MONEY)
    regions[MONEY + 0x118] = r.bytes(4)
    for vt in dialogbuild.KIND_VTABLES:   # (the label's slot 9)
        regions[vt + 0x24] = struct.pack('<I', recorder(30, 1))
    return Call(0x498d80, [], regions, ret='v', masks=masks)


def name_case(r):
    log = log_region()
    set_return(log, 22, r.getrandbits(32))
    return Call(0x486280, [r.choice([0, 1, THING, r.getrandbits(32)])], {LOG: log}, ret='a')


def case(r):
    return (open_case if r.deal([1] * 19 + [0]) else name_case)(r)


if __name__ == '__main__':
    B.main(case)
