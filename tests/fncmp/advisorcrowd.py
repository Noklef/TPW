# Four advisor criteria (src/game/core/advisorscoring: 0x59e9b0, 0x59ef60, 0x59f860, 0x59f8d0) against the originals: ParkAnalyser_ParkScore (k 1), Things_CountKind1OnPlainCell (k 2), Things_AveragePersonStat19c (k 3, a float
# answer, NaN now and then), Source_Field0c (k 4), Money_AnyLoanOffer (k 5) and ParkAnalyser_FindCutOff (k 6; the handle by a fill, the answer its argument) recorders; the table getters the real ones, the bounds near the answers.
# usage: advisorcrowd.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, echo, fill, log_region, put, set_float_return, set_return, strings  # noqa: E402

CALLS = {'Crowd480': (0x59f860, 0x480), 'Crowd490': (0x59f8d0, 0x490), 'Debt': (0x59e9b0, 0), 'CutOff': (0x59ef60, 0)}
B = Bench([(0x59f860, 0x59f8cd), (0x59f8d0, 0x59f93d), (0x59e9b0, 0x59e9ec), (0x59ef60, 0x59ef97), (0x519510, 0x51954d), (0x5195d0, 0x51960d), (0x5da3c0, 0x5da3d0)],
          hooks={0x4c7db0: _recorder(1, 1), 0x4c9130: _recorder(2, 0, cdecl=True), 0x4c7bb0: _recorder(3, 1), 0x6ad810: _recorder(4, 0), 0x4d0810: _recorder(5, 0), 0x4ccb00: _recorder(6, 2)})
SCORING, POINTER, GAME1, GAME2 = RAM, RAM + 0x800, RAM + 0x900, RAM + 0xa00
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def small(r):
    return r.randrange(-4, 5) & 0xffffffff


def game(r):
    window = bytearray(r.bytes(0x10))
    put(window, 0, 'H', r.randrange(1, 4))
    put(window, 6, 'H', r.randrange(1, 4))
    return window


def case(r):
    name = r.choice(OPS)
    address, bound = CALLS[name]
    log = log_region()
    scoring = bytearray(r.bytes(0x4a0))
    regions = {LOG: log, SCORING: scoring, 0x7cf6ec: struct.pack('<I', POINTER), POINTER: struct.pack('<I', GAME1), 0x7cf83c: struct.pack('<I', GAME2), GAME1 + 0x1da720: game(r), GAME2 + 0x1da720: game(r),
               0x7cfb90: put(bytearray(r.bytes(0x50)), 0, 'I', r.getrandbits(32) | 1)}
    if bound:
        set_return(log, 1, small(r))
        set_return(log, 2, small(r))
        x = r.randrange(-4, 5)
        set_float_return(log, 3, r.choice([float(x), float(r.randrange(-4, 5)), r.uniform(-5, 5), float('nan')]))
        put(scoring, bound, '3I', small(r), x & 0xffffffff, small(r))
    elif name == 'Debt':
        set_return(log, 4, r.choice([small(r), small(r), r.getrandbits(32)]))
        set_return(log, 5, r.choice([0, 0, 1, r.getrandbits(32)]))
    else:
        echo(log, 6, 0)
        strings(log, 6, 0, out=1)
        fill(log, 6, 0, struct.pack('<H', r.choice([0, 0, r.getrandbits(16) | 1])))
    return Call(address, [SCORING], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
