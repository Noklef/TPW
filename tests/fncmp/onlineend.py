# The online session's download and upload ends (src/game/online/session: 0x5b90e0, 0x5b8490, 0x5b79f0) against the originals. Their calls recorders (the resets, each with a bench of its own); the
# login's release and the upload's release run on the imports' recorders.
# usage: onlineend.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, fill, heap, log_region, put, recorder, set_return, strings, vtable  # noqa: E402

# (0 and 5: the vtable's deleting destructor and assign)
SAY, CURRENT, PROGRESS, AUTH_RELEASE, AUTH_DTOR, UPLOAD, CITY_START, CALLBACK = 1, 2, 3, 4, 6, 7, 22, 23
POST, OPEN, RELEASE_NEWS, NEWS, CITY, UP, DOWN, MESSAGE, HANGUP, SHOW, SET, INSTANCE, UP_START = range(8, 21)
B = Bench([(0x5b90e0, 0x5b92d0), (0x5b8490, 0x5b8680), (0x5b79f0, 0x5b7b80), (0x5b7b80, 0x5b7b9f), (0x5b83e0, 0x5b8400), (0x67acb0, 0x67acb1)],
          hooks={0x47f410: _recorder(POST, 2, cdecl=True), 0x5b72f0: _recorder(OPEN, 0), 0x5b6b20: _recorder(RELEASE_NEWS, 0), 0x5b6980: _recorder(NEWS, 0), 0x5b7430: _recorder(CITY, 0),
                 0x5b8400: _recorder(UP, 0), 0x5b9040: _recorder(DOWN, 0), 0x4c25e0: _recorder(MESSAGE, 0, cdecl=True), 0x4bae40: _recorder(HANGUP, 0, cdecl=True),
                 0x4c25b0: _recorder(SHOW, 1, cdecl=True), 0x4c2600: _recorder(SET, 1, cdecl=True), 0x48b390: _recorder(INSTANCE, 0, cdecl=True), 0x5b7cd0: _recorder(UP_START, 0)})
SESSION, OBJECTS, ANSWERS, VT = RAM, RAM + 0x800, RAM + 0x1000, RAM + 0x1800


def case(r):
    log = log_region()
    table = bytearray(0x600)
    put(table, 0x6fd1e0 - IMPORT_TABLE, 'I', recorder(SAY, 1, True))
    strings(log, SAY, 1)
    put(table, 0x6fd5d4 - IMPORT_TABLE, 'I', recorder(CURRENT, 1))
    put(table, 0x6fd5d8 - IMPORT_TABLE, 'I', recorder(PROGRESS, 3))
    strings(log, PROGRESS, 0, 0b110)
    put(table, 0x6fd45c - IMPORT_TABLE, 'I', recorder(AUTH_RELEASE, 0))
    put(table, 0x6fd460 - IMPORT_TABLE, 'I', recorder(AUTH_DTOR, 0))
    put(table, 0x6fd5ec - IMPORT_TABLE, 'I', recorder(UPLOAD, 0))
    put(table, 0x6fd52c - IMPORT_TABLE, 'I', recorder(CITY_START, 3))
    session = bytearray(r.bytes(0x400))
    objects = bytearray(r.bytes(0x400))
    put(objects, 0, 'I', VT)
    for at in (0xc, 0x20, 0x3c, 0x7c, 0x98, 0xb4, 0x348, 0x35c):
        put(session, at, 'I', VT)
    put(session, 0, 'I', r.choice([0, OBJECTS + 0x100, OBJECTS + 0x100]))
    put(session, 0x104, 'I', r.choice([0, OBJECTS + 0x140, OBJECTS + 0x140, OBJECTS + 0x140]))
    put(session, 0xd4, 'I', OBJECTS + 0x180)
    put(session, 0x3a0, 'I', r.choice([0, OBJECTS]))
    put(session, 0x39c, 'I', r.choice([0, HEAP + 0x100]))
    put(session, 0x3a4, 'I', OBJECTS + 0x1c0)
    for at in (0x320, 0x324, 0x328, 0x32c, 0x37c, 0x380, 0x388, 0x398):
        put(session, at, 'I', r.choice([0, 1, r.getrandbits(32)]))
    put(session, 0x324, 'I', r.choice([0, 1, 1, 1]))
    for at in (0x374, 0x3c0):
        put(session, at, 'I', r.choice([0, recorder(CALLBACK, 2, cdecl=True)]))
    set_return(log, OPEN, r.choice([0, 1]))
    set_return(log, INSTANCE, SESSION)
    block = bytearray(0x100)
    transfer = r.getrandbits(32)
    put(block, 0, '2I', *((0, transfer) if r.random() < 0.3 else (transfer, 0)))
    answers(log, CURRENT, ANSWERS)
    fill(log, PROGRESS, 1, struct.pack('<I', r.choice([0, 1, 3, 50, 100, r.randrange(1 << 25)])), 0)
    fill(log, PROGRESS, 2, struct.pack('<I', r.choice([0, 1, 3, 100, 0xffffffff, r.randrange(1, 1 << 30)])), 1)
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(), SESSION: session, OBJECTS: objects, ANSWERS: block, VT: vtable({0: 1, 5: 1})}
    op = r.randrange(3)
    if op == 0:
        return Call(0x5b90e0, [SESSION, r.choice([0, 0, 1, 2])], regions, conv='t', ret='v')
    if op == 1:
        return Call(0x5b8490, [SESSION], regions, conv='t', ret='v')
    return Call(0x5b79f0, [r.choice([0, 1, 1, 2])], regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)
