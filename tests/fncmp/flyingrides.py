# The flying rides' accessors (src/game/world/flyingrides: 0x55a570, 0x55d610-0x55de30) against
# the originals: indices in and out of 1-99, empty entries, slots in state 1 or not with steps
# either side of 0, and the ride part a step releases there, absent or not. Init (0x55a5a0): see init().
# usage: flyingrides.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=FirstRide,In,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, Bench, Call, HEAP, RAM, blob, global_heap, heap, import_table, log_region, put, set_return, vtable, LOG  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import particles  # noqa: E402

B = Bench([(0x55a570, 0x55a592), (0x55d610, 0x55d716), (0x55d720, 0x55d82f), (0x55d950, 0x55d9a8),
           (0x55d9b0, 0x55d9e1), (0x55d9f0, 0x55da61), (0x55dcf0, 0x55dd20), (0x55dd20, 0x55dd3f),
           (0x55dd40, 0x55dd74), (0x55dd80, 0x55ddc3), (0x55ddd0, 0x55ddf0), (0x55ddf0, 0x55de10),
           (0x55de10, 0x55de30), (0x55de30, 0x55de65), (0x44b220, 0x44b267), (0x44b4c0, 0x44b505),
           (0x475550, 0x47558b), (0x55e330, 0x55e3a8), (0x55c270, 0x55c340), (0x55ab50, 0x55abf0), (0x52c6c0, 0x52c6f0), (0x55a5a0, 0x55a61f),
           (0x5f5660, 0x5f5690), (0x67a830, 0x67a860),
           (0x55da70, 0x55dd00), (0x55aa80, 0x55ab60), (0x516330, 0x516380), (0x51bfc0, 0x51c060), (0x51feb0, 0x51ff20), (0x5da3c0, 0x5da3c1),
           (0x45bd30, 0x45bdb0), (0x472310, 0x4726e0), (0x472bc0, 0x472c70), (0x472d70, 0x472ef0), (0x472f60, 0x473280), (0x4732a0, 0x473450),
           (0x576270, 0x576320), (0x576410, 0x576550), (0x57a260, 0x57a430), (0x67a400, 0x690000)] + particles.RANGES)
TABLE, RIDES, SCRIPTS = 0x8791f8, 0x7a4610, 0x7b49f0
HOLDER, H8, MODEL, PARTS, PS, PSE, REC = (RAM + o for o in (0, 0x40, 0x100, 0x200, 0x300, 0x400, 0x1000))
CALLS = {'FirstRide': 0x55a570, 'In': 0x55d610, 'Out': 0x55d720, 'Cap': 0x55d950, 'Tenth': 0x55d9b0,
         'On': 0x55d9f0, 'Set4': 0x55dcf0, 'Get3c': 0x55dd20, 'Ready': 0x55dd40, 'Take3c': 0x55dd80,
         'Set44': 0x55ddd0, 'Set20': 0x55ddf0, 'Set24': 0x55de10, 'Busy': 0x55de30, 'Part': 0x55e330, 'Corner': 0x55c270, 'Bounds': 0x55ab50, 'Init': 0x55a5a0,
         'Mode': 0x55da70}
RETURNS = {'FirstRide', 'In', 'Out', 'Get3c', 'Ready', 'Take3c', 'Busy'}


def small(r):
    return r.choice([0, 1, 2, 3, -1, r.randrange(-5, 6), r.getrandbits(32) - (1 << 31)])


def record(r, op):
    rec = bytearray(r.bytes(0x122c))
    for off in (0, 4, 0xc, 0x34, 0x38, 0x3c, 0x48):
        put(rec, off, 'i', r.choice([0, 1]) if op in ('Take3c', 'Busy') and r.random() < 0.6 else small(r))
    if r.random() < 0.3:
        put(rec, 0x34, 'I', 3)
    first = r.choice([0, 1, 5, 19, r.randrange(20)])
    for i in range(20):
        s = 0x5c + i * 0xe4
        if op in ('In', 'Out', 'On', 'Ready', 'Mode') or r.random() < 0.3:
            state = 1 if i == first else r.choice([0, 0, 2, 1 if r.random() < 0.2 else 3])
            put(rec, s + 0x80, 'I', state)
            step = r.choice([0, 0, 1, 2, 11, 12, -1, -2, -11, -12]) if i >= first or r.random() < 0.3 else 0
            put(rec, s + 0x8c, 'i', step)
        put(rec, s + 0x84, 'i', r.choice([0, 1, 4, 0x12c, -3, r.getrandbits(32) - (1 << 31)]))
        put(rec, s + 0xb8, 'I', r.choice([0, 1, r.getrandbits(32)]))
        put(rec, s + 0xc0, 'I', r.choice([0, 1, 2, 3]))
        put(rec, s + 0xd8, 'i', r.choice([0, 1, 3, r.randrange(-8, 9)]))
        put(rec, s + 0xdc, 'i', r.choice([0, 1, 3, 4, 7, -3]))
    return rec


