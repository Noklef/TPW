# An object's ride claimed or made (src/game/world/objectride: Object_ClaimRide, 0x4dd500) against the original. Recorders (their own benches have them; driver.cpp's kObjectRideEnv): ObjectDefs_Find 10, Ride_Claim 11, 0x463060 12, Ride_SetRecordId 13, Coaster_RequeueWaitingPeeps 14. The description hashed.
# usage: objectride.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, blob, log_region, put, set_return  # noqa: E402

B = Bench([(0x4dd500, 0x4dd667)], hooks={0x412e90: _recorder(10, 1), 0x463ab0: _recorder(11, 9, cdecl=True), 0x463060: _recorder(12, 10, cdecl=True), 0x4685c0: _recorder(13, 2, cdecl=True), 0x4379a0: _recorder(14, 1, cdecl=True)})
SELF, DEF = RAM, RAM + 0x100


def case(r):
    log = log_region()
    struct.pack_into('<I', log, LOG_BYTES + 4 * 10, 1 << 16)   # (the id a word: the upper half the register's)
    d = bytearray(r.bytes(0x4a0))
    put(d, 0x38, 'I', r.choice([0, r.getrandbits(32)]))
    put(d, 0x20, 'I', r.choice([0, r.getrandbits(32)]))
    put(d, 0x24, 'I', r.choice([0, r.getrandbits(32)]))
    self = bytearray(r.bytes(0x40))
    put(self, 0x34, 'H', r.choice([0, 1, 2, 0x80, 0x81, r.getrandbits(16)]))
    set_return(log, 10, 0 if r.random() < 0.05 else DEF)
    set_return(log, 12, r.getrandbits(32))
    blob(log, 11, 3, 0xf)
    blob(log, 12, 2, 0xf)
    regions = {LOG: log, SELF: self, DEF: d, 0x785594: r.bytes(8)}
    return Call(0x4dd500, [SELF, r.choice([0, 1, r.getrandbits(32)])], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
