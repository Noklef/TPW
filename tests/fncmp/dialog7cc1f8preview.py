# Dialog7cc1f8_ShowPreview (src/game/ui/dialog7cc1f8preview, 0x4ab1b0) against the original. Recorders, as Dialog7cc1f8PreviewEnv's from 10 (their own benches have them): ObjectDefs_Find 10, GlobalSave_HasKey 11, Sign_PlaceOverRect 12 (the rectangle's words hashed), Window_ImplSlot7 13 (it fills the width and
# height), ImplSlot6 14, Format_FillInfo 15, Display_SetSurfaceInfo 16 (the fields the constructor sets hashed), Display_BoxFill 17 (the rectangle hashed, the index byte), MapGrid_ClassifyTypes 18 (its answer a grid), Records_DataIfTerminated 19, Language_Widen 20, Language_WideText 21; the label's slot 9 a vtable recorder.
# The windows by hand, the panel missing now and then; GuiRect and the window's calls real; the log folded (the boxes pass its 40).
# usage: dialog7cc1f8preview.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, blob, blob_bytes, fill, log_fold, log_region, put, set_return, strings, vtable  # noqa: E402

B = Bench([(0x4ab1b0, 0x4ab692), (0x65d7bc, 0x65d840), (0x5fa680, 0x5fa6a0), (0x659d00, 0x659d53), (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aaa0), (0x66bff7, 0x66c01b), (0x65d4f6, 0x65d520)],
          hooks={0x412e90: _recorder(10, 1), 0x4d4b70: _recorder(11, 1, True), 0x486410: _recorder(12, 3, cdecl=True), 0x65d6db: _recorder(13, 2), 0x65d745: _recorder(14, 0), 0x4925d0: _recorder(15, 4, cdecl=True), 0x5f9cb0: _recorder(16, 1, cdecl=True),
                 0x5f9a80: _recorder(17, 3, cdecl=True), 0x52c5b0: _recorder(18, 1, cdecl=True), 0x6ace60: _recorder(19, 1), 0x485ac0: _recorder(20, 1, cdecl=True), 0x485b00: _recorder(21, 1, cdecl=True)})
ID, DIALOG, PANEL, KIDS, DEF, GRID, VT = (RAM + 0x1000 * k for k in range(1, 8))


def case(r):
    log = log_region()
    log_fold(log)
    struct.pack_into('<I', log, LOG_BYTES + 4 * 10, 0b1 << 16)   # (the id's upper half: the stack's)
    struct.pack_into('<I', log, LOG_BYTES + 4 * 11, 0b1 << 16)
    struct.pack_into('<I', log, LOG_BYTES + 4 * 17, 0b100)       # (the colour's index byte; its padding the stack's)
    blob_bytes(log, 12, 0, 0xff0)
    strings(log, 13, 0, out=0b11)
    width, height = r.choice([64, 80, 96, 128, 7, 0, -9, r.randrange(1, 300)]), r.choice([64, 80, 96, 128, 7, 0, -9, r.randrange(1, 300)])
    fill(log, 13, 0, struct.pack('<i', width), which=0)
    fill(log, 13, 1, struct.pack('<i', height), which=1)
    strings(log, 15, 0, out=0b10)
    blob(log, 16, 0, 0b1111110000)
    blob(log, 17, 0, 0b1111)
    set_return(log, 10, DEF)
    set_return(log, 11, r.choice([0, 1, 0x100, 0x101, r.getrandbits(32)]))
    set_return(log, 13, r.choice([0, RAM, RAM, RAM, r.getrandbits(32) | 1]))
    set_return(log, 18, GRID)
    for k in (14, 19, 20, 21):
        set_return(log, k, r.getrandbits(32))
    columns, rows = r.choice([0, 1, 2, 3, 5, 8]), r.choice([0, 1, 2, 4, 8])
    grid = bytearray(0x410)
    for column in range(max(columns, 8)):
        for row in range(0x10):
            put(grid, column * 0x40 + row * 4, 'I', r.choice([0, 0, 1, 2, 3, 4, 5, 0x100, r.getrandbits(32)]))
    put(grid, 0x400, 'II', columns, rows)
    kids = bytearray(r.bytes(0x300))
    ids = [0x1eb, 0x1ec, r.getrandbits(32) | 0x1000]
    r.shuffle(ids)
    for k, ident in enumerate(ids):
        put(kids, 0x100 * k, 'I', VT)
        put(kids, 0x100 * k + 0x54, 'I', KIDS + 0x100 * (k + 1) if k + 1 < len(ids) else 0)
        put(kids, 0x100 * k + 0x8c, 'I', ident)
    panel = bytearray(r.bytes(0x100))
    put(panel, 0x5c, 'I', KIDS)
    put(panel, 0xe0, 'h', r.choice([0, 1, -5, 100, r.getrandbits(15)]))
    dialog = put(put(bytearray(r.bytes(0x100)), 0x5c, 'I', PANEL if r.random() < 0.97 else 0), 0, 'I', r.getrandbits(32))
    panel[0x54:0x58] = bytes(4)
    put(panel, 0x8c, 'I', 0x1ea)
    definition = bytearray(r.bytes(0x500))
    put(definition, 0xc4, 'i', r.choice([0, -1, 1, 5, r.getrandbits(31)]))
    regions = {LOG: log, ID: struct.pack('<H', r.choice([0, 1, 2, 0x7fff, 0xffff, 0xfffe, 0x8000, r.getrandbits(16)])), 0x7cc1f8: struct.pack('<I', DIALOG), DIALOG: dialog, PANEL: panel, KIDS: kids, DEF: definition,
               GRID: grid, VT: vtable({9: 1}), 0x78bad0: r.bytes(4), 0x7cf83c: r.bytes(4)}
    return Call(0x4ab1b0, [ID], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
