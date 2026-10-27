# The event queue (src/game/core/events: 0x5fbde0-0x5fc31f) against the originals: the queue at 0xfa7cb0 (or another) with a critical section
# of its own or none and six channels of zero to three listeners each, the listeners' callbacks COM-style recorders (stdcall: channel, event,
# context; the channel logged as `this`, 0), none, or other values, contexts few so that Unlisten's matches happen; the critical section's
# imports (InitializeCriticalSection, DeleteCriticalSection, Enter / Leave) COM-style recorders in the import table; atexit fncmp's cdecl log
# on both sides (driver.cpp's EventQueue env); allocations from the heap region, failing now and then. Post, Listen, Unlisten on channels 0-6
# (6 is "none"); Clear and the destructors over every list; the lists' constructor and destructors on a list of their own. PostKind4 (0x47e9d0)
# posts {4, value} on channel 0 from the stack (the callbacks log its first two dwords). ScreenReset (0x47ea00) posts {6, 0} and then
# {7, width, height, byte 0x10, 0} on channel 0 from one stack event (the callbacks log its first two dwords for kind 6, whose +8.. are unset,
# and for kind 7 its dwords 0-2, the byte at +0xc (the rest of that dword unset) and dword 4: blob_when on the event's kind), channel 0's
# listeners one to three recorders (or none, now and then), then releases the COM objects at 0x7b4b6c / 0x7b4b70 (recorders or none) and clears 0x7b4b64-0x7b4b70.
# usage: events.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=StaticInit,... to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _cdecl_log, blob, blob_bytes, blob_when, heap, import_table, log_region, put, recorder  # noqa: E402

B = Bench([(0x5fbde0, 0x5fc330), (0x47e9d0, 0x47e9f8), (0x47ea00, 0x47ea91), (0x67a740, 0x67a800), (0x67acf0, 0x67ae10), (0x67afa0, 0x67b0d0), (0x67dcd0, 0x67dd50), (0x67efc0, 0x67f1b0),
           (0x67fa20, 0x67fa50), (0x681740, 0x681970), (0x682720, 0x6827d0), (0x6828a0, 0x682a20), (0x682a70, 0x6830a0), (0x688710, 0x688740)],
          hooks={0x67a7d0: _cdecl_log})
CALLS = {'StaticInit': 0x5fbde0, 'Construct': 0x5fbdf0, 'RegisterExit': 0x5fbe00, 'AtExit': 0x5fbe10, 'Ctor': 0x5fbe20, 'ListCtor': 0x5fbe90,
         'ListDtor': 0x5fbeb0, 'ListDelDtor': 0x5fbef0, 'Dtor': 0x5fbf10, 'Post': 0x5fbf80, 'GuardRelease': 0x5fc010, 'Listen': 0x5fc030,
         'Unlisten': 0x5fc100, 'Clear': 0x5fc220, 'BaseDelDtor': 0x5fc2d0, 'PostKind4': 0x47e9d0,
         'ScreenReset': 0x47ea00}
Q = 0xfa7cb0
LOCK, NODES, LISTENERS, EVENT, GUARD, LIST, OTHER = RAM, RAM + 0x100, RAM + 0x400, RAM + 0x700, RAM + 0x780, RAM + 0x800, RAM + 0x900
IMPORTS = {'InitializeCriticalSection': (10, 0x6fd144, 1, 0, 0), 'DeleteCriticalSection': (11, 0x6fd0d0, 1, 0, 0),
           'EnterCriticalSection': (12, 0x6fd140, 1, 0, 0), 'LeaveCriticalSection': (13, 0x6fd130, 1, 0, 0)}
CALLBACKS = [recorder(1, 3, True), recorder(2, 3, True), recorder(3, 3, True)]
CONTEXTS = [0, 1, 2, RAM + 0x40]


def lists(r, out, at, channels, nodes, listeners):
    """`channels` lists at out + at, their nodes {listener, next, prev} and listeners {context, callback} in the given bytearrays."""
    used = 0
    for ch in range(channels):
        n = r.choice([0, 0, 1, 2, 3])
        first = used
        for i in range(n):
            k = used + i
            put(nodes, 0xc * k, '3I', LISTENERS + 8 * k, NODES + 0xc * (k + 1) if i + 1 < n else 0, NODES + 0xc * (k - 1) if i else 0)
            put(listeners, 8 * k, '2I', r.choice(CONTEXTS), r.choice(CALLBACKS + CALLBACKS + [0, r.getrandbits(32) | 0x80000000]))
        used += n
        put(out, at + 0x10 * ch, '4I', r.choice([0x703648, 0x70364c, r.getrandbits(32)]), NODES + 0xc * first if n else 0,
            NODES + 0xc * (used - 1) if n else 0, n)


