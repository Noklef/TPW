# The advisor's model loaded into a model slot (src/game/model/slots: 0x429ba0) against the original, in rideload.py's world: the slot
# (0x790398 + 0xb0 each, 0-3) of random bytes but for its part's vbtable (+0x48: the image's 0x6fdd00, the bases at +8 and +0xc) and the
# renderer object's vtable at the part's +8 (0x6fdcc8, ModelSlot_Ctor's; its slot 2 Stub_One); a
# folder (not empty), the third directory or none, a suffix or none (the name "Advisor%s.md2" / "Advisor.md2"); the scene flags
# (0x8bd508, SceneFlags_Touch with the stamp 0x8bcc40); the renderer [0x87a6d0] a pool of free nodes with empty slot lists (its tag
# 0x10 list takes the part's base, renderer.py's lists).
# usage: modelslot_load.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import rideload  # noqa: E402
import texdirs  # noqa: E402
from fncmp import RAM, Bench, Call, put  # noqa: E402

B = Bench(texdirs.merged(rideload.RANGES + [(0x429ba0, 0x429d59), (0x5836b0, 0x5836c2), (0x45bed0, 0x45bee3), (0x46fa70, 0x46facd),
                                            (0x576410, 0x576550), (0x576270, 0x576320),
                                            (0x42a030, 0x42a03b), (0x5d1410, 0x5d1416)]), hooks=rideload.HOOKS)
ARGS, RENDERER, POOL = RAM + 0x9e000, RAM + 0x9e400, RAM + 0x9e600


def case(r):
    slot = r.randrange(4)
    folder = r.choice([b'advisor', b'data\\adv', b'x', b'rides\\a\\'])
    suffix = r.choice([None, b'', b'B', b'02', b'_x'])
    name = b'Advisor' + (suffix or b'') + b'.md2'
    regions, _ = rideload.world(r, name, folder, 0x50120, sure=True)   # (the model used unchecked)
    third = r.choice([b'tex', b'c\\', None])
    text = folder + b'\0' + (third or b'') + b'\0' + (suffix or b'') + b'\0'
    regions[ARGS] = text + r.bytes(8)
    args = [ARGS, 0 if third is None else ARGS + len(folder) + 1, 0 if suffix is None else ARGS + len(folder) + 2 + len(third or b''), slot]
    s = bytearray(r.bytes(0xb0))
    put(s, 0x48, 'I', 0x6fdd00)
    put(s, 0x4c, 'II', 0, 0x6fdcc8)   # (the renderer's object at the part's +8: its vtable, ModelSlot_Ctor's)
    regions[0x790398 + 0xb0 * slot] = s
    regions[0x8bd508] = struct.pack('<I', r.choice([0, 0, r.getrandbits(32) & ~0x1000001])) + r.bytes(4)   # (no engine debug log, no load screen)
    regions[0x8bcc40] = r.bytes(4)
    renderer = bytearray(r.bytes(0x40))
    nodes = bytearray(0x80)
    prev = 0
    for i in range(r.choice([0, 1, 4, 16])):
        put(nodes, 8 * i, 'II', r.getrandbits(32), prev)
        prev = POOL + 8 * i
    put(renderer, 4, 'I', prev)
    put(renderer, 0xc, 'I', r.randrange(8))
    renderer[0x10:0x30] = bytes(0x20)
    regions[RENDERER] = renderer
    regions[POOL] = nodes
    regions[0x87a6d0] = struct.pack('<I', RENDERER)
    return Call(0x429ba0, args, regions)


if __name__ == '__main__':
    B.main(case)
