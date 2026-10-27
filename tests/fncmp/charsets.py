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
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import LOG, Bench, Call, HEAP, RAM, blob, echo, fill, heap, log_region, set_return, strings, vtable  # noqa: E402
import crt  # noqa: E402
import narrowwriter as nw  # noqa: E402
import widewriter as ww  # noqa: E402

B = Bench([(0x6ad860, 0x6ad8ec), (0x6ad950, 0x6ad9f2), (0x6ada90, 0x6adc7a), (0x6af760, 0x6af770), (0x6aca10, 0x6acb65), (0x6acec0, 0x6acfbf),
           (0x6af960, 0x6af9c9), (0x6afa30, 0x6afa6f), (0x6afc60, 0x6afc8d), (0x6acb70, 0x6acc94), (0x6acca0, 0x6ace5d), (0x6add40, 0x6add86),
           (0x6afc90, 0x6afd36), (0x6afd40, 0x6afe80), (0x6afe80, 0x6b0122), (0x6ad8f0, 0x6ad946)], hooks={0x67af00: crt.swprintf_hook})
# the formatter's cases (Format, FindArg, FormatNarrow, FormatWide): their own regions from FBASE (narrowwriter.py's and widewriter.py's
# writers and tables sit below it)
FBASE = RAM + 0x2000
CS, ENTRIES, PROGS, ARGS, ARGOBJ, VT_ARG = FBASE, FBASE + 0x40, FBASE + 0x80, FBASE + 0x300, FBASE + 0x340, FBASE + 0x440
STRS, DEST, CT, VT_H, CODES, REC, VT_REC, CUR = FBASE + 0x4c0, FBASE + 0x600, FBASE + 0x700, FBASE + 0x780, FBASE + 0x800, FBASE + 0xa00, \
    FBASE + 0xa80, FBASE + 0xb00
FORMAT_OPS = {'Format': 0x6acb70, 'FindArg': 0x6ace20, 'FormatNarrow': 0x6acca0, 'FormatWide': 0x6acd60}
THIS, RESULT, OUT, STREAM, VT_STREAM, LOADER = RAM, RAM + 0x80, RAM + 0x90, RAM + 0x100, RAM + 0x200, RAM + 0x300
INDEX, RECORDS, TABLE, VT_ENTRY, ENTRY_OBJ = RAM + 0x400, RAM + 0x480, RAM + 0x500, RAM + 0x600, RAM + 0x700
OPS = {'RangeCtor': 0x6ad860, 'RangeDtor': 0x6ad8c0, 'RangeDelete': 0x6ad8a0, 'RangeLoad': 0x6ad950,
       'CodeCtor': 0x6adbe0, 'CodeDtor': 0x6adc50, 'CodeDelete': 0x6adc30, 'CodeLoad': 0x6ada90,
       'CharsetCtor': 0x6aca10, 'CharsetDtor': 0x6acb00, 'CharsetDelete': 0x6acae0, 'CharsetLoad': 0x6acec0, **FORMAT_OPS}
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


