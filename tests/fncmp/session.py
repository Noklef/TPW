# The online session's getters (src/game/online/session: 0x5b6450-0x5b64c0) against the
# originals: logged in or not, the user's flags with and without 0x20. Its connections' release and reset
# (0x5b64d0, 0x5b6b20, 0x5b6980, 0x5b83e0, 0x5b8400, 0x5b9040) and the Auth object's deleting destructor
# (0x5b7b80): the Auth / News / Upload objects set or not, their methods (WEA*R imports, __thiscall)
# recorders, OutputDebugStringA one logging its text, the members' string assigns (vtable +0x14) and the
# download object's deleting destructor (vtable +0) recorders, the block at +0x39c a heap block or none. The login's answer (0x5b62e0):
# statuses 60 (OK), -7..-1, 0, 59, 61 or any, the record's count 1, 2, 0 or -1. The city's reset (0x5b7430): see reset_city().
# Its opening (0x5b72f0) and a park's unpublishing on it (0x5b7960): see open_city(). All connections' and
# the whole session's resets (0x5b6070, 0x5b5e40): see reset_all(). The constructor (0x5b50b0): see construct().
# usage: session.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=LoggedIn,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CDECL_LOG4, HEAP, IMPORT_TABLE, LOG, Bench, Call, RAM, _recorder, blob_array, heap, log_region, put, recorder, set_return, strings, vtable  # noqa: E402

B = Bench([(0x5b6450, 0x5b64c4), (0x5b64d0, 0x5b650b), (0x5b6b20, 0x5b6b5e), (0x5b6980, 0x5b6a27), (0x5b83e0, 0x5b83ff), (0x5b8400, 0x5b8483),
           (0x5b9040, 0x5b90d7), (0x5b7b80, 0x5b7b9f), (0x5b62e0, 0x5b63f0), (0x67acb0, 0x67acb1),
           (0x5b7430, 0x5b75cc), (0x5f5fa0, 0x5f5faa), (0x5f5f10, 0x5f5f7d), (0x5b9fd0, 0x5ba02a), (0x5ba290, 0x5ba2ea), (0x5b97a0, 0x5b981c),
           (0x5b9820, 0x5b989c), (0x67afa0, 0x67b0d0),
           (0x5b72f0, 0x5b742a), (0x4bae30, 0x4bae3f), (0x41a830, 0x41a834), (0x67acc0, 0x67acc1), (0x5b7960, 0x5b7987),
           (0x4bae40, 0x4bae62), (0x4c25e0, 0x4c25f9), (0x5b5e40, 0x5b606b), (0x5b6070, 0x5b622a), (0x658d9f, 0x658e04), (0x659710, 0x659740), (0x65f697, 0x65f6e1),
           (0x5b50b0, 0x5b55d7), (0x5b96e0, 0x5b97a0), (0x5bce60, 0x5bcee0), (0x5bd2a0, 0x5bd320), (0x5bd6b0, 0x5bd730), (0x5bdcd0, 0x5bde20),
           (0x5f5fc0, 0x5f7f00), (0x67a400, 0x67acb0), (0x67acd0, 0x67afa0), (0x67b0d0, 0x690000), (0x5b7cd0, 0x5b7fa0), (0x5b7fa0, 0x5b8160), (0x5b8260, 0x5b8290)],
          hooks={0x5b8290: _recorder(17, 0), 0x47f410: _recorder(18, 2, cdecl=True), 0x5bd230: _recorder(19, 1), 0x48b390: _recorder(26, 0, cdecl=True), 0x67ad90: _recorder(23, 1, cdecl=True)})
GETTERS = {'LoggedIn': 0x5b6450, 'Flag20': 0x5b6460, 'String14': 0x5b6480, 'String28': 0x5b6490,
           'UserName': 0x5b64a0, 'UserFlags': 0x5b64b0, 'User5c': 0x5b64c0}
RELEASES = {'ReleaseLogin': 0x5b64d0, 'ReleaseNews': 0x5b6b20, 'ResetNews': 0x5b6980, 'ReleaseUpload': 0x5b83e0, 'ResetUpload': 0x5b8400,
            'ResetDownload': 0x5b9040, 'AuthDelete': 0x5b7b80, 'LoginResult': 0x5b62e0, 'ResetCity': 0x5b7430,
            'OpenCity': 0x5b72f0, 'Unpublish': 0x5b7960, 'ResetConnections': 0x5b6070, 'ResetAll': 0x5b5e40, 'Construct': 0x5b50b0,
           'UploadStart': 0x5b7cd0, 'UploadEvent': 0x5b7fa0, 'UploadCallback': 0x5b8260}
