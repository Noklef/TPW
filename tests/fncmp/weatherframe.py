# The weather's frame (src/game/render/weatherfx: 0x57ff60 WeatherFx_Frame) against the original, __ftol real; the clock's time (k 8), the bolts (k 5), the streaks (k 6) and the drops (k 7) recorders. The gap since the last frame
# about its cap of 500, negative or any; the pause bit; the bolts failing; no drops; the slow bit; the camera's direction and the matrix; the wind and the place, with places of exactly 4 and -4, at the wrap, NaNs and huge numbers now and then;
# the mask's bits 0 and 1, the drawing failing.
# usage: weatherframe.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return  # noqa: E402

BOLTS, STREAKS, DROPS, CLOCK = 5, 6, 7, 8
B = Bench([(0x57ff60, 0x58031f), (0x67a830, 0x67a860)], hooks={0x57ee30: _recorder(BOLTS, 0), 0x57efd0: _recorder(STREAKS, 3), 0x57f490: _recorder(DROPS, 3), 0x402d70: _recorder(CLOCK, 0)})
FX, CAM, MATRIX = RAM, RAM + 0x10000, RAM + 0x10100
NANS = [0x7fc12345, 0x7f812345, 0xffc00000, 0x7f800000, 0xff800000]


def fnum(r, lo, hi, odd=0.01):
    """A float's bytes: in the range, or a NaN or a huge number now and then."""
    if r.random() < odd:
        return struct.pack('<I', r.choice(NANS)) if r.random() < 0.6 else struct.pack('<f', r.choice([1, -1]) * 10.0 ** r.randrange(10, 39))
    return struct.pack('<f', r.uniform(lo, hi))


def case(r):
    log = log_region()
    now = r.getrandbits(32)
    gap = r.choice([0, 1, 16, 33, 499, 500, 501, 1000, -1, -500, r.randrange(1 << 31), -r.randrange(1 << 31)])
    fields = bytearray(r.bytes(0x48))
    put(fields, 0x18, 'I', r.choice([0, 1, 5, r.getrandbits(32)]))         # +0x3318: the drops
    for off in (0x1c, 0x20, 0x24, 0x38, 0x3c):                             # +0x331c..: the wind; +0x3338, +0x333c: the speeds
        fields[off:off + 4] = fnum(r, -30, 30)
    exact = r.random() < 0.25
    for off in (0x28, 0x2c, 0x30):                                         # +0x3328..: the place
        fields[off:off + 4] = struct.pack('<f', r.choice([4.0, -4.0, 3.5, -3.5, 0.0])) if exact else fnum(r, -5, 5)
    fields[0x40:0x44] = fnum(r, -2, 2)                                     # +0x3340: the scale
    put(fields, 0x44, 'I', (now - gap) & 0xffffffff)                       # +0x3344: the last time
    camera = bytearray(r.bytes(0x40))
    for off in (0x20, 0x24, 0x28, 0x30, 0x34, 0x38):
        camera[off:off + 4] = fnum(r, -1000, 1000)
    set_return(log, CLOCK, now)
    set_return(log, BOLTS, r.choice([1, 1, 1, 1, 1, 1, 1, 0, 0x100]))
    for k in (STREAKS, DROPS):
        set_return(log, k, r.choice([1, 1, 0, 0x100]))
    pause = r.choice([0, 0, 4, r.getrandbits(32)])
    if exact:
        pause |= 4                                                          # (no time, no move: the place meets the wrap's comparisons as it is)
    regions = {LOG: log, FX: put(bytearray(r.bytes(0x10)), 0, 'I', r.getrandbits(32)), FX + 0x3300: fields, FX + 0x9d58: r.bytes(0x50), FX + 0xa1bc: r.bytes(0x10), CAM: camera, MATRIX: r.bytes(0x40),
               0x87afe8: struct.pack('<I', CAM), 0x87b154: struct.pack('<I', MATRIX), 0x8bd508: struct.pack('<I', pause), 0x790ab0: struct.pack('<I', r.choice([0, 0, 1, 2, 4, 8, 0x10, 0x16, 0xe9, r.getrandbits(32)]))}
    return Call(0x57ff60, [FX], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
