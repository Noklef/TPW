# The synchronisation wrappers of src/game/core/sync against the originals: the Win32 imports are recorders (fncmp import_table()) logging the object
# or handle and their arguments and answering a random value (WaitForSingleObject now and then WAIT_OBJECT_0); timeouts of either sign and the edges.
# usage: sync.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, Bench, Call, RAM, import_table, log_region, set_return  # noqa: E402

# The imports: slot, import table address, stack arguments, string arguments (mask), out arguments (mask).
IMPORTS = {'InitializeCriticalSection': (1, 0x6fd144, 1, 0, 0), 'DeleteCriticalSection': (2, 0x6fd0d0, 1, 0, 0),
           'EnterCriticalSection': (3, 0x6fd140, 1, 0, 0), 'LeaveCriticalSection': (4, 0x6fd130, 1, 0, 0), 'CreateEventA': (5, 0x6fd0d4, 4, 0, 0),
           'WaitForSingleObject': (6, 0x6fd128, 2, 0, 0), 'SetEvent': (7, 0x6fd124, 1, 0, 0)}
# name: (address, returns, takes a timeout)
CALLS = {'Init': (0x5c8b90, 'a', 0), 'Delete': (0x5c8ba0, 'v', 0), 'Enter': (0x5c8bb0, 'a', 0), 'Leave': (0x5c8bc0, 'a', 0),
         'EventCtor': (0x5c8bd0, 'a', 0), 'Wait': (0x5c8bf0, 'b', 1), 'Set': (0x5c8c20, 'a', 0)}
B = Bench([(0x5c8b90, 0x5c8c2c)])
OBJ = RAM
TIMEOUTS = [0, 1, 100, -1, -2, 0x7fffffff, -0x80000000]


def case(r):
    op = r.deal(OPS)
    address, ret, timed = CALLS[op]
    log = log_region()
    table = import_table(log, IMPORTS)
    for k, _at, _n, _m, _o in IMPORTS.values():
        set_return(log, k, r.choice([0, 0, 1, 0x80, 0x102, 0xffffffff]) if k == 6 else r.getrandbits(32))
    regions = {LOG: log, IMPORT_TABLE: table, OBJ: r.bytes(0x20)}
    args = [OBJ]
    if timed:
        args.append((r.choice(TIMEOUTS) if r.random() < 0.7 else r.getrandbits(32)) & 0xffffffff)
    return Call(address, args, regions, conv='t', ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