CALLS = {**GETTERS, **RELEASES}
# the imports: OutputDebugStringA (stdcall, its text logged), Auth::Release, Auth::~Auth, News::Release, News::~News, Upload::Release (thiscall)
OUTPUT = 0x6fd1e0
METHODS = [0x6fd45c, 0x6fd460, 0x6fd58c, 0x6fd588, 0x6fd5ec]
SESSION, OBJ, VT, REC, LISTS, ARRAYS = RAM, RAM + 0x800, RAM + 0x900, RAM + 0xa00, RAM + 0xc00, RAM + 0x2000
WINDOW = RAM + 0x8000
TEXTS = RAM + 0x4000


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


def open_city(r, unpublish=False):
    """0x5b72f0: a City (+0xd4) already or not; the block had or not (heap() failing its first); City::City, Connect (answering 1 mostly, else 0
    or its hash), SetSessionInfo, Release, ~City, timeGetTime and OutputDebugStringA recorders; the server name [0x7a1524] 0 or set (its high
    half pushed with the port word [0x7a1556]), the pick's flag byte [0x7cc53c] and the language's id (0x78b530 +8) anything."""
    log = log_region()
    table = bytearray(0x600)
    struct.pack_into('<I', table, OUTPUT - IMPORT_TABLE, recorder(1, 1, True))
    strings(log, 1, 1)
    for k, (slot, n) in enumerate([(0x6fd4e4, 0), (0x6fd534, 4), (0x6fd53c, 4), (0x6fd4f4, 0), (0x6fd540, 0), (0x6fd4fc, 3)]):
        struct.pack_into('<I', table, slot - IMPORT_TABLE, recorder(2 + k, n))
    struct.pack_into('<I', table, 0x6fd3ec - IMPORT_TABLE, recorder(8, 0, True))
    set_return(log, 8, r.getrandbits(32))
    if r.random() < 0.5:
        set_return(log, 2, r.choice([OBJ, OBJ + 0x40, 0]))
    answer = r.choice([1, 1, 1, 0, None])
    if answer is not None:
        set_return(log, 3, answer)
    s = bytearray(r.bytes(0x110))
    put(s, 0xd4, 'I', r.choice([0, 0, 0, OBJ]))
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(fail=r.choice([0, 0, 0, 1])), SESSION: s, OBJ: r.bytes(0xa0), 0xfa71e0: bytes(4),
               0x7a1524: put(bytearray(r.bytes(4)), 0, 'I', r.choice([0, 0, OBJ, r.getrandbits(32)])), 0x7a1556: r.bytes(2),
               0x7cc53c: bytes([r.choice([0, 1, 0x80])]), 0x78b530: r.bytes(0x10)}
    if unpublish:   # 0x5b7960 (park): City::UnpublishPark on the connection opened so
        return Call(0x5b7960, [SESSION, r.getrandbits(32)], regions, conv='t', ret='a')
    return Call(0x5b72f0, [SESSION], regions, conv='t', ret='a')


