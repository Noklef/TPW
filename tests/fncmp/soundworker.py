# The streamed-sound worker (src/game/core/soundworker) and a channel's update record (SoundUpdate_Apply, src/game/core/soundchannel) against the originals, on channelpool.py's fixture.
# - fill: the buffer's Lock answering or failing, its pointers a destination; the mix over channelpool's voice; [0xfb2724]'s slot 10. A null buffer now and then.
# - refill: a slot table of those buffers, positions under and over the count. - release: a list of 0-3 COM objects, its links whole.
# - worker / host: the counts round 30, the heap failing now and then; 0x67e050 a cdecl recorder. - update: each flag on and off with a recorder source.
# usage: soundworker.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, IMPORT_TABLE, RAM, Bench, Call, _recorder, fill, heap, import_table, recorder, log_region, set_return, strings, vtable  # noqa: E402
import channelpool as cp  # noqa: E402
import channelvoice as cv  # noqa: E402

CALLS = {'fill': (0x6d6f70, 'v'), 'refill': (0x6d3850, 'v'), 'release': (0x6d75c0, 'v'), 'worker': (0x6b6050, 'a'), 'host': (0x6b8380, 'a')}
B = Bench(cv.RANGES + [(0x6b7380, 0x6b738a), (0x6baa20, 0x6baa5d), (0x6bb320, 0x6bb3dc), (0x6bb510, 0x6bb587), (0x41a830, 0x41a850), (0x6d6db0, 0x6d6dc4), (0x6d6f70, 0x6d7053),
           (0x6d3850, 0x6d38fb), (0x6d75c0, 0x6d7685), (0x6b6050, 0x6b6120), (0x6b8380, 0x6b8402), (0x6b7040, 0x6b70ae), (0x6b7210, 0x6b7222), (0x67acf0, 0x67ada0), (0x67afa0, 0x67b0d0),
           (0x6bb9f0, 0x6bba81), (0x6bb940, 0x6bb990), (0x6b7150, 0x6b7190), (0x6b70c0, 0x6b70e0), (0x6b71a0, 0x6b71d0)],
          hooks={0x67e050: _recorder(14, 3, cdecl=True)})
BASE = RAM + 0x10000
STREAM, BUF, BUFVT, DEST, WORKER, WORKVT, OWNER, SLOTS, LIST, NODES, COMOBJ, COMVT, BLOCK, HOST = (BASE + 0x400 * i for i in range(14))
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
IMPORTS = dict(cp.IMPORTS, InitializeCriticalSection=(49, 0x6fd144, 1, 0, 0))
LOCK_K, UNLOCK_K, WORK_K, STOP_K, RELEASE_K = 13, 15, 16, 17, 18


