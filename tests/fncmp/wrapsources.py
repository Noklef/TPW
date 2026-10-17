# The sources that wrap another source and the compressor singleton (src/game/files/wrapsources) against the originals: a tracked, shared, buffer or slice source at
# THIS whose wrapped source (its link) is one of three RefObjects with recorder vtables answering reads, writes, seeks and positions from a few choices (the position
# often the source's own, so that the shared source's sync both seeks and does not) and the lock a semaphore whose WaitForSingleObject / ReleaseSemaphore import
# slots are recorders; the buffer's data a block it reads from and seeks in (cursor at, inside, past or before it; the -0x80000000 seek from the end); the
# compressor singleton (0xfa75f8 guard, 0xfa75fc the codec) with its guard set and a recorder vtable for the forwarders, or Compressor_Get with the guard clear
# (atexit 0x67a7d0 is fncmp's cdecl log on both sides, driver.cpp's WrapSources env); the callback stream's callbacks stdcall recorders. The stream
# decompressors (Buffer_DecompressStream, BufferSource_AttachZlib) run the game's zlib codec (its real vtable) over the zlib stand-ins (fncmp.zlib_results(),
# never Z_OK for inflate, whose loop would not end): their stack CallbackStream is only ever this to the reads, which reach a recorder (the read callback, or
# the source's read) filling the buffer with a BILZ header (or not) and packed bytes; BufferSource_AttachRefPack decodes a RefPack stream the source's read
# fills (the size it answers that stream's, or not).
# usage: wrapsources.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Read,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, HEAP, RAM, _cdecl_log, fill, heap, log_region, put, recorder, set_return, vtable, zlib_results  # noqa: E402

B = Bench([(0x619460, 0x619470), (0x619600, 0x619730), (0x5f81d0, 0x5f8240), (0x638d30, 0x638d80), (0x641f10, 0x642100), (0x5f8050, 0x5f81d0), (0x5f8140, 0x5f81d0), (0x618cd0, 0x618da0), (0x638840, 0x638c60), (0x638c60, 0x638d30), (0x638d80, 0x638f00),
           (0x630810, 0x630880), (0x6418e0, 0x6419c0), (0x6ad810, 0x6ad820), (0x67acb0, 0x67acb1)],
          hooks={0x67a7d0: _cdecl_log})
