# The C library's floating point exceptions (src/game/core/crtfpexcept: _handle_exc 0x685a20, _raise_exc 0x6856e0, _87except 0x68e6f0) against the
# originals: every flag set under every caller's control word (masks, rounding, precision), overflow values of both signs and NaNs, underflow values
# scaled by 2^1536 down to zero; RaiseException recorded; _87except with masked words only (its record is on the stack: a raise logs a stack address).
# usage: crt_fpexcept.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, import_table, log_region, set_return  # noqa: E402

B = Bench([(0x68e6f0, 0x68e7f0), (0x685a20, 0x685d50), (0x6856e0, 0x685a04), (0x686050, 0x686071), (0x6860b0, 0x686109), (0x686080, 0x6860a8),
           (0x685f40, 0x686043), (0x685ea0, 0x685ed9), (0x685e00, 0x685e28), (0x686a00, 0x686a19), (0x5d60c0, 0x5d60c3)], hooks={0x682460: _recorder(2, 0, cdecl=True)})
OPS = [n for n in ('handle', 'handle', 'raise', 'except', 'except') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
GETPTD, RAISE = 2, 5
REC, CW, ARG1, VALUE, EXC, PTD = RAM + 0x10, RAM + 0x100, RAM + 0x140, RAM + 0x180, RAM + 0x200, RAM + 0x300
INF, DBL_MAX, MATHERR_OFF = 0x781198, 0x7811a8, 0x781edc


def value(r):
    """A double's bits: as an overflow or underflow leaves them (scaled by 2^-+1536: exponents about the denormal range's), zeros, NaNs, random."""
    pick = r.randrange(7)
    if pick < 3:   # (2^460 is the last that underflows to zero)
        bits = struct.unpack('<Q', struct.pack('<d', r.uniform(1, 2) * 2.0 ** r.choice([r.randrange(455, 470), r.randrange(430, 530)])))[0] | r.choice([0, r.getrandbits(20)])
    elif pick < 5:
        bits = r.choice([0, 0, 0x7ff0000000000000, 0x7ff8000000000000, 0x7fefffffffffffff, 1, 0x0010000000000000, 0x3ff0000000000000])
    else:
        bits = r.getrandbits(63)
    return bits | (1 << 63 if r.random() < 0.4 else 0)


def flags(r):
    return r.choice([1, 1, 2, 2, 4, 8, 0x10, 0x11, 0x11, 0x12, 0x12, 0x18, 0x14, 0, 0x1f, r.getrandbits(5), r.getrandbits(32)])


def word(r):
    """A caller's control word: masks (all, none, some), rounding and precision any."""
    masks = r.choice([0x3f, 0x3f, 0, r.getrandbits(6)])
    return masks | r.choice([0, 0x400, 0x800, 0xc00]) | r.choice([0, 0x100, 0x200, 0x300]) | 0x40 | (r.getrandbits(16) & 0xf080 if r.random() < 0.2 else 0)


def globals_(r):
    pick = lambda d: struct.pack('<Q', d if r.random() < 0.8 else r.choice([r.getrandbits(64), 0x7ff4000000000001, 0x3ff0000000000000]))
    return {INF: pick(0x7ff0000000000000), DBL_MAX: pick(0x7fefffffffffffff), MATHERR_OFF: struct.pack('<I', r.choice([0, 0, 1, r.getrandbits(32)]))}


def case(r):
    name = r.deal(OPS)
    cw = r.choice([0x7f, 0x27f, 0x37f])
    regions = globals_(r)
    if name == 'handle':
        regions[VALUE] = struct.pack('<Q', value(r))
        return Call(0x685a20, [flags(r), VALUE, word(r)], regions, conv='c', ret='wc', cw=cw)
    log = log_region()
    regions[LOG] = log
    regions[PTD] = bytes(0x40)
    set_return(log, GETPTD, PTD)
    if name == 'raise':
        regions[IMPORT_TABLE] = import_table(log, {'RaiseException': (RAISE, 0x6fd218, 4, 0b1000, 0)})
        regions.update({REC: r.bytes(0x58), CW: struct.pack('<I', word(r) if r.random() < 0.8 else r.getrandbits(32)), ARG1: struct.pack('<Q', value(r)),
                        VALUE: struct.pack('<Q', value(r))})
        return Call(0x6856e0, [REC, CW, flags(r), r.choice([0x10, 0x16, 0x1d, 0, 0xfff, r.getrandbits(32)]), ARG1, VALUE], regions, conv='c', ret='c', cw=cw)
    kind = r.choice([1, 2, 3, 4, 5, 6, 7, 8, 0, 9, r.getrandbits(32)])
    regions[EXC] = struct.pack('<II3Q', kind, r.getrandbits(32), value(r), value(r), value(r))   # (type, name, arg1, arg2, retval)
    regions[CW] = struct.pack('<H', word(r) | 0x3f)   # (masked: no raise)
    return Call(0x68e6f0, [r.choice([0x10, 0x16, 0x1d, 0x18, r.getrandbits(8)]), EXC, CW], regions, conv='c', ret='c', cw=cw)


if __name__ == '__main__':
    B.main(case)
