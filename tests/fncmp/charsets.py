# The text library's character tables (src/game/files/charsets) against the originals. The loaders read from a stream with a recorder
# vtable: slot 2 reads (logging the bytes asked for; its buffer, the caller's stack or the new block, logged as 0) and writes its records
# (the header, then data of the same length) through the buffer, answering all it was asked for or a fixed count (now and then the header's
# size: a header that passes and data that does not); slots 5 and 6 answer a size and a position (the bytes left 0 to 0x40, now and then
# below 0: the block's size wraps). The heap fails now and then. The constructors over random bytes with random arguments; the deleting
# destructors with flags 0-3 or any.
# usage: charsets.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=RangeLoad,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, HEAP, RAM, echo, fill, heap, log_region, set_return, strings, vtable  # noqa: E402

B = Bench([(0x6ad860, 0x6ad8ec), (0x6ad950, 0x6ad9f2), (0x6ada90, 0x6adc7a), (0x6af760, 0x6af770)])
THIS, RESULT, OUT, STREAM, VT_STREAM, LOADER = RAM, RAM + 0x80, RAM + 0x90, RAM + 0x100, RAM + 0x200, RAM + 0x300
OPS = {'RangeCtor': 0x6ad860, 'RangeDtor': 0x6ad8c0, 'RangeDelete': 0x6ad8a0, 'RangeLoad': 0x6ad950,
       'CodeCtor': 0x6adbe0, 'CodeDtor': 0x6adc50, 'CodeDelete': 0x6adc30, 'CodeLoad': 0x6ada90}
RETURNS = {'RangeDtor': 'v', 'CodeDtor': 'v'}
MAGIC = {'RangeLoad': [0x4d554642], 'CodeLoad': [0x554d4642, 0x484d4642]}   # "BFUM"; "BFMU", "BFMH"
PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]


def flags(r):
    return r.choice([0, 1, 1, 2, 3, r.getrandbits(32)])


def load_case(r, op, regions):
    """(&result, stream, &object, key, index) on a loader object (unused): the stream's header (4 or 8 bytes) and its data."""
    log = regions[LOG]
    size = 4 if op == 'RangeLoad' else 8
    header = bytearray(r.bytes(size))
    struct.pack_into('<I', header, 0, r.choice(MAGIC[op]) if r.random() < 0.85 else r.choice([0x4d554642, 0x554d4642, 0x484d4642, r.getrandbits(32)]))
    if op == 'CodeLoad' and r.random() < 0.3:   # (header bytes 4 and 6 at their edges)
        header[4], header[6] = r.choice([0, 1, 0xff, 0x80]), r.choice([0, 1, 0xff, 0x80])
    fill(log, 2, 0, bytes(header) + r.bytes(size), records=2)
    strings(log, 2, 0, out=1)   # (the buffer: the caller's stack or the block)
    if r.random() < 0.75:
        echo(log, 2, 1)
    else:
        set_return(log, 2, r.choice([size, size, 0, 3, 4, 7, 0x10, 0xffffffff, r.getrandbits(32)]))
    tell = r.choice([0, 4, 8, 0x10, r.randrange(0x100)])
    left = r.randrange(0x41) if r.random() < 0.97 else r.choice([-4, -8, -0x10])
    set_return(log, 5, tell + left)
    set_return(log, 6, tell)
    regions[STREAM] = struct.pack('<I', VT_STREAM) + r.bytes(0xc)
    regions[VT_STREAM] = vtable({2: 2})
    regions[LOADER] = r.bytes(4)
    regions[OUT] = r.bytes(4)
    return [RESULT, STREAM, OUT, r.getrandbits(32), r.getrandbits(32)]


def case(r):
    op = r.deal(PICK)
    regions = {LOG: log_region(), HEAP: heap(0x2000, fail=1 if r.random() < 0.08 else 0)}
    this = THIS
    if op.endswith('Load'):
        stack = load_case(r, op, regions)
        this = LOADER
    elif op == 'RangeCtor':
        stack = [r.getrandbits(32)]
    elif op == 'CodeCtor':
        stack = [r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)]
    elif op.endswith('Delete'):
        stack = [flags(r)]
    else:
        stack = []
    regions[THIS] = r.bytes(0x20)
    regions[RESULT] = r.bytes(8)
    return Call(OPS[op], [this] + stack, regions, conv='t', ret=RETURNS.get(op, 'a'))


if __name__ == '__main__':
    B.main(case)
