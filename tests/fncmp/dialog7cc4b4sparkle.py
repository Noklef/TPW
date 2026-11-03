# The sparkle on the dialog [0x7cc4b4] (src/game/ui/dialog7cc4b4sparkle: Dialog7cc4b4_Sparkle, 0x4b8fd0) against the original. Recorders (driver.cpp's kDialog7cc4b4SparkleEnv): Setting_ReadMode 10, atexit 11, Window_GetRect 12 (it fills the rectangle), Particle_Spawn 13, Sound_PlayAt 14. Window_FindChild and GuiRect's run.
# usage: dialog7cc4b4sparkle.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fill, log_region, put, strings  # noqa: E402

B = Bench([(0x4b8fd0, 0x4b90e6), (0x65d7bc, 0x65d840), (0x659d00, 0x659d53), (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aaa0), (0x66bff7, 0x66c01b)],
          hooks={0x550ca0: _recorder(10, 0), 0x67a7d0: _recorder(11, 1, cdecl=True), 0x65d42e: _recorder(12, 2), 0x521e60: _recorder(13, 4, cdecl=True), 0x51bfc0: _recorder(14, 6, cdecl=True)})
DIALOG, KID = RAM, RAM + 0x100


def node(r, ident, kid=0):
    out = bytearray(r.bytes(0x100))
    put(out, 0x54, 'I', 0)
    put(out, 0x5c, 'I', kid)
    put(out, 0x8c, 'I', ident)
    return out


def case(r):
    log = log_region()
    strings(log, 12, 0, out=1)
    fill(log, 12, 0, struct.pack('<4h', *[r.choice([0, 1, 100, -100, 0x7fff, -0x8000, r.randrange(-0x8000, 0x8000)]) for _ in range(4)]), offset=4)
    regions = {LOG: log, DIALOG: node(r, 1, KID), KID: node(r, 0x1e0ea), 0x7cc4b4: struct.pack('<I', 0 if r.random() < 0.05 else DIALOG), 0xfb34bd: bytes([r.choice([0, 1, 1, 1, r.getrandbits(8)])]),
               0xfb3b7c: struct.pack('<I', r.choice([0, 1, 1, 2, 3, r.getrandbits(32)])), 0x803a48: r.bytes(4), 0x7a1a8c: r.bytes(4)}
    return Call(0x4b8fd0, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
