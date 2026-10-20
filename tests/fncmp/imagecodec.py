# The image codecs' loaders and the TGA codec (src/game/gfx/imagecodec: 0x5fa8a0-0x5fad34, thiscall) against the originals: the codec the game's
# TGA codec (vtable 0x6fefa8), the stream a StreamBase (the image loaders' file stream vtable, or the base's) over a MemorySource of one chunk
# holding a TGA file (tga.py's, now and then with the TGA 2.0 footer, or cut short), the reads going through the real source, buffered source
# and TGA reader code on both sides; the file loaders' StreamBase_OpenIn a stand-in (_open_in here, driver.cpp stub_open_in): the name's first
# dword the result, its second the source it attaches; InterlockedIncrement / Decrement recorders (Decrement now and then 0: the buffered
# source torn down); a destination of tga.py's (or one without bits), an info out or none.
# usage: imagecodec.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=InfoFile,InfoStream,LoadFile,LoadStream,GetInfo,Load,IsTga)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, Bench, Call, RAM, global_heap, heap, import_table, log_region, put, set_return  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from tga import ARGB8888, PAL8, RGB555, RGB888, dest, header, tga  # noqa: E402


def _open_in(e):  # thiscall (out, name, container): pops its three arguments (driver.cpp stub_open_in)
    sp, s = e.r['ESP'], e.r['ECX']
    out, name = e.u32(sp), e.u32(sp + 4)
    result, source = e.u32(name), e.u32(name + 4)
    e.write(out, struct.pack('<I', result))
    if result < 0x80000000:
        e.write(s + 4, struct.pack('<I', source))
    e.r['EAX'] = out
    e.r['ESP'] = sp + 12


B = Bench([(0x5f5660, 0x5f9000), (0x5fa8a0, 0x5fad50), (0x618000, 0x61af40), (0x62fa40, 0x6306b2), (0x6307c0, 0x630c97), (0x639230, 0x63b2c0),
           (0x642860, 0x6428a5), (0x67a400, 0x690000), (0x4036a0, 0x4036c0), (0x4217e0, 0x421800), (0x5da3c0, 0x5da3d0),
           (0x637460, 0x639230), (0x641500, 0x642400)], hooks={0x62fa40: _open_in})
CODEC, OUT, INFO, DSTI, STREAM, NAME, SOURCE, CHUNK, DATA, BITS = (RAM, RAM + 0x480, RAM + 0x490, RAM + 0x4c0, RAM + 0x500, RAM + 0x520, RAM + 0x540,
                                                                   RAM + 0x580, RAM + 0x1000, RAM + 0x4000)
FORMATS = 0xfaa268
FOOTER = b'TRUEVISION-XFILE.\x00'
CALLS = {'InfoFile': 0x5fa8a0, 'InfoStream': 0x5fa970, 'LoadFile': 0x5faa00, 'LoadStream': 0x5faab0, 'GetInfo': 0x5fabb0, 'Load': 0x5fabe0,
         'IsTga': 0x5fac10}
INCREMENT, DECREMENT = 50, 51
IMPORTS = {'InterlockedIncrement': (INCREMENT, 0x6fd0c0, 1, 0, 0), 'InterlockedDecrement': (DECREMENT, 0x6fd12c, 1, 0, 0), 'CloseHandle': (42, 0x6fd13c, 1, 0, 0),
           'CreateSemaphoreA': (43, 0x6fd16c, 4, 8, 0)}


def source(n):
    """A MemorySource of one chunk over DATA holding n bytes, at the start, and its chunk."""
    src = struct.pack('<11I', 0x703868, 0, 5, CHUNK, CHUNK, CHUNK, DATA, 0, n, 0x10000, 0)
    return src, struct.pack('<5I', 0, 0, DATA, 0, 0x10000)


def case(r):
    op = r.deal(OPS)
    h = header(r)
    data = tga(r, h)
    k = r.random()
    if k < 0.15:   # the TGA 2.0 footer
        data += r.bytes(8) + FOOTER
    elif k < 0.2:
        data = data[:r.randrange(0x13)]
    log = log_region()
    set_return(log, INCREMENT, r.choice([1, 2, 2]))
    set_return(log, DECREMENT, r.choice([0, 1, 1, 1, 2]))
    src, chunk = source(len(data))
    regions = {CODEC: r.bytes(0x450), OUT: r.bytes(4), INFO: r.bytes(0x28), FORMATS: struct.pack('<24I', *RGB888, *ARGB8888, *PAL8, *RGB555),
               HEAP: heap(), BITS: r.bytes(0x3000), DATA: data + bytes(1), SOURCE: src, CHUNK: chunk, LOG: log, IMPORT_TABLE: import_table(log, IMPORTS),
               STREAM: struct.pack('<2I', r.choice([0x7035e0, 0x7035e0, 0x703100]), SOURCE),
               NAME: struct.pack('<2I', r.choice([0, 0, 0, 0, 1, 0xffffffff, 0x80000000]), SOURCE)}
    regions.update(global_heap())
    codec = bytearray(regions[CODEC])
    put(codec, 0, 'I', 0x6fefa8)
    regions[CODEC] = codec
    dst = bytearray(dest(r, h))
    if r.random() < 0.1:   # (a destination without bits)
        put(dst, 0x18, 'I', 0)
    regions[DSTI] = dst
    info = INFO if r.random() < 0.8 else 0
    address = CALLS[op]
    if op == 'InfoFile':
        return Call(address, [CODEC, OUT, NAME, NAME + 4, info], regions, conv='t', ret='a')
    if op == 'InfoStream':
        return Call(address, [CODEC, OUT, STREAM, info], regions, conv='t', ret='a')
    if op == 'LoadFile':
        return Call(address, [CODEC, OUT, DSTI, NAME, NAME + 4, info], regions, conv='t', ret='a')
    if op == 'LoadStream':
        return Call(address, [CODEC, OUT, DSTI, STREAM, info], regions, conv='t', ret='a')
    if op == 'GetInfo':
        return Call(address, [CODEC, OUT, STREAM, r.getrandbits(32)], regions, conv='t', ret='a')
    if op == 'Load':
        return Call(address, [CODEC, OUT, DSTI, STREAM, r.getrandbits(32)], regions, conv='t', ret='a')
    if len(data) >= 0x12 and r.random() < 0.5:   # (IsTga's checks at their edges)
        edge = bytearray(regions[DATA])
        edge[1], edge[2], edge[16] = r.choice([0, 1]), r.choice([0, 3, 9, 11]), r.choice([8, 15, 16, 24, 32])   # (all valid, then one at an edge)
        at = r.choice([1, 2, 16])
        edge[at] = r.choice({1: [2, 0xff], 2: [4, 8, 12, 0x83], 16: [7, 9, 14, 17, 23, 25, 31, 33]}[at])
        regions[DATA] = edge
    return Call(address, [CODEC, STREAM], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
