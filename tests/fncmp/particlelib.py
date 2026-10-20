# The particle library loaded (0x51f370 ParticleLib_Load, src/game/render/particles) against the original: the file through vfs.py's
# mounts and memory source (texfile.py's way), its bytes the effects' count (0-0x69 or past it) and record size (0x140 or others), their
# records, the effectors' likewise (0-0x14 or past), the two settings (about the 10-500 clamp), now and then cut short; the particles' flag
# 0x816d40 set (mostly) or not, the globals from the effects (0x804370) to it in one region with voices.py's Reset set-up.
# usage: particlelib.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import vfs  # noqa: E402
import wctimage  # noqa: E402
from fncmp import HEAP, LOG, Bench, Call, RAM, global_heap, heap, log_region, put  # noqa: E402
from gamestring import LOCALE  # noqa: E402

B = Bench(wctimage.merged(vfs.RANGES + [
    (0x41ce90, 0x41d2d0), (0x421720, 0x421780), (0x4217e0, 0x421800), (0x424cf0, 0x424d20), (0x51f370, 0x51f640), (0x51fb70, 0x51fd30),
    (0x5c4d50, 0x5c4f70), (0x5c5480, 0x5c5520), (0x5f5b80, 0x5f5bc0), (0x5f5bf0, 0x5f5c20), (0x5f5e20, 0x5f5e60), (0x5f6040, 0x5f6160),
    (0x5f61a0, 0x5f6210), (0x5f7180, 0x5f71a0), (0x5f76c0, 0x5f7740), (0x5f7950, 0x5f7990), (0x5f7d60, 0x5f7e70), (0x67ad90, 0x67af10),
    (0x67c250, 0x67c2a0), (0x67dcd0, 0x67dd50), (0x67df40, 0x67e020), (0x67efc0, 0x67f1b0), (0x67faf0, 0x67fc00), (0x680430, 0x6804e0),
    (0x682720, 0x6827d0), (0x688f50, 0x689190), (0x68a170, 0x68a3a0), (0x5da3c0, 0x5da3c1)]))
EMITTERS = RAM + 0x60000   # (Reset's records, [0x80cef8])


def part(r, limit, record):
    """a count (to limit or past it), a record size and the records' bytes, and all the bytes a seek past the rest skips (a seek that
    fails would read the effectors' count and size from the records: a size past 0x68 overruns the last effector into the emitters'
    count 0x80cedc and records [0x80cef8], which Emitters_Reset then writes through, in the driver's own memory)"""
    count = r.choice([0, 1, 2, 5, limit - 1, limit, limit, limit + 1, limit + 3, r.randrange(limit + 8)])
    size = r.choice([record, record, record, 0, 4, record - 1, record + 1, 0x10])
    body = r.bytes(min(count, limit) * size)
    if count > limit:
        body += r.bytes((count - 0x14) * size)   # (sic: the seek is by the count less 0x14)
    return struct.pack('<2I', count, size) + body


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x10000, fail=r.choice([0] * 12 + [1])), LOCALE: bytes(4), **global_heap()}
    root = vfs.mounts(r, log, regions)
    effects = part(r, 0x69, 0x140)
    data = effects + part(r, 0x14, 0x68) + struct.pack('<2I', r.choice([0, 9, 10, 11, 100, 499, 500, 501, 0xffffffff]), r.getrandbits(32))
    if r.random() < 0.1:   # (cut short past the effectors' count and size, for part()'s reason; what is not there read from random bytes)
        data = data[:r.randrange(len(effects) + 8, len(data) + 1)]
    vfs.memory_source(r, log, regions, data)
    regions[vfs.NAME] = vfs.name_under(r, root) + b'.plb' + bytes(1) + r.bytes(7)
    state = bytearray(r.bytes(0x816d44 - 0x804370))
    put(state, 0x816d40 - 0x804370, 'I', r.choice([1, 1, 1, 1, 0, 0x100]))
    put(state, 0x80cedc - 0x804370, 'I', r.choice([0, 1, 2, 3, 5, 12]))
    put(state, 0x80cef8 - 0x804370, 'I', EMITTERS)
    for k in range(0x69):   # (the effects' word +0xa2: 0 now and then)
        if r.random() < 0.3:
            put(state, 0x140 * k + 0xa2, 'H', 0)
    regions[0x804370] = state
    regions[EMITTERS - 0x40] = r.bytes(0x40 + 0x34 * 12)
    regions[0x803b58] = r.bytes(8)
    return Call(0x51f370, [vfs.NAME], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
