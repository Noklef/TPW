# The handler of the dial-up progress box (src/game/ui/dialog7cc534handler: 0x4b9f70) against the original. The CRT's sprintf (and its %S path's imports) real; the others recorders as driver.cpp's kDialog7cc534HandlerEnv; the import slot 0x6fd5bc 10 fills the properties.
# usage: dialog7cc534handler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, fill, log_region, put, recorder, set_return, strings  # noqa: E402
from window705950draw import INTERLOCKED, LOCALE, interlocked  # noqa: E402

PROPERTIES, TEXT, POST, CREATE, BASE, SLOT = range(10, 16)
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
HOOKS = {0x485b00: C(TEXT, 1), 0x658ccc: C(POST, 4), 0x4bb070: C(CREATE, 1), 0x47eed0: C(BASE, 4)}
HOOKS.update({at: interlocked(step) for at, (_, step) in INTERLOCKED.items()})
B = Bench([(0x4b9f70, 0x4ba1c9), (0x67a400, 0x690000), (0x6918f0, 0x691a30), (0x691ba0, 0x691ce0)], hooks=HOOKS)
BOX, TABLE = RAM + 0x100, RAM + 0x300
LETTERS = 'abcXYZ019 _'


def case(r):
    log = log_region()
    imports = bytearray(0x400)
    for at, (slot, _) in INTERLOCKED.items():
        put(imports, slot - IMPORT_TABLE, 'I', at)
    regions = {LOG: log, LOCALE: bytes(4), IMPORT_TABLE: imports, 0xfb3ca4: bytes(8), 0x6fd5bc: struct.pack('<I', recorder(PROPERTIES, 2))}
    regions[BOX] = put(bytearray(r.bytes(0x20)), 0, 'I', TABLE)
    regions[TABLE] = put(bytearray(0x40), 0x24, 'I', recorder(SLOT, 1))
    state = bytearray(r.bytes(0x60))
    now = r.deal([-5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 0x7fffffff, -0x80000000])
    put(state, 0x14, 'i', now)
    put(state, 0x18, 'i', r.deal([now - 1, now - 1, now - 1, now - 1, now]) if -0x80000000 < now else now)
    put(state, 0x44, 'I', r.deal([BOX, BOX, BOX, 0]))
    regions[0x7cc4f0] = state
    name = ''.join(r.choice(LETTERS) for _ in range(r.deal([0, 1, 5, 12, 40])))
    fill(log, PROPERTIES, 1, name.encode('utf-16-le') + b'\0\0', offset=0x212)
    strings(log, PROPERTIES, 0, 1 << 1)
    set_return(log, TEXT, r.getrandbits(32))
    set_return(log, BASE, r.getrandbits(32))
    message = r.deal([0x1e] * 9 + [4, 4, 4, 0x14, 3, 5, 0x13, 0x15, 0x1d, 0x1f, r.getrandbits(32)])
    a = r.deal([1, 1, 1, 0, 2, 0x101, 0xffffffff])
    return Call(0x4b9f70, [RAM + 0x20000, message, a, r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
