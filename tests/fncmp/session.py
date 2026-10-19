# The online session's getters (src/game/online/session: 0x5b6450-0x5b64c0) against the
# originals: logged in or not, the user's flags with and without 0x20. Its connections' release and reset
# (0x5b64d0, 0x5b6b20, 0x5b6980, 0x5b83e0, 0x5b8400, 0x5b9040) and the Auth object's deleting destructor
# (0x5b7b80): the Auth / News / Upload objects set or not, their methods (WEA*R imports, __thiscall)
# recorders, OutputDebugStringA one logging its text, the members' string assigns (vtable +0x14) and the
# download object's deleting destructor (vtable +0) recorders, the block at +0x39c a heap block or none. The login's answer (0x5b62e0):
# statuses 60 (OK), -7..-1, 0, 59, 61 or any, the record's count 1, 2, 0 or -1. The city's reset (0x5b7430): see reset_city().
# usage: session.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=LoggedIn,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, Bench, Call, RAM, heap, log_region, put, recorder, set_return, strings, vtable  # noqa: E402

B = Bench([(0x5b6450, 0x5b64c4), (0x5b64d0, 0x5b650b), (0x5b6b20, 0x5b6b5e), (0x5b6980, 0x5b6a27), (0x5b83e0, 0x5b83ff), (0x5b8400, 0x5b8483),
           (0x5b9040, 0x5b90d7), (0x5b7b80, 0x5b7b9f), (0x5b62e0, 0x5b63f0), (0x67acb0, 0x67acb1),
           (0x5b7430, 0x5b75cc), (0x5f5fa0, 0x5f5faa), (0x5f5f10, 0x5f5f7d), (0x5b9fd0, 0x5ba02a), (0x5ba290, 0x5ba2ea), (0x5b97a0, 0x5b981c),
           (0x5b9820, 0x5b989c), (0x67afa0, 0x67b0d0)])
GETTERS = {'LoggedIn': 0x5b6450, 'Flag20': 0x5b6460, 'String14': 0x5b6480, 'String28': 0x5b6490,
           'UserName': 0x5b64a0, 'UserFlags': 0x5b64b0, 'User5c': 0x5b64c0}
RELEASES = {'ReleaseLogin': 0x5b64d0, 'ReleaseNews': 0x5b6b20, 'ResetNews': 0x5b6980, 'ReleaseUpload': 0x5b83e0, 'ResetUpload': 0x5b8400,
            'ResetDownload': 0x5b9040, 'AuthDelete': 0x5b7b80, 'LoginResult': 0x5b62e0, 'ResetCity': 0x5b7430}
CALLS = {**GETTERS, **RELEASES}
# the imports: OutputDebugStringA (stdcall, its text logged), Auth::Release, Auth::~Auth, News::Release, News::~News, Upload::Release (thiscall)
OUTPUT = 0x6fd1e0
METHODS = [0x6fd45c, 0x6fd460, 0x6fd58c, 0x6fd588, 0x6fd5ec]
SESSION, OBJ, VT, REC, LISTS, ARRAYS = RAM, RAM + 0x800, RAM + 0x900, RAM + 0xa00, RAM + 0xc00, RAM + 0x2000


