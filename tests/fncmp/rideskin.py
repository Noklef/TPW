# A ride's skins put back (src/game/world/rideskin: 0x4690a0) against the original: Ride_Destroy (k 1) a recorder; a record or none, a skin kind or none, its table entry's +0xd0 set or not, a model of 0 to 3 parts of 0 to 4 entries, their first
# dwords now and then the skins named.
# usage: rideskin.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

B = Bench([(0x4690a0, 0x469177)], hooks={0x463c80: _recorder(1, 1, cdecl=True)})
RECORD, ENTRY, INSTANCE, MEMBER, MODEL, PARTS, LISTS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x220, RAM + 0x240, RAM + 0x300, RAM + 0x800


def case(r):
    log = log_region()
    if r.random() < 0.05:
        return Call(0x4690a0, [0], {LOG: log}, ret='v')
    kind, ride = r.choice([0, 1, 2, 3]), r.randrange(1, 4)
    skins = [r.getrandbits(32) for _ in range(4)]
    record = bytearray(r.bytes(0x90))
    put(record, 4, 'I', ride)
    put(record, 0x74, '4I', *skins)
    put(record, 0x84, 'I', kind)
    n = r.randrange(0, 4)
    parts = bytearray(r.bytes(0xa0 * 3 + 0x60))
    regions = {LOG: log, RECORD: record, ENTRY: put(bytearray(r.bytes(0xd4)), 0xd0, 'I', r.choice([0, 1, 1, r.getrandbits(32)])), 0x7ae6a8: struct.pack('<4I', 0, ENTRY, ENTRY, ENTRY),
               0x7a4610: struct.pack('<4I', 0, INSTANCE, INSTANCE, INSTANCE), INSTANCE: put(bytearray(r.bytes(0x10)), 8, 'I', MEMBER), MEMBER: put(bytearray(r.bytes(8)), 4, 'I', MODEL),
               MODEL: put(put(bytearray(r.bytes(0x80)), 0x44, 'H', n), 0x70, 'I', PARTS), PARTS: parts}
    for i in range(n):
        m = r.randrange(0, 5)
        put(parts, 0xa0 * i + 0x5a, 'H', m)
        put(parts, 0xa0 * i + 0x5a + 0x12, 'I', LISTS + 0x80 * i)
        regions[LISTS + 0x80 * i] = b''.join(struct.pack('<I', r.choice([skins[2], skins[3], skins[0], r.getrandbits(32)])) + r.bytes(12) for _ in range(5))
    return Call(0x4690a0, [RECORD], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
