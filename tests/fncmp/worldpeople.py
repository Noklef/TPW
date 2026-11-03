# The world's people at the start and each tick (src/game/world/worldpeople: 0x4d7af0, 0x4d7b20) against the originals. Recorders as driver.cpp's kWorldPeopleEnv. The original's call of the empty 0x5da3c0 is left out by the port.
# usage: worldpeople.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region  # noqa: E402

B = Bench([(0x4d7af0, 0x4d7b12), (0x4d7b20, 0x4d7b67), (0x5da3c0, 0x5da3c1)],
          hooks={0x507bb0: _recorder(10, 0), 0x4f85a0: _recorder(11, 0), 0x4cf3d0: _recorder(12, 0), 0x5084f0: _recorder(13, 0), 0x4f8260: _recorder(14, 0), 0x4d4a00: _recorder(15, 0)})
WORLD, HOLDER, TICKS = RAM + 0x100, RAM + 0x200, RAM + 0x300


def case(r):
    log = log_region()
    ticks = r.deal([0, 100, 200, 99, 101, 1000, 0xffffffff, 0xfffffff0, r.getrandbits(32)])
    regions = {LOG: log, 0x7cf6ec: struct.pack('<I', HOLDER), HOLDER: struct.pack('<I', TICKS - 0x1da70c & 0xffffffff), TICKS: struct.pack('<I', ticks)}
    return Call(r.deal([0x4d7af0, 0x4d7b20, 0x4d7b20]), [WORLD], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
