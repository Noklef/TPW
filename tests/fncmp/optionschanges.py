# The options' changes applied and the first settings loaded (src/game/core/optionschanges: 0x4237f0, 0x424bb0, 0x449df0) against the originals. The strings and the heap real; WideCharToMultiByte a recorder (the wide path hashed, the narrow text
# written back); every other callee a recorder: the dialog's box hashed with the text its format wrote, the file calls with their local handle written back and each outcome dealt.
# usage: optionschanges.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob_bytes, echo, fill, global_heap, heap, import_table, log_region, set_return, strings  # noqa: E402
import gamestring  # noqa: E402

NOTE, SAVEB, POST, LEVEL, LOADB, FORMAT, DIALOG, SOUND, FOLDER, OPEN, WRITE, CLOSE, SERIAL, YAW, APPLY, WINDOW, SETG, SAVEA, MISC, SELECT = range(1, 21)
W2M = 40
SINGLETON = 27
B = Bench([(0x4237f0, 0x423ac9), (0x424bb0, 0x424c03), (0x449df0, 0x449eaa), (0x424dc0, 0x424e10), (0x5da3c0, 0x5da3c1), (0x5f5660, 0x5f5690), (0x5f56f0, 0x5f5710)] + gamestring.B_RANGES,
          hooks={0x423740: _recorder(NOTE, 0), 0x5ac630: _recorder(SAVEB, 1), 0x51bc70: _recorder(POST, 0, cdecl=True), 0x51b920: _recorder(LEVEL, 1, cdecl=True), 0x5ac5f0: _recorder(LOADB, 1), 0x6acd60: _recorder(FORMAT, 4, cdecl=True),
                 0x47f020: _recorder(DIALOG, 8, cdecl=True), 0x423dd0: _recorder(SOUND, 0), 0x5c7bd0: _recorder(FOLDER, 1), 0x5f5ad0: _recorder(OPEN, 4, cdecl=True), 0x5f5e50: _recorder(WRITE, 5, cdecl=True), 0x5f5b80: _recorder(CLOSE, 2, cdecl=True),
                 0x4242c0: _recorder(SERIAL, 3), 0x42aff0: _recorder(YAW, 0, cdecl=True), 0x51bd70: _recorder(APPLY, 0, cdecl=True), 0x4a4580: _recorder(WINDOW, 0, cdecl=True), 0x5508b0: _recorder(SETG, 1, cdecl=True), 0x5ac610: _recorder(SAVEA, 1),
                 0x424a60: _recorder(MISC, 1), 0x409790: _recorder(SELECT, 1, cdecl=True), 0x5c7420: _recorder(SINGLETON, 0)})
SLOTS, OPTIONS, STATUS, SESSION, FOLDER_T, GLOBALS_T = (RAM + a for a in (0, 0x100, 0x300, 0x340, 0x380, 0x400))
OPS = ['Apply', 'Apply', 'Apply', 'Confirm', 'Startup']


def wide(text):
    return text.encode('utf-16-le')


def case(r):
    op = r.deal(OPS)
    log = log_region()
    table = import_table(log, {'WideCharToMultiByte': (W2M, 0x6fd1dc, 8, 0, 0)})
    for at, stub in global_heap().items():
        table[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(0x4000, r.choice([0] * 9 + [r.randrange(1, 4)])), gamestring.LOCALE: bytes(4), 0xf82884: struct.pack('<I', r.choice([0, 0, SESSION])), SESSION: r.bytes(0x20)}
    if op == 'Confirm':
        return Call(0x424bb0, [], regions, ret='a')
    if op == 'Startup':
        regions[0x78d950] = struct.pack('<6I', *[r.getrandbits(32) for _ in range(6)])
        regions[0x78d968] = struct.pack('<I', r.choice([0, 0, 1, 7]))
        regions[0x7a1a14] = r.bytes(4)
        regions[0x7a1a8c] = r.bytes(4)
        regions[0x79d86c] = r.bytes(4)
        regions[0x79d9c8] = r.bytes(4)
        set_return(log, MISC, r.choice([1, 1, 0, 2, 0x101]))
        blob_bytes(log, MISC, 0, 0xff, inner=(8, 0xf))
        return Call(0x449df0, [], regions, ret='v')
    options = bytearray(r.bytes(0xa0))
    struct.pack_into('<I', options, 0x94, r.choice([0, 1, 1, 1, 2, 2, 3, -1 & 0xffffffff]))
    struct.pack_into('<I', options, 0x98, r.getrandbits(2))
    options[0x3a] = r.choice([0, 1, 0x80])
    regions[OPTIONS] = options
    regions[0x78ba94] = r.bytes(4)
    fill(log, FORMAT, 1, b'o\0k\0\0\0')
    strings(log, FORMAT, 0, out=1 << 1)
    blob_bytes(log, DIALOG, 0, 0xf70, inner=(0, 0x3f))
    regions[0xf81ebc] = struct.pack('<I', SLOTS if r.random() < 0.8 else 0)
    regions[SLOTS] = r.bytes(0x10)
    set_return(log, SINGLETON, SLOTS)
    n = r.choice([1, 2, 3])
    regions[FOLDER_T] = wide('fold'[:n]) + bytes(2) + r.bytes(0x20)
    fill(log, FOLDER, 0, struct.pack('<5I', gamestring.VT[1][0], n, FOLDER_T, 8, 8), which=1)
    strings(log, FOLDER, 0, out=1)
    echo(log, FOLDER, 0)
    for i, base in enumerate((0x78e6f8, 0x78e230)):
        gamestring.string(r, 1, base, GLOBALS_T + 0x60 * i, regions, wide(r.choice(['\\', '.a', 'b', ''])))
    narrow = b'p' * r.randrange(1, 6) + b'\\x'
    set_return(log, W2M, len(narrow) + 1)
    fill(log, W2M, 4, narrow + b'\0', which=2)
    blob_bytes(log, W2M, 2, 0xffffffff)
    regions[STATUS] = struct.pack('<ii', r.randrange(0, 5), -r.randrange(1, 5))
    set_return(log, OPEN, STATUS + r.choice([0, 0, 0, 4]))
    strings(log, OPEN, 1 << 2, out=0b11)
    fill(log, OPEN, 1, struct.pack('<I', r.getrandbits(32)), which=3)
    set_return(log, WRITE, STATUS + r.choice([0, 0, 0, 4]))
    strings(log, WRITE, 0, out=0b10001)
    blob_bytes(log, WRITE, 2, 0xf)
    fill(log, WRITE, 4, struct.pack('<I', r.choice([4, 4, 4, 0, 2])), which=4)
    strings(log, CLOSE, 0, out=1)
    set_return(log, SERIAL, r.choice([0, 1]))
    return Call(0x4237f0, [OPTIONS], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
