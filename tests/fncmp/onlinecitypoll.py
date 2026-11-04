# The city connection's poll (src/game/online/session: 0x5b75d0) against the original. All its calls recorders: the lists' starts and answers (each with a bench of its own), Time_GetMs, the dialogs'
# text and progress; the City's methods (WEA*R imports, thiscall), OutputDebugStringA. The nine lists busy or not (+0xc, +0x10), the first requested or not (+0xdc), the transfer's id any list's or none,
# its progress any (a total of 0 or less says nothing); no list busy: the time out passed or not, the City freed.
# usage: onlinecitypoll.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, fill, heap, log_region, put, recorder, set_return, strings  # noqa: E402

SAY, CURRENT, PROGRESS, CLOSE, DTOR = 1, 2, 3, 4, 5
START10, START26, START34, START66, DONE10, DONE26, DONE34, DONE66, DONE30, NOW, SHOW, MESSAGE, SET, WAITING = range(8, 22)
B = Bench([(0x5b75d0, 0x5b7910), (0x67acb0, 0x67acb1)],
          hooks={0x5bd070: _recorder(START10, 0), 0x5bd4b0: _recorder(START26, 0), 0x5bd8c0: _recorder(START34, 0), 0x5bcd10: _recorder(START66, 3), 0x5ba030: _recorder(DONE10, 1),
                 0x5ba2f0: _recorder(DONE26, 1), 0x5ba560: _recorder(DONE34, 1), 0x5b9d60: _recorder(DONE66, 1), 0x5b9b00: _recorder(DONE30, 1), 0x5f5fa0: _recorder(NOW, 0, cdecl=True),
                 0x4c25b0: _recorder(SHOW, 1, cdecl=True), 0x4c25e0: _recorder(MESSAGE, 0, cdecl=True), 0x4c2600: _recorder(SET, 1, cdecl=True), 0x4c0660: _recorder(WAITING, 1, cdecl=True)})
SESSION, CITY, LISTS, ANSWERS = RAM, RAM + 0x800, RAM + 0x1000, RAM + 0x2000
OFFSETS = [0xe0, 0xe4, 0xe8, 0xec, 0xf0, 0xf4, 0xf8, 0xfc, 0x100]


def case(r):
    log = log_region()
    table = bytearray(0x600)
    put(table, 0x6fd1e0 - IMPORT_TABLE, 'I', recorder(SAY, 1, True))
    strings(log, SAY, 1)
    put(table, 0x6fd4f8 - IMPORT_TABLE, 'I', recorder(CURRENT, 1))
    put(table, 0x6fd4e0 - IMPORT_TABLE, 'I', recorder(PROGRESS, 3))
    strings(log, PROGRESS, 0, 0b110)
    put(table, 0x6fd4f4 - IMPORT_TABLE, 'I', recorder(CLOSE, 0))
    put(table, 0x6fd540 - IMPORT_TABLE, 'I', recorder(DTOR, 0))
    session = bytearray(r.bytes(0x400))
    put(session, 0xd4, 'I', CITY if r.random() < 0.95 else 0)
    put(session, 0xdc, 'I', r.choice([0, 0, 1]))
    lists = bytearray(r.bytes(0x20 * 9))
    idle = r.random() < 0.3
    ids = [0x100 + i for i in range(9)]
    for i, offset in enumerate(OFFSETS):
        at = LISTS + 0x20 * i
        put(session, offset, 'I', at)
        put(lists, 0x20 * i + 4, 'I', ids[i])
        busy = 0 if idle else r.choice([0, 0, 1, 2, 0x80000000])
        put(lists, 0x20 * i + 0xc, 'I', busy)
        put(lists, 0x20 * i + 0x10, 'I', r.choice([0, 0, 1]))
    # (a list both flagged +0xc and +0x10 is answered and cleared: nothing busy then; none left and the time out)
    now = r.getrandbits(32)
    set_return(log, NOW, now)
    put(session, 0xd8, 'I', (now - r.choice([0, 1, 0x57e40, 0x57e41, 0x7fffffff, 0x80000000])) & 0xffffffff)
    current = r.choice(ids + [0, 0x55])
    first, second = (0, current) if r.random() < 0.3 else (current, 0)
    block = bytearray(0x100)
    put(block, 0, '2I', first, second)
    answers(log, CURRENT, ANSWERS)
    fill(log, PROGRESS, 1, struct.pack('<I', r.choice([0, 1, 3, 50, 100, r.randrange(1 << 25)])), 0)
    fill(log, PROGRESS, 2, struct.pack('<I', r.choice([0, 1, 3, 100, 0xffffffff, r.randrange(1, 1 << 30)])), 1)
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(), SESSION: session, CITY: r.bytes(0x40), LISTS: lists, ANSWERS: block}
    return Call(0x5b75d0, [SESSION], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
