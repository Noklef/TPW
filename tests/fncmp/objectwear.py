# A ride wearing down (src/game/world/objectwear: Object_WearTick, 0x4df670) against the original. Recorders (their own benches have them; driver.cpp's kObjectWearEnv): ObjectDefs_Find 10, Setting_ReadMode 11, atexit 12, Rides_FindById 13, Rides_GetElement 14 (answers in turn), Rides_GetWordC0 15,
# Object_DefRowScore 16 (st0), Rides_SetElement 17, Thing_GetHandle 18 (its out the stack's), GameTable7cfb90_Get1da71c_5194d0 19, Advisor_Post 20 (the message's written bytes hashed). AdvisorMsg_Init runs, on the id's record with a recorder 21 for its method.
# usage: objectwear.py [CASES [SEED]] [--wild|--both] [--show N]
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, answers, blob_bytes, log_region, put, recorder, set_float_return, set_return, strings  # noqa: E402

B = Bench([(0x4df670, 0x4df8e4), (0x59b590, 0x59b611), (0x59bf20, 0x59c093), (0x59bea0, 0x59bf0f), (0x5da3c0, 0x5da3c1), (0x481af0, 0x481b45), (0x5d1750, 0x5d1751)],
          hooks={0x412e90: _recorder(10, 1), 0x550ca0: _recorder(11, 0), 0x67a7d0: _recorder(12, 1, cdecl=True), 0x55a070: _recorder(13, 1, cdecl=True), 0x55a390: _recorder(14, 2, cdecl=True), 0x55a330: _recorder(15, 1, cdecl=True), 0x4df450: _recorder(16, 4),
                 0x55a0b0: _recorder(17, 3, cdecl=True), 0x50b350: _recorder(18, 1), 0x5194d0: _recorder(19, 0), 0x59a940: _recorder(20, 1)})
SELF, DEF, RIDE, HANDLE, ANSWERS, WORLD = RAM, RAM + 0x100, RAM + 0x400, RAM + 0x500, RAM + 0x600, RAM + 0x80000
TABLE = 0x76e300


def number(r):
    return r.choice([r.uniform(0, 100), r.uniform(0, 100), r.randrange(0, 101) * 1.0, r.uniform(-20, 140), 9.5, 10.0, 19.5, 20.0, 0.0, 100.0, 1e30, -1e30, math.nan])


def case(r):
    log = log_region()
    struct.pack_into('<I', log, LOG_BYTES + 4 * 10, 1 << 16)   # (the id a word: the upper half the stack's)
    self = bytearray(r.bytes(0x60))
    level = r.choice([0, 1, 2, 3])
    self[0x50] = level
    self[0x33] = r.choice([0, 1, 2, 3, r.getrandbits(8)])
    put(self, 0x44, 'f', number(r))
    put(self, 0x48, 'f', number(r))
    d = bytearray(r.bytes(0x280))
    put(d, 0x1b0 + (level << 6), 'I', r.choice([0, 1, 1, 1, 1, r.getrandbits(32)]))
    set_return(log, 10, DEF)
    set_return(log, 13, RIDE)
    set_return(log, 15, r.choice([0, 1, 7, r.getrandbits(32)]))
    wear = r.choice([0.0, 0.5, 1.0, 5.0, r.uniform(0, 30), r.uniform(-5, 5), 10.0, math.nan])
    set_float_return(log, 16, wear)
    nine = r.choice([0, 0, 1, 1, 1, 0xffffffff, r.getrandbits(32)])
    five = r.choice([0, 1, 1, 1, r.getrandbits(32)])
    table = struct.pack('<40I', *([nine, five, r.choice([0, 1, 100, r.getrandbits(32)]), r.choice([0, 1, 100, r.getrandbits(32)])] + [0] * 36))
    answers(log, 14, ANSWERS)
    set_return(log, 18, HANDLE)
    strings(log, 18, 0, out=1)
    set_return(log, 19, RAM + 0x800)
    blob_bytes(log, 20, 0, 0x7ffff3)
    set_return(log, 20, r.getrandbits(32))
    records = bytearray(r.bytes(0x38 * 0x15f))
    put(records, 0x38 * 0x42 + 4, 'I', 0x42)
    put(records, 0x38 * 0x42 + 0xc, 'IIi', r.choice([0, 1, 2]), recorder(21, 0), r.choice([0, 4, 0x10]))
    set_return(log, 21, r.choice([0, 1, 5, 100]))
    world = bytearray(r.bytes(0x10))
    put(world, 0xc, 'I', r.choice([0, 0x40, 0x80, 1, 0x3f, 0x20, 0x20, r.getrandbits(32)]) & r.choice([0xffffffc0, 0xffffffc0, 0xffffffff]))
    regions = {LOG: log, SELF: self, DEF: d, RIDE: r.bytes(0x20), HANDLE: r.bytes(4), ANSWERS: table, WORLD + 0x1da700: world, 0x80239c: struct.pack('<I', WORLD), 0xfb34b8: bytes([r.choice([0, 1, 1, 1, r.getrandbits(8)])]),
               0xfb3b7c: struct.pack('<I', r.choice([0, 0, 2, 1, r.getrandbits(32)])), TABLE: records, 0xfb34bc: bytes([1]), 0xfb2766: bytes([1]), 0xfb3540: r.bytes(0x63c), 0x7a1a8c: struct.pack('<I', r.getrandbits(32) & ~(7 << 23))}
    return Call(0x4df670, [SELF], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
