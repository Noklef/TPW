# The dial-up connection let go (src/game/ui/dialuphangup: 0x4b9f30, 0x4bae40) against the originals: the import slot 0x6fd5b8 a recorder.
# usage: dialuphangup.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, log_region, recorder  # noqa: E402

B = Bench([(0x4b9f30, 0x4b9f5e), (0x4bae40, 0x4bae62)])


def case(r):
    regions = {LOG: log_region(), 0x6fd5b8: struct.pack('<I', recorder(10, 1)), 0x7cc538: struct.pack('<I', r.deal([0, 1, 2, 3, r.getrandbits(32)])), 0x7cc540: struct.pack('<B', r.deal([0, 1, 0x80])), 0x7cc50c: r.bytes(4)}
    if r.random() < 0.6:
        return Call(0x4b9f30, [r.getrandbits(32)], regions, ret='a')
    return Call(0x4bae40, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
