# A frame's ground quad for each light (src/game/gfx/spriteframelights: 0x589770) against the original, with Texture_FormatBits as it is. Batch_DrawQuad (k 1; its spec compared but for dword 1, a stack address, 5 and 10, unset),
# LightList_First (k 2), LightList_Next (k 3) and SpriteFrame_GroundQuad (k 4; its quad, a stack address, logged as 0) are recorders, their answers in turn.
# usage: spriteframelights.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, fb, log_region, put, set_return, strings  # noqa: E402

B = Bench([(0x589770, 0x589990), (0x590c40, 0x590ce8)], hooks={0x565f90: _recorder(1, 2), 0x584e30: _recorder(2, 0), 0x584e50: _recorder(3, 0), 0x589410: _recorder(4, 4)})
FRAME, PANEL, LISTER, QUADS, NEXTS, COLOURS, DRAWS, TABLE = (RAM + 0x100 * k for k in range(8))
TEXTURES = RAM + 0x1000
ITEMS = RAM + 0x2000
SPEC = 0x3dd
NANS = [0x7fc12345, 0x7f812345]


def place(r, lo, hi, nan=0.0):
    return r.choice(NANS) if r.random() < nan else fb(r.uniform(lo, hi))


def case(r):
    log = log_region()
    nan = r.choice([0, 0, 0, 0.05])
    frame = bytearray(r.bytes(0x60))
    put(frame, 0x10, 'I', r.getrandbits(32) & ~0x10000200 | r.deal([0x10000200] * 10 + [0x200, 0x10000000, 0]))
    put(frame, 0x28, 'I', PANEL)
    put(frame, 0x34, 'IH', LISTER, r.randrange(0, 4))
    for k in range(4):
        frame[0x3c + k] = r.randrange(0, 4)
    textures = bytearray(r.bytes(4 * 0x40))
    for k in range(4):
        put(textures, k * 0x40 + 8, 'I', r.getrandbits(32) & ~0x101000 | r.deal([0, 0x1000]) | r.deal([0, 0, 0x100000]))
        put(textures, k * 0x40 + 0x20, 'IIII', place(r, 0, 2, nan), place(r, 0, 2, nan), place(r, -1, 1, nan), place(r, -1, 1, nan))
    items = bytearray(r.bytes(8 * 0x18))
    for k in range(8):
        put(items, k * 0x18, 'I', r.deal([TEXTURES + r.randrange(0, 4) * 0x40] * 4 + [0]))

    lights = r.deal([0, 1, 1, 2, 3, 4])
    addresses = [RAM + 0x3000 + 0x40 * k for k in range(lights)]
    set_return(log, 2, addresses[0] if addresses else 0)
    answers(log, 3, NEXTS)
    answers(log, 4, COLOURS)
    answers(log, 1, DRAWS)
    draws = [1] * 40
    if r.random() < 0.15:
        draws[r.randrange(0, 3)] = 0
    strings(log, 4, 0, out=1)
    blob(log, 1, 0, SPEC)
    regions = {LOG: log, FRAME: frame, PANEL: struct.pack('<I', r.deal([TEXTURES, TEXTURES, 0])), LISTER: bytes(4) + struct.pack('<I', ITEMS), QUADS: r.bytes(0x40), TEXTURES: textures, ITEMS: items,
               NEXTS: struct.pack('<40I', *(addresses[1:] + [0] * 40)[:40]), COLOURS: struct.pack('<40I', *[r.getrandbits(32) for _ in range(40)]), DRAWS: struct.pack('<40I', *draws),
               0x879ab0: struct.pack('<I', QUADS), 0x8bcbc8: struct.pack('<I', r.deal([0, 0x100000, 0xffefffff, 0xffffffff])), 0x8bd524: struct.pack('<I', r.deal([4, 1, 0, 2]))}
    return Call(0x589770, [FRAME], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
