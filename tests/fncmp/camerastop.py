# The camera's following stopped (src/game/world/camera: Camera_StopFollowing 0x42a190) against the original: Camera_FollowThing (k 10; out by a fill) and Camera_SnapToRide (k 11) recorders.
# usage: camerastop.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, _recorder, fill, log_region, strings  # noqa: E402

B = Bench([(0x42a190, 0x42a1c5)], hooks={0x42a560: _recorder(10, 6, cdecl=True), 0x42a1d0: _recorder(11, 0, cdecl=True)})


def case(r):
    log = log_region()
    strings(log, 10, 0, out=1 << 3)
    fill(log, 10, 3, struct.pack('<I', r.choice([0, 1, r.getrandbits(32)])))
    return Call(0x42a190, [], {LOG: log, 0x790ab0: bytes([r.getrandbits(8) | r.choice([0, 4])]), 0x790a44: r.bytes(4)}, ret='v')


if __name__ == '__main__':
    B.main(case)
