# The highlight update of src/game/core/model_slot_selection (0x598ca0) against the original: scene.py's slot environment (a holder with parts, the slot table) with the highlight
# globals random: kinds -1 to 14 and others, the flag bit 0x10000 either way, key lists at 0xf797a4 / 0xf797b4 that name parts of the holder or not.
# usage: model_slot_selection.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import scene  # noqa: E402
from fncmp import Bench, Call  # noqa: E402

B = Bench([(0x598ca0, 0x598d90), (0x429ee0, 0x429fd4), (0x429f60, 0x429fe0), (0x429fe0, 0x429ff4), (0x44a590, 0x44b140), (0x44b140, 0x44b2e0), (0x44b3c0, 0x44b410), (0x44b4c0, 0x44b505), (0x475550, 0x47558b)], sines=256)


def case(r):
    saved = scene.OPS
    scene.OPS = [r.choice(['SlotClear', 'SlotSet'])]
    try:
        c = scene.case(r)
    finally:
        scene.OPS = saved
    key, slot = c.args
    regions = dict(c.regions)
    kind = r.choice([-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, key, r.getrandbits(32)])
    keys = [r.choice([0, key, r.getrandbits(32), 0x13]) for _ in range(8)]
    regions[0xf797a4] = struct.pack('<8I', *keys[:4], *keys[4:])  # 0xf797a4 .. 0xf797c4: clear keys then set keys
    regions[0xf79660] = struct.pack('<I', r.choice([0, 0x10000, r.getrandbits(32)])) + bytes(0x10) + struct.pack('<I', slot) + bytes(8)
    return Call(0x598ca0, [kind & 0xffffffff], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