def case(r):
    op = r.deal(OPS)
    log = log_region()
    q = bytearray(r.bytes(0x74))   # (and a seventh list past the queue: channel 6's, which is "none")
    nodes, listeners = bytearray(r.bytes(0xc * 21)), bytearray(r.bytes(8 * 21))
    put(q, 0, 'I', LOCK if r.random() < 0.85 else 0)
    lists(r, q, 4, 7, nodes, listeners)
    lst = bytearray(r.bytes(0x10))
    if op in ('ListCtor', 'ListDtor', 'ListDelDtor', 'BaseDelDtor'):
        lists(r, lst, 0, 1, nodes, listeners)
    regions = {Q: q, LOCK: r.bytes(0x18), NODES: nodes, LISTENERS: listeners, EVENT: r.bytes(0x20), LIST: lst, OTHER: r.bytes(0x64),
               GUARD: struct.pack('<I', r.choice([LOCK, LOCK, 0])), LOG: log, IMPORT_TABLE: import_table(log, IMPORTS),
               HEAP: heap(fail=r.choice([0, 0, 0, 0, 0, 1]))}
    address = CALLS[op]
    if op == 'ScreenReset':
        for k in (1, 2, 3):
            blob_bytes(log, k, 1, 0xff)
            blob_when(log, k, 1, 7, 0xf1fff, entry=k - 1, deref=True)
        n = r.choice([1, 2, 3, 3, 0])   # (channel 0's list rebuilt from node 0: the other channels are not walked)
        for i in range(n):
            put(nodes, 0xc * i, '3I', LISTENERS + 8 * i, NODES + 0xc * (i + 1) if i + 1 < n else 0, NODES + 0xc * (i - 1) if i else 0)
            put(listeners, 8 * i, '2I', r.choice(CONTEXTS), r.choice(CALLBACKS))
        put(q, 4, '4I', r.choice([0x703648, 0x70364c]), NODES if n else 0, NODES + 0xc * (n - 1) if n else 0, n)
        regions[NODES], regions[LISTENERS], regions[Q] = nodes, listeners, q
        com = RAM + 0xa00
        regions[com] = struct.pack('<4I', com + 0x10, 0, com + 0x10, 0) + struct.pack('<3I', 0, 0, recorder(4, 1, True))
        regions[0x7b4b64] = struct.pack('<4I', r.getrandbits(32), r.getrandbits(32), r.choice([com, com, 0]), r.choice([com + 8, com + 8, 0]))
        regions[0x7a1968] = struct.pack('<I', r.choice([0x280, 0x320, 0x400, r.getrandbits(32)]))
        regions[0x7a1930] = struct.pack('<I', r.choice([0x1e0, 0x258, 0x300, r.getrandbits(32)]))
        return Call(address, [], regions, conv='c', ret='v')
    if op == 'PostKind4':
        for k in (1, 2, 3):
            blob(log, k, 1, 3)   # (the event: on the stack)
        return Call(address, [r.choice([0, 1, 0x100, r.getrandbits(32)])], regions, conv='c', ret='v')
    if op in ('StaticInit', 'Construct', 'RegisterExit', 'AtExit'):
        return Call(address, [], regions, conv='c', ret='a' if op == 'Construct' else 'v')
    if op == 'Ctor':
        return Call(address, [r.choice([Q, OTHER])], regions, conv='t', ret='a')
    if op in ('ListCtor', 'ListDtor'):
        return Call(address, [LIST], regions, conv='t', ret='a' if op == 'ListCtor' else 'v')
    if op in ('ListDelDtor', 'BaseDelDtor'):
        return Call(address, [LIST, r.choice([0, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')
    if op == 'GuardRelease':
        return Call(address, [GUARD], regions, conv='t', ret='v')
    if op in ('Dtor', 'Clear'):
        return Call(address, [Q], regions, conv='t', ret='v')
    channel = r.choice([0, 1, 2, 3, 4, 5, 6])
    if op == 'Post':
        return Call(address, [Q, channel, EVENT], regions, conv='t', ret='v')
    callback = r.choice(CALLBACKS + [0, r.getrandbits(32)])
    context = r.choice(CONTEXTS + [3])
    if op == 'Unlisten':
        callback, context = r.choice(CALLBACKS + [callback]), r.choice([0, context])
        put(q, 0, 'I', LOCK if r.random() < 0.9 else 0)
    elif r.random() < 0.85:   # (Listen: a failed node allocation writes through null)
        regions[HEAP] = heap()
    return Call(address, [Q, channel, callback, context], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
