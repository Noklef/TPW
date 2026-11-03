# A map cell's sprite effect (src/game/world/celleffect: MapCell_StartEffect, 0x4d93b0) against the original. Recorders (their own benches have them; driver.cpp's kCellEffectEnv): Setting_ReadMode 10, atexit 11, Map_SpreadAdd 12 (its cell a word), SpriteScript_Start 13, World_SetSeed 14, World_Rand 15 (answers in turn),
# SpriteScript_SetPosition 16, Particle_Spawn 17. The debug prints are the empty 0x5da3c0.
# usage: celleffect.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, answers, log_region, put, set_return  # noqa: E402

B = Bench([(0x4d93b0, 0x4d95d3), (0x5da3c0, 0x5da3c1)],
          hooks={0x550ca0: _recorder(10, 0), 0x67a7d0: _recorder(11, 1, cdecl=True), 0x4d8440: _recorder(12, 2), 0x475a10: _recorder(13, 7, cdecl=True), 0x516370: _recorder(14, 1), 0x516330: _recorder(15, 0), 0x475480: _recorder(16, 5, cdecl=True), 0x521e60: _recorder(17, 4, cdecl=True)})
CELL, ANSWERS = RAM, RAM + 0x100


def case(r):
    log = log_region()
    struct.pack_into('<I', log, LOG_BYTES + 4 * 12, 0b10 << 16)
    set_return(log, 13, r.getrandbits(32))
    set_return(log, 17, r.getrandbits(32))
    answers(log, 15, ANSWERS)
    cell = bytearray(r.bytes(0x44))
    put(cell, 0, 'H', r.choice([1, 0x81, 0x4000, 0x3fff, r.randrange(1, 0x4001), r.randrange(1, 0x4001), 0, 0xffff, r.getrandbits(16)]))
    put(cell, 0x2c, 'I', r.choice([0, 0, 0, 0, 0, 1, r.getrandbits(32)]))
    kind = r.choice(list(range(10)) + [7, 8, 8, 0xffffffff, r.getrandbits(32)])
    regions = {LOG: log, CELL: cell, ANSWERS: struct.pack('<40I', *[r.choice([0, 8, 9, 17, r.getrandbits(32)]) for _ in range(40)]), 0x8023a0: r.bytes(4), 0x80239c: r.bytes(4), 0xfb34b9: bytes([r.choice([0, 1, 1, 1, r.getrandbits(8)])]),
               0xfb3b7c: struct.pack('<I', r.choice([0, 0, 0, 2, 1, r.getrandbits(32)]))}
    return Call(0x4d93b0, [CELL, kind], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
