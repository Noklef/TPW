# The zlib codec (src/game/files/zlibcodec) against the originals: a ZlibCodec at THIS (the game's vtable, which the driver points at the replacements) over
# the zlib stand-ins (fncmp.zlib_results(): a copy of what fits, the calls logged with their arguments; the inits', deflate's and inflate's answers now
# and then forced, never to Z_OK for the streaming calls, whose loops would not end). The work buffer is the caller's (WORK, never touched) or the bench
# heap's (its next block 0 now and then: an allocation that fails); the stream writer (+4) and reader (+8) are recorders, the reader filling the buffer
# with a BILZ header (or not) and packed bytes. ArenaAlloc gets an arena whose cursor and end are near each other, and counts that overflow.
# usage: zlibcodec.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Compress,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, HEAP, RAM, fill, heap, log_region, vtable, zlib_results  # noqa: E402

B = Bench([(0x619250, 0x619730), (0x67a400, 0x690000)])
THIS, ARENA, IO, VT_IO = RAM, RAM + 0x40, RAM + 0x80, RAM + 0xc0
SRC, DST, BUF, WORK = RAM + 0x200, RAM + 0x600, RAM + 0xa00, RAM + 0xe00
CODEC_VTABLE, BILZ = 0x703128, 0x5a4c4942
K_WRITE, K_READ = 1, 2   # the writer's +4 (buffer, bytes), the reader's +8 (buffer, capacity)
# name: (address, stack arguments: A an arena, i items, s a size, D the destination, S the source, n a size, H a header (in BUF), W the work buffer, B the
# stream buffer, c its capacity, w the writer, r the reader; conv)
OPS = {
    'ArenaAlloc': (0x619250, 'Ais', 'c'), 'Compress': (0x619280, 'DSnW', 't'), 'Decompress': (0x619380, 'DHW', 't'), 'WorkSize': (0x619460, '', 't'),
    'InflateWorkSize': (0x619470, '', 't'), 'UnpackedSize': (0x619480, 'H', 't'), 'CompressStream': (0x6194a0, 'SnBcWw', 't'),
    'DecompressStream': (0x619600, 'DBcWr', 't'),
}


def header(r):
    """A BILZ header (or now and then not): magic, unpacked, packed, windowBits, memLevel, two zeroes."""
    magic = BILZ if r.random() < 0.85 else r.choice([0, BILZ ^ 1, r.getrandbits(32)])
    return struct.pack('<7I', magic, r.choice([0, 1, 0x10, 0x40, 0x100, 0x300, r.randrange(0x400)]), r.getrandbits(32), r.choice([15, 15, 9, 8, 0xfffffff1]),
                       r.choice([9, 8]), 0, 0)


def case(r):
    op = r.choice(PICK)
    address, args, conv = OPS[op]
    log = log_region()
    streaming = 'Stream' in op
    if r.random() < 0.3:   # (Z_OK only from the one-shot calls)
        zlib_results(log, init=r.choice([0, 0, -2, -4, -6, 1]) if r.random() < 0.5 else None,
                     deflate=r.choice([1, 2, -2, -5] + ([] if streaming else [0])) if r.random() < 0.6 else None,
                     inflate=r.choice([1, 2, -3, -5] + ([] if streaming else [0])) if r.random() < 0.6 else None)
    fill(log, K_READ, 0, header(r) + r.bytes(r.choice([0, 1, 0x10, 0x54])))
    heap_region = heap(0x1000)
    if r.random() < 0.05:
        struct.pack_into('<I', heap_region, 0, 0)   # (the next allocation answers 0)
    regions = {THIS: struct.pack('<I', CODEC_VTABLE) + r.bytes(0x3c), LOG: log, HEAP: heap_region, IO: struct.pack('<I', VT_IO) + r.bytes(0x3c),
               VT_IO: vtable({K_WRITE: 2, K_READ: 2}), SRC: r.bytes(0x400), DST: r.bytes(0x400), BUF: header(r) + r.bytes(0x3e4), WORK: r.bytes(0x40)}
    stack = []
    for x in args:
        if x == 'A':
            cursor = RAM + 0x1000 + r.randrange(0x100)
            struct.pack_into('<2I', regions.setdefault(ARENA, bytearray(0x40)), 0, cursor,
                             r.choice([cursor, cursor + 1, cursor + 0x10, cursor + 0x100, cursor - 1, 0, 0xffffffff]) & 0xffffffff)
            stack.append(ARENA)
        elif x in 'is':
            stack.append(r.choice([0, 1, 2, 4, 0x10, 0x100, 0x1000, 0x80000000, 0xffffffff, r.getrandbits(32)]))
        elif x == 'D':
            stack.append(DST)
        elif x == 'S':
            stack.append(SRC)
        elif x == 'n':
            stack.append(r.choice([0, 1, 0x1b, 0x1c, 0x1d, 0x20, 0x40, 0x100, 0x200, 0x3e4] + ([] if streaming else [0xffffffff])))
        elif x in 'HB':
            stack.append(BUF)
        elif x == 'W':
            stack.append(0 if r.random() < 0.5 else WORK)
        elif x == 'c':
            stack.append(r.choice([0, 1, 0x1b, 0x1c, 0x1d, 0x1e, 0x20, 0x40, 0x100, 0x200, 0x3e0]))
        elif x in 'wr':
            stack.append(IO)
    return Call(address, ([] if conv == 'c' else [THIS]) + stack, regions, conv=conv, ret='a')


PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
