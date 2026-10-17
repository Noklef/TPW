# The file library's counted classes (src/game/files/handles) against the originals: Handle (0x5f8240 .. 0x5f84e0: constructors, destructors, Assign, Reset, Set and the
# forwarders, and the folder handle's deleting destructor 0x41caf0), StreamBase (0x5f7180 .. 0x5f7290, 0x4036a0 / 0x4036c0, 0x639100, FileStream's destructors), RefObject
# (0x618cd0 .. 0x618de0) and RefObject2 (0x619800 .. 0x619940 but its constructor, destructors and Vfn5, which create a semaphore and a text writer). The holders are a
# handle or stream at THIS and another at OTHER, each holding one of up to three counted objects 0x40 apart (or none), each linked to the next or not: RefObject2s
# (count +0xc) for handles, RefObjects (count +8) for streams. Their vtables are recorders taking the stack arguments the forwarders pass; InterlockedIncrement /
# Decrement's import slots are recorders, the decrement answering a count from 2 down to -1 (or its hash); a heap for the deleting destructors' free.
# usage: handles.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, HEAP, RAM, heap, log_region, put, recorder, set_return, vtable  # noqa: E402

B = Bench([(0x41caf0, 0x41cb10), (0x41cc10, 0x41cc20), (0x4036a0, 0x4036e0), (0x414d30, 0x414d50), (0x5f7180, 0x5f72c0), (0x5f8240, 0x5f8510), (0x5fa950, 0x5fa970),
           (0x618cd0, 0x618df0), (0x619800, 0x6198b0), (0x619940, 0x619950), (0x639100, 0x639110), (0x67acb0, 0x67acb1)])
THIS, OTHER, RESULT = RAM, RAM + 0x10, RAM + 0x20
OBJECTS, SOURCES, PEER = RAM + 0x1000, RAM + 0x1800, RAM + 0x1c00
VTABLE, SOURCE_VTABLE, HOLDER_VTABLE, PEER_VTABLE = RAM + 0x2000, RAM + 0x2100, RAM + 0x2200, RAM + 0x2300
INCREMENT, DECREMENT = 0x6fd0c0, 0x6fd12c   # the import table's InterlockedIncrement and InterlockedDecrement
ADD, SUB = 40, 41   # the recorder slots the imports log as (clear of the vtables' 32)
# The stack arguments of each slot. RefObject2: 0 the deleting destructor, +4 .. +0x28 the handle forwarders' targets, +0x2c the attach, +0x30 the teardown.
# RefObject: 0 the deleting destructor, +4 .. +0x20 the stream forwarders' and its own defaults' targets, +0x24 the teardown, +0x28 the attach, +0x2c the lock.
OBJECT_SLOTS = {0: 1, 1: 5, 2: 3, 3: 2, 4: 3, 5: 4, 7: 3, 8: 2, 9: 2, 10: 3, 11: 1}
SOURCE_SLOTS = {0: 1, 1: 2, 2: 2, 3: 2, 6: 2, 7: 1, 8: 1, 10: 1}
# name: (address, this: h a handle, s a stream, r a RefObject, o a RefObject2; the stack arguments: f flags, H the other holder, S a source or none, O an object,
# R the result pointer, d a random dword, P the peer; returns)
OPS = {
    'Ctor': (0x5f8240, 'h', '', 'a'), 'CopyCtor': (0x5f8270, 'h', 'H', 'a'), 'DeletingDtor': (0x5f8250, 'h', 'f', 'a'), 'Dtor': (0x5f82a0, 'h', '', 'v'),
    'Assign': (0x5f82b0, 'h', 'H', 'a'), 'Reset': (0x5f82e0, 'h', '', 'v'), 'Set': (0x5f8420, 'h', 'O', 'v'), 'FolderDeletingDtor': (0x41caf0, 'h', 'f', 'a'),
    'Vfn1': (0x5f8370, 'h', 'Rdd', 'a'), 'Vfn2': (0x5f83a0, 'h', 'Rd', 'a'), 'Vfn3': (0x5f83d0, 'h', 'Rdddd', 'a'), 'Vfn4': (0x5f8300, 'h', 'Rdd', 'a'),
    'Vfn5': (0x5f8330, 'h', 'Rddd', 'a'), 'Vfn6': (0x5f8410, 'h', '', 'a'), 'Vfn7': (0x5f8450, 'h', 'Rdd', 'a'), 'Vfn8': (0x5f8480, 'h', 'Rd', 'a'),
    'Vfn9': (0x5f84b0, 'h', 'Rd', 'a'), 'Vfn10': (0x5f84e0, 'h', 'Rdd', 'a'),
    'StreamCtor': (0x5f7180, 's', '', 'a'), 'StreamCopyCtor': (0x5f71b0, 's', 'H', 'a'), 'StreamDtor': (0x5f71e0, 's', '', 'v'),
    'StreamDeletingDtor': (0x5f7190, 's', 'f', 'a'), 'Close': (0x5f71f0, 's', '', 'v'), 'Attach': (0x5f7240, 's', 'S', 'v'), 'Read': (0x4036a0, 's', 'dd', 'a'),
    'Write': (0x4036c0, 's', 'dd', 'a'), 'Seek': (0x5f7210, 's', 'dd', 'a'), 'Size': (0x639100, 's', '', 'a'), 'Tell': (0x5f7230, 's', '', 'a'),
    'StreamVfn7': (0x5f7270, 's', '', 'a'), 'StreamVfn8': (0x5f7290, 's', 'R', 'a'), 'FileDtor': (0x414d30, 's', '', 'v'), 'FileDeletingDtor': (0x5fa950, 's', 'f', 'a'),
    'AddRef': (0x618d20, 'r', '', 'a'), 'Release': (0x618d30, 'r', '', 'a'), 'SetLink': (0x618d80, 'r', 'S', 'v'), 'RefCtor': (0x618cd0, 'r', '', 'a'),
    'RefDtor': (0x618d10, 'r', '', 'v'), 'RefDeletingDtor': (0x618cf0, 'r', 'f', 'a'), 'RefVfn6': (0x618da0, 'r', 'dd', 'a'), 'RefVfn7': (0x618dc0, 'r', 'd', 'a'),
    'GetLock': (0x618dd0, 'r', '', 'a'), 'RefVfn8': (0x618de0, 'r', 'R', 'a'),
    'AddRef2': (0x619800, 'o', '', 'a'), 'Release2': (0x619810, 'o', '', 'a'), 'SetLink2': (0x619860, 'o', 'O', 'v'), 'Vfn4of2': (0x619880, 'o', 'RPd', 'a'),
    'Path': (0x619940, 'o', '', 'a'),
}


