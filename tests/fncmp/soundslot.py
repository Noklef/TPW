# A sound object's slot (src/game/core/soundslot) against the originals: a slot at SLOT whose +0 is a COM object at OBJ (a recorder vtable:
# slots 5, 7, 14 and 59, answering any) or null; the release list (0x6d7460's, made already: its flag at 0xfb2728 set) at LIST with its
# head null or a node, the heap failing now and then, Enter/LeaveCriticalSection recorders.
# usage: soundslot.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, heap, import_table, log_region, set_return, vtable  # noqa: E402

# name: (address, returns)
CALLS = {'init': (0x6d6dd0, 'a'), 'call7': (0x6d6df0, 'a'), 'call5': (0x6d6e10, 'a'), 'set': (0x6d6e40, 'a'), 'call59': (0x6d6e70, 'a'),
         'isset': (0x6d6eb0, 'a')}
B = Bench([(0x6d6dd0, 0x6d6dec), (0x6d6df0, 0x6d6e0b), (0x6d6e10, 0x6d6e40), (0x6d6e40, 0x6d6e6e), (0x6d6e70, 0x6d6ea8), (0x6d6eb0, 0x6d6eba),
           (0x6d7460, 0x6d7493), (0x6d7520, 0x6d75b7)])
SLOT, OUT, OBJ, OVT, NODE, NEW = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x500, RAM + 0x600
FLAG, LIST = 0xfb2728, 0xfb2730
ENTER_K, LEAVE_K = 40, 41
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def case(r):
    name = r.deal(OPS)
    address, ret = CALLS[name]
    log = log_region()
    imports = import_table(log, {'enter': (ENTER_K, 0x6fd140, 1, 0, 0), 'leave': (LEAVE_K, 0x6fd130, 1, 0, 0)})
    for k in (5, 7, 14, 59):
        if r.random() < 0.8:
            set_return(log, k, r.choice([0, 1, 0x80004005, r.getrandbits(32)]))
    slot = bytearray(r.bytes(0x2c))
    struct.pack_into('<I', slot, 0, r.choice([OBJ, OBJ, OBJ, 0]))
    head = r.choice([0, NODE])
    lst = bytearray(r.bytes(0x30))
    struct.pack_into('<4I', lst, 0, r.getrandbits(32), head, r.choice([NODE, 0]) if head else 0, r.randrange(0, 5))
    flag = bytearray(r.bytes(4))
    flag[0] |= 1
    regions = {SLOT: slot, OUT: r.bytes(4), OBJ: struct.pack('<I', OVT) + r.bytes(0xc), OVT: vtable({}, size=64, com={5: 2, 7: 2, 14: 1, 59: 3}),
               NODE: r.bytes(0xc), FLAG: flag, LIST: lst, IMPORT_TABLE: imports, LOG: log, HEAP: heap(0x40, fail=r.choice([0, 0, 0, 1]))}
    value = r.getrandbits(32)
    if name in ('init', 'isset'):
        return Call(address, [SLOT], regions, conv='t', ret=ret)
    if name == 'call7':
        return Call(address, [SLOT, value], regions, conv='t', ret=ret)
    if name == 'set':
        return Call(address, [SLOT, r.choice([NEW, 0, value])], regions, conv='t', ret=ret)
    if name == 'call5':
        return Call(address, [SLOT, OUT, value], regions, conv='t', ret=ret)
    return Call(address, [SLOT, OUT, value, r.getrandbits(32)], regions, conv='t', ret=ret)


if __name__ == '__main__':
    B.main(case)
