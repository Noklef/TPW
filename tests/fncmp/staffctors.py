# The handyman's, mechanic's, entertainer's and guard's constructors (src/game/world/staffctors: 0x4d6b60, 0x4d9eb0, 0x4d9db0, 0x4d4340, 0x4d4250, 0x4d5d40, 0x4d5de0) against the originals. All calls recorders numbered as StaffCtorsEnv from 10 (their own benches have them) but ScriptRef_Start and the message centre's making (thing_ctors.py's): a staff member random bytes, the centre there or not, the memory failing now and then.
# usage: staffctors.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Handyman,Mechanic,MechanicLoad,Entertainer,EntertainerLoad,Guard,GuardInit)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import staffctor  # noqa: E402
import thing_ctors  # noqa: E402
from fncmp import HEAP, LOG, LOG_BYTES, RAM, Bench, Call, _recorder, heap, log_region, set_return  # noqa: E402

CALLS = {'Handyman': (0x4d6b60, 0x130), 'Mechanic': (0x4d9eb0, 0x130), 'MechanicLoad': (0x4d9db0, 0x100), 'Entertainer': (0x4d4340, 0x110), 'EntertainerLoad': (0x4d4250, 0xf0), 'Guard': (0x4d5d40, 0xa0), 'GuardInit': (0x4d5de0, 0xc2)}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench(staffctor.merged([(a, a + n) for a, n in CALLS.values()] + [(0x4d4140, 0x4d4170), (0x5da3c0, 0x5da3c1), (0x67acf0, 0x67ada0), (0x475010, 0x476a30), (0x540b90, 0x540d78), (0x541f60, 0x541f70)] + thing_ctors.CENTRE_RANGES),
          hooks={**thing_ctors.CENTRE_HOOKS, 0x504b10: _recorder(10, 4), 0x504a90: _recorder(11, 2), 0x6acd60: _recorder(12, 4, cdecl=True), 0x519650: _recorder(13, 1), 0x5197c0: _recorder(14, 1), 0x519930: _recorder(15, 1),
                 0x4da370: _recorder(16, 1), 0x4d8440: _recorder(17, 2), 0x40f9e0: _recorder(18, 2), 0x4d7100: _recorder(19, 0), 0x4da5b0: _recorder(20, 0), 0x4d46d0: _recorder(21, 0),
                 0x519aa0: _recorder(22, 1), 0x541f70: _recorder(23, 1, cdecl=True), 0x506a40: _recorder(24, 0), 0x4f9490: _recorder(25, 0), 0x4d65d0: _recorder(26, 1)})
STAFF, CENTRE, WORLD, MAP, CHARSETS = RAM + 0x20000, RAM + 0x21000, RAM + 0x22000, RAM + 0x23000, RAM + 0x24000


def case(r):
    op = r.deal(OPS)
    log = log_region()
    struct.pack_into('<I', log, LOG_BYTES + 4 * 17, 0b10 << 16)   # (Map_SpreadAdd's cell: its low word; the upper half is the stack's in the original)
    regions = {LOG: log, STAFF: r.bytes(0x240), CENTRE: r.bytes(0x20), WORLD + 0x1da70c: r.bytes(4), MAP: r.bytes(0x20)}
    regions[HEAP] = heap(0x1000, r.choice([0, 0, 0, 0, 0, 1]))
    regions[0x788d3c] = struct.pack('<I', r.choice([CENTRE, CENTRE, 0]))
    regions[0x7b49f0] = bytearray(16)   # (the script list not ready: scriptref.py's)
    regions[0x80239c] = struct.pack('<I', WORLD)
    regions[0x8023a0] = struct.pack('<I', MAP)
    regions[0x78baa8] = struct.pack('<4I', CHARSETS, CHARSETS + 4, CHARSETS + 8, CHARSETS + 12)
    regions[0x75c7f8] = r.bytes(2)
    regions[0x7cdbec] = r.bytes(4)
    regions[0x7cdb60] = struct.pack('<I', r.choice([0, 1, 2, 3, 0xffffffff, r.getrandbits(32)]))
    set_return(log, 23, r.choice([0, 1, 5, r.getrandbits(32)]))
    set_return(log, 24, r.choice([0, 0, 0, 1, r.getrandbits(32)]))
    set_return(log, 25, r.choice([0, 1, r.getrandbits(32)]))
    args = [STAFF]
    if op != 'GuardInit':
        args.append(r.getrandbits(32))
    if not op.endswith(('Load', 'Init')):
        args += [r.getrandbits(32), r.choice([0, 1, 0x100, r.getrandbits(32)]), r.choice([0, 1, 0x22, 0x23, 0xffffffff, r.getrandbits(32)])]
    return Call(CALLS[op][0], args, regions, conv='t', ret='v' if op == 'GuardInit' else 'a')


if __name__ == '__main__':
    B.main(case)
