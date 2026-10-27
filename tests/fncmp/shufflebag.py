# The sound code's shuffle bag (src/game/core/shufflebag: 0x6be100-0x6be24e) against the original: a bag at BAG (+0 the list LIST, +4 any
# seed, +8 an index about the count, strides +0xc / +0xd 0-3 or any, the flag +0xe 0, 1 or any), the list's count 1 to 24 (1 and 2 often;
# entries of 0x2a random bytes at ENTRIES), +8 now and then entries one or two back; the constructor's Time_GetMs through timeGetTime (a
# recorder: any time).
# usage: shufflebag.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, log_region, recorder, set_return  # noqa: E402

CALLS = {'shuffle': 0x6be140, 'setlist': 0x6be130, 'ctor': 0x6be100, 'next': 0x6be210}
B = Bench([(0x6be100, 0x6be24f), (0x5f5f10, 0x5f5f7d), (0x5f5fa0, 0x5f5faa)])
BAG, LIST, ENTRIES = RAM, RAM + 0x40, RAM + 0x100
CLOCK, TIME_GET_TIME, TIME_K = 0xfa71e0, 0x6fd3ec, 12
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def case(r):
    name = r.deal(OPS)
    log = log_region()
    n = r.choice([1, 2, 2, 3, r.randrange(1, 8), r.randrange(8, 25)])
    first = ENTRIES + r.choice([0, 0, 0, 0x2a, 0x54])   # (entries one or two back left mapped: a may start below +8)
    bag = bytearray(r.bytes(0x10))
    struct.pack_into('<III', bag, 0, LIST, r.getrandbits(32), r.choice([0, n - 1, n, n + 1, r.randrange(n + 1), r.getrandbits(32)]) & 0xffffffff)
    bag[0xc], bag[0xd] = r.choice([0, 1, 1, 2, 3, r.getrandbits(8)]), r.choice([0, 1, 2, 2, 3, r.getrandbits(8)])
    bag[0xe] = r.choice([0, 1, r.getrandbits(8)])
    set_return(log, TIME_K, r.getrandbits(32))
    regions = {BAG: bag, LIST: r.bytes(4) + struct.pack('<II', n, first) + r.bytes(4), ENTRIES: r.bytes(0x2a * 27), LOG: log, CLOCK: bytes(0x20),
               TIME_GET_TIME: struct.pack('<I', recorder(TIME_K, 0, True))}
    if name == 'shuffle':
        return Call(CALLS[name], [BAG], regions, conv='t', ret='v')
    if name == 'next':
        return Call(CALLS[name], [BAG, r.choice([0, 1, 1, r.getrandbits(32)])], regions, conv='t', ret='a')
    if name == 'ctor':
        return Call(CALLS[name], [BAG, LIST], regions, conv='t', ret='a')
    return Call(CALLS[name], [BAG, LIST], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
