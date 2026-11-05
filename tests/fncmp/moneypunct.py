# The C++ library's moneypunct facets (src/game/core/moneypunct: 0x6a8830, 0x6ab880, 0x6a18a0, 0x6a4f20) against the originals: a localeconv made here, the game heap's stand-in,
# _Mbrtowc a recorder whose answers come by the call.
# usage: moneypunct.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, blob, fill, heap, log_region, set_return, strings  # noqa: E402

MBRTOWC = 23
B = Bench([(0x6a1e70, 0x6a1ed8), (0x691530, 0x691536), (0x693dc0, 0x693e20), (0x6a18a0, 0x6a1b1d), (0x6a4f20, 0x6a5172), (0x6a8830, 0x6a8a4f), (0x6ab880, 0x6aba77)], hooks={0x693bc0: _recorder(MBRTOWC, 5, cdecl=True)})
FACET, LCONV, TEXTS = RAM, RAM + 0x40, RAM + 0x100
OPS = [n for n in ('makepat', 'char', 'wide', 'char_init', 'wide_init') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
WORDS = [b'', b'$', b'.', b',', b'\x03\x03', b'USD ', b'\xa3', b'-', b'+', b'long text']


def case(r):
    name = r.deal(OPS)
    log = log_region()
    pointers, texts = [], bytearray()
    for _ in range(10):
        pointers.append(TEXTS + len(texts))
        texts += r.choice(WORDS) + b'\0'
    lconv = struct.pack('<10I', *pointers) + bytes(r.choice([0, 0, 0, 1, 1, 2, 0x7f, 0x7e, 0x80, 0xff, 0x10]) for _ in range(2))
    lconv += bytes(r.choice([0, 0, 1, 1, 1, 2, 0x81, 0xfe, 0xff]) for _ in range(4)) + bytes(r.choice([0, 1, 2, 3, 4, 4, 5, 0x7f, 0x80, 0xff]) for _ in range(2))
    if r.random() < 0.5:
        strings(log, MBRTOWC, 0, out=9)
        blob(log, MBRTOWC, 1, 1)
    else:
        strings(log, MBRTOWC, 0, out=3)
        blob(log, MBRTOWC, 3, 1)
    fill(log, MBRTOWC, 0, struct.pack('<8I', *[r.getrandbits(32) for _ in range(8)]), records=8)
    set_return(log, MBRTOWC, r.choice([0, 1, 1, 0xffffffff, r.getrandbits(32)]))
    regions = {LOG: log, HEAP: heap(), LCONV: lconv, TEXTS: bytes(texts), 0x781fc8: struct.pack('<I', LCONV), FACET: r.bytes(0x2c)}
    if name == 'makepat':
        regions[FACET] = r.bytes(0x28) + bytes([r.choice([0, 0, 0, 1, 0x80])]) + r.bytes(3)
        regions[0x709d10] = r.bytes(0x60)
        flag = [0, 0, 1, 1, 2, 0x80, 0x100, 0x101, r.getrandbits(32)]
        return Call(0x6a1e70, [FACET, FACET + 0x24, r.choice(flag), r.choice(flag), r.choice([0, 1, 2, 3, 4, 5, 0x7f, 0x80, 0xff, 0x104, 0xffffffff])], regions, conv='t', ret='a')
    if name in ('char_init', 'wide_init'):
        return Call(0x6ab880 if name == 'char_init' else 0x6a4f20, [FACET, r.getrandbits(32)], regions, conv='t')
    return Call(0x6a8830 if name == 'char' else 0x6a18a0, [FACET, r.getrandbits(32), r.getrandbits(32), r.choice([0, 1, 0x80, 0x12300])], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