def init(r):
    """0x55a5a0: the flag 0x8791ec 0 mostly (else nothing done), the table and 0x8791f0 / 0x8791c8 any; the sine table from GlobalAlloc
    (the heap stubs; never failing: the table is written unchecked)."""
    log = log_region()
    table = import_table(log, {})
    for at, stub in global_heap().items():
        table[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    flag = 0 if r.random() < 0.85 else r.choice([1, r.getrandbits(32)])
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(0x5000), 0x8791c8: r.bytes(4), 0x8791ec: struct.pack('<I', flag) + r.bytes(0x19c)}
    return Call(0x55a5a0, [], regions, ret='v')


def case(r):
    op = r.choice(OPS)
    if op == 'Init':
        return init(r)
    index = r.choice([1, 2, 99, r.randrange(1, 100)] * 4 + [0, 100, -1, r.getrandbits(32) - (1 << 31)])
    table = bytearray(r.bytes(0x194))  # and the entry past the end, at 100
    if 0 < index <= 100:
        put(table, index * 4, 'I', REC if r.random() < 0.9 else 0)
    rides = bytearray(r.bytes(0x10))
    for k in range(1, 4):
        put(rides, k * 4, 'I', HOLDER)
    holder = bytearray(r.bytes(0x40))
    put(holder, 8, 'I', H8)
    h8 = bytearray(r.bytes(0x40))
    put(h8, 4, 'I', MODEL)
    put(h8, 0x28, 'I', PS)
    model = bytearray(r.bytes(0x100))
    n = r.choice([0, 1, 4, 8])
    put(model, 0x48, 'H', n)
    put(model, 0x7c, 'I', PARTS)
    parts = bytearray(r.bytes(0xa0))
    for i in range(8):
        put(parts, i * 0x14, '2I', r.choice([0x80, 0x81, 1, 0, 0x100, r.getrandbits(32)]), r.choice([1, 2, 3, 4, 5, 8]))
    ps = bytearray(r.bytes(0x10))
    put(ps, 4, 'I', PSE)
    put(ps, 8, 'I', r.choice([1, 2, 5]))
    scripts = bytearray(r.bytes(0x30))
    put(scripts, 0x24, 'I', 0)
    regions = {TABLE: table, RIDES: rides, SCRIPTS: scripts, HOLDER: holder, H8: h8, MODEL: model,
               PARTS: parts, PS: ps, PSE: r.bytes(0xa0), REC: record(r, op)}
    args = [index & 0xffffffff]
    if op == 'Mode':
        return mode_case(r, index, regions)
    if op == 'Corner':
        cam = bytearray(r.bytes(0x100))
        idx = r.randrange(1, 100)
        put(cam, 0, 'I', idx)
        put(cam, 0x88, 'I', r.choice([0, 1, 2, 3, 4, 7, 0xffffffff, r.getrandbits(32)]))
        rec = bytearray(r.bytes(0x40))
        for off in (0x14, 0x18, 0x1c):
            put(rec, off, 'i', r.choice([0, 1000, -1000, 9000, 14000, 20000, r.randrange(-60000, 60000)]))
        tbl = bytearray(r.bytes(0x194))
        put(tbl, idx * 4, 'I', RAM + 0x2000)
        regions[TABLE] = tbl
        regions[RAM + 0x1000] = cam
        regions[RAM + 0x2000] = rec
        return Call(CALLS[op], [RAM + 0x1000], regions, ret='v')
    if op == 'Bounds':
        regions.update({0x7660b8: r.bytes(0x10), 0x8186e0: r.bytes(0x10), 0x818c00: r.bytes(0x10)})
        return Call(CALLS[op], [], regions, ret='v')
    if op == 'Part':
        args = [(index if 0 < index < 100 else 1) & 0xffffffff, r.randrange(20), r.choice([0, 1, 9, 10, -1, -9, 64, r.getrandbits(32)]) & 0xffffffff]
        regions[TABLE] = bytearray(regions[TABLE])
        put(regions[TABLE], args[0] * 4, 'I', REC)
        return Call(CALLS[op], args, regions, ret='v')
    if op not in RETURNS:
        args.append(r.choice([0, 1, 9, 10, 64, 63, -64, -65, -10, 0x12c * 64, r.getrandbits(32)]) & 0xffffffff)
    return Call(CALLS[op], args, regions, ret='a' if op in RETURNS else 'v')


