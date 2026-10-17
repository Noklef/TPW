# Two of the engine's objects created at start-up against the originals: the block at 0x879d54 (src/game/core/globals: 0x56b6c0) and the procedural
# terrain texture object at 0x879d60 (src/game/terrain/procedural: 0x56ddf0), each with no object yet mostly (else one), on fncmp's heap whose
# allocation fails now and then (the error then pushed onto the engine's list at 0x879ad0, with a count of 0 to 0x40 and the engine debug flag at
# 0x8bd508 half the time: the log 0x564650 is fncmp's one-argument cdecl logger, its text hashed, as in engineerrors.py).
# usage: enginecreate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, heap, log_region, strings  # noqa: E402

B = Bench([(0x56b6c0, 0x56b755), (0x56ddf0, 0x56de86), (0x56af20, 0x56af9c), (0x56a770, 0x56ae60), (0x67a400, 0x690000)],
          hooks={0x564650: _cdecl_log})
OPS = {'Block': (0x56b6c0, 0x879d54, 0x2d8), 'Texture': (0x56ddf0, 0x879d60, 0x12974)}


def case(r):
    address, slot, size = OPS[r.deal(list(OPS))]
    log = log_region()
    strings(log, 0x3f, 1)
    errors = bytearray(r.bytes(0x204))
    struct.pack_into('<i', errors, 0x200, r.choice([r.randrange(0x41), 0, 0x3f, 0x40]))
    flags = r.getrandbits(32) & ~0x1000000 | (0x1000000 if r.random() < 0.5 else 0)
    regions = {slot: struct.pack('<I', RAM if r.random() < 0.2 else 0), RAM: r.bytes(0x10), HEAP: heap(size + 0x40, 1 if r.random() < 0.3 else 0),
               0x879ad0: errors, 0x8bd508: struct.pack('<I', flags), LOG: log}
    return Call(address, [], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
