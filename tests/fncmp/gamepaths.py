# The game's folder paths of src/game/files/gamepaths against the originals: the fonts folder's path (0x468250) from the root folder's string (the first of
# the Strings5 object at 0xf816c0; only its data pointer at +8 is read): no text, an empty one (the byte before it read: "\" or not), roots with and
# without a trailing "\", up to 0x100 chars; the output buffer's bytes past the path compared too.
# usage: gamepaths.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x468250, 0x4682b0), (0x5c5520, 0x5c5530), (0x67a400, 0x690000)])
PATHS = 0xf816c0
ROOT, OUT = RAM + 0x100, RAM + 0x1000
CHARS = b'abcXYZ09:\\/. _%'


def case(r):
    regions = {}
    paths = bytearray(r.bytes(0x14))
    if r.random() < 0.1:
        put(paths, 8, 'I', 0)
    else:
        n = r.choice([0, 1, 2, 3, 8, 20, 60, 0x100])
        text = bytes(r.choice(CHARS) for _ in range(n))
        if n and r.random() < 0.4:
            text = text[:-1] + b'\\'
        before = bytes([r.choice([0x5c, 0x41, 0])])
        regions[ROOT - 1] = before + text + b'\0' + r.bytes(4)
        put(paths, 8, 'I', ROOT)
    regions[PATHS] = bytes(paths)
    regions[OUT] = r.bytes(0x140)
    return Call(0x468250, [OUT], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', 'FontsFolder').split(',') if k]

if __name__ == '__main__':
    B.main(case)
