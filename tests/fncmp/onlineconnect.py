# The lobby connection (src/game/online/session: 0x5b8290) against the original: the imports (OutputDebugStringA, WEA*R's release, constructor, connect and language calls) and WideCharToMultiByte
# recorders, Owner2c_FindKind100 a recorder answering a record with a wide host name and a port. Connected or not; a connection there before or not; the block there or not.
# usage: onlineconnect.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, fill, global_heap, heap, log_region, put, recorder, set_return, strings  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402

SAY, RELEASE, MAKE, CONNECT, LOBBY, W2M, FIND = 1, 2, 3, 4, 6, 7, 29
B = Bench([(0x5b8290, 0x5b83e0), (0x4bae30, 0x4bae40), (0x41a830, 0x41a840), (0x424dc0, 0x424e10), (0x5f5660, 0x5f5690), (0x5f56f0, 0x5f5710)] + gamestring.B_RANGES, hooks={0x5bd690: _recorder(FIND, 0)})
SESSION, OLD, NEW, RECORD, TEXT = RAM, RAM + 0x800, RAM + 0x900, RAM + 0xa00, RAM + 0xb00


def case(r):
    log = log_region()
    table = bytearray(0x600)
    for at, stub in global_heap().items():
        table[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    put(table, 0x6fd1e0 - IMPORT_TABLE, 'I', recorder(SAY, 1, True))
    strings(log, SAY, 1)
    put(table, 0x6fd1dc - IMPORT_TABLE, 'I', recorder(W2M, 8, True))
    strings(log, W2M, 0, 0b10100)
    host = r.choice([b'h\0', b'host.example\0', b'a.b\0'])
    set_return(log, W2M, len(host) if r.random() < 0.9 else 0)
    fill(log, W2M, 4, host, 1)
    put(table, 0x6fd5ec - IMPORT_TABLE, 'I', recorder(RELEASE, 0))
    put(table, 0x6fd5dc - IMPORT_TABLE, 'I', recorder(MAKE, 0))
    set_return(log, MAKE, NEW)
    put(table, 0x6fd5e0 - IMPORT_TABLE, 'I', recorder(CONNECT, 4))
    strings(log, CONNECT, 1)
    set_return(log, CONNECT, r.choice([1, 1, 1, 0, 2, 0xffffffff]))
    put(table, 0x6fd5e4 - IMPORT_TABLE, 'I', recorder(LOBBY, 4))
    set_return(log, FIND, RECORD)
    session = bytearray(r.bytes(0x400))
    put(session, 0x104, 'I', r.choice([0, 0, OLD]))
    record = bytearray(r.bytes(0x40))
    put(record, 0x2c, 'I', TEXT)
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(fail=r.choice([0, 0, 0, 0, 1])), SESSION: session, OLD: r.bytes(0x10), NEW: r.bytes(0x10), RECORD: record,
               TEXT: 'host'.encode('utf-16-le') + bytes(2), 0x0078b530: r.bytes(0x10), 0x007cc53c: bytes([r.choice([0, 1])])}
    return Call(0x5b8290, [SESSION], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
