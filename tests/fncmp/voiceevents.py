# The channel voices' events (src/game/core/voiceevents) against the originals: the constructor (0x6c6120; counts 0-4 or any, the
# allocation failing now and then), the destructor and the deleting one (0x6c6090, 0x6c6100; 0-5 handles, flags any) and the stop
# (0x6c6160); CloseHandle and SetEvent recorders, the flags +0x14 any.
# usage: voiceevents.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, heap, import_table, log_region, set_return  # noqa: E402

CALLS = {'ctor': (0x6c6120, 'a'), 'dtor': (0x6c6090, 'v'), 'ddtor': (0x6c6100, 'a'), 'stop': (0x6c6160, 'a')}
B = Bench([(0x6c6090, 0x6c60ff), (0x6c6100, 0x6c611e), (0x6c6120, 0x6c615d), (0x6c6160, 0x6c6179), (0x67acb0, 0x67acb1)])
SELF, HANDLES, VOICES = RAM, RAM + 0x100, RAM + 0x200
IMPORTS = {'CloseHandle': (10, 0x6fd13c, 1, 0, 0), 'SetEvent': (11, 0x6fd124, 1, 0, 0)}
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def case(r):
    name = r.deal(OPS)
    address, ret = CALLS[name]
    log = log_region()
    for k in (10, 11):
        set_return(log, k, r.getrandbits(32))
    count = r.choice([0, 1, 2, 3, 4])
    obj = bytearray(r.bytes(0x18))
    struct.pack_into('<5I', obj, 4, VOICES, count + 1, count, HANDLES, r.choice([0, 1, 2, 3, r.getrandbits(32)]))
    regions = {SELF: obj, HANDLES: r.bytes(0x18), IMPORT_TABLE: import_table(log, IMPORTS), LOG: log, HEAP: heap(0x100, fail=r.choice([0, 0, 0, 1]))}
    args = []
    if name == 'ctor':
        args = [VOICES, r.choice([0, 1, 2, 4, 0x20, r.getrandbits(32) if r.wild else 3])]
    elif name == 'ddtor':
        args = [r.choice([0, 1, 2, 3, r.getrandbits(32)])]
    return Call(address, [SELF] + args, regions, conv='t', ret=ret)


if __name__ == '__main__':
    B.main(case)
