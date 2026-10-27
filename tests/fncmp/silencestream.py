# The silence stream's read (src/game/core/silencestream: 0x6c67f0) against the original: a stream at STREAM (+0x2c 0, 1 or any; +0x14
# about +0x18: below, at, past or any), its next stream a recorder (slot 7 logged, answering 0, 1 or any, writing a count through done
# now and then); a buffer of 0x400 bytes, the bytes up to 0x400 and *done up to them (past them now and then: a fault in both).
# usage: silencestream.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, fill, log_region, set_return, vtable  # noqa: E402

B = Bench([(0x6c67f0, 0x6c688c)])
STREAM, SVT, NEXT, NVT, DONE, BUFFER = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x1000


def case(r):
    log = log_region()
    bytes_ = r.choice([0, 1, 2, r.randrange(0x400), 0x400])
    done = r.choice([0, 0, bytes_, r.randrange(bytes_ + 1), bytes_ - 1 if bytes_ else 0, bytes_ + 1 if r.random() < 0.05 else 0])
    total = r.choice([0, r.randrange(0x400), r.randrange(0x400, 0x10000), r.getrandbits(32)])
    played = r.choice([0, total, total - 1, total + 1, total - r.randrange(0x400), r.getrandbits(32)]) & 0xffffffff
    stream = bytearray(r.bytes(0x30))
    struct.pack_into('<II', stream, 0, SVT, NEXT)
    struct.pack_into('<II', stream, 0x14, played, total)
    stream[0x2c] = r.choice([0, 0, 1, r.getrandbits(8)])
    set_return(log, 7, r.choice([0, 1, 1, 0x100, r.getrandbits(32)]))
    if r.random() < 0.5:
        fill(log, 7, 2, struct.pack('<I', r.randrange(bytes_ + 1)))
    regions = {STREAM: stream, SVT: vtable({}), NEXT: struct.pack('<I', NVT) + r.bytes(0xc), NVT: vtable({7: 4}), DONE: struct.pack('<I', done),
               BUFFER: r.bytes(0x400), LOG: log}
    return Call(0x6c67f0, [STREAM, BUFFER, bytes_, DONE, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
