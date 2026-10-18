# The track rides' chunk lifetime (src/game/world/trackrides: 0x543400, 0x543420, 0x5434d0, 0x544290) against the originals: the GlobalAlloc
# helper with sizes 0 and others and failing allocations; a chunk begun on a stream (a recorder: slot 3 the write, its buffer logged as a hash of
# the 12-byte header it is handed; slot 4 the seek; slot 6 the tell, returning anything) or not got; chunks ended and closed (from any start,
# count and total) with the write answering all, part or none of the header.
# usage: trakchunk.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Alloc,Begin,End,Close)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, blob, echo, global_heap, heap, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x543400, 0x543415), (0x543420, 0x54348b), (0x5434d0, 0x543552), (0x544290, 0x5442d1), (0x5f5bb0, 0x5f5bb9), (0x5f5bf0, 0x5f5c04),
           (0x5f5e50, 0x5f5e7d), (0x5da3c0, 0x5da3c1)])
CHUNK, STREAM, SVT = RAM, RAM + 0x100, RAM + 0x200


def stream(r, regions):
    log = log_region()
    blob(log, 3, 0, 0b111)
    if r.random() < 0.6:
        echo(log, 3, 1)   # (all of the header written)
    else:
        set_return(log, 3, r.choice([0, 11, 13, 0xffffffff, r.getrandbits(32)]))
    set_return(log, 6, r.choice([0, 12, 0x100, r.getrandbits(32)]))
    regions.update({LOG: log, STREAM: struct.pack('<I', SVT) + r.bytes(0x20), SVT: vtable({3: 2, 4: 2, 6: 0})})


def alloc(r):
    size = r.choice([0, 0, 1, 0x18, 0x100, r.randrange(0x400)])
    regions = {HEAP: heap(fail=r.choice([0, 0, 0, 1])), **global_heap()}
    return Call(0x543400, [size], regions, ret='a')


def begin(r):
    regions = {HEAP: heap(fail=r.choice([0, 0, 0, 0, 1])), **global_heap()}
    stream(r, regions)
    return Call(0x543420, [r.choice([1, 0, r.getrandbits(32)]), STREAM], regions, ret='a')


def ended(r, address):
    regions = {HEAP: heap(), **global_heap()}
    stream(r, regions)
    chunk = bytearray(r.bytes(0x20))
    put(chunk, 0, 'I', r.choice([0, 4, 0x100, r.getrandbits(32)]))
    put(chunk, 4, 'I', STREAM)
    put(chunk, 0x10, 'I', r.choice([0, 12, 0x40, r.getrandbits(32)]))
    regions[CHUNK] = chunk
    return Call(address, [CHUNK], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', 'Alloc,Begin,End,Close').split(',') if k]


def case(r):
    op = r.deal(OPS)
    if op == 'Alloc':
        return alloc(r)
    if op == 'Begin':
        return begin(r)
    return ended(r, 0x5434d0 if op == 'End' else 0x544290)


if __name__ == '__main__':
    B.main(case)
