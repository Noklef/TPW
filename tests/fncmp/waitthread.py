# The thread waiting on event handles (src/game/core/waitthread) against the originals: the imports are recorders, the vectors real with the real heap; the range error's recorder returns, and the
# originals then run off the handles to a fault, as the port does.
# usage: waitthread.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, fill, heap, import_table, log_region, put, set_return, strings, vtable  # noqa: E402

ADDRESS = {'Proc': (0x632460, 13), 'Loop': (0x6324e0, 0x116), 'Run': (0x632470, 109), 'Open': (0x632750, 134), 'Close': (0x6327e0, 89), 'Send': (0x632840, 105), 'Add': (0x6328b0, 31), 'Remove': (0x6328d0, 31), 'DoAdd': (0x6328f0, 100), 'DoRemove': (0x632960, 216), 'Insert': (0x632ef0, 0x223), 'Start': (0x632600, 107), 'Stop': (0x632670, 102), 'Shutdown': (0x632730, 17), 'Dtor': (0x6323e0, 118), 'Del': (0x6323c0, 30),
           'Init': (0x6326e0, 77)}
OPS = [n for n in ADDRESS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
SET, WAIT, CREATE, CLOSE, ERROR = 30, 31, 32, 33, 34
RANGE = 25
B = Bench([(a, a + n) for a, n in ADDRESS.values()] + [(0x6322e0, 0x632310),(0x47cf20, 0x47d140), (0x47db00, 0x47db20), (0x5fa650, 0x5fa678), (0x6b70d0, 0x6b70e0), (0x67acb0, 0x67acb1), (0x67a400, 0x690000)],
          hooks={0x632a40: _recorder(RANGE, 0), 0x67f780: _recorder(24, 6, cdecl=True)})
START, SLEEP, EXITCODE = 24, 35, 36
OBJ, OUT, ITEM, HANDLES, OBJECTS, ANSWERS, ELEMENT, VT = RAM, RAM + 0x80, RAM + 0x90, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500
FORWAIT = 26


def imports(log):
    return import_table(log, {'SetEvent': (SET, 0x6fd124, 1, 0, 0), 'WaitForSingleObject': (WAIT, 0x6fd128, 2, 0, 0), 'CreateEventA': (CREATE, 0x6fd0d4, 4, 0, 0), 'CloseHandle': (CLOSE, 0x6fd13c, 1, 0, 0),
                              'GetLastError': (ERROR, 0x6fd138, 0, 0, 0), 'WaitForMultipleObjects': (FORWAIT, 0x6fd160, 4, 0, 0), 'Sleep': (SLEEP, 0x6fd1d4, 1, 0, 0), 'GetExitCodeThread': (EXITCODE, 0x6fd0b8, 2, 0, 0b10)})


def vector(r, obj, offset, at, count, spare):
    """A vector of count random dwords and room for spare more at at; empty ones without a block, sometimes."""
    if not count and not spare and r.random() < 0.7:
        put(obj, offset + 4, '3I', 0, 0, 0)
    else:
        put(obj, offset + 4, '3I', at, at + 4 * count, at + 4 * (count + spare))
    return bytearray(r.bytes(0x100))


def case(r):
    op = r.deal(OPS)
    address = ADDRESS[op][0]
    log = log_region()
    obj = bytearray(r.bytes(0x48))
    count = r.choice([0, 1, 2, 3, 4])
    handles = vector(r, obj, 0x24, HANDLES, count + 1 if r.random() < 0.7 else count, r.choice([0, 0, 1, 4]))
    objects = vector(r, obj, 0x34, OBJECTS, count, r.choice([0, 0, 1, 4]))
    item = bytearray(r.bytes(0x10))
    regions = {LOG: log, OBJ: obj, OUT: r.bytes(8), ITEM: item, HANDLES: handles, OBJECTS: objects, IMPORT_TABLE: imports(log), HEAP: heap()}
    set_return(log, SET, r.choice([0, 1, 1, 1]))
    set_return(log, WAIT, r.choice([0, 0, 0, 0x102, 0xffffffff]))
    set_return(log, ERROR, r.choice([0, 5, 0x12345]))
    set_return(log, CLOSE, r.getrandbits(32))
    put(obj, 4, 'I', r.choice([0, 1, 2, 3, 3, 4]) if op == 'Send' or op == 'Run' else struct.unpack_from('<I', obj, 4)[0])
    put(obj, 8, 'I', r.choice([0, ITEM, ITEM, ITEM]))
    if op in ('Run', 'DoRemove'):
        if count and r.random() < 0.8:   # (in the list)
            put(objects, 4 * (count - 1 if r.random() < 0.5 else r.randrange(count)), 'I', ITEM)   # (the last: one past the handles when they lack the command's)
    if op == 'Run':
        put(obj, 4, 'I', r.choice([1, 2, 3, 0, 4]))
    if op in ('Proc', 'Loop'):
        # (The loop ends on the command 3 with the answer 0, the last of the table.)
        put(obj, 4, 'I', 3)
        put(obj, 0x10, 'I', r.choice([0, 0, 0, 0, 1]))
        table = [r.choice([1, 1, 2, 2, 3, 4, 5, 0xffffffff, 0xfffffffe, 0x80, 0x102]) for _ in range(r.choice([0, 1, 2, 3, 5, 8]))] + [0]
        regions[ANSWERS] = struct.pack('<40I', *(table + [0] * (40 - len(table))))
        answers(log, FORWAIT, ANSWERS)
        regions[VT] = vtable({})
        put(obj, 0x28, 'II', HANDLES, HANDLES + 4 * (count + 1))   # (a block, or the last answer would not end it)
        put(objects, 0, '8I', *[r.choice([ELEMENT, ELEMENT + 0x10, ELEMENT + 0x20]) for _ in range(8)])
        regions[ELEMENT] = struct.pack('<I', VT) + b'\0' * 12 + struct.pack('<I', VT) + b'\0' * 12 + struct.pack('<I', VT)
        if op == 'Proc':
            return Call(address, [OBJ], regions, conv='s', ret='a')
        return Call(address, [OBJ], regions, conv='t', ret='a')
    if op == 'Init':   # (Open, then Start when Open did not fail: the two setups)
        regions[ANSWERS] = struct.pack('<40I', *[r.choice([0, 0x3000, 0x3000, 0x4000]) for _ in range(40)])
        answers(log, CREATE, ANSWERS)
    if op in ('Start', 'Stop', 'Shutdown', 'Dtor', 'Del', 'Init'):
        put(obj, 0x14, 'I', r.choice([0, 0x777, 0x777, 0x777]))
        put(obj, 0x18, 'II', r.choice([0, 0x1000]), r.choice([0, 0x2000]))
        put(obj, 0xc, 'I', r.choice([0, 0, 0, 1, 0x80000000]))
        put(obj, 4, 'I', r.getrandbits(32))
        set_return(log, EXITCODE, r.choice([0, 1, 1, 1]))
        fill(log, EXITCODE, 1, struct.pack('<I', r.choice([0x103, 0x103, 0, 1])))
        set_return(log, START, r.choice([0, 0x888, 0x888]))
        strings(log, START, 0, out=0b100000)
        regions[HEAP] = heap(0x100)
        if op in ('Start', 'Init'):
            return Call(address, [OBJ, OUT], regions, conv='t', ret='a')
        if op == 'Del':
            return Call(address, [OBJ, r.choice([0, 1, 2, 3])], regions, conv='t', ret='a')
        return Call(address, [OBJ], regions, conv='t', ret='v')
    if op == 'Insert':
        where = struct.unpack_from('<I', obj, 0x38)[0] + 4 * r.randint(0, count) if struct.unpack_from('<I', obj, 0x38)[0] else 0
        return Call(address, [OBJ + 0x34, where, r.choice([0, 1, 1, 2, 3, 6]), ITEM], regions, conv='t', ret='v')
    if op == 'Open':
        put(obj, 0x18, 'II', r.choice([0, 0, 0x1000]), r.choice([0, 0, 0x2000]))
        regions[ANSWERS] = struct.pack('<40I', *[r.choice([0, 0x3000, 0x3000, 0x4000]) for _ in range(40)])
        answers(log, CREATE, ANSWERS)
    if op == 'Close':
        put(obj, 0x18, 'II', r.choice([0, 0x1000]), r.choice([0, 0x2000]))
    if op in ('Add', 'Remove'):
        return Call(address, [OBJ, OUT, r.getrandbits(32)], regions, conv='t', ret='a')
    if op in ('Run', 'Close'):
        return Call(address, [OBJ], regions, conv='t', ret='v')
    return Call(address, [OBJ, OUT], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
