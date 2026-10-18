# The 0x7cc4b4 dialog's particle effect toggle (src/game/ui/dialogs: 0x4b8ee0) against the original: on or off (the byte; upper bits
# random), the system [0x7cc4bc] none, a system's handle (its serial) or random; the window's rectangle +8 and insets random (small or edges); the
# particle state as particles.py's Spawn cases (effect 0x61 among the definitions, the set-up mostly on).
# usage: dialogeffect.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import RAM, Bench, Call, put  # noqa: E402
import particles  # noqa: E402

B = Bench(particles.RANGES + [(0x4b8ee0, 0x4b8fc6), (0x65d42e, 0x65d49e), (0x65a43e, 0x65a493), (0x65aa20, 0x65aa90), (0x659d00, 0x659d43),
                              (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00), (0x66bff7, 0x66c00b)])
WINDOW = RAM + 0x30000


def case(r):
    regions = dict(particles.spawn(r).regions)
    win = bytearray(r.bytes(0x120))
    for k in list(range(0xc, 0x14, 2)) + list(range(0xf8, 0x100, 2)):
        put(win, k, 'H', r.choice([0, 1, 2, 5, 0x7fff, 0x8000, 0xffff, r.randrange(-50, 50) & 0xffff, r.getrandbits(16)]))
    put(win, 8, 'I', 0x705920)
    regions[WINDOW] = win
    slot = r.randrange(6)
    serial = struct.unpack_from('<H', regions[0x80cefc], 0x140 * slot + 0xa)[0]
    handle = r.choice([0, 0, serial << 16 | slot, serial << 16 | slot, r.getrandbits(32)])   # (a system's own serial: alive)
    regions[0x7cc4bc] = struct.pack('<I', handle)
    on = r.choice([0, 1, 1, 0x80, 0x100, r.getrandbits(32)])
    return Call(0x4b8ee0, [WINDOW, on], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
