# Dialog7cc4b4_Update (src/game/ui/dialog7cc4b4update, 0x4b9340) against the original. Recorders (Dialog7cc4b4UpdateEnv): GlobalSave_Construct 12, 0x5b09c0 13 (the String's text hashed), GlobalSave_Instance 14, GlobalSave_CountSetLevel 15 and the theme's field 16 (answers in turn from tables), Window_SetShown 17, Window_Notify1 18,
# Window_GetRect 23 (it fills the rectangle), Particle_Spawn 19, ParticleSystem_End 20, swprintf 21, Dialog7cc4b4_SetFlag 22; crt_atexit fncmp._cdecl_log; the children's slots 5 and 9 vtable recorders. String's functions, Window_FindChild, the setting's read and GuiRect real. The dialog's children by hand.
# usage: dialog7cc4b4update.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, _recorder, answers, blob_bytes, fill, heap, log_region, put, set_return, strings, vtable  # noqa: E402

B = Bench([(0x4b9340, 0x4b9839), (0x65d7bc, 0x65d840), (0x5f60b0, 0x5f6160), (0x5f76c0, 0x5f7720), (0x5f61e0, 0x5f6210),(0x550ca0, 0x550d80), (0x659d00, 0x659d53), (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aaa0), (0x66bff7, 0x66c01b), (0x5da3c0, 0x5da3c1)],
          hooks={0x67a7d0: _cdecl_log, 0x5aef50: _recorder(12, 0), 0x5b09c0: _recorder(13, 1), 0x409410: _recorder(14, 0, cdecl=True), 0x5af680: _recorder(15, 0), 0x5b11a0: _recorder(16, 0), 0x65d9dd: _recorder(17, 1), 0x65d3a3: _recorder(18, 1), 0x65d42e: _recorder(23, 2), 0x521e60: _recorder(19, 4, cdecl=True),
                 0x51feb0: _recorder(20, 1, cdecl=True), 0x67af00: _recorder(21, 3, cdecl=True), 0x4b9840: _recorder(22, 1, cdecl=True)})
DIALOG, KIDS, VT, GAME, GAME4, GAMEC, NAME, THEME, SAVES, LEVELS, FIELDS, PANEL_KIDS, SECOND_KIDS = (RAM + 0x1000 * k for k in range(13))
TOP_IDS = (0x1e0ea, 0x1e0ec, 0x1e0f0, 0x1e0f1)


def chain(r, base, ids, kids=None):
    out = bytearray(r.bytes(0x100 * len(ids)))
    for k, ident in enumerate(ids):
        put(out, 0x100 * k, 'I', VT)
        put(out, 0x100 * k + 0x54, 'I', base + 0x100 * (k + 1) if k + 1 < len(ids) else 0)
        put(out, 0x100 * k + 0x8c, 'I', ident)
        if kids and ident in kids:
            put(out, 0x100 * k + 0x5c, 'I', kids[ident])
    return out


def case(r):
    log = log_region()
    strings(log, 23, 0, out=0b1)
    fill(log, 23, 0, struct.pack('<4h', *[r.choice([0, 1, 100, -100, 0x7fff, -0x8000, r.randrange(-0x8000, 0x8000)]) for _ in range(4)]), offset=4)
    blob_bytes(log, 13, 0, 0, inner=(8, 0x7f))
    set_return(log, 12, SAVES)
    set_return(log, 13, 0 if r.random() < 0.1 else THEME)
    set_return(log, 14, SAVES)
    answers(log, 15, LEVELS)
    answers(log, 16, FIELDS)
    set_return(log, 19, r.getrandbits(32))
    ids = list(TOP_IDS)
    r.shuffle(ids)
    regions = {LOG: log, HEAP: heap(0x8000, r.choice([0] * 30 + [1, 2])), DIALOG: put(bytearray(r.bytes(0x100)), 0x5c, 'I', KIDS), KIDS: chain(r, KIDS, ids, {0x1e0ea: PANEL_KIDS, 0x1e0ec: SECOND_KIDS}), PANEL_KIDS: chain(r, PANEL_KIDS, [0x1e0eb]), SECOND_KIDS: chain(r, SECOND_KIDS, [0x1e0ee]), VT: vtable({5: 1, 9: 1}),
               NAME: bytes(r.randrange(1, 0x80) for _ in range(r.choice([0, 1, 5, 20]))) + b'\0' * 32, THEME: r.bytes(16), SAVES: r.bytes(16),
               LEVELS: struct.pack('<40I', *[r.choice([0, 1, 2, 3, 0xffffffff, 8]) for _ in range(40)]), FIELDS: struct.pack('<40I', *[r.choice([0, 1, 2, 3, 0x80000000, 8]) for _ in range(40)]),
               GAMEC: put(bytearray(r.bytes(0x40)), 0x30, 'I', NAME), GAME4: put(bytearray(r.bytes(0x20)), 0xc, 'I', r.choice([GAMEC] * 8 + [0])), GAME: put(bytearray(r.bytes(0x20)), 4, 'I', r.choice([GAME4] * 8 + [0])),
               0x7cc4b4: struct.pack('<II', 0 if r.random() < 0.05 else DIALOG, r.getrandbits(32)), 0x7cc4bc: struct.pack('<I', r.choice([0, 0, r.getrandbits(32)])), 0xf85614: struct.pack('<I', r.choice([GAME] * 8 + [0])), 0xf7c58c: struct.pack('<I', r.choice([SAVES] * 4 + [0])),
               0xfb34bd: bytes([r.choice([0, 1, 1, 1, r.getrandbits(8)])]), 0xfb3b7c: struct.pack('<I', r.choice([0, 1, 1, 2, 3, r.getrandbits(32)])), 0x7a1a8c: r.bytes(4)}
    return Call(0x4b9340, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
