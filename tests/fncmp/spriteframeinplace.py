# A frame's quads at the lights drawn in place (src/game/gfx/spriteframelights: 0x589990) against the original: as spriteframelights.py, the texture given, the panel's or none, render flag 0x40000 or not.
# usage: spriteframeinplace.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, fill, log_region, put, set_return, strings  # noqa: E402
from spriteframelights import place  # noqa: E402

DRAW, FIRST, NEXT, GROUND = 5, 2, 3, 4
B = Bench([(0x589990, 0x589b47), (0x590c40, 0x590ce8)], hooks={0x566220: _recorder(DRAW, 2), 0x584e30: _recorder(FIRST, 0), 0x584e50: _recorder(NEXT, 0), 0x589410: _recorder(GROUND, 4)})
FRAME, PANEL, QUADS, NEXTS, COLOURS, DRAWS, TEXTURE, OWN = (RAM + 0x100 * k for k in range(8))
SPEC = 0x7fffdd   # (not its +4, the quad's address, nor its +0x14, never set)


def case(r):
    log = log_region()
    nan = r.choice([0, 0, 0, 0.05])
    frame = bytearray(r.bytes(0x60))
    put(frame, 0x10, 'I', r.getrandbits(32) & ~0x10000200 | r.deal([0x10000200] * 10 + [0x200, 0x10000000, 0]))
    put(frame, 0x28, 'I', PANEL)

    def texture():
        data = bytearray(r.bytes(0x40))
        put(data, 8, 'I', r.getrandbits(32) & ~0x101000 | r.deal([0, 0x1000]) | r.deal([0, 0, 0x100000]))
        put(data, 0x20, 'IIII', place(r, 0, 2, nan), place(r, 0, 2, nan), place(r, -1, 1, nan), place(r, -1, 1, nan))
        return data

    lights = r.deal([0, 1, 1, 2, 3, 4])
    addresses = [RAM + 0x3000 + 0x40 * k for k in range(lights)]
    set_return(log, FIRST, addresses[0] if addresses else 0)
    answers(log, NEXT, NEXTS)
    answers(log, GROUND, COLOURS)
    answers(log, DRAW, DRAWS)
    draws = [1] * 40
    if r.random() < 0.15:
        draws[r.randrange(0, 3)] = 0
    strings(log, GROUND, 0, out=1)
    fill(log, GROUND, 0, r.bytes(0x40))
    blob(log, DRAW, 0, SPEC)
    regions = {LOG: log, FRAME: frame, PANEL: struct.pack('<I', r.deal([TEXTURE, TEXTURE, 0])), QUADS: r.bytes(0x40), TEXTURE: texture(), OWN: texture(),
               NEXTS: struct.pack('<40I', *(addresses[1:] + [0] * 40)[:40]), COLOURS: struct.pack('<40I', *[r.getrandbits(32) for _ in range(40)]), DRAWS: struct.pack('<40I', *draws),
               0x879ab0: struct.pack('<I', QUADS), 0x8bcbc8: struct.pack('<I', r.deal([0, 0x40000, 0xfffbffff, 0xffffffff])), 0x8bd524: struct.pack('<I', r.deal([4, 1, 0, 2]))}
    return Call(0x589990, [FRAME, r.deal([0, OWN])], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
