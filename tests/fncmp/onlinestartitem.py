# The online session started on an item, the start's callbacks and the download's FTP event (src/game/online/session: 0x5b86a0, 0x5b8680, 0x5b87a0, 0x5b88e0, 0x5b92d0) against the originals: the session's own calls (releases and resets, the dialog's message,
# Online_Instance, Class70293c_Assign, Online_DownloadStart, Online_DownloadEnd, Online_DownloadNext, ListRequest_Start66, Dialog7cc530_Open) recorders, each with a bench of its own; the three strings' slot 5 and OutputDebugStringA too.
# usage: onlinestartitem.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=item,itemlogin,startlogin,answered,event)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, log_region, put, recorder, set_return, strings, vtable  # noqa: E402

ASSIGN_SLOT, SAY, ASSIGN, START, END, NEXT, RELEASE, CITY, UPLOAD, DOWNLOAD, MESSAGE, INSTANCE, LIST, OPEN = 5, 9, 10, 11, 12, 13, 21, 22, 23, 24, 25, 26, 27, 28
B = Bench([(0x5b86a0, 0x5b8799), (0x5b8680, 0x5b869f), (0x5b87a0, 0x5b87d1), (0x5b88e0, 0x5b890e), (0x5b92d0, 0x5b9380), (0x5b96c0, 0x5b96d9)],
          hooks={0x5b6b20: _recorder(RELEASE, 0), 0x5b7430: _recorder(CITY, 0), 0x5b8400: _recorder(UPLOAD, 0), 0x5b9040: _recorder(DOWNLOAD, 0), 0x4c25e0: _recorder(MESSAGE, 0, cdecl=True), 0x48b390: _recorder(INSTANCE, 0, cdecl=True),
                 0x5bcd10: _recorder(LIST, 3), 0x4ba980: _recorder(OPEN, 1, cdecl=True), 0x5bdfd0: _recorder(ASSIGN, 1), 0x5b8910: _recorder(START, 1), 0x5b90e0: _recorder(END, 1), 0x5b9380: _recorder(NEXT, 0)})
OPS = [n for n in ('item', 'itemlogin', 'startlogin', 'answered', 'event', 'callback') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
SESSION, THE_INSTANCE, VT, ANSWER, CELL = RAM, RAM + 0x1000, RAM + 0x2000, RAM + 0x2100, RAM + 0x2200


def case(r):
    name = r.deal(OPS)
    log = log_region()
    set_return(log, INSTANCE, THE_INSTANCE)
    session = bytearray(r.bytes(0x3f0))
    for slot in (0x7c, 0x98, 0xb4):
        put(session, slot, 'I', VT)
    put(session, 0x60, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    regions = {LOG: log, SESSION: session, THE_INSTANCE: r.bytes(0x3f0), VT: vtable({ASSIGN_SLOT: 1})}
    if name == 'item':
        return Call(0x5b86a0, [SESSION, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='v')
    if name == 'itemlogin':
        return Call(0x5b8680, [r.choice([0, 0, 1, r.getrandbits(32)])], regions, ret='v')
    if name == 'startlogin':
        return Call(0x5b87a0, [r.choice([0, 0, 1, r.getrandbits(32)])], regions, ret='v')
    if name == 'answered':
        regions[ANSWER] = struct.pack('<2I', r.getrandbits(32), CELL)
        regions[CELL] = struct.pack('<I', r.choice([0, r.getrandbits(32), r.getrandbits(32)]))
        return Call(0x5b88e0, [r.choice([0, 0, 1, r.getrandbits(32)]), r.getrandbits(32), ANSWER], regions, ret='v')
    table = bytearray(0x400)
    put(table, 0x6fd1e0 - IMPORT_TABLE, 'I', recorder(SAY, 1, True))
    strings(log, SAY, 1)
    regions[IMPORT_TABLE] = table
    if name == 'callback':
        instance = bytearray(regions[THE_INSTANCE])
        put(instance, 0x3a0, 'I', r.choice([0, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)]))
        regions[THE_INSTANCE] = instance
        return Call(0x5b96c0, [r.choice([0, 1, 1, 2, 2, 3, 3, 4, 0xffffffff, r.getrandbits(32)]), r.choice([0, 0, 1, r.getrandbits(32)])], regions, ret='v')
    put(session, 0x3a0, 'I', r.choice([0, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)]))
    return Call(0x5b92d0, [SESSION, r.choice([0, 1, 1, 2, 2, 3, 3, 4, 0xffffffff, r.getrandbits(32)]), r.choice([0, 0, 1, r.getrandbits(32)])], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