def buffers(r, log, regions, n):
    """n holders of a recorder buffer, Lock giving DEST and a small size, a voice as channelpool's mix."""
    regions[BUF] = struct.pack('<I', BUFVT) + r.bytes(0xc)
    vt = vtable({})
    struct.pack_into('<I', vt, 0x2c, recorder(LOCK_K, 8, com=True))
    struct.pack_into('<I', vt, 0x30, recorder(UNLOCK_K, 5, com=True))
    regions[BUFVT] = vt
    set_return(log, LOCK_K, r.choice([0, 0, 0, 0, 1, 0xffffffff, 0x80000000]))
    set_return(log, UNLOCK_K, r.getrandbits(32))
    strings(log, LOCK_K, 0, out=0x78)
    size = r.choice([0, 1, 3, 4, 7, 0x20, 0x40])
    fill(log, LOCK_K, 3, struct.pack('<I', DEST))
    fill(log, LOCK_K, 4, struct.pack('<I', size), which=1)
    strings(log, UNLOCK_K, 0)
    regions[DEST] = r.bytes(0x40)
    set_return(log, 48, r.choice([0, 0x102, 0x102, 1, 0x80]))
    v = regions[cv.VOICES]
    for i in range(len(v) // 0x8c):
        struct.pack_into('<II', v, 0x8c * i + 0x68, RAM + 0x7000, r.getrandbits(32))
        struct.pack_into('<II', v, 0x8c * i + 0x70, cv.OUT, 0)
        struct.pack_into('<I', v, 0x8c * i + 0x30, r.choice([0, 0, 1, r.getrandbits(32)]))
        struct.pack_into('<I', v, 0x8c * i + 0x20, r.choice([0, 0x100, r.getrandbits(32)]))
    regions.update({cp.COPIER: struct.pack('<I', cp.COPYVT) + r.bytes(4), cp.COPYVT: vtable({0: 3}), 0xfb2018: struct.pack('<I', cp.COPIER)})
    wv = vtable({})
    struct.pack_into('<I', wv, 0x28, recorder(WORK_K, 2))
    strings(log, WORK_K, 0, out=1)
    regions.update({WORKER: struct.pack('<I', WORKVT) + r.bytes(0xc), WORKVT: wv, 0xfb2724: struct.pack('<I', WORKER)})
    regions[SLOTS] = b''.join(struct.pack('<I', r.choice([BUF, BUF, BUF, 0])) + r.bytes(0x28) for _ in range(n))


def nodes(r, regions):
    count = r.choice([0, 1, 2, 3])
    data = bytearray(0x10 * max(count, 1))
    for i in range(count):
        struct.pack_into('<III', data, 0x10 * i, COMOBJ, NODES + 0x10 * (i + 1) if i + 1 < count else 0, NODES + 0x10 * (i - 1) if i else 0)
    regions[NODES] = data
    regions[LIST] = struct.pack('<4I', r.getrandbits(32), NODES if count else 0, NODES + 0x10 * (count - 1) if count else 0, count) + r.bytes(0x20)
    regions[COMOBJ] = struct.pack('<I', COMVT)
    vt = vtable({})
    struct.pack_into('<I', vt, 0x18, recorder(STOP_K, 1, com=True))
    struct.pack_into('<I', vt, 8, recorder(RELEASE_K, 1, com=True))
    regions[COMVT] = vt


def case(r):
    name = r.deal(OPS)
    address, ret = CALLS[name]
    log = log_region()
    n = r.choice([1, 1, 2, 3, 4])
    regions = cp.pool_regions(r, log, n)
    regions[IMPORT_TABLE] = import_table(log, IMPORTS)
    if name == 'fill':
        buffers(r, log, regions, 1)
        regions[STREAM] = struct.pack('<I', r.choice([BUF, BUF, BUF, BUF, 0]))
        return Call(address, [STREAM, cv.VOICES, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret=ret)
    if name == 'refill':
        buffers(r, log, regions, n)
        regions[OWNER] = struct.pack('<I', r.getrandbits(32)) + struct.pack('<I', n) + r.bytes(0x20) + struct.pack('<I', SLOTS) + r.bytes(8) + struct.pack('<I', cv.POOL) + r.bytes(0x20)
        return Call(address, [OWNER, r.choice([0, 1, n - 1, n, n + 1, 2 * n, r.randrange(0x100), r.getrandbits(32)])], regions, conv='t', ret=ret)
    if name == 'release':
        nodes(r, regions)
        regions[HEAP] = heap(0x100)
        return Call(address, [LIST], regions, conv='t', ret=ret)
    regions[HEAP] = heap(0x1000, fail=r.choice([0, 0, 0, 0, 1, 2]))
    if name == 'worker':
        regions[BLOCK] = r.bytes(0x60)
        return Call(address, [BLOCK, r.choice([0, 1, 12, 29, 30, 31, 0x7fffffff, 0x80000000, r.getrandbits(32)])], regions, conv='t', ret=ret)
    regions[HOST] = r.bytes(0x40)
    return Call(address, [HOST], regions, conv='t', ret=ret)


if __name__ == '__main__':
    B.main(case)
