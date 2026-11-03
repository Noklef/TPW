# The dial-up dialog's handler (src/game/ui/dialog7cc530handler: Dialog7cc530_Handler 0x4ba530) against the original. Window_FindChild among decoys, the +0x138 accessor and the CRT's sprintf real; recorders (driver.cpp's kDialog7cc530HandlerEnv): Window_SendMessage 11, Focus_Restore 12, Dialog_BaseHandler 13, Window7059f0_NthItemData 14 (its out filled),
# Dialog7cc5f4_Create 15, Dialog7cc5f4_CreateFrom 16 (the properties' first 16 bytes hashed), DialupList_Fill 17; the import slot 0x6fd5bc 10 (fills the properties' head and name) and the function [0x7cc52c] 18.
# usage: dialog7cc530handler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from dialog7cb400 import kids  # noqa: E402
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob_bytes, fill, log_region, put, recorder, set_return, strings  # noqa: E402
from window705950draw import INTERLOCKED, LOCALE, interlocked  # noqa: E402

C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
HOOKS = {0x658d9f: C(11, 4), 0x4862a0: C(12, 0), 0x47ee90: C(13, 4), 0x664c71: _recorder(14, 2), 0x4bb070: C(15, 1), 0x4ba200: C(16, 2), 0x4ba850: C(17, 1)}
HOOKS.update({at: interlocked(step) for at, (_, step) in INTERLOCKED.items()})
B = Bench([(0x4ba530, 0x4ba73d), (0x65d7bc, 0x65d840), (0x66a281, 0x66a295), (0x67a400, 0x690000), (0x6918f0, 0x691a30), (0x691ba0, 0x691ce0)], hooks=HOOKS)
DIALOG, FRONT, KIDS = RAM + 0x100, RAM + 0x300, RAM + 0x1000
LETTERS = 'abcXYZ019 _'


def case(r):
    log = log_region()
    imports = bytearray(0x400)
    for at, (slot, _) in INTERLOCKED.items():
        put(imports, slot - IMPORT_TABLE, 'I', at)
    regions = {LOG: log, LOCALE: bytes(4), IMPORT_TABLE: imports, 0xfb3ca4: bytes(8), FRONT: r.bytes(0x10), 0xf82884: struct.pack('<I', FRONT), 0x6fd5bc: struct.pack('<I', recorder(10, 2))}
    first, ids = kids(r, regions, KIDS, (0xc, 0xd))
    regions[DIALOG] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', first)
    list_at, radio = ids[0xc], ids[0xd]
    put(regions[radio], 0x138, 'I', r.choice([0xf, 0xf, 0xf, 0xe, 0, r.getrandbits(32)]))
    state = bytearray(r.bytes(0x50))
    put(state, 0x3c, 'I', r.choice([0, recorder(18, 0, cdecl=True), recorder(18, 0, cdecl=True)]))
    put(state, 0x40, 'I', r.choice([DIALOG] * 5 + [RAM + 0x200]))
    regions[RAM + 0x200] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', first)
    regions[0x7cc4f0] = state
    fill(log, 14, 1, struct.pack('<i', r.choice([0, 1, 2, 7, 0x7fffffff, -1, -2])), which=2)
    strings(log, 14, 0, 1 << 1)
    name = ''.join(r.choice(LETTERS) for _ in range(r.choice([0, 1, 5, 12, 40])))
    fill(log, 10, 1, struct.pack('<I', r.choice([0, 0, 1, r.getrandbits(32)])) + r.bytes(12), which=0)
    fill(log, 10, 1, name.encode('utf-16-le') + b'\0\0', which=1, offset=0x212)
    strings(log, 10, 0, 1 << 1)
    blob_bytes(log, 16, 1, 0xffff)
    set_return(log, 13, r.getrandbits(32))
    message = r.deal([0x14, 0x15, 0x100, 0x100, 0x100, 0x100, 0x100, 0x101, 0x101, 0x13, 0x16, 0xff, 0x102])
    a = r.choice([0xfffffffe, 0xffffffff, 0xffffffff, 0xffffffff, 0, 5]) if message == 0x100 else r.choice([0xe, 0xe, 0xf, 0xfffffffe, 0xffffffff])
    b = r.choice([1, 1, 1, 0, 2, r.getrandbits(32)])
    return Call(0x4ba530, [DIALOG, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
