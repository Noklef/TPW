# The sprite frames' and images' constructors and a frame set's texture flags
# (src/game/gfx/spriteframe: 0x588d70, 0x588320, 0x5882e0) against the originals: any bytes before;
# sets of 0 to 6 images (and negative counts), textures shared and over the set itself.
# usage: spriteframe.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Frame,Image,Mark)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x5882e0, 0x588342), (0x588d70, 0x588d8f)])
CALLS = {'Frame': 0x588d70, 'Image': 0x588320, 'Mark': 0x5882e0}
SET = RAM + 0x40         # images at RAM + 0x60, textures at RAM + 0x100


def case(r):
    op = r.choice(OPS)
    if op != 'Mark':
        return Call(CALLS[op], [RAM], {RAM: r.bytes(0x50)}, conv='t', ret='a')
    mem = bytearray(r.bytes(0x200))
    put(mem, 0x40, 'i', r.choice([0, 1, 2, 3, 6, -1, r.randrange(7)]))
    put(mem, 0x44, 'I', RAM + 0x60)
    for i in range(6):   # a texture's flags at +8: over the count (RAM + 0x38) now and then
        put(mem, 0x60 + i * 0x18, 'I', RAM + r.choice([0x100, 0x110, 0x120, 0x130, 0x38, 0x3c]))
    return Call(CALLS[op], [SET], {RAM: mem}, conv='t')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
