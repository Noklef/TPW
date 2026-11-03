# A ride slot's debug dump (src/game/world/rides: RideSystem_DumpSlot, 0x464580) against the original: its lines are formatted (sprintf, hooked here to do nothing: the real one reaches the CRT's imports) and handed to the empty Debug_Printf, so nothing changes; a slot with +4 = 0xc formats none.
# usage: ridedumpslot.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb, put  # noqa: E402

B = Bench([(0x464580, 0x464794), (0x5da3c0, 0x5da3c1)], hooks={0x67a650: lambda e: None})
SLOT, SYSTEM, TYPE, NAME = RAM, RAM + 0x40, RAM + 0x80, RAM + 0xa0


def case(r):
    slot = bytearray(r.bytes(0x38))
    put(slot, 4, 'I', r.choice([0xc, 1, 2, 3, r.randrange(0, 0x100)]))
    put(slot, 0x1c, 'I', fb(r.uniform(-100, 100)))
    put(slot, 0x20, 'I', fb(r.uniform(-100, 100)))
    system = bytearray(r.bytes(0x20))
    put(system, 8, 'I', TYPE)
    put(system, 0x10, 'I', SLOT - 0x38 * r.randrange(0, 8))
    regions = {SLOT: slot, SYSTEM: system, TYPE: struct.pack('<I', 0) + struct.pack('<I', NAME - 0x18) + r.bytes(8), NAME: b'ride name\0'}
    return Call(0x464580, [SLOT, SYSTEM], regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)
