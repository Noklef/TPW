# The text library's character tables (src/game/files/charsets) against the originals. The loaders read from a stream with a recorder
# vtable: slot 2 reads (logging the bytes asked for; its buffer, the caller's stack or the new block, logged as 0) and writes its records
# (the header, then data of the same length) through the buffer, answering all it was asked for or a fixed count (now and then the header's
# size: a header that passes and data that does not); slots 5 and 6 answer a size and a position (the bytes left 0 to 0x40, now and then
# below 0: the block's size wraps). The heap fails now and then. The constructors over random bytes with random arguments; the deleting
# destructors with flags 0-3 or any. The charset's index (Charset_Ctor, Charset_Load) a ResIndex of 0-4 records (keys from 0-1 up by 1 or 2,
# kinds mostly 0, 1 or 3, objects a recorder or none) that Table12_Find searches; Charset_Dtor's +0x10 / +0x14 a recorder or none.
# usage: charsets.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=RangeLoad,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, HEAP, RAM, echo, fill, heap, log_region, set_return, strings, vtable  # noqa: E402

B = Bench([(0x6ad860, 0x6ad8ec), (0x6ad950, 0x6ad9f2), (0x6ada90, 0x6adc7a), (0x6af760, 0x6af770), (0x6aca10, 0x6acb65), (0x6acec0, 0x6acfbf),
           (0x6af960, 0x6af9c9), (0x6afa30, 0x6afa6f), (0x6afc60, 0x6afc8d)])
THIS, RESULT, OUT, STREAM, VT_STREAM, LOADER = RAM, RAM + 0x80, RAM + 0x90, RAM + 0x100, RAM + 0x200, RAM + 0x300
INDEX, RECORDS, TABLE, VT_ENTRY, ENTRY_OBJ = RAM + 0x400, RAM + 0x480, RAM + 0x500, RAM + 0x600, RAM + 0x700
OPS = {'RangeCtor': 0x6ad860, 'RangeDtor': 0x6ad8c0, 'RangeDelete': 0x6ad8a0, 'RangeLoad': 0x6ad950,
       'CodeCtor': 0x6adbe0, 'CodeDtor': 0x6adc50, 'CodeDelete': 0x6adc30, 'CodeLoad': 0x6ada90,
       'CharsetCtor': 0x6aca10, 'CharsetDtor': 0x6acb00, 'CharsetDelete': 0x6acae0, 'CharsetLoad': 0x6acec0}
RETURNS = {'RangeDtor': 'v', 'CodeDtor': 'v', 'CharsetDtor': 'v'}
MAGIC = {'RangeLoad': [0x4d554642], 'CodeLoad': [0x554d4642, 0x484d4642], 'CharsetLoad': [0x54534642]}   # "BFUM"; "BFMU", "BFMH"; "BFST"
HEADER = {'RangeLoad': 4, 'CodeLoad': 8, 'CharsetLoad': 12}
PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]


def flags(r):
    return r.choice([0, 1, 1, 2, 3, r.getrandbits(32)])


def index(r, regions):
    """A ResIndex: 0-4 records (keys from 0 or 1 up by 1 or 2, kinds mostly 0, 1 or 3, objects a recorder or none)."""
    n = r.choice([0, 1, 2, 2, 3, 4])
    recs = bytearray(r.bytes(12 * 5))
    key = r.randrange(2)
    for k in range(5):
        struct.pack_into('<IH', recs, 12 * k, key, r.choice([0, 1, 1, 3]) if r.random() < 0.85 else r.choice([2, 4, 5, 6, r.getrandbits(16)]))
        struct.pack_into('<I', recs, 12 * k + 8, ENTRY_OBJ if r.random() < 0.7 else 0)
        key += r.choice([1, 1, 2])
    regions[RECORDS] = bytes(recs)
    regions[INDEX] = struct.pack('<4I', r.getrandbits(32), 1, RECORDS, RECORDS + 12 * (n - 1))
    regions[VT_ENTRY] = vtable({})
    regions[ENTRY_OBJ] = struct.pack('<2I', VT_ENTRY, 1)
    return INDEX


def load_case(r, op, regions):
    """(&result, stream, &object, key, index) on a loader object (unused): the stream's header (4, 8 or 12 bytes) and data of its length."""
    log = regions[LOG]
    size = HEADER[op]
    header = bytearray(r.bytes(size))
    struct.pack_into('<I', header, 0, r.choice(MAGIC[op]) if r.random() < 0.85 else r.choice([0x4d554642, 0x554d4642, 0x484d4642, r.getrandbits(32)]))
    if op == 'CodeLoad' and r.random() < 0.3:   # (header bytes 4 and 6 at their edges)
        header[4], header[6] = r.choice([0, 1, 0xff, 0x80]), r.choice([0, 1, 0xff, 0x80])
    key = r.getrandbits(32)
    if op == 'CharsetLoad':   # (the key in the second dword mostly; a count of 0-3 entries, now and then 5)
        key = r.choice([1, 2, 0x100, key])
        struct.pack_into('<2I', header, 4, key if r.random() < 0.85 else key ^ r.choice([1, 0x100]), r.choice([0, 1, 2, 3, 3, 5]))
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
    return [RESULT, STREAM, OUT, key, index(r, regions) if op == 'CharsetLoad' else r.getrandbits(32)]


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
    elif op == 'CharsetCtor':   # (count, table, index): the table's offsets random
        regions[TABLE] = r.bytes(0x20)
        stack = [r.choice([0, 1, 2, 4, 8]), TABLE, index(r, regions)]
    elif op.endswith('Delete'):
        stack = [flags(r)]
    else:
        stack = []
    regions[THIS] = r.bytes(0x20)
    if op in ('CharsetDtor', 'CharsetDelete'):   # (+0x10 / +0x14 held or none)
        regions[VT_ENTRY] = vtable({})
        regions[ENTRY_OBJ] = struct.pack('<2I', VT_ENTRY, 1)
        regions[THIS] =r.bytes(0x10) + struct.pack('<2I', *[ENTRY_OBJ if r.random() < 0.7 else 0 for _ in range(2)]) + r.bytes(8)
    regions[RESULT] = r.bytes(8)
    return Call(OPS[op], [this] + stack, regions, conv='t', ret=RETURNS.get(op, 'a'))


if __name__ == '__main__':
    B.main(case)
