# The sound stream filter (src/game/core/streamfilter) against the originals: a stream at STREAM (a recorder vtable: slot 18 answers the
# inner stream INNER, now and then the stream itself) whose inner is a recorder vtable (slots 12 and 15 one argument, 9 two); the
# forwarders' arguments any dword, the query's out a dword in RAM.
# usage: streamfilter.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, log_region, set_return, vtable  # noqa: E402

# name: (address, stack arguments, returns)
CALLS = {'rate': (0x6bb130, 0, 'a'), 'restart': (0x6bb160, 0, 'a'), 'position': (0x6bb0f0, 0, 'a'), 'length': (0x6bb100, 0, 'a'),
         'bits': (0x6bb140, 0, 'a'), 'channels': (0x6bb150, 0, 'a'), 'seek': (0x6bb0d0, 1, 'a'), 'format': (0x6bb110, 1, 'a'),
         'query': (0x6d4cf0, 2, 'a')}
RANGES = [(0x6bb0d0, 0x6bb16c), (0x6d4cf0, 0x6d4d0d)]
B = Bench(RANGES)
STREAM, SVT, INNER, IVT, OUT, RECORD = (RAM + 0x100 * i for i in range(6))
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def case(r):
    name = r.deal(OPS)
    address, n, ret = CALLS[name]
    log = log_region()
    inner = r.choice([INNER, INNER, INNER, STREAM])
    set_return(log, 18, inner)
    if r.getrandbits(1):
        for k in (6, 8, 9, 12, 13, 14, 15, 16, 17):
            set_return(log, k, r.getrandbits(32))
    regions = {STREAM: struct.pack('<I', SVT) + r.bytes(0x14), SVT: vtable({9: 2, 12: 1, 15: 1}), INNER: struct.pack('<I', IVT) + r.bytes(0x14),
               IVT: vtable({9: 2, 12: 1, 15: 1}), OUT: r.bytes(4), RECORD: r.bytes(0x48), LOG: log}
    args = [r.getrandbits(32) for _ in range(n)] if name != 'query' else [r.choice([OUT, 0]), RECORD]
    return Call(address, [STREAM] + args, regions, conv='t', ret=ret)


if __name__ == '__main__':
    B.main(case)
