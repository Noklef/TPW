# A person's thought shown (src/game/world/personscripts Thoughts_Show, 0x50be80) against the original: thoughts 0 to 0x17 and others, with
# force or not, the mode setting 0xfb3b7c (1 or not) read or not (its guard 0xfb34aa), the flags 0x790ab0, a bubble script (+0x84) or none,
# the world's clock ([[0x7cf6ec]] + 0x1da70c) around +0x8c plus the thought's wait (wrapping too). Setting_ReadMode, atexit (0x67a7d0),
# SpriteScript_Kill and SpriteScript_Start are recorders (the emulator's hooks= on their addresses, the driver's ThoughtsEnv: driver.cpp).
# usage: thoughtshow.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region  # noqa: E402

THOUGHTS, POINTER, WORLD = RAM + 0x100, RAM + 0x200, RAM + 0x10000
WAITS = [1, 1, 1, 1, 0, 0, 1, 3, 3, 3, 3, 0, 0, 1, 0, 0, 0, 3, 2, 0, 0, 0]   # (thoughts 1 to 0x16)

B = Bench([(0x50be80, 0x50c075), (0x5da3c0, 0x5da3c1)],
          hooks={0x550ca0: _recorder(0, 0), 0x67a7d0: _recorder(1, 1, cdecl=True), 0x475550: _recorder(2, 1, cdecl=True),
                 0x475a10: _recorder(3, 7, cdecl=True)})
THOUGHTS_DEALT = list(range(0x18)) + list(range(1, 0x17))


def case(r):
    thought = r.deal(THOUGHTS_DEALT) if r.random() < 0.95 else r.choice([0x17, 0xff, 0x100 | r.randrange(1, 0x17), r.getrandbits(32)])
    force = r.choice([0, 1, 0, 1, 0x100, r.getrandbits(32)])
    thoughts = bytearray(r.bytes(0x90))
    struct.pack_into('<I', thoughts, 0x84, r.choice([0, 0, r.randrange(1, 64), r.getrandbits(32)]))
    last = r.choice([0, r.randrange(100), r.getrandbits(32), 0xffffffff - r.randrange(60)])
    struct.pack_into('<I', thoughts, 0x8c, last)
    wait = 20 * WAITS[(thought & 0xff) - 1] if 1 <= thought <= 0x16 else 20
    clock = (last + r.choice([wait, wait - 1, wait + 1, 0, r.randrange(80), r.getrandbits(32)])) & 0xffffffff
    world = struct.pack('<I', clock)
    regions = {
        LOG: log_region(), THOUGHTS: thoughts, POINTER: struct.pack('<I', WORLD - 0x1da70c), WORLD: world,
        0x7cf6ec: struct.pack('<I', POINTER),
        0x790ab0: bytes([r.choice([0, 0, 0, 0, 1, 2, 4, 0x10, 0x20, 0xe9, r.getrandbits(8)])]),
        0xfb34aa: bytes([r.choice([1, 1, 1, 0, 2, 3, r.getrandbits(8)])]),
        0xfb3b7c: struct.pack('<I', r.choice([0, 1, 1, 2, r.getrandbits(32)])),
    }
    return Call(0x50be80, [THOUGHTS, thought, force], regions, conv='t')


if __name__ == '__main__':
    B.main(case)
