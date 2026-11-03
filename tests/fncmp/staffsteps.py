# The handyman's and mechanic's update and work states (src/game/world/staffsteps: 0x4d7060, 0x4d7580, 0x4d76a0, 0x4da740, 0x4da830) against the originals. The state and handle accessors and the game table's lookup real, all other calls recorders numbered as StaffStepsEnv from 10 (their own benches have them).
# usage: staffsteps.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Update,WalkToLitter,Sweep,WalkToObject,Repair)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, log_region, put, set_return, strings  # noqa: E402

CALLS = {'Update': 0x4d7060, 'WalkToLitter': 0x4d7580, 'Sweep': 0x4d76a0, 'WalkToObject': 0x4da740, 'Repair': 0x4da830}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench([(0x4d7060, 0x4d7100), (0x4d7580, 0x4d7790), (0x4da740, 0x4da8e0), (0x50b350, 0x50b35d), (0x519510, 0x51954d), (0x4e0370, 0x4e037f), (0x4e03d0, 0x4e03d4), (0x4e03e0, 0x4e03ef), (0x5da3c0, 0x5da3c1)],
          hooks={0x506760: _recorder(10, 0), 0x4fa2a0: _recorder(11, 0), 0x4fa930: _recorder(12, 2), 0x4d7330: _recorder(13, 1), 0x4d9620: _recorder(14, 0), 0x4d7100: _recorder(15, 0), 0x4da5b0: _recorder(16, 0), 0x4da370: _recorder(17, 1),
                 0x4e03f0: _recorder(18, 0), 0x4e0410: _recorder(19, 0), 0x4df8f0: _recorder(20, 1), 0x505490: _recorder(21, 0), 0x50be40: _recorder(22, 0), 0x4c8ed0: _recorder(23, 4), 0x4e0220: _recorder(24, 1)})
STAFF, ANSWER, GAME, LITTER, OBJECT, RECORD = RAM + 0x20000, RAM + 0x20400, RAM + 0x21000, RAM + 0x22000, RAM + 0x23000, RAM + 0x24000
MAP = LITTER + 0x80


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log}
    # (the litter records' words, Thing_PlaceAt's upper halves are the registers' in the original, the nearest cell's the stack's)
    struct.pack_into('<I', log, LOG_BYTES + 4 * 12, 0b11 << 16)
    struct.pack_into('<I', log, LOG_BYTES + 4 * 23, 0b0110 << 16)
    struct.pack_into('<I', log, LOG_BYTES + 4 * 20, 0b1 << 16)
    me = bytearray(r.bytes(0x240))
    handle = r.choice([0, 1, r.getrandbits(16)])
    put(me, 0, 'H', handle)
    kind = r.randrange(0, 8)
    put(me, 0x1e4, 'I', kind)
    put(me, 0x19c, 'I', r.choice([0, 0, 0, 1, r.getrandbits(32)]))
    put(me, 5, 'B', r.randrange(0, 3))
    put(me, 7, 'B', 0)
    number = r.randrange(0, 4)
    put(me, 0x218, 'H', number)
    put(me, 0x214, 'I', r.choice([0, 0, 0, 1, 2, 0x100, r.getrandbits(32)]))
    regions[STAFF] = me
    # the litter: records 0 to 4
    litter = bytearray(r.bytes(0x400))
    for k in range(5):
        base = 0x80 + k * 0x44 - 0x44
        put(litter, base + 0x30, 'H', r.choice([handle, handle, handle, handle, r.getrandbits(16)]))
        put(litter, base + 0x2c, 'I', r.choice([0, 1, r.getrandbits(32)]))
    regions[LITTER] = litter
    regions[0x8023a0] = struct.pack('<I', MAP)
    # the world: its ticks, the record's slot
    index = r.randrange(5, 0x40)
    regions[0x80239c] = struct.pack('<I', GAME)
    ticks = r.choice([0, 1, 12, 60, 120, r.getrandbits(16), r.getrandbits(32)])
    regions[GAME + 0x1da700] = bytes(0xc) + struct.pack('<I', ticks) + bytes(0x10) + struct.pack('<H', index) + bytes(2)
    regions[0x7cfb90 + 20 * index] = struct.pack('<I', RECORD)
    regions[RECORD] = r.bytes(8)
    # the tables by +0x1e4
    regions[0x785330] = struct.pack('<%dI' % (8 * 4), *[r.choice([1, 2, 3, 4, 12, 60, ticks | 1, ticks or 1]) for _ in range(8 * 4)])
    regions[0x7853ec] = struct.pack('<%dI' % (8 * 3), *[r.choice([0, 1, ticks, (ticks - 1) & 0xffffffff, r.getrandbits(32)]) for _ in range(8 * 3)])
    # the object: the table 0x7cfb90
    obj = bytearray(r.bytes(0x200))
    put(obj, 0x19c, 'I', r.choice([0, 1, 1, 2, 3, r.getrandbits(32)]))
    put(obj, 0x64, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    regions[OBJECT] = obj
    regions[0x7cfb90 + 20 * number] = struct.pack('<I', OBJECT)
    # the answers
    regions[ANSWER] = struct.pack('<HH', r.choice([0, 0, 1, r.getrandbits(16)]), r.choice([handle, handle, handle, r.getrandbits(16)]))
    set_return(log, 23, ANSWER)
    strings(log, 23, 0, 1 << 0)
    set_return(log, 24, ANSWER + 2)
    strings(log, 24, 0, 1 << 0)
    set_return(log, 11, r.choice([0, 0, 0, 2, 2, 1, r.getrandbits(32)]))
    set_return(log, 19, r.choice([0, 0, 1, r.getrandbits(32)]))
    return Call(CALLS[op], [STAFF], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
