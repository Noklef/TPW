# The sign texture search of src/game/core/model_signs against the originals: models of 0 to 5 entries with flags 2, 8 and 0x10 and names that are the sign textures (any case) or not.
# usage: model_signs.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x468950, 0x4689f0), (0x46b460, 0x46b600), (0x67df40, 0x67e040), (0x67faf0, 0x67fb20), (0x682720, 0x682780)])
NAMES = [b'sign1.tga', b'SIGN1.TGA', b'Sign2.Tga', b'sign2.tga', b'sign3.tga', b'sign1.tg', b'', b'x']


def case(r):
    if r.random() < 0.3:  # the remap: every code about the table, with garbage above
        return Call(0x46b460, [r.choice([r.randrange(0x20, 0x90)] * 5 + [r.getrandbits(16), r.getrandbits(32)]) | (r.getrandbits(16) << 16 if r.random() < 0.4 else 0)], {}, ret='a')
    n = r.choice([0, 1, 2, 3, 5])
    model = bytearray(r.bytes(0x80))
    put(model, 0x36, 'H', n)
    put(model, 0x50, 'I', r.choice([0, 0x1000, r.getrandbits(32)]))
    put(model, 0x54, 'I', RAM + 0x200)
    entries = bytearray(r.bytes(0x10 * 6))
    regions = {RAM: model, RAM + 0x200: entries, RAM + 0x300: bytes(0x10)}
    for k in range(n):
        put(entries, 0x10 * k, 'I', r.choice([0, 2, 8, 0xa, 0x12, 0x1a, 3, 0x10, r.getrandbits(8)]))
        put(entries, 0x10 * k + 0xc, 'I', RAM + 0x400 + 0x20 * k)
        regions[RAM + 0x400 + 0x20 * k] = r.choice(NAMES) + bytes(0x10)
    return Call(0x468950, [RAM, RAM + 0x300, RAM + 0x308], regions)


if __name__ == '__main__':
    B.main(case)
