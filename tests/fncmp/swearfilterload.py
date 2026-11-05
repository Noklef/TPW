# The swear filter's word-list load (src/game/core/swearfilter: 0x5d2de0, 0x5d3140) against the originals: the real stream, buffer and read; recorders for the buffer's open (it fills the get area and the file: the original's frame is on the stack, its address masked),
# fclose, fgetc (the file's last bytes, then eof), ios_base's init (it sets the ios, as the real one does) and the classic locale's getter. Lines stop short of 64 units (more runs away off the frame in both), a '#' first line's file at 0x170 bytes (past that, the frame's end).
# usage: swearfilterload.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=File,Load)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _free, _recorder, answers, fill_this, heap, log_region, mask_this, set_return, strings, vtable  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import stdwstring  # noqa: E402

OPEN, LOCALE_GET, FCLOSE, IOS_INIT, FGETC = 9, 15, 18, 20, 24
B = Bench([(0x405280, 0x405390), (0x45b590, 0x45b5f0), (0x479230, 0x479270), (0x47b750, 0x47b980), (0x47ba30, 0x47ba60), (0x47bad0, 0x47bc10), (0x47db20, 0x47db90), (0x4ef7b0, 0x4ef800), (0x4ef830, 0x4ef850), (0x4f0110, 0x4f0150), (0x4f1620, 0x4f1650),
           (0x4f23f0, 0x4f2450), (0x4f6020, 0x4f6090), (0x5cde40, 0x5cde60), (0x5ce920, 0x5cea40), (0x5ceaf0, 0x5ceba0), (0x5cec80, 0x5cede0), (0x5cf1c0, 0x5cf410), (0x5cfd90, 0x5cfe50), (0x5d0080, 0x5d0340), (0x5d06c0, 0x5d08e0), (0x5d0a90, 0x5d0b80), (0x5d0e00, 0x5d0e80),
           (0x5d2de0, 0x5d3240), (0x5d3530, 0x5d3690), (0x67acd0, 0x67ad00), (0x6827c0, 0x682810), (0x6918f0, 0x691a30), (0x693340, 0x6934d0), (0x693830, 0x693a00), (0x693a70, 0x693aa0)],
          hooks={0x5d08e0: _recorder(OPEN, 2), 0x67b620: _recorder(FCLOSE, 1, cdecl=True), 0x67cb90: _recorder(FGETC, 1, cdecl=True), 0x699da0: _recorder(LOCALE_GET, 0, cdecl=True), 0x693960: _recorder(IOS_INIT, 0),
                 0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None, 0x67ad90: _free})
