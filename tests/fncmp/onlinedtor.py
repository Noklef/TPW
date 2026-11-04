# The online session's destructor (src/game/online/session: 0x5b5920) against the original. Its calls recorders: the resets (each with a bench of its own), the strings' destructors, the imports; the
# objects' deleting destructors (vtable +0) and the first strings' assign (vtable +0x14) recorders. Each member there or not; the list at +0x3cc of 0, 1 or 3 nodes.
# usage: onlinedtor.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, heap, log_region, put, recorder, strings, vtable  # noqa: E402

SAY, RELEASE_NEWS, NEWS_DTOR, UPLOAD = 1, 2, 3, 4
LOGIN, NEWS, CITY, UP, DOWN, MESSAGE, HANGUP, SDTOR, WDTOR = range(8, 17)
B = Bench([(0x5b5920, 0x5b5c00), (0x67acb0, 0x67acb1)],
          hooks={0x5b64d0: _recorder(LOGIN, 0), 0x5b6980: _recorder(NEWS, 0), 0x5b7430: _recorder(CITY, 0), 0x5b8400: _recorder(UP, 0), 0x5b9040: _recorder(DOWN, 0),
                 0x4c25e0: _recorder(MESSAGE, 0, cdecl=True), 0x4bae40: _recorder(HANGUP, 0, cdecl=True), 0x5f61e0: _recorder(SDTOR, 0), 0x5f6a60: _recorder(WDTOR, 0)})
SESSION, OBJECTS, NODES, VT = RAM, RAM + 0x800, RAM + 0x1000, RAM + 0x1800


def case(r):
    log = log_region()
    table = bytearray(0x600)
    put(table, 0x6fd1e0 - IMPORT_TABLE, 'I', recorder(SAY, 1, True))
    strings(log, SAY, 1)
    put(table, 0x6fd58c - IMPORT_TABLE, 'I', recorder(RELEASE_NEWS, 0))
    put(table, 0x6fd588 - IMPORT_TABLE, 'I', recorder(NEWS_DTOR, 0))
    put(table, 0x6fd5ec - IMPORT_TABLE, 'I', recorder(UPLOAD, 0))
    session = bytearray(r.bytes(0x404))
    objects = bytearray(0x400)
    for i, at in enumerate((0xe0, 0xe4, 0xe8, 0xec, 0xf0, 0xf4, 0xf8, 0xfc, 0x100, 0x3a0, 0x3a4)):
        put(objects, 0x20 * i, 'I', VT)
        put(session, at, 'I', r.choice([0, OBJECTS + 0x20 * i, OBJECTS + 0x20 * i]))
    for at in (0xc, 0x20, 0x3c):
        put(session, at, 'I', VT)
    put(session, 0x6c, 'I', r.choice([0, OBJECTS + 0x300]))
    put(session, 0x104, 'I', r.choice([0, OBJECTS + 0x340]))
    put(session, 0x39c, 'I', r.choice([0, HEAP + 0x100]))
    n = r.choice([0, 1, 3])
    nodes = bytearray(r.bytes(0x40))
    for i in range(n):
        put(nodes, 0x10 * i + 4, 'I', NODES + 0x10 * (i + 1) if i + 1 < n else 0)
    put(session, 0x3cc, 'I', NODES if n else 0)
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(), SESSION: session, OBJECTS: objects, NODES: nodes, VT: vtable({0: 1, 5: 1})}
    return Call(0x5b5920, [SESSION], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
