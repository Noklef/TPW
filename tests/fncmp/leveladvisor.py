# Bench: Level_LoadTerrainAndAdvisor 0x457a90: the seed drawn, the terrain's texture argument from the detail flags, the advisors' paths, and the calls in order. The CRT runs.
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import LOG, RAM, RAND_SEED, Bench, Call, _recorder, log_region, set_return, strings  # noqa: E402
from texdirs import merged  # noqa: E402

VERTS, MOTION, CAMERA, BASE, TERRAIN, ADVISOR, INIT, LIGHT, TINTS = 14, 17, 18, 19, 20, 21, 22, 23, 24
B = Bench(merged([(0x457a90, 0x457c26), (0x5c5520, 0x5c5530), (0x619240, 0x619244), (0x67a400, 0x690000)]),
          hooks={0x462870: _recorder(VERTS, 1, cdecl=True), 0x4740d0: _recorder(MOTION, 1, cdecl=True), 0x42a990: _recorder(CAMERA, 0, cdecl=True), 0x40ef40: _recorder(BASE, 0, cdecl=True),
                 0x4504c0: _recorder(TERRAIN, 2, cdecl=True), 0x429ba0: _recorder(ADVISOR, 4, cdecl=True), 0x5989c0: _recorder(INIT, 0, cdecl=True), 0x4584d0: _recorder(LIGHT, 0, cdecl=True),
                 0x469350: _recorder(TINTS, 0, cdecl=True), 0x5da3c0: lambda e: None})
BASE_TEXT, NAME, LANGUAGE = RAM, RAM + 0x40, RAM + 0x80


def word(r):
    return bytes(r.choice(b'ab\\XYZ') for _ in range(r.randrange(1, 20))) + bytes(1)


def case(r):
    log = log_region()
    set_return(log, BASE, r.choice([BASE_TEXT] * 7 + [0]))
    strings(log, ADVISOR, 7)
    regions = {LOG: log, RAND_SEED: r.bytes(4), 0x7b0958: r.bytes(4), 0x79d86c: struct.pack('<I', r.deal([0, 1, 2, 3, 4, 5, 6, 7]) | r.getrandbits(32) & ~7), BASE_TEXT: word(r), NAME: word(r),
               LANGUAGE: word(r), 0xf816f0: struct.pack('<I', NAME), 0x786b68: r.bytes(8) + struct.pack('<I', LANGUAGE), 0x7af650: word(r)}
    return Call(0x457a90, [], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
