# The camera's step each frame (src/game/world/cameraupdate: 0x42b1c0) against the original, with its matrix library and the C library's fmod and modf; recorders (driver.cpp's kCameraUpdateEnv): 1 the clock,
# 2 the keys (answers in turn), 3 the blocked steps, 4 the ride objects (answers in turn), 5 the definition, 6 the follow, 7 the ground's height, 8 whether the matrices are close. Each camera mode, the keys, the
# mouse in and past its dead zones, points near cell edges and map edges; now and then a NaN. Whether the matrices are close is the recorder's answer, as a real match needs this frame's result beforehand.
# usage: cameraupdate.py [CASES [SEED]] [--wild|--both] [--show N]
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, fb, log_fold, log_region, put, set_float_return, set_return  # noqa: E402

HOOKS = {0x402d80: _recorder(1, 0), 0x40ca20: _recorder(2, 1), 0x4d8750: _recorder(3, 4, cdecl=True), 0x42a340: _recorder(4, 2, cdecl=True), 0x412e90: _recorder(5, 1), 0x4e15b0: _recorder(6, 1),
         0x452560: _recorder(7, 3, cdecl=True), 0x42b070: _recorder(8, 2, cdecl=True)}
B = Bench([(0x42b1c0, 0x42cdc0), (0x46f150, 0x470a00), (0x578800, 0x578e00), (0x67a400, 0x690000)], hooks=HOOKS, sines=256)
G = 0x7908e0
SIZE = 0x790ae8 - G
VIEW, EXTENT, TARGET, PART, MAP, OBJECT, HELD, BLOCKED, RIDES = (RAM + 0x100 * k for k in range(9))
FLAGS = [0, 0, 0, 0x100, 0x1000, 0x800, 2, 2, 2, 2, 2, 2 | 0x800, 2 | 0x1000, 2 | 0x100, 4, 4 | 0x800, 0x10, 8, 0x20, 0x80, 0x40, 2 | 0x40]
KEYS = [0, 0, 0, 4, 4, 8, 0x10, 0x20, 0x40, 0x80, 0x84, 0x280, 0x100, 0x200, 0x204]
OFFSET = [0.0, 0.0, 0.1, -0.1, 0.3, -0.3, 0.5, -0.5, 0.97, -0.97, 2.0, -2.0]


def f(r, nan, lo, hi):
    return r.choice([0x7fc00000, 0xffc00000, 0x7fa00000]) if r.random() < nan else fb(r.uniform(lo, hi))


def table(r, values):
    return struct.pack('<64I', *(r.deal(values) for _ in range(64)))


