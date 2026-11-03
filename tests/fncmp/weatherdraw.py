# One lightning bolt into a batch (WeatherFx_DrawBolt, src/game/render/weatherfx: 0x57ec50) against the original: Batch_Begin (k 1), Batch_TransformVertices (k 2), Batch_Close (k 3),
# WeatherFx_BoltSubdivide (k 4) and Ribbon_AddPoint (k 5) are recorders. The world: the lightning's texture none, without an id, or with one and UV floats (now and then NaNs); each call
# answering 0 now and then, so that each exit is taken.
# usage: weatherdraw.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return  # noqa: E402

B = Bench([(0x57ec50, 0x57ed49)], hooks={0x565240: _recorder(1, 5), 0x565f50: _recorder(2, 1), 0x5658c0: _recorder(3, 1), 0x57e810: _recorder(4, 2), 0x57dd10: _recorder(5, 2)})
FX, TEX = RAM, RAM + 0x10000


def case(r):
    log = log_region()
    fx = bytearray(r.bytes(0xa1b0))
    texture = bytearray(r.bytes(0x40))
    kind = r.choice(['none', 'plain', 'id', 'id', 'id', 'id', 'id0'])
    flags = r.getrandbits(32)
    put(texture, 8, 'I', flags & ~0x1000 if kind == 'plain' else flags | 0x1000)
    if kind == 'id0':
        put(texture, 0x1c, 'I', 0)
    wild = r.random() < 0.1
    for off in (0x20, 0x24, 0x28, 0x2c):
        put(texture, off, 'f' if not wild else 'I', r.uniform(-2, 2) if not wild else r.choice([0x7f800001, 0xff800001, 0x7fc00000, r.getrandbits(32)]))
    put(fx, 0x334c, 'I', 0 if kind == 'none' else TEX)
    for k, chance in ((1, 0.1), (4, 0.1), (5, 0.1)):
        if r.random() < chance:
            set_return(log, k, 0)
    if r.random() < 0.3:
        set_return(log, 3, r.choice([0, 1, r.getrandbits(32)]))
    regions = {LOG: log, FX: fx, TEX: texture, 0x879ab0: struct.pack('<I', r.getrandbits(32))}
    return Call(0x57ec50, [FX, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
