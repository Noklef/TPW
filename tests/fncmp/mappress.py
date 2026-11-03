# The game mode's map press (src/game/world/mappress: GameMode_MapPress 0x524790) against the original: the gates' globals dealt, mode 0x16 mostly ([0x81ae2c];
# the mode tests the real 0x52f860), its calls recorders (the emulator's hooks= on their addresses, the driver's MapPressEnv), so the log holds them in order.
# The lines' ends written through their out arguments (fill) or not; a row of 0x200 makes the cell wrap at the word. The world [0x7cf83c] mapped where the
# 0x28-byte cells of ids 0..0x90 lie.
# usage: mappress.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, _recorder, fill, log_region, set_return, strings  # noqa: E402

B = Bench([(0x524790, 0x52495a), (0x52f860, 0x52f872)],
          hooks={0x5234d0: _recorder(30, 3, cdecl=True), 0x403c20: _recorder(31, 3), 0x403c80: _recorder(31, 4), 0x5276d0: _recorder(10, 1, cdecl=True),
                 0x52fbf0: _recorder(20, 2, cdecl=True), 0x536100: _recorder(0, 5, cdecl=True), 0x445a70: _recorder(23, 2, cdecl=True),
                 0x446300: _recorder(15, 1, cdecl=True)})
WORLD = 0x31000000
CELLS = WORLD + 0x6cde * 0x28
IDS = 0x92


def case(r):
    log = log_region()
    x, y = r.randrange(8), r.choice([0, 1, 0x200, 0x201])
    strings(log, 0, 0, out=0x18)   # (the lines' end pointers: on the stack)
    set_return(log, 23, r.getrandbits(32))
    if r.random() < 0.6:
        fill(log, 0, 3, struct.pack('<4i', *[r.randrange(8) for _ in range(4)]), which=0, records=4)
    if r.random() < 0.6:
        fill(log, 0, 4, struct.pack('<4i', *[r.choice([0, 1, 0x200, 0x201]) for _ in range(4)]), which=1, records=4)
    mode = r.choice([0x16] * 8 + [0x38, 0x38, 0, 1, 0x15, r.getrandbits(32)])
    regions = {LOG: log, CELLS: r.bytes(IDS * 0x28), 0x7cf83c: struct.pack('<I', WORLD), 0x7639a0: r.bytes(4), 0x81ae2c: struct.pack('<I', mode)}
    for at in (0x785a2c, 0x8187f8):
        regions[at] = struct.pack('<I', r.choice([0, 0, 1, r.getrandbits(32)]))
    regions[0x816d48] = struct.pack('<I', r.choice([0, 0, 0, 0, 1, r.getrandbits(32)]))
    regions[0x816d50] = r.bytes(4)
    regions[0x8186d8] = r.bytes(1)
    regions[0x8186b4] = struct.pack('<II', r.choice([0, 1, 0x200, 0x201]), r.randrange(8))   # (the kept point: y, x)
    regions[0x818c54] = bytes([r.choice([0, 1, 1, 0xff])])
    flags = r.choice([0, 0, 0x10000, 1, r.getrandbits(32)])
    return Call(0x524790, [x, y, flags, r.choice([0, 0, 1, r.getrandbits(32)])], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
