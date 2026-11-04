# The draw buffer's batches sorted and drawn (src/game/render/drawbuffer: 0x582ad0 DrawBuffer_DrawAll) against the original: recorders for TrackedGlobal_Alloc (cdecl, k 1; its answer the order's region, or 0), TrackedGlobal_Free (cdecl, k 2), EngineErrors_Push (k 3), Obj_ApplyIfShared (k 4; it can set a record's flag 0x10),
# the three sorts (k 5, 6, 7) and Obj_CallSlot10IfSized (k 8; failing now and then). The order array shows in its region; the mode, the render flags and the device flags vary.
# usage: drawbufferdraw.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, fill, log_region, put, set_return  # noqa: E402

B = Bench([(0x582ad0, 0x582d7a)], hooks={0x579b40: _recorder(1, 1, cdecl=True), 0x579b80: _recorder(2, 1, cdecl=True), 0x56af20: _recorder(3, 2), 0x565930: _recorder(4, 1), 0x582810: _recorder(5, 3), 0x5828f0: _recorder(6, 3), 0x5829e0: _recorder(7, 3), 0x565560: _recorder(8, 1)})
BUFFER, RECORDS, ORDER, TABLE, DEVICE, VERTS = RAM, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x3100, RAM + 0x4000
RECORD = 0x38
MAX = 14


def case(r):
    log = log_region()
    set_return(log, 1, 0 if r.random() < 0.05 else ORDER)
    fails = r.choice([0, 0, 0, 0.1])
    table = struct.pack('<40I', *[0 if r.random() < fails else r.choice([1, 1, 2, 0x80000000]) for _ in range(40)])
    answers(log, 8, TABLE)
    fill(log, 4, 0, b''.join(struct.pack('<I', r.getrandbits(32) & ~0x10 | (0x10 if r.random() < 0.3 else 0)) for _ in range(10)), records=10)

    first = r.choice([0, 1, 2, 4, 6])
    low = r.choice([-1, 0, 1, first, first + 1])
    total = r.randrange(0, MAX)
    records = bytearray(r.bytes(RECORD * MAX))
    verts = bytearray(r.bytes(0x18 * 4 * MAX))
    for i in range(MAX):
        at = i * RECORD
        put(records, at, 'I', r.getrandbits(32) & ~0x10 | (0x10 if r.random() < 0.25 else 0))
        put(records, at + 4, 'I', r.getrandbits(32) & ~0x1000 | (0x1000 if r.random() < 0.5 else 0))
        put(records, at + 0x20, 'I', r.randrange(0, 5))
        put(records, at + 0x28, 'I', VERTS + i * 0x18 * 4)
    buffer = bytearray(r.bytes(0x60))
    put(buffer, 0x44, 'I', RECORDS)
    put(buffer, 0x48, 'I', total)
    put(buffer, 0x50, 'I', first)
    put(buffer, 0x54, 'i', low)

    render = r.getrandbits(32) & ~0x2000000c | r.choice([0, 4, 8, 0xc, 0xc]) | r.choice([0, 0x20000000])
    device = r.getrandbits(32) & ~0x80000020 | r.choice([0, 0x20, 0x80000000, 0x80000020, 0x80000020])
    regions = {LOG: log, BUFFER: buffer, RECORDS: records, ORDER: r.bytes(0x100), TABLE: table, VERTS: verts, DEVICE: r.bytes(0x10),
               0x879ab0: struct.pack('<I', DEVICE), 0x8bd524: struct.pack('<I', r.choice([4, 4, 4, 1, r.getrandbits(32)])), 0x8bcbc8: struct.pack('<I', render), 0x8bd508: struct.pack('<I', device)}
    return Call(0x582ad0, [BUFFER], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