OPS = [n for n in ('File', 'Load') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
BLOCKS = stdwstring.BLOCKS
LOCALE, LOCALE_VT, LISTS, ANSWERS, PATH, PATH2, NODES = (RAM + 0x100 * i for i in range(7))
NODES2, DATA = NODES + 0x80, RAM + 0x4000
FILE = 0x12345678


def file_text(r):
    """Wide lines: 0-63 units, some with a stray 13 or 0, ended 10 with or without 13; a '#' first low byte at times. The last line short, unended or not."""
    words = [0x41, 0x2a, 0x20, 0x7a, 0x123, 0xff9e, 0xbf, 0, 0xd, 0xa23, 0x23]
    out = []
    hashed = False
    for _ in range(r.choice([0, 1, 1, 2, 2, 3])):   # (each unit goes through the whole stream: the time goes here)
        n = r.choice([0, 0, 1, 2, 3, 5, 3 if r.random() < 0.9 else 62])
        units = [r.choice(words[:7] if r.random() < 0.9 else words) for _ in range(n)]
        hashed = hashed or (units and units[0] & 0xff == 0x23)
        if not r.randrange(8):
            hashed = True
            units[:0] = [r.choice([0x23, 0x123, 0xff23])]
        out += units[:62] + ([0xd, 0xa] if r.randrange(2) else [0xa])
    data = struct.pack('<%dH' % len(out), *out)
    if hashed:
        data = data[:0x170]
    if not r.randrange(4):
        data += struct.pack('<H', r.choice([0x41, 0x20]))    # (a last line unended)
    elif not r.randrange(8):
        data += bytes([r.choice([0x41, 0x20])])              # (half a unit: only its low byte taken)
    return data


def list_nodes(r, regions, blocks, nodes, first_slot):
    """A list's three words (allocator, sentinel, count), with 0-3 nodes at `nodes`."""
    sentinel = nodes
    n = r.choice([0, 0, 1, 2, 3])
    place = bytearray(0x20 + n * 0x20)
    addresses = [sentinel + 0x20 * i for i in range(1, n + 1)]
    ring = [sentinel] + addresses
    for i, a in enumerate(ring):
        struct.pack_into('<2I', place, a - sentinel, ring[(i + 1) % len(ring)], ring[i - 1])
    for i in range(n):
        ptr, length, res = stdwstring.string(r, blocks, first_slot + i)
        struct.pack_into('<4I', place, addresses[i] - sentinel + 8, r.getrandbits(32), ptr, length, res)
    regions[nodes] = place
    return struct.pack('<3I', r.getrandbits(32), sentinel, n)


def case(r):
    name = r.deal(OPS)
    log = log_region()
    blocks = bytearray(r.bytes(6 * stdwstring.SLOT))
    regions = {LOG: log, HEAP: heap(0x2000, fail=r.choice([0] * 39 + [1, 2])), BLOCKS: blocks}
    regions[PATH] = b'swears.txt\0'
    regions[PATH2] = b'allowed.txt\0'
    regions[LOCALE] = struct.pack('<2I', LOCALE_VT, r.choice([0, 1, 1, 2, 5, 0xffffffff]))
    regions[LOCALE_VT] = vtable({0: 1})
    regions[0xfb1dd0] = struct.pack('<I', LOCALE)
    regions[0xfb2760] = r.bytes(4)
    set_return(log, LOCALE_GET, LOCALE)
    mask_this(log, IOS_INIT)
    fill_this(log, IOS_INIT, struct.pack('<8I', 0, 0, 0x201, 6, 0, 0, 0, LOCALE), 4, 0)
    mask_this(log, OPEN)
    strings(log, OPEN, 0b01)
    data = file_text(r)
    get, tail = data, b''
    if r.random() < 0.3:     # (the last bytes through fgetc)
        k = max(0, len(data) - r.randrange(0, 24))   # (the table is spent by the first file: the second is shorter, and its failed reads look at the first's units)
        get, tail = data[:k], data[k:]
    regions[DATA] = get + r.bytes(4)
    regions[ANSWERS] = struct.pack('<40I', *([*tail] + [0xffffffff] * 40)[:40])
    answers(log, FGETC, ANSWERS)
    has_file = r.random() < 0.9
    if has_file:
        fill_this(log, OPEN, struct.pack('<I', DATA), 0x14, 1)
        fill_this(log, OPEN, struct.pack('<I', len(get)), 0x24, 2)
        fill_this(log, OPEN, struct.pack('<I', FILE), 0x50, 3)
    set_return(log, OPEN, 1 if has_file else 0)   # (the real one answers 0 just when it leaves no file. Failbit with a file would read on forever, in both)
    set_return(log, FCLOSE, r.choice([0, 0, 0, 1, 0xffffffff]))
    flag = r.choice([1, 1, 1, 1, 0xffffff01, 0, 0x100])
    if name == 'File':
        regions[LISTS] = list_nodes(r, regions, blocks, NODES, 0)
        return Call(0x5d2de0, [r.getrandbits(32), PATH, LISTS, flag], regions, conv='t', ret='b')
    regions[LISTS] = list_nodes(r, regions, blocks, NODES, 0) + list_nodes(r, regions, blocks, NODES2, 3)
    return Call(0x5d3140, [LISTS, PATH, PATH2, flag], regions, conv='t', ret='b')


if __name__ == '__main__':
    B.main(case)
