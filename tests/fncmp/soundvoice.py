# A sound mixer's voice (src/game/core/soundvoice) against the originals: a voice at VOICE (+0x4c a parameter block or null, the heap
# failing now and then; +0x24 a sound or null), the sound system [0xfb1f24] a recorder vtable whose slot 4 gets the stack functor, logged
# by its vtable and sound and the record's value bytes (+0x28; the record's other set fields fall outside the recorder's 15-byte window).
# usage: soundvoice.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, blob_bytes, heap, log_region, set_return, vtable  # noqa: E402

# name: (address, convention, returns)
CALLS = {'ensure': (0x6b7150, 'v'), 'pending': (0x6bc900, 'a')}
B = Bench([(0x6b7150, 0x6b7190), (0x6bc900, 0x6bca10), (0x6c5130, 0x6c5150)])
VOICE, OUT, SYS, SVT = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300
SYSTEM = 0xfb1f24
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def case(r):
    name = r.deal(OPS)
    address, ret = CALLS[name]
    log = log_region()
    blob_bytes(log, 4, 1, 0xff, inner=(8, 0x7000, 28))   # the functor: its vtable, the sound; the record's +0x28-+0x2a
    set_return(log, 4, r.getrandbits(32))
    voice = bytearray(r.bytes(0x50))
    struct.pack_into('<I', voice, 0x4c, r.choice([0, 0, 0, r.getrandbits(32) | 1]))
    struct.pack_into('<I', voice, 0x24, r.choice([0, r.getrandbits(32) | 1, r.getrandbits(16) + 1 << 16]))   # (a sound whose low word is 0 too)
    regions = {VOICE: voice, OUT: r.bytes(4), SYSTEM: struct.pack('<I', SYS), SYS: struct.pack('<I', SVT) + r.bytes(0xc), SVT: vtable({4: 2}),
               LOG: log}
    if name == 'ensure':
        regions[HEAP] = heap(0x40, fail=r.choice([0, 0, 0, 1]))
        return Call(address, [VOICE], regions, conv='t', ret=ret)
    return Call(address, [VOICE, OUT, r.choice([0, 1, 0xffff, 0x10000, r.getrandbits(32)])], regions, conv='t', ret=ret)


if __name__ == '__main__':
    B.main(case)
