# The game clock's save and load records (src/game/core/gameclock: 0x4030f0-0x4031b0, 0x402e00,
# 0x402e30) against the originals, through a record stream with room for the bytes or not; the double at +0x20 (0x403390). The clocks' reads
# and the functions on them (0x402d70-0x402f10, 0x402f40-0x4030d0, 0x403150-0x4031f0, 0x4033a0): the game's Timer and its imports as
# thunks_static.clock() sets them up, a clock object of random times, paused, frozen or not, run doubles and rates from 0 and 1 to junk.
# The frozen time's step (0x402ef0) and the constructor (0x4032b0) on such a clock object. The real time less the pauses (0x5988e0): the clock object at 0x785970, its statics (in a pause or not, the pause's start, the total) of random times.
# usage: gameclock.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Records,Now,... to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put, record_stream  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from thunks_static import clock as timer  # noqa: E402

B = Bench([(0x402e00, 0x402e25), (0x402e30, 0x402e55), (0x4030f0, 0x403116), (0x403120, 0x403146), (0x403180, 0x4031a6), (0x4031b0, 0x4031d6),
           (0x5f5e20, 0x5f5e50), (0x5f5e50, 0x5f5e80), (0x403390, 0x403394), (0x402d70, 0x402e00), (0x402e60, 0x4030f0), (0x403150, 0x403172),
           (0x4031e0, 0x403202), (0x4033a0, 0x4033f0), (0x5f5f10, 0x5f5fc0), (0x5988e0, 0x598955), (0x4032b0, 0x4032ca)])
CLOCK, OUT, SINK = RAM, RAM + 0x1000, RAM + 0x2000
# name: (address, arguments past this, returns)
FUNCS = {'Now': (0x402d70, 0, 1), 'RealNow': (0x402d80, 0, 1), 'Pause': (0x402d90, 0, 0), 'Resume': (0x402db0, 0, 0), 'Toggle': (0x402dd0, 0, 0),
         'SaveNow': (0x402e60, 0, 0), 'RestoreNow': (0x402e80, 0, 0), 'GFreeze': (0x402ea0, 1, 0), 'GUnfreeze': (0x402ed0, 0, 0), 'GTime': (0x402f10, 0, 1),
         'RRead': (0x402f40, 0, 1), 'RPause': (0x402f80, 0, 0), 'RResume': (0x402fa0, 0, 0), 'RToggle': (0x402fd0, 0, 0), 'RElapsed': (0x403010, 0, 1),
         'GPause': (0x403030, 0, 0), 'GResume': (0x403050, 0, 0), 'GToggle': (0x403080, 0, 0), 'GElapsed': (0x4030d0, 0, 1), 'RNow': (0x403150, 0, 1),
         'RRestoreNow': (0x403160, 0, 0), 'GNow': (0x4031e0, 0, 1), 'GRestoreNow': (0x4031f0, 0, 0), 'GRead': (0x4033a0, 0, 1),
         'Advance': (0x402ef0, 0, 1), 'CtorOne': (0x4032b0, 0, 1)}
REAL = {'RRead', 'RPause', 'RResume', 'RToggle', 'RElapsed', 'RNow', 'RRestoreNow'}   # (on the real-time clock at +0x50)


def run_double(r):
    if r.wild and r.random() < 0.3:
        return r.bytes(8)
    return struct.pack('<d', r.choice([0.0, 1.0, 1000.5, float(r.getrandbits(24)), float(r.getrandbits(32)), r.uniform(0, 2 ** 31), -r.uniform(0, 1e6), 2.0 ** 63, 1e300]))


def rate_double(r):
    if r.wild and r.random() < 0.3:   # (any sign and significand, the infinities and NaNs, exponents up to 2^990: x87emu's double registers overflow on a
        b = bytearray(r.bytes(8))     # product past the double range, which the game's FPU keeps (and stores as the infinity all the same))
        if 990 + 1023 < (b[7] & 0x7f) << 4 | b[6] >> 4 < 0x7ff:
            b[7] = b[7] & 0x80 | 0x7d
        return bytes(b)
    return struct.pack('<d', r.choice([1.0, 1.0, 0.5, 2.0, 0.0, 0.1, -1.0, r.uniform(0, 4), r.uniform(-1e3, 1e3)]))


def clocks(r, name):
    regions = {}
    timer(r, regions, True)
    c = bytearray(r.bytes(0xa0))
    for base in (0, 0x50):
        put(c, base + (0x30 if base == 0 else 0x18), 'I', r.choice([0, 0, 1, r.getrandbits(32)]))   # paused
    put(c, 0x38, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))   # frozen
    c[0x18:0x20] = run_double(r)
    c[0x20:0x28] = rate_double(r)
    c[0x58:0x60] = run_double(r)
    if r.random() < 0.3:   # (the last reads near the timer's ms)
        put(c, 0x10, 'I', r.getrandbits(8))
        put(c, 0x50, 'I', r.getrandbits(8))
    regions[CLOCK] = c
    address, nargs, returns = FUNCS[name]
    args = [CLOCK + 0x50 if name in REAL else CLOCK] + [r.choice([1, 2, 25, 30, 60, 1000, 1001, 0xffffffff, 0 if r.random() < 0.2 else 50])] * nargs
    return Call(address, args, regions, conv='t', ret='a' if returns else 'v')


def less_pauses(r):
    """0x5988e0 (cdecl): a clocks object as clocks() makes, at 0x785970; the statics in a pause or not, with times near the clock's or random."""
    c = clocks(r, 'RealNow')
    c.regions[0x785970] = c.regions.pop(CLOCK)
    times = [r.getrandbits(32), r.getrandbits(8), 0]
    c.regions[0xf7977c] = struct.pack('<3I', r.choice([0, 0, 1, r.getrandbits(32)]), r.choice(times), r.choice(times))
    return Call(0x5988e0, [], c.regions, ret='a')


def case(r):
    name = 'Records' if 'Records' in OPS and r.random() < 0.3 else OPS[r.deal(range(len(OPS)))]   # (the six records share one op: weighted so each still gets its cases)
    if name == 'LessPauses':
        return less_pauses(r)
    if name != 'Records':
        return clocks(r, name)
    if r.random() < 0.15:  # the double at +0x20: 1.0, 0, small, negative, junk
        clock = bytearray(r.bytes(0x40))
        struct.pack_into('<d', clock, 0x20, r.choice([1.0, 0.0, 0.5, -2.25, 1e-310, 3.0e38]))
        if r.random() < 0.3:
            clock[0x20:0x28] = r.bytes(8)
        return Call(0x403390, [CLOCK], {CLOCK: clock}, conv='t', ret='d')
    at = r.choice([0x4030f0, 0x403120, 0x403180, 0x4031b0, 0x402e00, 0x402e30])
    left = r.choice([4, 4, 8, 0x100, r.randrange(8)])
    return Call(at, [CLOCK, OUT], {CLOCK: r.bytes(0x100), OUT: record_stream(OUT, SINK, left), SINK: r.bytes(0x100)},
                conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(['Records'] + list(FUNCS) + ['LessPauses'])).split(',') if k]

if __name__ == '__main__':
    B.main(case)
