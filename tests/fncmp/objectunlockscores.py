# The four score checks (src/game/world/objectunlock: ObjectUnlock_CheckScores, 0x4d4e50) against the original (objectunlock.py benches the module's others). The store's getter thiscall recorder 19;
# Things_TallestCoaster, Things_BestRatedDef9c1 and Rides_BestPiecesOfKind stdcall recorders 20-22 (the out word the stack's, logged as 0; the id's upper half dirty in the original: its word logged),
# Map_CountSpecialCells cdecl recorder 23, GlobalSave_MarkScore and ObjectUnlock_AdviseScore thiscall recorders 24 and 25 (their own benches have them; driver.cpp's kObjectUnlockScoresEnv). Each value over,
# at or under its limit [0x7857ac], [0x7857b0], [0x7857b4], [0x7857bc]; the mark's answer 0 now and then. The original's debug print (0x5af4a0 of the instance, the empty 0x5da3c0) is left out by the port.
# usage: objectunlockscores.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, log_region, set_return, strings  # noqa: E402

B = Bench([(0x4d4e50, 0x4d4f9f), (0x409410, 0x409490), (0x5da3c0, 0x5da3c1)],
          hooks={0x519510: _recorder(19, 0), 0x4cc920: _recorder(20, 2, com=True), 0x4cc3b0: _recorder(21, 1, com=True), 0x4cc6f0: _recorder(22, 2, com=True),
                 0x4cd7f0: _recorder(23, 0, cdecl=True), 0x5af940: _recorder(24, 2), 0x4d5270: _recorder(25, 2), 0x5af4a0: lambda e: None})
SELF, GAME, INFO = RAM + 0x100, RAM + 0x1000, RAM + 0x2000


def case(r):
    log = log_region()
    struct.pack_into('<I', log, LOG_BYTES + 4 * 20, 1 << 17)
    for k in (20, 21, 22):
        strings(log, k, 0, 0b1)
    limits = {}
    for k, at in ((20, 0x7857ac), (21, 0x7857b0), (22, 0x7857b4), (23, 0x7857bc)):
        limit = r.choice([0, 1, 5, 100, r.randrange(-100, 100), r.getrandbits(31), -r.getrandbits(31)])
        limits[at] = struct.pack('<i', limit)
        set_return(log, k, limit + r.choice([1, 1, 2, r.randrange(1, 100), 0, -1, r.randrange(-100, 0)]))
    set_return(log, 19, r.getrandbits(32))
    if r.random() < 0.25:
        set_return(log, 24, 0)
    regions = {LOG: log, 0x80239c: struct.pack('<I', GAME), 0xf7c58c: struct.pack('<I', INFO), **limits}
    return Call(0x4d4e50, [SELF], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