THIS, RESULT, SEM = RAM, RAM + 0x40, RAM + 0x60
SOURCES, DATA, OUT = RAM + 0x1000, RAM + 0x2000, RAM + 0x3000
SOURCE_VTABLE, CODEC_VTABLE = RAM + 0x4000, RAM + 0x4100
STREAM, WORK = RAM + 0x5000, RAM + 0x6000   # the stream decompressor's buffer and work block
ZLIB_CODEC_VTABLE, BILZ = 0x703128, 0x5a4c4942
GUARD, CODEC = 0xfa75f8, 0xfa75fc
WAIT, RELEASE = 0x6fd128, 0x6fd170   # the import table's WaitForSingleObject and ReleaseSemaphore
CALLBACK = 3   # the callback stream's callbacks: stdcall recorders (buffer, count, context)
# The wrapped source's slots (stack arguments): 0 the deleting destructor, +4 read, +8 write, +0xc seek, +0x10 size, +0x14 position, +0x18 map, +0x1c unmap,
# +0x28 attach, +0x2c the lock.
SOURCE_SLOTS = {0: 1, 1: 2, 2: 2, 3: 2, 6: 2, 7: 1, 10: 1}
READ, WRITE, SEEK, SIZE, POSITION, MAP, LOCK = 1, 2, 3, 4, 5, 6, 11
# The codec's: +4 Compress (dst, src, size, work), +8 Decompress (dst, src, work), +0x14 UnpackedSize (header).
CODEC_SLOTS = {0: 1, 1: 4, 2: 3, 5: 1}
# name: (address, this: t tracked, b buffer, l slice, c callback stream, - none; stack arguments: f flags, R the result, B a buffer, n a count, o an offset,
# g an origin, S a source or none, d a dword, s a size, P a source (for the read callback), K the stream buffer, c its capacity, X the read callback,
# W the work block or none; conv; returns)
OPS = {
    'Field0c': (0x6ad810, 't', '', 't', 'a'), 'TrackedCtor': (0x6418e0, 't', '', 't', 'a'), 'TrackedDtor': (0x641900, 't', '', 't', 'v'),
    'TrackedDeletingDtor': (0x638860, 't', 'f', 't', 'a'), 'TrackedClear': (0x641910, 't', '', 't', 'v'), 'TrackedRead': (0x641920, 't', 'Bn', 't', 'a'),
    'TrackedWrite': (0x641950, 't', 'Bn', 't', 'a'), 'TrackedSeek': (0x641980, 't', 'og', 't', 'a'), 'TrackedSize': (0x6419a0, 't', '', 't', 'a'),
    'SharedCtor': (0x638840, 't', '', 't', 'a'), 'SharedRead': (0x638880, 't', 'Bn', 't', 'a'), 'SharedWrite': (0x638910, 't', 'Bn', 't', 'a'),
    'SharedSeek': (0x6389a0, 't', 'og', 't', 'a'), 'SharedSize': (0x638a30, 't', '', 't', 'a'), 'Sync': (0x638ab0, 't', '', 't', 'v'),
    'SharedAttach': (0x638ae0, 't', 'S', 't', 'v'),
    'BufferCtor': (0x638b00, 'b', '', 't', 'a'), 'ZlibBufferCtor': (0x638c90, 'b', '', 't', 'a'), 'BufferOpen': (0x638b20, 'b', 'Rs', 't', 'a'),
    'BufferWrite': (0x638b50, 'b', 'Bn', 't', 'a'), 'BufferRead': (0x638b60, 'b', 'Bn', 't', 'a'), 'BufferSeek': (0x638cb0, 'b', 'og', 't', 'a'),
    'BufferTell': (0x638bb0, 'b', '', 't', 'a'), 'BufferClear': (0x638bc0, 'b', '', 't', 'v'), 'ReadCallback': (0x638d80, '-', 'BnP', 's', 'a'),
    'CodecDtor': (0x638c60, 'b', '', 't', 'v'), 'CodecDeletingDtor': (0x638c70, 'b', 'f', 't', 'a'),
    'SliceCtor': (0x638da0, 'l', '', 't', 'a'), 'SliceOpen': (0x638dc0, 'l', 'Rss', 't', 'a'), 'SliceClear': (0x638de0, 'l', '', 't', 'v'),
    'SliceRead': (0x638df0, 'l', 'Bn', 't', 'a'), 'SliceSeek': (0x638e40, 'l', 'og', 't', 'a'), 'SliceMap': (0x638ec0, 'l', 'dd', 't', 'a'),
    'CompressorGet': (0x5f8080, '-', '', 'c', 'a'), 'CompressorAtExit': (0x5f80c0, '-', '', 'c', 'v'), 'ZlibCodecCtor': (0x5f80d0, 'b', '', 't', 'a'),
    'CodecBaseDeletingDtor': (0x5f80e0, 'b', 'f', 't', 'a'), 'Compress': (0x5f8050, '-', 'BBsd', 'c', 'a'), 'Decompress': (0x5f8100, '-', 'BBd', 'c', 'a'),
    'UnpackedSize': (0x5f8120, '-', 'B', 'c', 'a'),
    'CallbackWrite': (0x5f8140, 'c', 'Bn', 't', 'a'), 'CallbackRead': (0x5f8160, 'c', 'Bn', 't', 'a'), 'CallbackDtor': (0x5f81a0, 'c', '', 't', 'v'),
    'CallbackDeletingDtor': (0x5f8180, 'c', 'f', 't', 'a'), 'CallbackBaseDeletingDtor': (0x5f81b0, 'c', 'f', 't', 'a'),
    'DecompressStream': (0x5f81d0, '-', 'BKcXdW', 'c', 'a'), 'AttachZlib': (0x638d30, 'b', 'P', 't', 'v'), 'AttachRefPack': (0x638be0, 'b', 'P', 't', 'v'),
}


def header(r):
    """A BILZ header (or now and then not): magic, unpacked, packed, windowBits, memLevel, two zeroes."""
    magic = BILZ if r.random() < 0.85 else r.choice([0, BILZ ^ 1, r.getrandbits(32)])
    return struct.pack('<7I', magic, r.choice([0, 1, 0x10, 0x40, 0x100, r.randrange(0x100)]), r.getrandbits(32), 15, 9, 0, 0)


def refpack(r):
    """A RefPack stream: the header (0x10fb, the size big-endian), literal runs (0xe0 + n: 4n + 4 bytes), the stop (0xfc + 0-3 literals)."""
    runs = [r.choice([0, 1, 3]) for _ in range(r.choice([0, 1, 2, 3]))]
    body = b''.join(bytes([0xe0 + n]) + r.bytes(4 * n + 4) for n in runs)
    tail = r.randrange(4)
    return b'\x10\xfb' + struct.pack('>I', sum(4 * n + 4 for n in runs) + tail)[1:] + body + bytes([0xfc + tail]) + r.bytes(tail)


def source(r):
    return SOURCES + 0x40 * r.randrange(3) if r.random() < 0.9 else 0


