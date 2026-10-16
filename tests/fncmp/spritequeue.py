# The sprite queue's appends (src/game/render/spritequeue: 0x58a550, 0x58a6b0, 0x58a6e0) against the
# originals: lists and frame sets from empty to full, stamps matching or not, the second one's counts overrun included (0x101, 0x102).
# usage: spritequeue.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ready,Add,Add2)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x58a550, 0x58a5b9), (0x58a6b0, 0x58a6de), (0x58a6e0, 0x58a712)])
CALLS = {'Ready': 0x58a550, 'Add': 0x58a6b0, 'Add2': 0x58a6e0}


def case(r):
    op = r.choice(OPS)
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
