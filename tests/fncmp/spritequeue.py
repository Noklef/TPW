# The sprite queue's appends and its release (src/game/render/spritequeue: 0x58a550, 0x58a6b0, 0x58a6e0, 0x58a490) against the
# originals: lists and frame sets from empty to full, stamps matching or not, the second one's counts overrun included (0x101, 0x102); no
# queue, or one whose texture array (as texturemem.py) is set or not.
# usage: spritequeue.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ready,Add,Add2,Free)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import texturemem  # noqa: E402
from fncmp import Bench, Call, HEAP, LOG, RAM, global_heap, heap, log_region, put  # noqa: E402

B = Bench([(0x58a550, 0x58a5b9), (0x58a6b0, 0x58a6de), (0x58a6e0, 0x58a712), (0x58a490, 0x58a4ec), (0x590a70, 0x590a75), (0x590b90, 0x590c39),
           (0x579b40, 0x579c13), (0x58dc50, 0x58dd5e), (0x67afa0, 0x67b060), (0x67acb0, 0x67acb1)])
CALLS = {'Ready': 0x58a550, 'Add': 0x58a6b0, 'Add2': 0x58a6e0, 'Free': 0x58a490}
QUEUE, ARRAY = RAM + 0x8000, RAM + 0x6000   # (texturemem.py's regions below RAM + 0x4000)


def free(r):
    regions = {HEAP: heap(), **global_heap(), LOG: log_region()}
    if r.random() < 0.1:
        regions[0x8c47a0] = bytes(4)
        return Call(0x58a490, [], regions, conv='c', ret='a')
    queue = bytearray(r.bytes(0x2410))
    if r.random() < 0.8:
        n = r.choice([0, 1, 2, 3])
        regions[ARRAY] = struct.pack('<I', n) + texturemem.textures(r, regions, ARRAY + 4, n + 1)
        put(queue, 0x2400, 'I', ARRAY + 4)
    else:
        put(queue, 0x2400, 'I', 0)
    regions[QUEUE] = queue
    regions[0x8c47a0] = struct.pack('<I', QUEUE)
    return Call(0x58a490, [], regions, conv='c', ret='a')


def case(r):
    op = r.choice(OPS)
    if op == 'Free':
        return free(r)
    queue = bytearray(r.bytes(0x4010))
    if op == 'Ready':
        stamp = r.bytes(8)
        obj = bytearray(r.bytes(0x40))
        put(obj, 0xc, '8s', stamp if r.random() < 0.3 else r.bytes(8))
        if r.random() < 0.2:
            put(obj, 0xc, 'I', struct.unpack_from('<I', stamp)[0])
        put(queue, 0x240c, 'I', r.choice([0, 1, 0x7e, 0x7f, 0x80, 0x81, r.randrange(0x80)]))
        regions = {RAM: obj, RAM + 0x1000: queue, 0x8c47a0: struct.pack('<I', RAM + 0x1000), 0x8bcc40: stamp}
        return Call(0x58a550, [RAM + 0x20], regions, conv='t', ret='a')
    counts = [0, 1, 0x7f, 0x80, 0x100, 0x101, 0x102, 0x103, 0x7ff, 0x800, 0x801, r.randrange(0x800)]
    put(queue, 0x2404, 'I', r.choice(counts))
    put(queue, 0x2408, 'I', r.choice(counts))
    return Call(CALLS[op], [RAM, r.getrandbits(32)], {RAM: queue}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
