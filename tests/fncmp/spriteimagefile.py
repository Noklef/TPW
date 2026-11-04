# A sprite image from an image file (src/game/gfx/spriteframe: 0x588820) against the original: names ending in B, b or others, empty ones with a byte B or not before them; TexFile_Load (recorder) found or not, the second read's palette or none, its
# sizes differing from the first; the sizes' any bits (odd, negative, a height of 0 in the low word only); SpriteImage_SetTexture and TrackedGlobal_Free recorders, the first failing now and then.
# usage: spriteimagefile.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, fill, log_region, put, set_return, strings  # noqa: E402

LOAD, SETTEX, FREE = 7, 8, 9
B = Bench([(0x588820, 0x58893f)], hooks={0x574bd0: _recorder(LOAD, 6, cdecl=True), 0x588410: _recorder(SETTEX, 7), 0x579b80: _recorder(FREE, 1, cdecl=True)})
IMAGE, TEXTURE, ANSWERS, NAME = RAM, RAM + 0x40, RAM + 0x100, RAM + 0x201


def size(r):
    return r.choice([r.randrange(0x400), r.randrange(0x400), r.randrange(1, 0x400) << 16, 0, 1, 0x7fffffff, 0xffffffff, r.getrandbits(32), 0x10000 | r.randrange(0x100)])


def case(r):
    log = log_region()
    strings(log, LOAD, 1, out=1 << 1 | 1 << 2)
    answers(log, LOAD, ANSWERS)
    table = struct.pack('<2I', r.choice([0, RAM + 0x1000, RAM + 0x1000]), r.choice([0, RAM + 0x2000])) + bytes(38 * 4)
    fill(log, LOAD, 1, struct.pack('<2I', size(r), size(r)), which=0, records=2)
    fill(log, LOAD, 2, struct.pack('<2I', size(r), size(r)), which=1, records=2)
    set_return(log, SETTEX, r.choice([1, 1, 1, 0, 0x100]))
    name = r.choice([b'a.tga', b'a.TGA', b'pic.b', b'pic.B', b'B', b'b', b'', b'' + r.bytes(3).replace(b'\0', b'x')])
    me = bytearray(r.bytes(0x20))
    put(me, 0, 'I', TEXTURE)
    return Call(0x588820, [IMAGE, NAME], {LOG: log, IMAGE: me, TEXTURE: r.bytes(0x60), ANSWERS: table, NAME - 1: r.choice([b'B', b'b', b'x']) + name + b'\0'}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
