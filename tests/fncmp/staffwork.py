# An idle handyman's, mechanic's and entertainer's next work (src/game/world/staffwork: 0x4d7100, 0x4da5b0, 0x4d46d0) against the originals. The generator, the game table's lookups and the advisor message real (advisor.py's PostId set-up, as headquarters.py), the other calls recorders numbered as kStaffWorkEnv from 10. Words passed in stack slots logged as words (their upper halves are the stack's), the out pointers as 0.
# usage: staffwork.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Handyman,Mechanic,Entertainer)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, Mask, _cdecl_log, _recorder, log_region, put, set_return, strings  # noqa: E402
from advisor import ADVISOR, NOW, RANGES, queue  # noqa: E402

CALLS = {'Handyman': 0x4d7100, 'Mechanic': 0x4da5b0, 'Entertainer': 0x4d46d0}
B = Bench(RANGES + [(0x4d7100, 0x4d7330), (0x4da5b0, 0x4da732), (0x4d46d0, 0x4d480d), (0x4217f0, 0x4217fa), (0x50b350, 0x50b35d), (0x516330, 0x516370), (0x516370, 0x516380), (0x519490, 0x5194d0), (0x519510, 0x519550), (0x4e03d0, 0x4e03f0)],
          hooks={0x67a7d0: _cdecl_log, 0x506a40: _recorder(10, 0), 0x506680: _recorder(11, 0), 0x4f9490: _recorder(12, 0), 0x4c8ed0: _recorder(13, 4), 0x4d7880: _recorder(14, 1), 0x4fa5f0: _recorder(15, 2), 0x4d9600: _recorder(16, 1), 0x4d7330: _recorder(17, 1), 0x4daa90: _recorder(18, 2), 0x4fa530: _recorder(19, 1), 0x4e01f0: _recorder(20, 1), 0x4da370: _recorder(21, 1), 0x5054d0: _recorder(22, 1), 0x4c8eb0: _recorder(23, 3, com=True), 0x541fa0: _recorder(24, 2, cdecl=True)})
STAFF, ANSWER, GAME, MAP, OBJECTS = RAM + 0x20000, RAM + 0x20400, RAM + 0x21000, RAM + 0x22000, RAM + 0x23000


def words(log, k, mask):
    struct.pack_into('<I', log, LOG_BYTES + 4 * k, mask << 16)


def common(r, log):
    staff = bytearray(r.bytes(0x220))
    put(staff, 0, 'H', r.getrandbits(16))
    set_return(log, 10, r.choice([0, 0, 0, 0, 0, 0, 1, r.getrandbits(32)]))
    set_return(log, 11, r.choice([0, 0, 0, 0, 1]))
    set_return(log, 12, r.choice([0, 0, 1, r.getrandbits(32)]))
    return staff


def handyman(r):
    log = log_region()
    regions = {LOG: log}
    me = common(r, log)
    kind = r.randrange(0, 8)
    put(me, 0x1e4, 'I', kind)
    regions[STAFF] = me
    regions[0x7853f0 + 12 * kind] = r.bytes(4)
    index = r.randrange(1, 0x400)
    regions[0x7cfb90 + 20 * index] = struct.pack('<I', RAM + 0x24000 + index)
    regions[0x80239c] = struct.pack('<I', GAME)
    regions[0x8023a0] = struct.pack('<I', MAP)
    regions[GAME + 0x1da700] = bytes(8) + r.bytes(4) + bytes(0x14) + struct.pack('<H', index) + bytes(2)
    target = r.choice([0, 1, 0x80, 0x81, r.randrange(1, 0x3fff), r.randrange(1, 0x3fff), r.randrange(1, 0x10000)])
    regions[ANSWER] = struct.pack('<HH', target, r.choice([0, r.randrange(1, 0x10000)]))
    set_return(log, 13, ANSWER)
    set_return(log, 14, ANSWER + 2)
    set_return(log, 15, r.choice([1, 1, 1, 0, 2, r.getrandbits(32)]))
    strings(log, 13, 0, 1 << 0)
    strings(log, 14, 0, 1 << 0)
    words(log, 13, 0b0110)
    words(log, 15, 0b11)
    words(log, 16, 0b1)
    return Call(CALLS['Handyman'], [STAFF], regions, conv='t', ret='v')


def mechanic(r):
    c = queue(r, 'PostId')
    regions = c.regions
    log = regions[LOG]
    me = common(r, log)
    regions[STAFF] = me
    if r.random() < 0.6:
        put(regions[0x76e300], 0x38 * 0x46 + 4, 'I', 0x46)
    if r.random() < 0.6:
        put(regions[ADVISOR], 0xe0 + 0x10 * 0x46, 'I', 0)
    target = r.randrange(1, 6)
    advisor, record = 6, 7
    table = bytearray(r.bytes(20 * (record + 1)))
    put(table, 20 * advisor, 'I', ADVISOR)
    put(table, 20 * record, 'I', r.getrandbits(32))
    thing = bytearray(r.bytes(0x220))
    put(thing, 0x64, 'I', r.choice([0, 0, 0, 1, r.getrandbits(32)]))
    put(thing, 0x19c, 'I', r.choice([0, 1, 2, 3, r.getrandbits(32)]))
    regions[OBJECTS] = thing
    put(table, 20 * target, 'I', OBJECTS)
    for k in [k for k in regions if 0x7cfb90 <= k < 0x7cfb90 + 0x1000]:   # (PostId's own entry for the advisor)
        del regions[k]
    regions[0x7cfb90] = table
    world = bytearray(r.bytes(0x40))
    world[0:4] = regions[NOW][0:4]
    put(world, 0xe, 'H', record)
    put(world, 0x10, 'H', advisor)
    regions[NOW] = world
    regions[ANSWER] = struct.pack('<H', r.choice([0, target, target, target, target]))
    set_return(log, 18, ANSWER)
    set_return(log, 19, r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    strings(log, 18, 0, 1 << 0)
    words(log, 18, 0b10)
    words(log, 19, 0b1)
    words(log, 20, 0b1)
    offsets = sorted([0x16d0 + k for k in (2, 3, 0x17)] + [0x14 + 0x18 * s + k for s in range(8) for k in (2, 3, 0x17)])
    mask = Mask(ADVISOR, offsets, "the stack message's bytes +2, +3, +0x17, never written (0x4da5b0), copied into a slot or +0x16d0")
    return Call(CALLS['Mechanic'], [STAFF], regions, conv='t', ret='v', masks=[mask])


def entertainer(r):
    log = log_region()
    regions = {LOG: log}
    me = common(r, log)
    kind = r.randrange(0, 8)
    put(me, 0x1e4, 'I', kind)
    regions[STAFF] = me
    regions[0x7853a0 + 16 * kind] = r.bytes(4)
    index = r.randrange(1, 0x400)
    regions[0x7cfb90 + 20 * index] = struct.pack('<I', RAM + 0x24000 + index)
    regions[0x80239c] = struct.pack('<I', GAME)
    regions[GAME + 0x1da700] = bytes(8) + r.bytes(8) + bytes(0x10) + struct.pack('<H', index) + bytes(2)
    set_return(log, 23, r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    set_return(log, 24, r.choice([0, 1, 1, 2, 3, 5, 9, r.getrandbits(32)]))
    words(log, 23, 0b10)
    return Call(CALLS['Entertainer'], [STAFF], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def case(r):
    return {'Handyman': handyman, 'Mechanic': mechanic, 'Entertainer': entertainer}[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)
