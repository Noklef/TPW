# The online session's login and news connection (src/game/online/sessionlogin: 0x5b5cb0-0x5b5d10, 0x5b6510-0x5b6b60) against the originals. The Auth
# and News imports recorders (connect answering 1 mostly), the dialogs, the balance file, the command queue and CRT_free recorders by hook and env,
# OutputDebugStringA logging, the members' string assigns recorders. Finish (0x5b6b60): the real strings and FileStream, fileopen.py's imports.
# usage: sessionlogin.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Closed,OpenDialog,Answer,LogIn,Finish,NewsAnswer,OpenNews,Result,Poll,NewsFinish)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import CDECL_LOG2, HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, fill, heap, import_table, log_region, put, recorder, set_return, strings, vtable  # noqa: E402
import fileopen  # noqa: E402

DIALOG, FINISH, LISTS, SHOW, MESSAGE4, PROGRESS, LOAD, APPLY, NEWS_DIALOG, POST, FREE = range(16, 27)
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench(fileopen.RANGES + [(0x41a830, 0x41a834), (0x48b390, 0x48b440), (0x4bae30, 0x4bae3f), (0x5b5cb0, 0x5b5e40), (0x5b62e0, 0x5b63f0), (0x5b64d0, 0x5b650b), (0x5b6510, 0x5b6da0)],
          hooks={0x4ba980: C(DIALOG, 1), 0x4bb390: C(FINISH, 1), 0x4bc710: C(LISTS, 0), 0x4c25b0: C(SHOW, 1), 0x4c25e0: C(MESSAGE4, 0), 0x4c2600: C(PROGRESS, 1),
                 0x401650: _recorder(LOAD, 2), 0x45aee0: C(APPLY, 0), 0x4bb800: C(NEWS_DIALOG, 0), 0x47f410: C(POST, 2), 0x67ad90: C(FREE, 1)})
OUTPUT = 0x6fd1e0
SLOTS = {0x6fd45c: (2, 0), 0x6fd460: (3, 0), 0x6fd46c: (4, 0), 0x6fd464: (5, 4), 0x6fd468: (6, 4), 0x6fd58c: (7, 0), 0x6fd588: (8, 0), 0x6fd5a0: (9, 0), 0x6fd59c: (10, 4),
         0x6fd598: (11, 3), 0x6fd594: (12, 3), 0x6fd590: (13, 2), 0x6fd584: (14, 3), 0x6fd5a4: (15, 1)}
AUTH_NEW, AUTH_CONNECT, NEWS_NEW, NEWS_CONNECT, GET_PROGRESS, GET_ACTIVE = 4, 5, 9, 10, 14, 15
SESSION, OBJ, VT, REC, TEXT = RAM + 0x1000, RAM + 0x1800, RAM + 0x1900, RAM + 0x1a00, RAM + 0x1a80
OPS = [k for k in os.environ.get('OPS', 'Closed,OpenDialog,Answer,LogIn,Finish,NewsAnswer,OpenNews,Result,Poll,NewsFinish').split(',') if k]
CALLS = {'Closed': 0x5b5cb0, 'OpenDialog': 0x5b5cc0, 'Answer': 0x5b5ce0, 'LogIn': 0x5b5d10, 'Finish': 0x5b6510, 'NewsAnswer': 0x5b6630, 'OpenNews': 0x5b6660,
         'Result': 0x5b67d0, 'Poll': 0x5b6a30, 'NewsFinish': 0x5b6b60}


def world(r, op):
    log = log_region()
    fileworld = op in ('Finish', 'OpenNews', 'Poll', 'NewsFinish')
    table = bytearray(import_table(log, fileopen.IMPORTS) if fileworld else b'').ljust(0x600, b'\0')
    struct.pack_into('<I', table, OUTPUT - IMPORT_TABLE, recorder(1, 1, True))
    strings(log, 1, 1)
    for slot, (k, n) in SLOTS.items():
        struct.pack_into('<I', table, slot - IMPORT_TABLE, recorder(k, n))
    strings(log, GET_PROGRESS, 0, 6)
    set_return(log, AUTH_NEW, r.choice([OBJ, OBJ + 0x40]))
    set_return(log, NEWS_NEW, r.choice([OBJ, OBJ + 0x40]))
    set_return(log, AUTH_CONNECT, r.choice([1, 1, 1, 0, 2]))
    set_return(log, NEWS_CONNECT, r.choice([1, 1, 1, 0, 2]))
    set_return(log, LISTS, r.deal([0, 0, 1, 2]))
    set_return(log, LOAD, r.choice([1, 1, 0, 2]))
    s = bytearray(r.bytes(0x404))
    for at in (0xc, 0x20, 0x3c, 0x7c, 0x98, 0xb4):
        put(s, at, 'I', VT)
    for at in (0, 0x6c):
        put(s, at, 'I', r.choice([0, OBJ, OBJ + 0x40]))
    put(s, 4, 'I', r.choice([0, CDECL_LOG2]))
    put(s, 0x60, 'I', r.choice([0, 1, r.getrandbits(32)]))
    put(s, 0x68, 'I', r.choice([0, 0, 0x4e, r.getrandbits(32)]))
    put(s, 0x70, 'I', r.choice([0, 1]))
    put(s, 0xc8, 'I', r.choice([0, CDECL_LOG2]))
    put(s, 0xd0, 'I', r.choice([0, 0, 0x4f, r.getrandbits(32)]))
    for at in (0x78, 0x94, 0xb0):
        put(s, at, 'I', r.choice([0, 1, 1, 1]))
    rec = bytearray(r.bytes(0x70))
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(0x10000, r.deal([0, 0, 0, 1])), SESSION: s, OBJ: r.bytes(0x80), VT: vtable({0: 1, 5: 1, 10: 1}), REC: rec,
               TEXT: struct.pack('<H', r.choice([0xfffe, 0xfeff, 0x41, 0])) + r.bytes(0x10), 0xf7d88c: struct.pack('<I', SESSION),
               0x7a1514: put(bytearray(r.bytes(4)), 0, 'I', r.choice([0, 0, OBJ, r.getrandbits(32)])), 0x7a1554: r.bytes(6),
               0x7a1534: put(bytearray(r.bytes(4)), 0, 'I', r.choice([0, 0, OBJ, r.getrandbits(32)])),
               0x7cc53c: bytes([r.choice([0, 1, 0x80])]), 0x78b530: r.bytes(0x10), 0x78a374: bytes(4)}
    return regions, s, rec, log


