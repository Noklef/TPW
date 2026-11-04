# The online session's next download step (src/game/online/session: 0x5b9380) against the original: the next queued file's address and path built, the last file, or the end. The strings real; the
# item lookups and the FTP calls recorders; the conversions' imports recorders answering a text of their own, their first bytes compared.
# usage: onlinenext.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob, fill, global_heap, heap, log_region, put, recorder, set_return, strings  # noqa: E402

SAY, W2M, M2W = 1, 2, 3
RECORD, FIND, FETCH, FINISH = 8, 9, 10, 11
B = Bench([(0x5b9380, 0x5b96a0), (0x424db0, 0x424e6e), (0x5c5480, 0x5c5510), (0x5c5520, 0x5c5530), (0x5f5660, 0x5f5690), (0x5f56f0, 0x5f5710)] + gamestring.B_RANGES,
          hooks={0x5bd670: _recorder(RECORD, 0), 0x5bc9c0: _recorder(FIND, 1), 0x5d22a0: _recorder(FETCH, 2), 0x5d2180: _recorder(FINISH, 0)})
SESSION, RECS, NODE, TEXTS = RAM, RAM + 0x800, RAM + 0xc00, RAM + 0x1000
ITEM, ROOT_TEXT, TAIL_TEXT, SLASH_TEXT, ITEM_TEXT = RAM + 0xd00, TEXTS + 0x100, TEXTS + 0x200, TEXTS + 0x300, TEXTS + 0x400
HOST, THEME, NARROW, LAST = TEXTS, TEXTS + 0x40, TEXTS + 0x80, TEXTS + 0xc0
PATH_GLOBAL, TAIL_GLOBAL = 0xf816c0, 0xf7ccd8


def wide(text):
    return text.encode('utf-16-le') + bytes(2)


def case(r):
    log = log_region()
    table = bytearray(0x600)
    put(table, 0x6fd1e0 - IMPORT_TABLE, 'I', recorder(SAY, 1, True))
    strings(log, SAY, 1)
    put(table, 0x6fd1dc - IMPORT_TABLE, 'I', recorder(W2M, 8, True))
    put(table, 0x6fd174 - IMPORT_TABLE, 'I', recorder(M2W, 6, True))
    for at, stub in global_heap().items():
        table[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    strings(log, W2M, 0, 0b10000)
    blob(log, W2M, 2, 0xf)
    strings(log, M2W, 4, 0b10000)
    narrow = b'conv\0'
    set_return(log, W2M, len(narrow))
    fill(log, W2M, 4, narrow)
    back = 'back\0'.encode('utf-16-le')
    set_return(log, M2W, len(back) // 2)
    fill(log, M2W, 4, back, 1)
    strings(log, FETCH, 3)
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(0x4000)}
    gamestring.string(r, 0, PATH_GLOBAL, ROOT_TEXT, regions, r.choice([b'C:\\tp', b'C:\\tp\\']))
    gamestring.string(r, 0, TAIL_GLOBAL, TAIL_TEXT, regions, b'in')
    gamestring.string(r, 0, 0xf80be0, SLASH_TEXT, regions, b'\\')
    session = bytearray(r.bytes(0x400))
    recs = bytearray(r.bytes(0x400))
    put(recs, 0x74, 'I', HOST)
    put(recs, 0x100 + 0x4c, 'I', THEME)
    put(recs, 0x200 + 0x84, 'I', NARROW)
    set_return(log, RECORD, RECS)
    set_return(log, FIND, RECS + 0x100)
    put(session, 0x3a4, 'I', RECS + 0x200)
    put(session, 0x3e4, 'I', LAST)
    gamestring.string(r, 1, ITEM, ITEM_TEXT, regions, wide('file')[:-2] if r.random() < 0.8 else wide('a')[:-2])
    regions[TEXTS] = wide('ftp.x') + bytes(0x40 - 12)
    regions[THEME] = wide('th') + bytes(0x40 - 6)
    regions[NARROW] = b'name.dat\0' + bytes(0x40 - 9)
    regions[LAST] = b'last\0' + bytes(0x40 - 5)
    node = bytearray(r.bytes(0x20))
    put(node, 0, 'I', ITEM)
    follower = bytearray(r.bytes(0x20))
    has_next = r.random() < 0.5
    put(node, 4, 'I', NODE + 0x20 if has_next else 0)
    regions[NODE] = node
    regions[NODE + 0x20] = follower
    queued = r.choice([0, 1, 1, 2, 0x80000000])
    put(session, 0x3d4, 'I', queued)
    put(session, 0x3cc, 'I', NODE if r.random() < 0.97 else 0)
    put(session, 0x3d8, 'I', r.choice([0, 1, r.getrandbits(32)]))
    regions[SESSION] = session
    regions[RECS] = recs
    return Call(0x5b9380, [SESSION], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
