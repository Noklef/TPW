# The GUI library's window timers (src/game/ui/windowtimers: 0x661faa, 0x6620e2, 0x662306, 0x662500) against the originals: a list of
# 0-6 nodes (linked both ways, now and then a previous link missing), their windows among two and 0 and their ids among the window
# timers' 0x80080-0x80083 and others (repeats included), the head [0xfaa634] the first node, a later one or 0; the window and id asked
# for from the nodes' or others, the delete's flags with bit 0 set or not. New / Add (0x661f2b, 0x66220d): a pool [0xfaa630] of 0-6
# nodes, each free (+0x14 set) or not, the count [0xfaa62c] theirs, fewer, 0 or negative (the pool now and then none); Ctor / Add
# (0x661ece): the clock [0x77c490] a recorder vtable whose slot 0 answers a random time. Start (0x6620ac): a node and the clock. A
# window's SetTimer / StartTimer / StopTimer (0x65ef90, 0x65f024, 0x65f077): its four nodes +0x120 each set or 0 (one more dword after
# them), ids 0x80080-0x80084, 0-4, with other high bits or any (a large index faults in both), Add's list, pool and clock.
# usage: windowtimers.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Unlink,Find,Delete,Kill,New,Ctor,Add,Start,SetTimer,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, log_region, put, set_return, vtable  # noqa: E402

CALLS = {'Unlink': 0x661faa, 'Find': 0x6620e2, 'Delete': 0x662500, 'Kill': 0x662306, 'New': 0x661f2b, 'Ctor': 0x661ece,
         'Add': 0x66220d, 'Start': 0x6620ac, 'SetTimer': 0x65ef90, 'StartTimer': 0x65f024, 'StopTimer': 0x65f077}
B = Bench([(0x661f9b, 0x661fe5), (0x6620e2, 0x66213e), (0x662306, 0x662384), (0x662500, 0x66252c), (0x661ece, 0x661f2b),
           (0x661f2b, 0x661f9b), (0x66220d, 0x6622d2), (0x65968e, 0x6596a2),
           (0x6620ac, 0x6620cc), (0x6620cc, 0x6620e2), (0x662097, 0x6620ac), (0x65ef90, 0x65f024), (0x65f024, 0x65f077), (0x65f077, 0x65f0ca)])
NODES = RAM
POOL, CLOCK, VTABLE = RAM + 0x2000, RAM + 0x3000, RAM + 0x3100
WINDOWS = [RAM + 0x1000, RAM + 0x1200, 0]
IDS = [0x80080, 0x80081, 0x80082, 0x80083, 1, 0x10]


def pool(r, regions):
    n = r.choice([0, 1, 2, 3, 4, 6])
    buf = bytearray(r.bytes(0x20 * n))
    for i in range(n):
        put(buf, 0x20 * i + 0x14, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    if n:
        regions[POOL] = buf
    regions[0xfaa62c] = struct.pack('<i', r.choice([n, n, n, n - 1, 0, -1, r.randrange(-0x80000000, 0)]))
    regions[0xfaa630] = struct.pack('<I', POOL if n or r.random() < 0.5 else 0)


def clock(r, regions):
    log = log_region()
    set_return(log, 0, r.getrandbits(32))
    regions.update({LOG: log, VTABLE: vtable({}), CLOCK: struct.pack('<I', VTABLE), 0x77c490: struct.pack('<I', CLOCK)})


def case(r):
    op = r.deal(OPS)
    if op == 'New':
        regions = {}
        pool(r, regions)
        return Call(CALLS[op], [r.choice([0x20, r.getrandbits(32)])], regions, ret='a')
    args = [r.choice(WINDOWS + [r.getrandbits(32)]), r.choice([0x2ee, 0, r.getrandbits(32)]), r.choice(IDS + [r.getrandbits(32)])]
    if op == 'Start':
        regions = {POOL: r.bytes(0x20)}
        clock(r, regions)
        return Call(CALLS[op], [POOL], regions, conv='t', ret='a')
    if op in ('SetTimer', 'StartTimer', 'StopTimer'):
        win = bytearray(r.bytes(0x134))
        for i in range(5):
            put(win, 0x120 + 4 * i, 'I', r.choice([0, 0, NODES + 0x20 * i]))
        regions = {WINDOWS[0]: win, NODES: r.bytes(0xa0)}
        regions[0xfaa634] = struct.pack('<I', r.choice([0, NODES + 0x20 * r.randrange(5)]))
        pool(r, regions)
        clock(r, regions)
        ident = r.choice([0x80080, 0x80081, 0x80082, 0x80083, 0x80084, 0, 1, 2, 3, 4]) | r.choice([0] * 24 + [0x80000, 0x80, 0x100])
        if r.random() < 0.05:
            ident = r.getrandbits(32)
        args = [WINDOWS[0], ident] + ([r.choice([0x2ee, 0, r.getrandbits(32)])] if op == 'SetTimer' else [])
        return Call(CALLS[op], args, regions, conv='t', ret='w')
    if op == 'Ctor':
        regions = {POOL: r.bytes(0x20)}
        clock(r, regions)
        return Call(CALLS[op], [POOL] + args, regions, conv='t', ret='a')
    n = r.choice([0, 1, 1, 2, 3, 4, 6]) if op in ('Kill', 'Add') else r.choice([1, 2, 3, 4, 6])
    nodes = [NODES + 0x20 * i for i in range(n)]
    buf = bytearray(r.bytes(0x20 * n))
    for i in range(n):
        put(buf, 0x20 * i, 'I', r.choice(WINDOWS[:2] * 3 + [0, r.getrandbits(32)]))
        put(buf, 0x20 * i + 0xc, 'I', r.choice(IDS))
        put(buf, 0x20 * i + 0x18, 'I', nodes[i + 1] if i + 1 < n else 0)
        put(buf, 0x20 * i + 0x1c, 'I', nodes[i - 1] if i and r.random() < 0.9 else 0)
    regions = {NODES: buf} if n else {}
    regions[0xfaa634] = struct.pack('<I', r.choice([nodes[0]] * 6 + nodes + [0]) if n else 0)
    window = r.choice(WINDOWS + [r.getrandbits(32)])
    ident = r.choice(IDS + [r.getrandbits(32)])
    if r.random() < 0.5 and n:   # one that is there
        k = r.randrange(n)
        window, ident = struct.unpack_from('<I', buf, 0x20 * k)[0], struct.unpack_from('<I', buf, 0x20 * k + 0xc)[0]
    if op == 'Add':
        pool(r, regions)
        clock(r, regions)
        return Call(CALLS[op], args, regions, ret='a')
    if op == 'Kill':
        return Call(CALLS[op], [window, ident], regions, ret='a')
    node = r.choice(nodes)
    if op == 'Find':
        return Call(CALLS[op], [node, window, ident], regions, conv='t', ret='a')
    if op == 'Delete':
        return Call(CALLS[op], [node, r.choice([0, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')
    return Call(CALLS[op], [node], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
