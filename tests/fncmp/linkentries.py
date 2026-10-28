# A coaster link's entries removed and freed, and the track cleared under them, against the original: TrackCell_ClearAt (0x452ca0,
# src/game/world/trackline), CoasterLink_RemoveEntry (0x442ea0) and CoasterLink_FreeEntries (0x43eaf0, both src/game/coaster/registry).
# trackline.py's map (trackremove.py's scene in the game's layout) with its size for the clamps ([0x7a0314] columns, [0x7a0374] rows: the
# laid-out map's mostly, now and then smaller, 1, 0 or below), the spots on it mostly, now and then off it either way (TrackCell_ClearAt's
# track cell there of a line kind now and then, owning the cells east, south and south-east mostly). The link: its
# chain (+0x4c first, +0x50 last, +0x40 count) of up to four 0x48-byte entries (next +0x28, previous +0x2c, +0 their index, +4 flag 1
# now and then, +0xc / +0x10 the spot, +0x18 a car with flags 4 / 8 (+0) or not, +0x30 a count of up to three cars from +0x34), now and
# then its first and last not the chain's. The cars as carfree.py's (their rides (+0x70) to Ride_Destroy 0x463c80, a cdecl recorder 31;
# their blocks freed through the GlobalFree import); each entry freed that way too.
# usage: linkentries.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=ClearAt,Remove,FreeAll)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, Bench, Call, global_heap, heap, put  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import cellapply  # noqa: E402
import trackline  # noqa: E402
import trackremove as tr  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

ident = trackline.ident
B = Bench(cellapply.merged(trackline.RANGES + [(0x452ca0, 0x452d24), (0x442ea0, 0x442f35), (0x43eaf0, 0x43eb6b), (0x434570, 0x43464e),
                                                (0x433240, 0x433277), (0x4346c0, 0x434700), (0x4418d0, 0x4418f0)]),
          sines=256, hooks=trackline.HOOKS)
LINK = 0x3c000000
ENTRIES, CARS, DEF = LINK + 0x1000, LINK + 0x2000, LINK + 0x8000
OTHER = DEF + 0x100


def spot(r, size):
    return r.choice([r.randrange(size)] * 6 + [-1, -50, size, 300])


def car(r):
    data = bytearray(r.bytes(0x110))
    put(data, 0x70, 'I', r.choice([0, 0, 1, 7, r.getrandbits(32)]))
    for o in (0x7c, 0x80, 0x88, 0x8c):
        put(data, o, 'I', r.choice([0, LINK + 0x20000 + o * 0x10, r.getrandbits(32)]))
    put(data, 0x100, 'I', DEF)
    return data


def case(r):
    regions = dict(trackline.case(r).regions)
    w, h = struct.unpack('<i', regions[0x81d740])[0], struct.unpack('<i', regions[0x81b1b0])[0]
    wl, hl = max(w, 4) + 4, max(h, 4) + 4   # (the laid-out map)
    regions.update({0x7a0314: struct.pack('<i', r.choice([wl] * 4 + [wl - 2, 1, 0, -3])),
                    0x7a0374: struct.pack('<i', r.choice([hl] * 4 + [hl - 2, 1, 0, -3])), HEAP: heap(), **global_heap()})
    op = r.deal(OPS)
    if op == 'ClearAt':
        x, y = spot(r, wl), spot(r, hl)
        sx = min(max(x, 0), struct.unpack('<i', regions[0x7a0314])[0] - 1)
        sy = min(max(y, 0), struct.unpack('<i', regions[0x7a0374])[0] - 1)
        if 0 <= sx < wl - 1 and 0 <= sy < hl - 1 and r.random() < 0.6:   # (its track cell of a line kind, owning those after it)
            trackline.put_kind(regions, sx, sy, r.choice(tr.LINES))
            for dx, dy in ((1, 0), (0, 1), (1, 1)):
                if r.random() < 0.8:
                    trackline.put_owner(regions, sx + dx, sy + dy, ident(sx, sy))
        return Call(0x452ca0, [x, y], regions, ret='v')
    definition = bytearray(r.bytes(8))
    definition[4] = r.getrandbits(8) & ~1 | r.choice([0, 1])
    other = bytearray(r.bytes(0x10))
    other[0] = r.getrandbits(8) & ~0xc | r.choice([0, 4, 4, 8, 0xc])
    regions.update({DEF: definition, OTHER: other})
    cars = [CARS + i * 0x200 for i in range(8)]
    for c in cars:
        regions[c] = car(r)
    regions[0x79c640] = struct.pack('<I', r.choice(cars[:2] + [OTHER, 0, r.getrandbits(32)]))
    n = r.choice([0, 1, 2, 3, 4, 4] if op == 'FreeAll' else [1, 2, 3, 4, 4])
    entries = [ENTRIES + i * 0x80 for i in range(n)]
    for i, e in enumerate(entries):
        data = bytearray(r.bytes(0x48))
        put(data, 0, 'I', r.choice([i, i, r.getrandbits(32)]))
        data[4] = r.getrandbits(8) & ~1 | r.choice([0, 0, 1])
        put(data, 0xc, 'i', spot(r, wl))
        put(data, 0x10, 'i', spot(r, hl))
        put(data, 0x18, 'I', OTHER)
        put(data, 0x28, 'I', entries[i + 1] if i + 1 < n else 0)
        put(data, 0x2c, 'I', entries[i - 1] if i else 0)
        put(data, 0x30, 'I', r.choice([0, 1, 2, 3]))
        for k in range(5):
            put(data, 0x34 + k * 4, 'I', r.choice(cars))
        regions[e] = data
    link = bytearray(r.bytes(0x60))
    first, last = (entries[0], entries[-1]) if entries else (0, 0)
    if r.random() < 0.15:
        first, last = r.choice(entries + [0]), r.choice(entries + [0])
    put(link, 0x4c, 'I', first)
    put(link, 0x50, 'I', last)
    put(link, 0x40, 'I', r.choice([n, n, 0, r.getrandbits(32)]))
    regions[LINK] = link
    if op == 'FreeAll':
        return Call(0x43eaf0, [LINK], regions, ret='v')
    return Call(0x442ea0, [LINK, r.choice(entries), r.choice([0, 0, 1, r.getrandbits(32)])], regions, ret='v')


CALLS = ['ClearAt', 'Remove', 'FreeAll']
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