def release(r, op):
    log = log_region()
    table = bytearray(0x600)
    struct.pack_into('<I', table, OUTPUT - IMPORT_TABLE, recorder(1, 1, True))
    strings(log, 1, 1)
    for k, slot in enumerate(METHODS):
        struct.pack_into('<I', table, slot - IMPORT_TABLE, recorder(2 + k, 0))
    s = bytearray(r.bytes(0x400))
    for at in (0x7c, 0x98, 0xb4, 0x348, 0x35c, 0x3dc):
        put(s, at, 'I', VT)
    for at in (0, 0x6c, 0x104):
        put(s, at, 'I', r.choice([0, OBJ, OBJ, OBJ + 0x40]))
    put(s, 0x3a0, 'I', r.choice([0, OBJ + 0x80]))
    put(s, 0x39c, 'I', r.choice([0, HEAP + 0x100, HEAP + 0x200]))
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(), SESSION: s, OBJ: r.bytes(0x80) + struct.pack('<I', VT) + r.bytes(0x1c),
               VT: vtable({0: 1, 5: 1})}
    if op == 'LoginResult':
        put(s, 0x3c, 'I', VT)
        rec = bytearray(r.bytes(0x70))
        put(rec, 0x68, 'I', r.choice([1, 1, 1, 2, 0, 0xffffffff]))
        regions[REC] = rec
        status = r.choice([60, 60, 60, -6, -5, -4, -3, -2, -1, -7, 0, 59, 61, r.getrandbits(32)])
        return Call(0x5b62e0, [SESSION, status & 0xffffffff, r.getrandbits(32), r.getrandbits(32), REC], regions, conv='t', ret='v')
    if op == 'AuthDelete':
        return Call(0x5b7b80, [r.choice([OBJ, OBJ + 0x40]), r.choice([0, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')
    return Call(RELEASES[op], [SESSION], regions, conv='t', ret='v')


def reset_city(r):
    """0x5b7430: the City (+0xd4) set or not; its methods, the lists' native releases (ccCityList / ccParkList::Release), timeGetTime (the game's
    Timer at 0xfa71e0 off) and OutputDebugStringA recorders; the nine lists apart, their +0x30 none, a native list, or (for the shared ones) a
    record of 2 or more uses, or of 1 (freed) with no array or one of 0-3 elements (each destroyed through the record kind's import slot)."""
    log = log_region()
    table = bytearray(0x600)
    struct.pack_into('<I', table, OUTPUT - IMPORT_TABLE, recorder(1, 1, True))
    strings(log, 1, 1)
    for k, slot in enumerate([0x6fd4f4, 0x6fd540, 0x6fd538, 0x6fd4e8]):
        struct.pack_into('<I', table, slot - IMPORT_TABLE, recorder(2 + k, 0))
    struct.pack_into('<I', table, 0x6fd3ec - IMPORT_TABLE, recorder(6, 0, True))
    for k, slot in enumerate([0x6fd504, 0x6fd50c, 0x6fd508, 0x6fd510]):   # the shared records' element destructors (thiscall)
        struct.pack_into('<I', table, slot - IMPORT_TABLE, recorder(7 + k, 0))
    set_return(log, 6, r.getrandbits(32))
    s = bytearray(r.bytes(0x110))
    put(s, 0xd4, 'I', r.choice([0, OBJ, OBJ, OBJ + 0x40]))
    lists = bytearray(r.bytes(0x40 * 9))
    recs = bytearray(r.bytes(0x10 * 9))
    arrays = bytearray(r.bytes(0x200 * 9))
    for i in range(9):
        put(s, 0xe0 + 4 * i, 'I', LISTS + 0x40 * i)
        if i < 4:
            native = r.choice([0, 0, REC + 0x10 * i])
        else:
            native = r.choice([0, REC + 0x10 * i, REC + 0x10 * i])
            put(recs, 0x10 * i + 0xc, 'I', r.choice([2, 3, 0x10, 1]))
            if struct.unpack_from('<I', recs, 0x10 * i + 0xc)[0] == 1:   # its array (a count, then the elements) destroyed and freed, or none
                put(recs, 0x10 * i + 4, 'I', r.choice([0, ARRAYS + 0x200 * i + 4]))
                put(arrays, 0x200 * i, 'I', r.choice([0, 1, 2, 3]))
        put(lists, 0x40 * i + 0x30, 'I', native)
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(), SESSION: s, OBJ: r.bytes(0xa0), LISTS: lists, REC: recs, ARRAYS: arrays, 0xfa71e0: bytes(4)}
    return Call(0x5b7430, [SESSION], regions, conv='t', ret='v')


def case(r):
    op = r.deal(OPS)
    if op == 'ResetCity':
        return reset_city(r)
    if op in RELEASES:
        return release(r, op)
    s = bytearray(r.bytes(0x80))
    put(s, 0x60, 'I', r.choice([0, 1, 0x100, r.getrandbits(32)]))
    put(s, 0x58, 'I', r.choice([0, 0x20, 0x1f, 0x2000, r.getrandbits(32)]))
    return Call(GETTERS[op], [RAM], {RAM: s}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
