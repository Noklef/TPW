# The particle system's set-up (src/game/render/particles: 0x51faa0) against the original: its flag 0x816d40 clear mostly (set: nothing
# done), the globals from the sine table (0x803b68) to the flag in one region of random bytes with guards, the arguments a callback or
# none (0x5d60c0 stored then), a flag or none (1 stored then) and an emitter count about the 100-0x7fff clamp (signed); the emitters
# (0x34 bytes each) and the 0xaf0-byte block from the global heap (heap(), its blocks compared; a count past its room fails the
# allocation: Emitters_Reset then writes through none, a fault in both), Emitters_Reset's records set.
# usage: particleinit.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Init)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, global_heap, heap, put  # noqa: E402

CALLS = {'Init': 0x51faa0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench([(0x51faa0, 0x51fb68), (0x51f320, 0x51f36a), (0x51f630, 0x51f654), (0x51fb70, 0x51fd16), (0x5f5660, 0x5f5690),
           (0x5da3c0, 0x5da3c1)])
STATE = 0x803b58   # (8 guard bytes before the sine table)


def case(r):
    r.deal(OPS)
    state = bytearray(r.bytes(0x816d48 - STATE))
    put(state, 0x816d40 - STATE, 'I', r.choice([0] * 8 + [1, 0x100]))
    regions = {STATE: state, HEAP: heap(0x20000, fail=r.choice([0] * 12 + [1])), **global_heap()}
    count = r.choice([0, 1, 99, 100, 101, 150, 300, 1000, -5, -0x80000000, 0x7fff if r.random() < 0.2 else 200, r.randrange(2000)])
    callback = r.choice([0, 0, 0x401000, r.getrandbits(32)])
    flag = r.choice([0, 0, 1, 2, r.getrandbits(32)])
    return Call(CALLS['Init'], [callback, flag, count & 0xffffffff], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