def case(r):
    log = log_region()
    log_fold(log)
    nan = r.deal([0, 0, 0, 0, 0, 0.01])
    g = bytearray(struct.pack('<%dI' % (SIZE // 4), *(f(r, nan, -1, 1) for _ in range(SIZE // 4))))
    now = r.getrandbits(32)
    set_return(log, 1, now)
    answers(log, 2, HELD)
    answers(log, 3, BLOCKED)
    answers(log, 4, RIDES)
    set_float_return(log, 7, r.choice([0.0, r.uniform(-5, 50), r.uniform(-5, 50), float('nan') if nan else 1.0]))
    set_return(log, 8, r.deal([0, 0, 1]))

    def s(address, fmt, *values):
        put(g, address - G, fmt, *values)

    s(0x7908e8, 'I', r.deal([0, 0, 0, 1]))
    s(0x7909e4, 'I', (now - r.deal([0, 1, 16, 33, 100, 250, r.getrandbits(32)])) & 0xffffffff)
    s(0x790ab0, 'I', r.deal(FLAGS))
    keys = r.deal(KEYS)
    s(0x790aac, 'I', keys)
    s(0x7909d0, 'I', r.choice([keys, r.deal(KEYS + [8, 0x10, 0x18])]))
    s(0x790a30, 'I', PART)
    s(0x790a98, 'I', r.choice([0x790990, 0x7909f0]))
    # The point: on cell edges (each 10), near them, anywhere, past the map.
    cell = lambda: r.randrange(0, 128) * 10.0   # noqa: E731
    near = lambda: r.deal([cell(), cell() + r.uniform(-0.2, 0.2), r.uniform(0, 1280), r.uniform(-5, 1290)])   # noqa: E731
    place = lambda: r.choice([0x7fc00000, 0xffc00000, 0x7fa00000]) if r.random() < nan else fb(near())   # noqa: E731
    s(0x7908f0, '3I', place(), f(r, nan, -3, 3), place())
    s(0x7909ec, 'I', fb(r.deal([110.0, r.uniform(30, 320), r.uniform(30, 320), 40.0, 300.0, 70.0, 130.0])))
    s(0x790a38, 'I', f(r, nan, -7, 7))
    s(0x7909e0, 'I', r.choice([f(r, nan, -7, 7), struct.unpack_from('<I', g, 0x790a38 - G)[0]]))
    speed = lambda: fb(r.deal([0.0, 0.0, 5e-5, -5e-5, r.uniform(-0.1, 0.1), r.uniform(-0.1, 0.1), r.uniform(-1, 1)]))   # noqa: E731
    s(0x790a88, '3I', speed(), speed(), speed())
    s(0x790adc, '2I', fb(r.uniform(-0.6, 0.6)), fb(r.uniform(-0.6, 0.6)))
    s(0x790ae4, 'I', fb(r.uniform(-5, 50)))
    mouse = r.uniform(-3, 3), r.uniform(-3, 3)
    mx, my = r.deal(OFFSET), r.deal(OFFSET)
    half = r.choice([1.0, 320.0, 400.0, 512.0, 0.5]), r.choice([1.0, 240.0, 300.0, 384.0, 0.5])   # (the screen's half size: the game sets it with the window)
    s(0x790ac8, '2I', f(r, nan, mouse[0] + mx * half[0], mouse[0] + mx * half[0]), f(r, nan, mouse[1] - my * half[1], mouse[1] - my * half[1]))
    extent = bytearray(r.bytes(0x40))
    put(extent, 0x10, '2I', fb(10.0), fb(r.choice([10.0, 10.0, 5.0])))
    put(extent, 0x18, '2I', r.choice([128, 128, 64]), r.choice([128, 128, 64, 0x10080]))   # (the last: the high word the definition's id keeps)
    put(extent, 0x28, '2I', r.deal([0, 1, 1, 1]), r.deal([0, 1, 1, 1]))
    regions = {G: g, 0x78d910: bytes([r.deal([0, 0, 1]), 0, r.deal([0, 0, 1]), 0]), 0x7a1a14: bytes([r.deal([0, 8, 8]), 0, 0, 0]), 0x7a1c54: struct.pack('<2I', fb(mouse[0]), fb(mouse[1])), 0x74d104: struct.pack('<2I', fb(half[0]), fb(half[1])),
               0x7a0854: struct.pack('<I', VIEW), 0x787c14: struct.pack('<I', MAP), 0x87afe8: struct.pack('<I', TARGET), 0x7a0314: struct.pack('<I', r.choice([128, 64])), 0x7a0374: struct.pack('<I', r.choice([128, 64])),
               0xfb1330: b'\1\0\0\0', VIEW: put(bytearray(r.bytes(0x70)), 0x6c, 'I', EXTENT), EXTENT: extent, TARGET: r.bytes(0x40), PART: struct.pack('<16I', *(f(r, nan, -2, 2) for _ in range(16))), MAP: r.bytes(8),
               OBJECT: r.bytes(0x20), HELD: table(r, [0, 0, 0, 0, 0, 1, 0x10000]), BLOCKED: table(r, [0, 0, 0, 1, 0x100]), RIDES: table(r, [0] * 30 + [OBJECT]), LOG: log}
    return Call(0x42b1c0, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
