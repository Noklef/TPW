# The bank account's constructor (src/game/world/bankaccount: BankAccount_Ctor 0x4cf7c0) against the original, with the callees run: thing_ctors.py's thing table and message centre, the loan table 0x7851d8 of random amounts, rates and terms (0: a division by nothing).
# _CIpow 0x67c5d0 is recorder 10 (the driver's env pow; here its two doubles from the x87 stack), answering a float of the case.
# usage: bankaccountctor.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, Bench, Call, _recorder, log_region, put, set_float_return  # noqa: E402
from thing_ctors import CENTRE_HOOKS, CENTRE_RANGES, OBJ, centre, thing_regions  # noqa: E402

POW = _recorder(10, 4, cdecl=True)


def _pow(e):  # _CIpow: st1 to the power st0, popping both; the recorder sees them as the env's (x, y)
    y, x = e.st.pop(0), e.st.pop(0)
    sp = e.r['ESP']
    e.r['ESP'] = sp - 16
    e.write(sp - 16, struct.pack('<2d', x, y))
    POW(e)
    e.r['ESP'] = sp


B = Bench([(0x4cf7c0, 0x4cf91c), (0x4d91f0, 0x4d9290), (0x50afe0, 0x50b090), (0x50ba80, 0x50bac0), (0x50f1f0, 0x50f200), (0x50ffe0, 0x5100f0), (0x516270, 0x516320), (0x5d1750, 0x5d1751), (0x67b050, 0x67b0d0), (0x5da3c0, 0x5da3c1), (0x516330, 0x516370), (0x5100f0, 0x510100),
           (0x510160, 0x510180)] + CENTRE_RANGES, hooks={**CENTRE_HOOKS, 0x67c5d0: _pow})


def case(r):
    regions = thing_regions(r)
    regions[OBJ] = r.bytes(0x128)
    centre(r, regions)
    log = regions.setdefault(LOG, log_region())
    set_float_return(log, 10, r.choice([1.0, 1.25, 2.5, r.uniform(0, 100), r.uniform(0, 1e6)]))
    table = bytearray(r.bytes(8 + 0x80))
    for k in range(8):
        put(table, 8 + 0x10 * k, 'I', r.choice([0, 10000, 10001, 9999, 5000, 20000, 100000, r.randrange(1, 1000000), r.getrandbits(32)]))
        put(table, 8 + 0x10 * k + 4, 'I', r.choice([0, 1, 5, 12, r.randrange(3000)]))
        put(table, 8 + 0x10 * k + 8, 'I', r.choice([12, 24, 36, 48, 60, 120, r.randrange(1, 600), r.randrange(1, 600), 0]))
    regions[0x7851d8] = table
    return Call(0x4cf7c0, [OBJ], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
