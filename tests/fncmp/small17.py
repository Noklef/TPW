# The name comparisons of src/game/core/small17 against the originals (the CRT's _stricmp runs under the emulator, the port reaches the driver's
# stand-in): names that differ in case only, in the last letter or in length; models with mouth part names among their elements.
# usage: small17.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

CALLS = {'After': 0x424cc0, 'Equal': 0x424cf0, 'Mouth': 0x44b2e0}
B = Bench([(0x424cc0, 0x424cf0), (0x424cf0, 0x424d10), (0x44b2e0, 0x44b3c0), (0x67df40, 0x67e040), (0x67faf0, 0x67fb20), (0x682720, 0x682780)])
NAMES = [b'mouth - normal', b'mouth - aah', b'mouth - eee', b'mouth - ooh', b'mouth - sss', b'bug head']
TEXTS, ARR = RAM + 0x1000, RAM + 0x2000


def varied(r, base):
    s = bytearray(base)
    for i in range(len(s)):
        if r.random() < 0.3:
            s[i] ^= 0x20
    if r.random() < 0.3 and s:
        s[r.randrange(len(s))] = r.choice(b'abzAZ@[`{ -\x80\xff')
    if r.random() < 0.15:
        s = s[:r.randrange(len(s) + 1)]
    return bytes(s)


def case(r):
    name = r.choice(OPS)
    address = CALLS[name]
    regions = {}
    if name in ('After', 'Equal'):
        base = r.choice(NAMES + [b'', b'Zz', b'abc\x80'])
        a, b = varied(r, base) + b'\0', varied(r, base) + b'\0'
        regions[TEXTS] = a + bytes(0x40 - len(a)) if len(a) < 0x40 else a
        regions[TEXTS + 0x100] = b + bytes(0x40)
        pa, pb = bytearray(0x20), bytearray(0x20)
        put(pa, 8, 'I', TEXTS)
        put(pb, 8, 'I', TEXTS + 0x100)
        regions[RAM], regions[RAM + 0x100] = pa, pb
        return Call(address, [RAM, RAM + 0x100], regions, conv='c', ret='a')
    # a model: +8 -> X, X+4 -> Y (count at +0x42, elements at +0x70 -> Z, each 0xa0 with the name pointer at +0x54)
    n = r.choice([0, 1, 2, 3, 5, 8])
    model, x, y, z = bytearray(0x20), bytearray(0x20), bytearray(0x100), bytearray(0xa0 * 8 + 0x60)
    texts = bytearray(0x40 * 8)
    put(model, 8, 'I', RAM + 0x100)
    put(x, 4, 'I', RAM + 0x200)
    put(y, 0x42, 'H', n)
    put(y, 0x70, 'I', ARR)
    for i in range(n):
        s = varied(r, r.choice(NAMES)) + b'\0'
        texts[0x40 * i:0x40 * i + len(s)] = s
        put(z, 0xa0 * i + 0x54, 'I', TEXTS + 0x40 * i)
    regions.update({RAM: model, RAM + 0x100: x, RAM + 0x200: y, ARR: z, TEXTS: texts})
    return Call(address, [RAM, r.getrandbits(32), r.choice([1, 2, 3, 4, 5, -1, 0, 6, 0x7fffffff])], regions, conv='c', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
