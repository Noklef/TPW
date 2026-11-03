# The version text's window (src/game/ui/versionlabel: VersionLabel_Create 0x48b020) against the original. The CRT's swprintf real, the heap heap()'s; recorders (driver.cpp's kVersionLabelEnv): GuiText_Measure 10 (the rect hashed, the box filled), Window_CreateChild 11, TextPart_Ctor 12,
# Window_SetImpl 13, Window_Set4c 14, Window_NotifyImpl100 15 (the text hashed).
# usage: versionlabel.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob, blob_bytes, fill, heap, log_region, put, set_return, strings  # noqa: E402
from window705950draw import INTERLOCKED, LOCALE, interlocked  # noqa: E402

HOOKS = {0x4929f0: _recorder(10, 7, cdecl=True), 0x65dd7b: _recorder(11, 7), 0x48f6a0: _recorder(12, 1), 0x65d1c8: _recorder(13, 1), 0x65c388: _recorder(14, 1), 0x65f5b7: _recorder(15, 1)}
HOOKS.update({at: interlocked(step) for at, (_, step) in INTERLOCKED.items()})
B = Bench([(0x48b020, 0x48b175), (0x67a400, 0x690000), (0x6918f0, 0x691a30), (0x691ba0, 0x691ce0)], hooks=HOOKS)
WINDOW, FRONT, OTHER = RAM + 0x100, RAM + 0x200, RAM + 0x300
SCALES = [0x400, 0x800, 0x300, 0x1000, 1, 2, -2, 3, 0x7fff, -0x8000]


def case(r):
    log = log_region()
    box = [r.randrange(-0x100, 0x100), r.randrange(-0x100, 0x100), r.randrange(0x7d00), r.randrange(0x7d00)]
    if r.random() < 0.1:
        box = [r.getrandbits(32) for _ in box]
    strings(log, 10, 1, 1 << 6)
    blob(log, 10, 1, 0xf)
    fill(log, 10, 6, struct.pack('<4I', *[b & 0xffffffff for b in box]), which=0)
    set_return(log, 11, WINDOW)
    set_return(log, 12, HEAP + 0x20)
    blob_bytes(log, 15, 0, 0xfff)
    imports = bytearray(0x400)
    for at, (slot, _) in INTERLOCKED.items():
        put(imports, slot - IMPORT_TABLE, 'I', at)
    regions = {LOCALE: bytes(4), IMPORT_TABLE: imports, 0xfb3ca4: bytes(8), LOG: log, HEAP: heap(fail=r.choice([0, 0, 0, 1])), WINDOW: r.bytes(0x20), FRONT: r.bytes(4), OTHER: struct.pack('<2I', 0, r.getrandbits(32)), 0xfaa5d8: struct.pack('<I', FRONT), 0xf82884: struct.pack('<I', OTHER),
               0xfaa5c0: struct.pack('<h', r.choice(SCALES)), 0xfaa5c4: struct.pack('<h', r.choice(SCALES)), 0x78b534: struct.pack('<I', r.randrange(8))}
    return Call(0x48b020, [r.deal([0, 1, 0x100, 0xff, r.getrandbits(32)])], regions)


if __name__ == '__main__':
    B.main(case)