def pool(r, base, slots):
    """Three counted objects 0x40 apart at base (vtable, then each linked to the next or not), random bytes otherwise."""
    block = bytearray(r.bytes(0xc0))
    for k in range(3):
        put(block, 0x40 * k, 'I', slots)
        put(block, 0x40 * k + 4, 'I', base + 0x40 * (k + 1) if k < 2 and r.random() < 0.5 else 0)
    return block


def pick(r, base):
    """One of a pool's objects or (now and then) none."""
    return base + 0x40 * r.randrange(3) if r.random() < 0.8 else 0


def case(r):
    address, kind, args, ret = OPS[r.choice(PICK)]
    held = SOURCES if kind == 's' else OBJECTS
    holders = bytearray(r.bytes(0x40))
    struct.pack_into('<2I', holders, 0, HOLDER_VTABLE, pick(r, held))
    struct.pack_into('<2I', holders, 0x10, HOLDER_VTABLE, pick(r, held))
    log = log_region()
    if r.random() < 0.8:
        set_return(log, SUB, r.choice([2, 1, 0, 0, 0, -1]))
    regions = {RAM: holders, OBJECTS: pool(r, OBJECTS, VTABLE), SOURCES: pool(r, SOURCES, SOURCE_VTABLE), PEER: struct.pack('<I', PEER_VTABLE) + r.bytes(0x3c),
               VTABLE: vtable(OBJECT_SLOTS), SOURCE_VTABLE: vtable(SOURCE_SLOTS), HOLDER_VTABLE: vtable({}), PEER_VTABLE: vtable({6: 1, 10: 1}),
               INCREMENT: struct.pack('<I', recorder(ADD, 1, True)), DECREMENT: struct.pack('<I', recorder(SUB, 1, True)), LOG: log, HEAP: heap()}
    this = THIS if kind in 'hs' else (OBJECTS if kind == 'o' else SOURCES) + 0x40 * r.randrange(3)
    stack = []
    for a in args:
        stack.append({'f': lambda: r.choice([0, 1, 2, 3, r.getrandbits(32)]), 'H': lambda: r.choice([OTHER, OTHER, THIS]), 'S': lambda: pick(r, SOURCES),
                      'O': lambda: OBJECTS + 0x40 * r.randrange(3), 'R': lambda: RESULT, 'd': lambda: r.getrandbits(32), 'P': lambda: PEER}[a]())
    return Call(address, [this] + stack, regions, conv='t', ret=ret)


PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