def reset_all(r, op):
    """0x5b6070, 0x5b5e40: reset_city()'s world, the Auth, News and Upload as release()'s, the other strings recorders; the dialog [0x7cca94] none or
    up (byte +4 now and then set, handler +0x114 fncmp's logger or none). 0x5b5e40's end: the flag byte [0x7cc540] and Dialup::Disconnect's
    recorder (its argument [0x7cc50c])."""
    regions = reset_city(r).regions
    table = regions[IMPORT_TABLE]
    for k, slot in enumerate(METHODS):
        struct.pack_into('<I', table, slot - IMPORT_TABLE, recorder(11 + k, 0))
    struct.pack_into('<I', table, 0x6fd5b8 - IMPORT_TABLE, recorder(16, 1))
    s = bytearray(r.bytes(0x400))
    s[:0x110] = regions[SESSION]
    for at in (0xc, 0x20, 0x3c, 0x7c, 0x98, 0xb4, 0x348, 0x35c, 0x3dc):
        put(s, at, 'I', VT)
    for at in (0, 0x6c, 0x104):
        put(s, at, 'I', r.choice([0, OBJ, OBJ + 0x40]))
    put(s, 0x3a0, 'I', r.choice([0, OBJ + 0x80]))
    put(s, 0x39c, 'I', r.choice([0, HEAP + 0x100, HEAP + 0x200]))
    window = bytearray(r.bytes(0x120))
    window[4] = r.choice([0, 0, 0, 1])
    put(window, 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
    regions.update({SESSION: s, VT: vtable({0: 1, 5: 1}), OBJ: r.bytes(0x80) + struct.pack('<I', VT) + r.bytes(0x1c), WINDOW: window,
                    0x7cca94: struct.pack('<I', r.choice([0, WINDOW])), 0xfaa5fc: bytes(4), 0x7cc540: bytes([r.deal([0, 1, 0x80])]), 0x7cc50c: r.bytes(4)})
    return Call(CALLS[op], [SESSION], regions, conv='t', ret='v')


def upload_start(r):
    """0x5b7cd0: reset_all()'s world for its failures. Flag 0x10 of +0x58 or not, the Upload (+0x104) there or Online_UploadConnect (a recorder) answering; the request (0x6fd5d0, a recorder) with its
    stack record hashed whole, 0x42e bytes (five at a time, the first argument's number), when the two names fill their fields (19 and 255 wide characters), else not: the rest would be stack garbage."""
    regions = reset_all(r, 'ResetAll').regions
    log, s = regions[LOG], regions[SESSION]
    put(regions[IMPORT_TABLE], 0x6fd5d0 - IMPORT_TABLE, 'I', recorder(21, 3))
    blob_array(log, 21, 1, 0, 5)
    set_return(log, 17, r.choice([0, 1, 2]))
    set_return(log, 19, r.getrandbits(32))
    put(s, 0x58, 'I', r.getrandbits(32) | 0x10 if r.random() < 0.7 else r.getrandbits(32) & ~0x10)
    put(s, 0x320, 'I', r.choice([0, 1, r.getrandbits(32)]))
    put(s, 0x374, 'I', r.choice([0, recorder(20, 2, cdecl=True)]))
    full = r.random() < 0.7
    put(s, 0x338, 'I', 214 if full else 0)
    first, second = TEXTS, TEXTS + 0x100
    short = r.choice([0, 1, 7])
    put(s, 0x350, 'I', first)
    put(s, 0x364, 'I', second)
    regions[first] = b''.join(struct.pack('<H', r.randrange(1, 0x10000)) for _ in range(19 if full else short)) + bytes(2)
    regions[second] = b''.join(struct.pack('<H', r.randrange(1, 0x10000)) for _ in range(255 if full else short)) + bytes(2)
    return Call(0x5b7cd0, [SESSION], regions, conv='t', ret='v')


def upload_event(r, op):
    """0x5b7fa0 (and 0x5b8260, Online_Instance a recorder answering the session): the codes of its switch, their neighbours and any; for 0x50 a record of uses 1 to 3, its block (+0) a heap block or none."""
    log = log_region()
    table = bytearray(0x600)
    struct.pack_into('<I', table, OUTPUT - IMPORT_TABLE, recorder(1, 1, True))
    strings(log, 1, 1)
    struct.pack_into('<I', table, 0x6fd5e8 - IMPORT_TABLE, recorder(22, 2))
    set_return(log, 26, SESSION)
    code = r.choice([-40, -20, -6, -5, -4, -2, -1, 0, 0x50, 0x50, 0x50, -41, -3, 1, 0x51, r.getrandbits(32)])
    record = bytearray(r.bytes(0x10))
    put(record, 0, 'I', r.choice([0, HEAP + 0x100]))
    put(record, 8, 'I', r.choice([1, 1, 2, 3]))
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(), SESSION: r.bytes(0x400), REC: record, 0x78ba94: r.bytes(4)}
    args = [code & 0xffffffff, r.getrandbits(32), r.getrandbits(32), REC]
    if op == 'UploadCallback':
        return Call(0x5b8260, args, regions, conv='s', ret='v')
    return Call(0x5b7fa0, [SESSION] + args, regions, conv='t', ret='v')


def construct(r):
    """0x5b50b0: the session's memory random, the heap failing no block, or one of the first 15 (12 strings', then the first lists: a list not had
    faults in the city's reset; fncmp's heap fails none later, so the download object's block, +0x3a4, is always had); timeGetTime and OutputDebugStringA recorders; the strings real."""
    log = log_region()
    table = bytearray(0x600)
    struct.pack_into('<I', table, OUTPUT - IMPORT_TABLE, recorder(1, 1, True))
    strings(log, 1, 1)
    struct.pack_into('<I', table, 0x6fd3ec - IMPORT_TABLE, recorder(6, 0, True))
    set_return(log, 6, r.getrandbits(32))
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(0x4000, r.deal([0] * 10 + [1, 6, 12, 13, 14, 15])), SESSION: r.bytes(0x404), 0xfa71e0: bytes(4),
               0x78a374: bytes(4), 0x786de4: bytes(4)}
    return Call(0x5b50b0, [SESSION], regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS)
    if op == 'Construct':
        return construct(r)
    if op == 'UploadStart':
        return upload_start(r)
    if op in ('UploadEvent', 'UploadCallback'):
        return upload_event(r, op)
    if op in ('ResetConnections', 'ResetAll'):
        return reset_all(r, op)
    if op == 'ResetCity':
        return reset_city(r)
    if op in ('OpenCity', 'Unpublish'):
        return open_city(r, op == 'Unpublish')
    if op in RELEASES:
        return release(r, op)
    s = bytearray(r.bytes(0x80))
    put(s, 0x60, 'I', r.choice([0, 1, 0x100, r.getrandbits(32)]))
    put(s, 0x58, 'I', r.choice([0, 0x20, 0x1f, 0x2000, r.getrandbits(32)]))
    return Call(GETTERS[op], [RAM], {RAM: s}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
