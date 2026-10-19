# zlib 1.1.3 (src/game/lib/zlib: the library built from third_party/zlib-1.1.3) against the exe's copies of it. A deflate_state at S
# (0x16b8 bytes, random except the fields a function reads as sizes or pointers), its pending buffer at PEND, input bytes at BUF.
# usage: zlib.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=adler32,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {
    'adler32': (0x63ea00, 0x63eb30), 'init_block': (0x63eba0, 0x63ec10), 'set_data_type': (0x640460, 0x6404d0),
    'bi_flush': (0x6404f0, 0x640580), 'bi_windup': (0x640580, 0x6405f0), 'copy_block': (0x6405f0, 0x640690),
}
B = Bench(sorted(FUNCS.values()))
S, PEND, BUF, BIG = RAM, RAM + 0x2000, RAM + 0x3000, RAM + 0x10000
STATE = 0x16b8
# deflate_state offsets (deflate.h; src/game/lib/zlib.cpp checks them)
PENDING_BUF, PENDING, DATA_TYPE, DYN_LTREE, LAST_EOB_LEN, BI_BUF, BI_VALID = 0x8, 0x14, 0x1c, 0x8c, 0x16ac, 0x16b0, 0x16b4
PEND_SIZE = 0x800


def state(r, pending_max=0x100):
    """A random state with a pending buffer (PEND) filled up to `pending` and bits waiting in bi_buf."""
    s = r.bytes(STATE)
    struct.pack_into('<I', s, PENDING_BUF, PEND)
    struct.pack_into('<i', s, PENDING, r.randrange(pending_max))
    valid = r.choice([0, 1, 7, 8, 9, 15, 16, r.randrange(17)])
    struct.pack_into('<Hi', s, BI_BUF, r.getrandbits(16) & ((1 << valid) - 1) if r.random() < 0.8 else r.getrandbits(16), valid)
    return s


def regions(r, s):
    return {S: s, PEND: r.bytes(PEND_SIZE)}


def adler32(r):
    n = r.choice([0, 1, 15, 16, 17, r.randrange(64), r.randrange(0x1000), 5551, 5552, 5553, r.randrange(0x15b0, 0x4000)])
    data = bytes([0xff] * n) if r.random() < 0.2 else r.bytes(n)  # (all 0xff: the sums at their largest before the modulo)
    adler = r.choice([1, 0, 0xfff00000 | r.getrandbits(16), r.getrandbits(32), 0xfff0fff0])
    if n == 0 and r.random() < 0.5:
        return Call(0x63ea00, [adler, 0, r.randrange(100)], {BIG: b'\0' * 4}, ret='a')  # a null buffer: 1
    return Call(0x63ea00, [adler, BIG, n], {BIG: data or b'\0' * 4}, ret='a')


def init_block(r):
    s = state(r)
    return Call(0x63eba0, [S], regions(r, s))


def set_data_type(r):
    s = state(r)
    for i in range(256):  # frequencies: mostly text, sometimes binary
        f = r.randrange(50) if 32 <= i < 128 else r.choice([0, 0, 0, r.randrange(30), r.randrange(400)])
        struct.pack_into('<H', s, DYN_LTREE + 4 * i, f)
    return Call(0x640460, [S], regions(r, s))


def bi_flush(r):
    return Call(0x6404f0, [S], regions(r, state(r)))


def bi_windup(r):
    return Call(0x640580, [S], regions(r, state(r)))


def copy_block(r):
    s = state(r)
    n = r.choice([0, 1, r.randrange(0x40), r.randrange(0x400)])
    return Call(0x6405f0, [S, BUF, n, r.choice([0, 1])], {**regions(r, s), BUF: r.bytes(max(n, 4))})


OPS = [f for f in (adler32, init_block, set_data_type, bi_flush, bi_windup, copy_block)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
