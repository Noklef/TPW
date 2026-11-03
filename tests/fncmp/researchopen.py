# ResearchPanel_Open (src/game/ui/levelmeter, 0x4aa480) against the original. Recorders, as ResearchOpenEnv's: crt_atexit fncmp._cdecl_log, the message box 0x47f020 15 (agent B's; its record's set bytes hashed: +7 stays stack garbage), SoundUi_Play 16,
# GameTable7cfb90_Get1da720_519510 17, Global7852f8_Times78548c 18, Things_SumChain 19, GameTable7cfb90_Get1da722_519550 25, ShowName 21, ShowProgress 22, Update 23, ResearchLab_GroupEffort 24. dialog7ca308open.py's new window with the script at 0x754490: the slider 0xd2d10 (0x705b20, a child
# 0xd2d12; present: it is not checked again later), the sliders 0xd2d0b-f and the meters 0xd2d01, 7, 8, 9, a (children 0xd2d05, 0xd2d06) each missing now and then; the script empty now and then (nothing built); [0x7c2534] a source now and then; the panel open now
# and then; the guard and the mode setting; the word +0x1da742; advisor.py's PostId for 0xc8. dialogbuild.long_open(): its calls run past the log's 40.
# usage: researchopen.py [CASES [SEED]] [--wild|--both] [--show N]
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
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, _recorder, blob_bytes, heap, put, recorder, set_return  # noqa: E402

OP = 'ResearchPanel_Open'
dialog7ca150.TABS[OP] = (0x754490, 6, 0x752930, 0xac)
TEXTS = [0x1d3, 0x1d4]
B = Bench(dialog7ca150.merged(dialog7ca150.ALL_RANGES + dialog7ca150.OPEN_RANGES + [
    (0x4aa480, 0x4aaeb8), (0x65d7bc, 0x65d840), (0x66b47d, 0x66b508), (0x477870, 0x477bf6), (0x477c00, 0x477c30), (0x477cc0, 0x477d19), (0x477d20, 0x477d70), (0x4786f0, 0x478715), (0x477d70, 0x477dae),
    (0x477db0, 0x477fef), (0x478400, 0x4786e8), (0x476c50, 0x476e80), (0x4700f0, 0x4702d0), (0x46fc00, 0x46fcb0), (0x48c8d0, 0x48c8f0), (0x65d3a3, 0x65d40c)]),
    hooks={**panelparts.animatormesh.HOOKS, 0x67a7d0: _cdecl_log, 0x47f020: _recorder(15, 8, cdecl=True), 0x485aa0: _recorder(16, 1, cdecl=True), 0x519510: _recorder(17, 0),
           0x4c9ed0: _recorder(18, 0, cdecl=True), 0x4c9e60: _recorder(19, 0, cdecl=True), 0x519550: _recorder(25, 0), 0x4aa120: _recorder(21, 1, cdecl=True), 0x4a9bd0: _recorder(22, 1, cdecl=True), 0x4a9c70: _recorder(23, 1, cdecl=True),
           0x503850: _recorder(24, 1)})
LAB = RAM + 0x61000
SLIDERS = (0xd2d0b, 0xd2d0c, 0xd2d0d, 0xd2d0e, 0xd2d0f)
METERS = (0xd2d01, 0xd2d07, 0xd2d08, 0xd2d09, 0xd2d0a)


def meter(r, top, ident):
    inner = []
    for k in r.sample((0xd2d05, 0xd2d06), 2):
        dialogbuild.child(r, inner, k, kind=1)
    dialogbuild.child(r, top, ident, inner + [5], 160, 1)


def script(r):
    top = []
    kids = [0xd2d10] + [k for k in SLIDERS + METERS if r.random() < 0.9] + ['decoy'] * r.choice([0, 0, 1])
    r.shuffle(kids)
    for k in kids:
        if k == 0xd2d10:
            inner = []
            dialogbuild.child(r, inner, 0xd2d12, kind=1)
            dialogbuild.child(r, top, k, inner + [5], 160, 3)
        elif k in SLIDERS:
            dialogbuild.child(r, top, k, kind=3)
        elif k == 'decoy':
            dialogbuild.child(r, top, r.choice([0, 1, r.getrandbits(32) | 0x1000]))
        else:
            meter(r, top, k)
    w = []
    for _ in range(r.choice([0, 0, 1, 2])):
        dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]))
    dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]), top + [5], 160, 1)
    return w + [5] * 4


def case(r):
    regions = dialog7ca150.list_dialog(r, OP, lambda r, regions: TEXTS)
    w = script(r) if r.random() < 0.97 else [5] * 4
    regions[0x754490] = struct.pack('<%dH' % len(w), *w)
    log = regions[LOG]
    dialogbuild.long_open(log)
    regions[HEAP] = heap(0x28000, r.choice([0] * 40 + [r.randrange(1, 16)]))
    regions[0x7cc1d8] = struct.pack('<2I', r.choice([0] * 12 + [r.getrandbits(32)]), r.getrandbits(32))
    regions[0xfb34be] = bytes([r.choice([0, 1, 1, 1, r.getrandbits(8)])])
    regions[0xfb3b7c] = struct.pack('<I', r.choice([2] + [0, 1, 3, r.getrandbits(32)] * 4))
    regions[0x6fee88 + 0xc] = struct.pack('<I', recorder(3, 2))   # (the bar mesh's slots 3 and 5; panelparts' for the overlay's vtable)
    regions[0x6fee88 + 0x14] = struct.pack('<I', recorder(5, 1))
    masks = scriptchain.advisor_env(r, regions, 0xc8)
    regions[advisor.WORLD + 0x26] = struct.pack('<H', r.choice([0] + [1, r.getrandbits(16) | 1] * 5))
    regions[0x754bc8] = struct.pack('<I', r.choice([7] * 6 + list(range(13))))
    regions[0x7cc1b4] = r.bytes(12)
    regions[0x7cc1c0] = r.bytes(4)
    regions[0x7cc1c8] = r.bytes(8)
    regions[0x7cc1d0] = r.bytes(4)
    regions[0x7cc1bc] = r.bytes(1)
    if r.random() < 0.08:
        source = RAM + 0x62000
        regions[source] = r.bytes(0x40)
        regions[0x7c2534] = struct.pack('<I', source)
    regions[LAB + 0x1460] = struct.pack('<i', r.choice([0x46, 100, 0, r.randrange(200), r.getrandbits(32) - (1 << 31)]))
    set_return(log, 25, LAB)
    set_return(log, 18, r.choice([1, 2, 10, 100, 1024, r.getrandbits(31) | 1, -r.randrange(1, 1000) & 0xffffffff]))
    set_return(log, 19, r.choice([0, 50, 100, r.randrange(200), r.getrandbits(32)]))
    set_return(log, 16, r.getrandbits(32))
    set_return(log, 17, r.getrandbits(32))
    set_return(log, 24, r.getrandbits(32))
    blob_bytes(log, 15, 0, 0xff7f)
    for vt in dialogbuild.KIND_VTABLES:   # (the label's slot 9)
        regions[vt + 0x24] = struct.pack('<I', recorder(30, 1))
    return Call(0x4aa480, [], regions, ret='v', masks=masks)


if __name__ == '__main__':
    B.main(case)