def program(r):
    """An entry's program: 0-8 ops (1 literal bytes below 0x7f, 2 an argument 0-4, 3 inline dwords, 4 a dword, 5 / 6 a word), then 0 (finish)
    or now and then an op past 6."""
    words = []
    for _ in range(r.choice([0, 1, 2, 3, 5, 8])):
        op = r.choice([1, 1, 2, 2, 2, 3, 4, 5, 6])
        if op == 1:
            n = r.choice([0, 1, 3, 4, 5, 9])
            words.append(1 | n << 8)
            data = bytes(r.randrange(0x20, 0x7f) for _ in range(n)) + r.bytes(-n % 4)
            words += struct.unpack('<%dI' % (len(data) // 4), data)
        elif op == 2:
            words.append(2 | r.choice([0, 1, 2, 3, 4, 7]) << 8)
        elif op == 3:
            n = r.randrange(4)
            words.append(3 | n << 8)
            words += [r.getrandbits(32) for _ in range(n)]
        else:
            words.append(op | r.getrandbits(24) << 8)
    words.append(0 | r.getrandbits(24) << 8 if r.random() < 0.9 else r.choice([7, 8, 0xff]) | r.getrandbits(24) << 8)
    return words[:0x1f] + [0]


def arguments(r, regions, writer_on_stack):
    """The arguments: 0-4 pointers (the game's number or string argument, or a recorder answering 0, 1 or below 0; the tag now and then
    another, the numbers 0-4), then a null or an untagged one."""
    log = regions[LOG]
    regions[VT_ARG] = vtable({0: 2})
    set_return(log, 0, r.choice([0, 0, 0, 1, 0xffffffff, 0x80000000]))
    blob(log, 0, 1, 7)   # (&state: on the stack; its three dwords)
    if writer_on_stack:
        strings(log, 0, 0, out=1)
    ptrs, objs, strs = [], bytearray(), bytearray()
    for k in range(r.choice([0, 1, 2, 3, 4])):
        kind = r.choice([0x6ff170, 0x6ff174, VT_ARG])
        text = [r.randrange(0x20, 0x7f) for _ in range(r.choice([0, 1, 4, 9]))] + [0]
        s = STRS + len(strs)
        strs += struct.pack('<%dH' % len(text), *text)
        tag = 0x5641524d if r.random() < 0.93 else r.getrandbits(32)
        objs += struct.pack('<5I', kind, tag, r.randrange(5) if r.random() < 0.9 else r.getrandbits(32),
                            s if kind == 0x6ff174 else r.choice([0, 7, 0xffffffff, 12345, r.getrandbits(32)]), r.randrange(2)) + bytes(0xc)
        ptrs.append(ARGOBJ + 0x20 * k)
    end = 0 if r.random() < 0.8 else ARGOBJ + 0x20 * len(ptrs)
    objs += struct.pack('<3I', VT_ARG, r.getrandbits(32), r.randrange(5)) + bytes(0x14)
    regions[ARGOBJ] = bytes(objs)
    regions[STRS] = bytes(strs) + bytes(4)
    return ptrs + [end]


def charset(r, regions):
    """The charset: +0xc the entries (0-3, each a program), +0x10 a code table (lead 0xff or 0x80, 0x100 codes) or none, +0x14 the range
    table of narrowwriter.py's world or none."""
    progs = bytearray()
    entries = []
    for _ in range(4):
        entries.append(PROGS + len(progs))
        progs += struct.pack('<%dI' % 0x20, *(program(r) + [0] * 0x20)[:0x20])
    regions[PROGS] = bytes(progs)
    regions[ENTRIES] = struct.pack('<4I', *entries)
    regions[VT_H] = vtable({})
    regions[CT] = struct.pack('<2IBBHI', VT_H, 1, r.choice([0xff, 0x80]), 0, 0x100, CODES)
    regions[CODES] = r.bytes(0x200)
    regions[CS] = struct.pack('<6I', r.getrandbits(32), 1, 4, ENTRIES, CT if r.random() < 0.85 else 0,
                              nw.TABLE if r.random() < 0.85 else 0)


def format_case(r, op, regions):
    entry = r.randrange(4)
    if op == 'FindArg':   # (key, &cursor)
        args = arguments(r, regions, False)
        regions[ARGS] = struct.pack('<%dI' % len(args), *args)
        regions[CUR] = struct.pack('<I', ARGS) + r.bytes(4)
        return Call(OPS[op], [r.choice([0, 1, 2, 3, 4, 7]), CUR], regions, ret='a')
    regions.update(nw.world(r))   # (its range table; its writer at nw.W for Format)
    charset(r, regions)
    args = arguments(r, regions, op != 'Format')
    if op == 'Format':   # (writer, entry, args): the game's wide-text writer, its narrow-text writer or a recorder
        u = r.random()
        if u < 0.4:
            ww.writer(r, regions, CT if r.random() < 0.85 else 0)
            writer = ww.W
        elif u < 0.75:
            writer = nw.W
        else:
            regions[VT_REC] = vtable({3: 2, 5: 2})
            regions[REC] = struct.pack('<I', VT_REC) + r.bytes(0xc)
            strings(regions[LOG], 5, 0, out=1)   # (the number argument's stack buffer)
            writer = REC
        regions[ARGS] = struct.pack('<%dI' % len(args), *args)
        return Call(OPS[op], [CS, writer, entry, ARGS], regions, conv='t', ret='a')
    size = r.choice([0, 1, 2, 5, 0x10, 0x30, r.randrange(0x31)])
    regions[DEST] = r.bytes(0x80)
    return Call(OPS[op], [CS, DEST, size, entry] + args, regions, ret='a')


def case(r):
    op = r.deal(PICK)
    regions = {LOG: log_region(), HEAP: heap(0x2000, fail=1 if r.random() < 0.08 else 0)}
    if op in FORMAT_OPS:
        return format_case(r, op, regions)
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
