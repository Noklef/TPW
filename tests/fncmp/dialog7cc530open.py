# Dialog7cc530_Open (src/game/ui/dialog7cc530open, 0x4ba980) against the original. Recorders, as Dialog7cc530OpenEnv's: Dialog_Create 10 (it answers the window built here), HoverLabel_SetYellow 11,
# RadioGroup_Select 12, BuildRows 27, SetColumnBottom 14, GuiWindows_FindByName 15, SetHighlight 16, ButtonText_Init 17, DialupList_Fill 18, Language_WideText 19 by address (its answers in the logged
# arguments). The windows: dialogbuild.chain()'s row with its children 0xb, 0xc (0x10, 0x11) and 0xd (0xe, 0xf) among decoys, in any order; the windows' slot 9 recorded. An allocation fails now and then.
# usage: dialog7cc530open.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialogbuild  # noqa: E402
import listrows  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, put, recorder, set_return, vtable  # noqa: E402

B = Bench(dialogbuild.ALL_RANGES + [(0x4ba980, 0x4bacb9), (0x65d7bc, 0x65d840), (0x65c4b9, 0x65c55a)],
          hooks={0x47ed80: _recorder(10, 3, cdecl=True), 0x492d20: _recorder(11, 1, cdecl=True), 0x66a295: _recorder(12, 1), 0x6639cb: _recorder(27, 2), 0x6636b2: _recorder(14, 2), 0x659bc9: _recorder(15, 1, cdecl=True),
                 0x662eee: _recorder(16, 1), 0x486660: _recorder(17, 7, cdecl=True), 0x4ba850: _recorder(18, 1, cdecl=True), 0x485b00: _recorder(19, 1, cdecl=True)})
ROW = dialogbuild.ROW
GROUP, LIST = RAM + 0x20000, RAM + 0x21000


def link(r, regions, parent, base, ids):
    """the windows at base, of those ids in any order, parent's children"""
    ids = list(ids)
    r.shuffle(ids)
    for k, ident in enumerate(ids):
        w = dialogbuild.kid(r, parent)
        at = base + dialogbuild.KID * k
        put(w, 0x8c, 'I', ident)
        put(w, 0x54, 'I', at + dialogbuild.KID if k + 1 < len(ids) else 0)
        put(w, 0x5c, 'I', 0)
        regions[at] = w
    return ids


def case(r):
    regions = dialogbuild.chain(r, 'Dialog7cc530_Open', 0)
    log = regions[LOG]
    dialogbuild.long_open(log)
    regions[dialogbuild.KIDVT] = vtable({5: 1, 7: 1, 9: 1})
    regions[HEAP] = heap(0x20000, r.choice([0] * 12 + [r.randrange(1, 5)]))
    regions[0x6ff320 + 0xc] = struct.pack('<I', recorder(3, 2))   # (the text parts told: game code the driver cannot run)
    regions[0x6ff2d8 + 0xc] = struct.pack('<I', recorder(3, 2))
    regions[0x6ff2d8 + 0x14] = struct.pack('<I', recorder(31, 1))
    decoys = [r.getrandbits(32) | 0x1000 for _ in range(r.choice([0, 0, 1]))]
    tops = link(r, regions, ROW, dialogbuild.KIDS, [0xb, 0xc, 0xd] + decoys)
    put(regions[ROW], 0x5c, 'I', dialogbuild.KIDS)
    put(regions[dialogbuild.KIDS + dialogbuild.KID * tops.index(0xd)], 0x5c, 'I', GROUP)
    put(regions[dialogbuild.KIDS + dialogbuild.KID * tops.index(0xc)], 0x5c, 'I', LIST)
    link(r, regions, dialogbuild.KIDS, GROUP, [0xe, 0xf])
    link(r, regions, dialogbuild.KIDS, LIST, [0x10, 0x11])
    set_return(log, 10, ROW)
    set_return(log, 15, r.getrandbits(32))
    set_return(log, 19, r.getrandbits(32))
    regions[0x7cc52c] = r.bytes(4)
    regions[0x7cc530] = r.bytes(4)
    regions[0x7cc53c] = r.bytes(1)
    regions[0x7cc540] = r.bytes(1)
    return Call(0x4ba980, [r.getrandbits(32)], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
