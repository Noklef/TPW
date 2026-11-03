# The guard's escort (src/game/world/guardescort: 0x4d6260, 0x4d6790, 0x4d6a50) against the originals. GameTable7cfb90_Get1da720_519510 and the +0xc's setter real, the other calls recorders numbered as kGuardEscortEnv from 10: Sprite_SetDestinationCell's and Visitor_Arrive's cell logged as a word (the slot's upper half is the stack's). The guard over a visitor of the table, on its cell or not; the walk's answers 0, 2, others; the destination's answers in turn from a table.
# usage: guardescort.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Send,Chase,Leave)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, answers, log_region, put, set_return, strings  # noqa: E402

CALLS = {'Send': 0x4d6260, 'Chase': 0x4d6790, 'Leave': 0x4d6a50}
B = Bench([(0x4d6260, 0x4d6326), (0x4d6790, 0x4d6a4a), (0x4d6a50, 0x4d6b20), (0x519510, 0x519550), (0x4217f0, 0x4217fa), (0x50b350, 0x50b35d), (0x5da3c0, 0x5da3c1)],
          hooks={0x506680: _recorder(10, 0), 0x506760: _recorder(11, 0), 0x4fa2a0: _recorder(12, 0), 0x4fa530: _recorder(13, 1), 0x4fec10: _recorder(14, 0), 0x4c74f0: _recorder(15, 1), 0x4faa00: _recorder(16, 2), 0x4feb10: _recorder(17, 0), 0x4d8610: _recorder(18, 2), 0x4d8690: _recorder(19, 2), 0x4d65d0: _recorder(20, 1), 0x4feb50: _recorder(21, 1)})
GUARD, VISITOR, RECORD, GAME, MAP, ANSWERS, CELLS = RAM, RAM + 0x400, RAM + 0x800, RAM + 0x1000, RAM + 0x2000, RAM + 0x2100, RAM + 0x2200


def case(r):
    op = r.deal(list(CALLS))
    log = log_region()
    regions = {LOG: log}
    guard = bytearray(r.bytes(0x220))
    visitor = bytearray(r.bytes(0x20))
    if r.random() < 0.5:   # (on the visitor's cell)
        visitor[5], visitor[7] = guard[5], guard[7]
    elif r.random() < 0.3:   # (a cell next to it)
        visitor[5], visitor[7] = guard[5] ^ 1, guard[7]
    handle = r.randrange(1, 0x400)
    put(guard, 0x212, 'H', r.choice([handle] * 9 + [0]))
    put(guard, 0x214, 'I', r.choice([1, 2, 5, 5, 5, 0x10, 0xffffffff, 0]))
    regions[GUARD], regions[VISITOR] = guard, visitor
    index = r.randrange(0x400, 0x500)
    regions[0x7cfb90 + 20 * handle] = struct.pack('<I', VISITOR)
    regions[0x7cfb90 + 20 * index] = struct.pack('<I', RECORD)
    regions[RECORD] = r.bytes(0x10)
    regions[GAME + 0x1da700] = bytes(0xc) + r.bytes(4) + bytes(0x10) + struct.pack('<H', index) + bytes(2)
    regions[0x80239c] = struct.pack('<I', GAME)
    regions[0x8023a0] = struct.pack('<I', MAP)
    regions[MAP] = r.bytes(0x10)
    regions[0x75c7f0] = r.bytes(2)
    regions[0x803a30] = r.bytes(4)
    regions[CELLS] = r.bytes(4)
    regions[ANSWERS] = struct.pack('<40I', *[r.choice([0, 0, 1, r.getrandbits(32)]) for _ in range(40)])
    answers(log, 13, ANSWERS)
    set_return(log, 10, r.choice([0, 0, 0, 1]))
    set_return(log, 12, r.choice([0, 0, 0, 2, 2, 1, 3, r.getrandbits(32)]))
    set_return(log, 14, r.choice([0, 1, 1, 1]))
    set_return(log, 18, CELLS)
    set_return(log, 19, CELLS)
    for k in (15, 16, 20):
        set_return(log, k, r.getrandbits(32))
    for k in (18, 19):
        strings(log, k, 0, 1 << 0)
    for k in (13, 21):
        struct.pack_into('<I', log, LOG_BYTES + 4 * k, 1 << 16)
    args = [GUARD] + ([handle | r.getrandbits(16) << 16] if op == 'Send' else [])
    return Call(CALLS[op], args, regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