def case(r):
    address, kind, args, conv, ret = OPS[r.choice(PICK)]
    obj = bytearray(r.bytes(0x40))
    position = r.choice([0, 1, 0x10, 0x40, r.getrandbits(32)])
    size = r.choice([0, 1, 0x10, 0x40, 0x80, r.randrange(0x100)])
    if kind == 't':   # RefObject (vtable, link, refs), position
        put(obj, 4, 'I', source(r))
        put(obj, 0xc, 'I', position)
    elif kind == 'b':   # RefObject, size, data, cursor
        put(obj, 4, 'I', source(r))
        put(obj, 0xc, 'I', size)
        put(obj, 0x10, 'I', DATA if r.random() < 0.9 else 0)
        put(obj, 0x14, 'I', DATA + r.choice([0, 1, size // 2, size, size + 4, -4, r.randrange(0x100)]))
    elif kind == 'l':   # RefObject, size, position, start, view
        put(obj, 4, 'I', source(r))
        put(obj, 0xc, 'I', size)
        put(obj, 0x10, 'I', r.choice([0, 1, size // 2, size, size + 1, r.getrandbits(32)]))
        put(obj, 0x14, 'I', r.choice([0, 0x10, 0x1000, r.getrandbits(31)]))
        put(obj, 0x18, 'I', r.choice([0, 0, DATA, r.getrandbits(32)]))
    elif kind == 'c':   # vtable, write, read, context
        struct.pack_into('<3I', obj, 4, recorder(CALLBACK, 3, True), recorder(CALLBACK + 1, 3, True), r.getrandbits(32))
    sources = bytearray(r.bytes(0xc0))
    for k in range(3):
        put(sources, 0x40 * k, 'I', SOURCE_VTABLE)
        put(sources, 0x40 * k + 4, 'I', SOURCES + 0x40 * (k + 1) if k < 2 and r.random() < 0.5 else 0)
    log = log_region()
    count = r.choice([0, 1, 4, 0x10, 0x40, 0x100])
    for slot, choices in [(READ, [0, count, count // 2, -1]), (WRITE, [0, count, -1]), (SEEK, [0, 4, size, -1, r.getrandbits(32)]), (SIZE, [0, 0x40, size]),
                          (POSITION, [position, position, 0, r.getrandbits(32)]), (MAP, [DATA, 0]), (LOCK, [SEM])]:
        if slot == LOCK or r.random() < 0.85:
            set_return(log, slot, r.choice(choices))
    if r.random() < 0.8:
        set_return(log, 40, r.choice([0, 0, 0x102]))   # WaitForSingleObject: signalled, or timed out
    if address in (0x5f81d0, 0x638d30):   # the stream decompressors: what the reads fill, the zlib stand-ins' answers
        fill(log, CALLBACK + 1 if address == 0x5f81d0 else READ, 0, header(r) + r.bytes(r.choice([0, 1, 0x10, 0x54])))
        if r.random() < 0.3:
            zlib_results(log, init=r.choice([0, 0, -2, -4, -6, 1]) if r.random() < 0.5 else None,
                         inflate=r.choice([1, 2, -3, -5]) if r.random() < 0.6 else None)
    elif address == 0x638be0:   # the packed entry: its size, then the bytes the read fills
        packed = refpack(r)
        fill(log, READ, 0, packed)
        set_return(log, SIZE, r.choice([len(packed), len(packed), 0x40, 0, size]))
    codec = struct.pack('<II', r.choice([1, 1, 1, 0, 3, 0xfe]), CODEC_VTABLE)   # the guard (bit 0: built), the codec's vtable
    if address in (0x5f8050, 0x5f8100, 0x5f8120):   # the forwarders must find the codec built: the clear guard's constructor would set the game's zlib vtable
        codec = struct.pack('<II', 1 | r.getrandbits(8), CODEC_VTABLE)
    elif address in (0x5f81d0, 0x638d30):   # the game's zlib codec, built or not (the clear guard builds it)
        codec = struct.pack('<II', r.choice([1, 1, 1, 0, 0xfe]), ZLIB_CODEC_VTABLE)
    regions = {THIS: obj, RESULT: bytes(8), SEM: struct.pack('<I', r.getrandbits(32)), SOURCES: sources, DATA: r.bytes(0x100), OUT: r.bytes(0x100),
               SOURCE_VTABLE: vtable(SOURCE_SLOTS), CODEC_VTABLE: vtable(CODEC_SLOTS), GUARD: codec, LOG: log, HEAP: heap(),
               WAIT: struct.pack('<I', recorder(40, 2, True)), RELEASE: struct.pack('<I', recorder(41, 3, True)), STREAM: header(r) + r.bytes(0x3e4),
               WORK: r.bytes(0x40)}
    stack = []
    for a in args:   # a second buffer argument (the forwarders' source) is DATA, so that swapping it with the first shows
        stack.append({'f': lambda: r.choice([0, 1, 2, 3, r.getrandbits(32)]), 'R': lambda: RESULT, 'B': lambda: DATA if OUT in stack else OUT, 'n': lambda: count,
                      'o': lambda: r.choice([0, 1, 4, size, size + 1, -1, -size, -0x80000000, r.getrandbits(32)]), 'g': lambda: r.choice([0, 1, 2, 2, 3]),
                      'S': lambda: source(r), 'd': lambda: r.getrandbits(32), 's': lambda: r.choice([0, 1, 0x10, 0x40, r.randrange(0x400)]),
                      'P': lambda: SOURCES + 0x40 * r.randrange(3), 'K': lambda: STREAM,
                      'c': lambda: r.choice([0, 1, 0x1b, 0x1c, 0x1d, 0x1e, 0x20, 0x40, 0x100, 0x200, 0x3e0]), 'X': lambda: recorder(CALLBACK + 1, 3, True),
                      'W': lambda: 0 if r.random() < 0.5 else WORK}[a]())
    return Call(address, ([THIS] if kind != '-' else []) + stack, regions, conv=conv, ret=ret)


PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
