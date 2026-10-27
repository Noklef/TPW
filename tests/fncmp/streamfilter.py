# The sound stream filter (src/game/core/streamfilter) against the originals: a stream at STREAM (a recorder vtable: slot 18 answers the
# inner stream INNER, now and then the stream itself) whose inner is a recorder vtable (slots 12 and 15 one argument, 9 two); the
# forwarders' arguments any dword, the query's out a dword in RAM. The describe's record: bit 6 of +0x18 now and then. The loops' query:
# the status the inner's slot 9 writes through the local (logged as 0) negative now and then (unwritten now and then: the record's address, past 0x80000000 now and then), bit 6 of the
# record's +0x14 now and then. The 0x70abf8 stream's describe (0x6c6e50): slot 18's answer's slot 5 with the record.
# The read: the next stream (+4, the inner too or another recorder) reads (slot 7) gots filled from up to 8 records (never 0; the
# count for a count past 0x200), answering 1 after 0-3 calls, now and then up to 23 (fncmp counts them past the log's room); the loops 1-3, 9999, 0 or any.
# usage: streamfilter.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, fail_after, fill, log_region, set_return, strings, vtable  # noqa: E402

# name: (address, stack arguments, returns)
CALLS = {'rate': (0x6bb130, 0, 'a'), 'restart': (0x6bb160, 0, 'a'), 'position': (0x6bb0f0, 0, 'a'), 'length': (0x6bb100, 0, 'a'),
         'bits': (0x6bb140, 0, 'a'), 'channels': (0x6bb150, 0, 'a'), 'seek': (0x6bb0d0, 1, 'a'), 'format': (0x6bb110, 1, 'a'),
         'query': (0x6d4cf0, 2, 'a'), 'describe': (0x6c64f0, 1, 'a'), 'queryloops': (0x6c64a0, 2, 'a'), 'read': (0x6c6520, 4, 'a'),
         'describe18': (0x6c6e50, 1, 'v')}
RANGES = [(0x6bb0d0, 0x6bb16c), (0x6d4cf0, 0x6d4d0d), (0x6c64a0, 0x6c65b0), (0x6c5110, 0x6c5122), (0x6c6e50, 0x6c6e64)]
B = Bench(RANGES)
STREAM, SVT, INNER, IVT, OUT, RECORD, OTHER, BUFFER = (RAM + 0x100 * i for i in range(8))
SLOTS = {5: 1, 7: 4, 9: 2, 12: 1, 15: 1}
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
    stream = bytearray(r.bytes(0x18))
    struct.pack_into('<II', stream, 0, SVT, r.choice([inner, OTHER]))
    struct.pack_into('<I', stream, 0x14, r.choice([1, 1, 2, 3, 9999, 9999, 0, r.getrandbits(32)]))
    record = bytearray(r.bytes(0x48))
    record[0x14] = r.choice([0x40, 0, r.getrandbits(8)])
    record[0x18] = r.choice([0x40, 0, r.getrandbits(8)])
    regions = {STREAM: stream, SVT: vtable(SLOTS), INNER: struct.pack('<I', IVT) + r.bytes(0x14), IVT: vtable(SLOTS),
               OTHER: struct.pack('<I', IVT) + r.bytes(0x14), OUT: r.bytes(4), RECORD: record, LOG: log}
    if name == 'query':
        args = [r.choice([OUT, 0]), RECORD]
    elif name in ('describe', 'describe18'):
        args = [RECORD]
    elif name == 'queryloops':
        strings(log, 9, 0, out=1)   # (the status: a local)
        if r.random() < 0.8:
            fill(log, 9, 0, struct.pack('<I', r.choice([0, 1, 0x7fffffff, 0x80000000, 0xffffffff, r.getrandbits(32)])))
            args = [OUT, RECORD]
        else:   # (unwritten, the status is the record's address: a negative one is never read)
            args = [OUT, r.choice([RECORD, r.getrandbits(32) | 0x80000000])]
    elif name == 'read':
        count = r.choice([0, 1, r.randrange(2, 0x40), r.randrange(0x40, 0x200), r.getrandbits(32)])
        records = r.randrange(1, 9)
        if count > 0x200:   # (gots of the count, or the loop would run for ever: past the log's room the records stay at one)
            gots = [count] * records
        else:
            gots = [r.choice([1, r.randrange(2, 0x20), r.randrange(0x20, 0x100), count]) or 1 for _ in range(records)]
        fill(log, 7, 2, struct.pack('<%dI' % records, *gots), records=records)
        strings(log, 7, 0, out=4)   # (got: a local)
        fail_after(log, 7, r.choice([0, 1, 2, 3, r.randrange(4, 24)]), 1, counted=True)
        regions[BUFFER] = r.bytes(0x10)
        args = [BUFFER, count, OUT, r.getrandbits(32)]
    else:
        args = [r.getrandbits(32) for _ in range(n)]
    return Call(address, [STREAM] + args, regions, conv='t', ret=ret)


if __name__ == '__main__':
    B.main(case)
