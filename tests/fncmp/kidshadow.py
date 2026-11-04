# The kids' shadow texture loaded (src/game/render/weatherfx: 0x550930 KidShadow_LoadTexture) against the original: the sprite queue [0x8c47a0] or none, SpriteQueue_LoadTexture a recorder (k 20).
# usage: kidshadow.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region  # noqa: E402

LOAD = 20
B = Bench([(0x550930, 0x550945)], hooks={0x58a5c0: _recorder(LOAD, 1)})


def case(r):
    regions = {LOG: log_region(), RAM: r.bytes(0x10), 0x8c47a0: struct.pack('<I', r.choice([0, RAM, RAM]))}
    return Call(0x550930, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