def result_args(r, s, rec):
    """A status of the news or the login, its request one of the session's, the record's text and count."""
    a = r.choice([0x111, 0x222, 0x333, r.getrandbits(32)])
    put(s, 0x90, 'I', r.choice([0x111, 0x444]))
    put(s, 0xac, 'I', r.choice([0x222, 0x444]))
    put(s, 0x74, 'I', r.choice([0, 0x333, r.getrandbits(32)]))
    put(rec, 0, 'I', r.choice([TEXT, TEXT, TEXT]))
    put(rec, 8, 'I', r.choice([1, 1, 2, 0, 0xffffffff]))
    status = r.choice([50, 50, 50, 51, 51, -6, -5, -4, -3, -2, -1, -7, 0, 49, 52, 44, 45, r.getrandbits(32)])
    return [status & 0xffffffff, a, r.getrandbits(32), REC]


def file_world(r, regions, log):
    """NewsFinish's path: the root and two texts; fileopen.py's imports with the file opening or not and the write's answer."""
    for imp, k in fileopen.K.items():
        if imp != 'CreateFileA' and r.random() < 0.9:
            set_return(log, k, r.choice({'GetFileAttributesA': [0x10, 0x10, 0x10, 0x20], 'WriteFile': [1, 1, 0], 'IsDBCSLeadByte': [0, 0, 1], 'InterlockedDecrement': [0, 1, 1, 2],
                                         'InterlockedIncrement': [1, 2], 'CreateSemaphoreA': [0, 0x44], 'WaitForSingleObject': [0, 0, 0x102], 'GetCommandLineA': [0],
                                         'GetFileSize': [0, 0x58]}.get(imp, [0, 1, r.getrandbits(32)])))
    set_return(log, fileopen.K['CreateFileA'], r.choice([0x77, 0x77, 0xffffffff]))
    base = b'C:\\tp\\'
    globals_ = bytearray(r.bytes(0x318))
    globals_[0:8] = b'tp.exe\0\0'
    globals_[0x108:0x109 + len(base)] = base + b'\0'
    root = b'C:\\data\\\0'
    regions.update({fileopen.GLOBALS: globals_, fileopen.PATHS: r.bytes(0x210), fileopen.LOCALE: bytes(4),
                    0xf816e8: struct.pack('<IIIII', 0x703090, len(root) - 1, RAM + 0x1f00, len(root), 0), RAM + 0x1f00: root,
                    0xf7cd80: struct.pack('<IIIII', 0x703090, 4, RAM + 0x1f10, 5, 0), RAM + 0x1f10: b'bal\\\0', 0xf7c9f0: struct.pack('<IIIII', 0x703090, 3, RAM + 0x1f20, 4, 0), RAM + 0x1f20: b'on\0\0'})


def case(r):
    op = r.deal(OPS)
    regions, s, rec, log = world(r, op)
    address = CALLS[op]
    if op in ('Finish', 'OpenNews', 'Poll', 'NewsFinish'):
        file_world(r, regions, log)
        put(s, 0x80, 'I', 8)
        put(s, 0x84, 'I', TEXT)
    if op == 'Closed':
        return Call(address, [], regions, ret='v')
    if op == 'OpenDialog':
        return Call(address, [SESSION, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='v')
    if op == 'Answer':
        put(rec, 0x68, 'I', r.choice([1, 2, 0]))
        put(s, 0x3c, 'I', VT)
        return Call(address, [r.choice([60, 60, -6, -5, -4, -2, -1, 0, r.getrandbits(32)]), r.getrandbits(32), r.getrandbits(32), REC], regions, conv='s', ret='v')
    if op == 'LogIn':
        return Call(address, [SESSION, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='v')
    if op == 'Finish':
        return Call(address, [SESSION, r.choice([0, 0, 0, 1, 2, r.getrandbits(32)])], regions, conv='t', ret='v')
    if op == 'NewsAnswer':
        return Call(address, result_args(r, s, rec), regions, conv='s', ret='v')
    if op == 'Result':
        return Call(address, [SESSION] + result_args(r, s, rec), regions, conv='t', ret='v')
    if op == 'Poll':
        for at, v in ((0x74, 0x111), (0x90, 0x222), (0xac, 0x333)):
            put(s, at, 'I', v)
        set_return(log, GET_ACTIVE, r.choice([0, 0x111, 0x222, 0x333, 0x444]))
        fill(log, GET_PROGRESS, 1, struct.pack('<i', r.choice([0, 1, 50, 99, -3, r.getrandbits(31)])))
        fill(log, GET_PROGRESS, 2, struct.pack('<i', r.choice([0, 0, 1, 100, 7, -1, r.getrandbits(31)])), which=1)
    return Call(address, [SESSION], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
