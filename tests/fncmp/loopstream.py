# The looping sound stream's read (src/game/core/loopstream: 0x6c6a60) against the original: a stream at STREAM (a recorder vtable: slot 18
# answers the inner stream INNER, slot 3 logged; +0x14 the loops left: 1-3, 9999, 0 or any; +0x1c the start, +0x20 the end), the inner a
# recorder vtable (slot 13 the position: about the end, now and then any; slot 12 the seek; slot 7 the read, its got filled from up to
# 8 records of small or odd counts, answering 1 after 0-3 calls, now and then 4-23, so that the loop ends); the count 0, small, or any; the done counter.
# usage: loopstream.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, fail_after, fill, log_region, set_return, strings, vtable  # noqa: E402

B = Bench([(0x6c6a60, 0x6c6bc0)])
STREAM, SVT, INNER, IVT, DONE, BUFFER = (RAM + 0x100 * i for i in range(6))


def case(r):
    log = log_region()
    end = r.choice([0, 0x10, 0x100, r.getrandbits(16), r.getrandbits(32)])
    stream = bytearray(r.bytes(0x24))
    struct.pack_into('<I', stream, 0, SVT)
    struct.pack_into('<I', stream, 0x14, r.choice([1, 1, 2, 3, 9999, 9999, 0, r.getrandbits(32)]))
    struct.pack_into('<II', stream, 0x1c, r.getrandbits(16), end)
    set_return(log, 18, INNER)
    set_return(log, 13, r.choice([end, end, end - r.randrange(1, 0x20), end + r.randrange(0x20), r.getrandbits(32)]) & 0xffffffff)
    count = r.choice([0, 1, r.randrange(2, 0x40), r.randrange(0x40, 0x200), r.getrandbits(32)])
    records = r.randrange(1, 9)
    gots = [r.choice([0, 1, r.randrange(2, 0x20), r.randrange(0x20, 0x100), count, r.getrandbits(32)]) & 0xffffffff for _ in range(records)]
    fill(log, 7, 2, struct.pack('<%dI' % records, *gots), records=records)
    strings(log, 7, 0, out=4)   # (got: a local)
    fail_after(log, 7, r.choice([0, 1, 2, 3, r.randrange(4, 24)]), 1, counted=True)   # (counted: past the log's 40 entries, a pass up to 8)
    regions = {STREAM: stream, SVT: vtable({}), INNER: struct.pack('<I', IVT) + r.bytes(0xc), IVT: vtable({7: 4, 12: 1}), DONE: r.bytes(4),
               BUFFER: r.bytes(0x10), LOG: log}
    return Call(0x6c6a60, [STREAM, BUFFER, count, DONE, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
