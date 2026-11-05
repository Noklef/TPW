# The online mail's client (src/game/online/mailbox) against the original: built (0x5c93c0), connected (0x5c9f10), an attachment read and encoded (0x5c9720). Recorders: the file calls, the base64, the imports (the winsock start, the mail library's
# constructor and connect) and the client's callback; the strings real. The allocator bytes the constructors copy are stack garbage in the original, so those bytes start at 0 here, as the emulator's stack does.
# usage: mailclient.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,Connect,Attachment)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob, fill, global_heap, heap, log_fold, log_region, put, recorder, set_return, strings  # noqa: E402

FOPEN, FSEEK, FTELL, FREAD, ENCODE = 15, 16, 17, 18, 20
FCLOSE = 11
WSA, LIBCTOR, LIBCONNECT, CALLBACK = 25, 26, 27, 28
B = Bench([(0x5c93c0, 0x5c956c), (0x5c9720, 0x5c9961), (0x5c9f10, 0x5ca004), (0x657cfc, 0x657d02), (0x45b340, 0x45b9a0), (0x4f23f0, 0x4f2450)] + gamestring.B_RANGES,
          hooks={0x67b8a0: _recorder(FOPEN, 2, cdecl=True), 0x67c100: _recorder(FSEEK, 3, cdecl=True), 0x67e350: _recorder(FTELL, 1, cdecl=True), 0x67e1d0: _recorder(FREAD, 4, cdecl=True), 0x67b620: _recorder(FCLOSE, 1, cdecl=True),
                 0x5c92f0: _recorder(ENCODE, 3, cdecl=True)})
SELF, CLIENTAT, TEXTS, FILE = RAM, RAM + 0x200, RAM + 0x400, RAM + 0x600


def base(r, fail=0):
    log = log_region()
    log_fold(log)
    imports = bytearray(0x600)
    for at, stub in global_heap().items():
        imports[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    put(imports, 0x6fd42c - IMPORT_TABLE, 'I', recorder(WSA, 2, True))
    put(imports, 0x6fd57c - IMPORT_TABLE, 'I', recorder(LIBCTOR, 0))
    put(imports, 0x6fd574 - IMPORT_TABLE, 'I', recorder(LIBCONNECT, 5))
    return log, {LOG: log, IMPORT_TABLE: imports, HEAP: heap(0x4000, fail), 0x6fe5ac: struct.pack('<I', r.choice([0xffffffff, 0xffffffff, 0xfffffffe]))}


def ctor(r):
    log, regions = base(r)
    # The fields it sets start as anything, its allocator bytes as 0.
    self = bytearray(r.bytes(0x94))
    for at in (0, 0x10, 0x1c, 0x2c, 0x44, 0x54, 0x74):
        self[at] = 0
    regions[SELF] = self
    set_return(log, WSA, r.getrandbits(32))
    strings(log, WSA, 0, out=0b10)
    return Call(0x5c93c0, [SELF], regions, conv='t')


def connect(r):
    log, regions = base(r, r.choice([0, 0, 0, 2]))
    self = bytearray(r.bytes(0x94))
    put(self, 0x18, 'I', r.choice([0, 1, 2]))
    put(self, 0x70, 'I', r.choice([0, 0, CLIENTAT]))
    put(self, 0x90, 'I', r.choice([0, recorder(CALLBACK, 2, cdecl=True)]))
    regions[SELF] = self
    set_return(log, LIBCTOR, r.choice([CLIENTAT, CLIENTAT, 0]))
    set_return(log, LIBCONNECT, r.choice([0, 1, 0x100]))
    return Call(0x5c9f10, [SELF, TEXTS, r.getrandbits(32), r.getrandbits(32), r.choice([0, FILE])], regions, conv='t')


def attachment(r):
    log, regions = base(r, r.choice([0] * 7 + [2]))
    self = bytearray(r.bytes(0x24))
    self[0] = self[0x10] = 0
    regions[SELF] = self
    regions[TEXTS] = b'pic.jpg\0'
    size = r.choice([0, 1, 2, 3, 4, 5, 75, 76, 77, 300, 0x7fffffff, 0xffffffff, 0xfffffffe])
    set_return(log, FOPEN, r.choice([FILE, FILE, FILE, 0]))
    set_return(log, FTELL, size)
    set_return(log, FSEEK, r.choice([0, 1]))
    strings(log, FOPEN, 0b11)
    fill(log, FREAD, 0, r.bytes(0x10))
    blob(log, ENCODE, 1, 0b1111)
    return Call(0x5c9720, [SELF, TEXTS], regions, conv='t')


OPS = {'Ctor': ctor, 'Connect': connect, 'Attachment': attachment}


def case(r):
    names = os.environ.get('OPS', 'Ctor,Connect,Attachment,Attachment').split(',')
    return OPS[r.choice(names)](r)


if __name__ == '__main__':
    B.main(case)