MANAGER, VT, SINES, NODES, SEED = RAM + 0x3000, RAM + 0x3100, RAM + 0x4000, RAM + 0x5000, RAM + 0x6000


def mode_case(r, index, regions):
    """0x55da70 (index, mode): case()'s record (its slots stepping out as op Out has them) with the mode +0x34 the new one now and then
    (nothing done), +0x114 / +0xd8 / +0x120 set or not (the handle one of particles.spawn()'s systems, its sines and nodes moved clear
    of the record, or any), positions about multiples of 300; slot 0 without a ride or already in the motion (0xa, 0, 0) (no animator
    is served: a changed motion crashes); ridesound.py's sound globals and manager (its command logged), the world's seed ([0x7cf83c]
    + 0x1da708) any."""
    rec = bytearray(regions[REC])
    mode = r.choice([1, 2, 3, 4] * 4 + [0, 5, -1, r.getrandbits(32)]) & 0xffffffff
    put(rec, 0x34, 'I', mode if r.random() < 0.1 else r.choice([0, 1, 2, 3, 4, 5]))
    put(rec, 0x114, 'I', r.choice([0, 1, r.getrandbits(32)]))
    put(rec, 0xd8, 'I', r.choice([0, 1, r.getrandbits(32)]))
    put(rec, 0x28, 'i', small(r))
    for off in (0x60, 0x64, 0x68):
        put(rec, off, 'I', r.choice([0, 299, 300, -300, -301, r.randrange(-10 ** 6, 10 ** 6), r.getrandbits(32)]) & 0xffffffff)
    if r.random() < 0.5:
        put(rec, 0x5c + 0xc0, 'I', 0)
    else:
        put(rec, 0x5c + 0xcc, '3I', 0xa, 0, 0)
    p = dict(particles.spawn(r).regions)
    p[SINES], p[NODES] = p.pop(particles.SINES), p.pop(particles.NODES)
    p[0x80ced0] = put(put(bytearray(p[0x80ced0]), 0x1c, 'I', SINES), 0x28, 'I', NODES)
    systems = p[0x80cefc]
    handles = [int.from_bytes(systems[0x140 * k + 0xa:0x140 * k + 0xc], 'little') << 16 | k for k in range(6)]
    put(rec, 0x120, 'I', r.choice([0, 0, r.choice(handles), r.choice(handles), r.getrandbits(32)]))
    regions.update(p)
    log = log_region()
    g = bytearray(r.bytes(0x20))   # 0x802bc0..
    put(g, 8, 'I', r.choice([0, 1, 1, 1, 1]))
    put(g, 0xc, 'I', MANAGER)
    put(g, 0x14, 'I', r.choice([0, 1, 1, 1, 1]))
    blob(log, 2, 1, 0x7f)
    if r.random() < 0.85:
        set_return(log, 2, r.getrandbits(32))
    regions.update({LOG: log, 0x802bc0: g, 0x8791a0: struct.pack('<I', r.choice([0, 1, 1, 1, 1])), MANAGER: struct.pack('<I', VT) + r.bytes(0xc),
                    VT: vtable({2: 2}, 4), 0x803a28: r.bytes(4), 0x7cf83c: struct.pack('<I', SEED - 0x1da708), SEED: r.bytes(4), REC: rec})
    return Call(CALLS['Mode'], [index & 0xffffffff, mode], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
