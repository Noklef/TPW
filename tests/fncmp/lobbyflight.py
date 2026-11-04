# A lobby model set flying between two random points (src/game/frontend/lobbyflight: 0x5d9830) against the original: the model's new, slot and place recorders, the owner's slot 1 a recorder; the generator's seed and the vectors of the case, NaNs and equal points (spread 0: no direction) among them.
# usage: lobbyflight.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fb, log_region, put, set_return, vtable  # noqa: E402

NEW, SLOT, PLACE = 4, 5, 6
B = Bench([(0x5d9830, 0x5d9b1f)], hooks={0x5d8870: _recorder(NEW, 6), 0x5d83f0: _recorder(SLOT, 2), 0x5d8190: _recorder(PLACE, 1)})
SELF, OWNER, VTABLE, CENTRE, SPREAD, GENERATOR = (RAM + 0x100 * k for k in range(6))


def vector(r, scale):
    buf = bytearray(12)
    r.floats(buf, [0, 4, 8], scale)
    return buf


def case(r):
    log = log_region()
    centre = vector(r, r.choice([1.0, 100.0, 5000.0]))
    spread = vector(r, r.choice([1.0, 10.0, 1000.0])) if r.randrange(8) else bytes(12)
    if r.randrange(16) == 0:
        put(r.choice([centre, spread]) if isinstance(spread, bytearray) else centre, r.choice([0, 4, 8]), 'I', r.choice([0x7fc00001, 0x7f800001, 0x7f800000]))
    for buf in [centre] + ([spread] if isinstance(spread, bytearray) else []):   # (one coordinate 0 or any float)
        if r.randrange(4) == 0:
            put(buf, r.choice([0, 4, 8]), 'I', r.choice([fb(0.0), r.f()]))
    set_return(log, NEW, r.getrandbits(32))
    regions = {LOG: log, SELF: r.bytes(0x60), OWNER: struct.pack('<I', VTABLE), VTABLE: vtable({1: 1}), CENTRE: centre, SPREAD: spread, GENERATOR: r.bytes(0x38),
               0xf83774: r.bytes(4), 0xf82884: struct.pack('<I', GENERATOR)}
    return Call(0x5d9830, [SELF, OWNER, r.getrandbits(32), r.getrandbits(32), CENTRE, SPREAD, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
