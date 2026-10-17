# The file library's Handle (src/game/files/handles: the constructors 0x5f8240 / 0x5f8270, the destructors 0x5f82a0 / 0x5f8250, Assign 0x5f82b0, Reset 0x5f82e0, Set 0x5f8420,
# and the folder handle's deleting destructor 0x41caf0) against the originals: a handle (its vtable recorders, its object or none) and another handle, the objects up to
# three RefObject2s (count at +0xc) 0x40 apart, each linked to the next or not, their vtables recorders (the deleting destructor with its flag, the attach +0x2c with the old
# object, the teardown +0x30); InterlockedIncrement / Decrement's import slots recorders, the decrement answering a count from 2 down to -1 (or its hash); a heap for the
# deleting destructors' free.
# usage: handles.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, HEAP, RAM, heap, log_region, put, recorder, set_return, vtable  # noqa: E402

B = Bench([(0x41caf0, 0x41cb10), (0x41cc10, 0x41cc20), (0x5f8240, 0x5f8300), (0x5f8420, 0x5f8450), (0x619800, 0x619870), (0x67acb0, 0x67acb1)])
THIS, OTHER = RAM, RAM + 0x10
OBJECTS, VTABLE, HANDLE_VTABLE = RAM + 0x1000, RAM + 0x2000, RAM + 0x2100
INCREMENT, DECREMENT = 0x6fd0c0, 0x6fd12c   # the import table's InterlockedIncrement and InterlockedDecrement
ADD, SUB = 40, 41   # the recorder slots the imports log as (clear of the vtables' 32)
# name: (address, arguments: f the flags, h the other handle, o an object)
OPS = {'Ctor': (0x5f8240, ''), 'CopyCtor': (0x5f8270, 'h'), 'DeletingDtor': (0x5f8250, 'f'), 'Dtor': (0x5f82a0, ''), 'Assign': (0x5f82b0, 'h'), 'Reset': (0x5f82e0, ''),
       'Set': (0x5f8420, 'o'), 'FolderDeletingDtor': (0x41caf0, 'f')}


def obj(r):
    """One of the objects or (now and then) none."""
    return OBJECTS + 0x40 * r.randrange(3) if r.random() < 0.8 else 0


def case(r):
    address, args = OPS[r.choice(PICK)]
    block = bytearray(r.bytes(0xc0))
    for k in range(3):
        put(block, 0x40 * k, 'I', VTABLE)
        put(block, 0x40 * k + 4, 'I', OBJECTS + 0x40 * (k + 1) if k < 2 and r.random() < 0.5 else 0)
    handles = bytearray(0x20)
    struct.pack_into('<2I', handles, 0, HANDLE_VTABLE, obj(r))
    struct.pack_into('<2I', handles, 0x10, HANDLE_VTABLE, obj(r))
    log = log_region()
    if r.random() < 0.8:
        set_return(log, SUB, r.choice([2, 1, 0, 0, 0, -1]))
    regions = {RAM: handles, OBJECTS: block, VTABLE: vtable({0: 1, 11: 1}), HANDLE_VTABLE: vtable({}), INCREMENT: struct.pack('<I', recorder(ADD, 1, True)),
               DECREMENT: struct.pack('<I', recorder(SUB, 1, True)), LOG: log, HEAP: heap()}
    extra = {'': [], 'f': [r.choice([0, 1, 2, 3, r.getrandbits(32)])], 'h': [r.choice([OTHER, OTHER, THIS])], 'o': [OBJECTS + 0x40 * r.randrange(3)]}[args]
    return Call(address, [THIS] + extra, regions, conv='t', ret='v' if address in (0x5f82a0, 0x5f82e0, 0x5f8420) else 'a')


PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
