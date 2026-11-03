# The staff list dialog's handler (src/game/ui/dialog7ca168handler: Dialog7ca168_WindowHandler 0x495da0) against the original. Window_FindChild among decoys real; recorders (driver.cpp's kDialog7ca168HandlerEnv, in its order from 10): the texts hashed, the items' data and the handle from fills and returns.
# usage: dialog7ca168handler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from dialog7cb400 import kids  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fill, log_region, put, set_return, strings  # noqa: E402

C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
CENTRE = C(27, 1)


def _centre(e):  # (the original pushes the handle's word over what ECX held: the port's word is zero extended)
    sp = e.r['ESP']
    e.write(sp, struct.pack('<I', e.u32(sp) & 0xffff))
    CENTRE(e)


B = Bench([(0x495da0, 0x496097), (0x65d7bc, 0x65d840), (0x67a830, 0x67a860)],
          hooks={0x658d9f: C(10, 4), 0x664c71: _recorder(11, 2), 0x5064f0: _recorder(12, 0), 0x506eb0: _recorder(13, 0), 0x6ace60: _recorder(14, 1), 0x485ac0: C(15, 1), 0x506870: _recorder(16, 0), 0x506550: _recorder(17, 0), 0x6644ea: _recorder(18, 5), 0x488ba0: C(19, 4), 0x485b00: C(20, 1),
                 0x485d20: C(21, 3), 0x486b40: C(22, 1), 0x4a52a0: C(23, 0), 0x495aa0: C(24, 2), 0x493530: C(25, 1), 0x50b350: _recorder(26, 1), 0x4867b0: _centre, 0x4b6080: _recorder(28, 1)})
DIALOG, STAFF, WORD, KIDS = RAM + 0x100, RAM + 0x400, RAM + 0x300, RAM + 0x2000
LIST_ID = 0x321
CHILD = 0x4321


def case(r):
    log = log_region()
    regions = {LOG: log, 0x7ca168: struct.pack('<I', r.choice([DIALOG] * 8 + [0])), 0x75118c: r.bytes(4), 0x78baa4: r.bytes(4), 0x7cc3b0: r.bytes(4), WORD: r.bytes(4)}
    first, ids = kids(r, regions, KIDS, (LIST_ID, CHILD))
    regions[DIALOG] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', first)
    put(regions[ids[LIST_ID]], 0x150, 'i', r.choice([0, 1, 2, 3, 5, -1]))
    staff = bytearray(r.bytes(0x400 * 2))
    for at in (0x1f8, 0x5f8):
        put(staff, at, 'f', r.choice([0.0, 1.0, 50.5, 99.9, 100.0, 255.0, 256.0, 1000.5, -1.5, r.uniform(0, 300)]))
    regions[STAFF] = staff
    data = b''.join(struct.pack('<I', r.choice([STAFF, STAFF + 0x400])) for _ in range(8))
    fill(log, 11, 1, data, records=8)
    strings(log, 11, 0, 1 << 1)
    strings(log, 26, 0, 1 << 0)
    blob(log, 18, 1, 0x1f)
    set_return(log, 12, r.choice([0, 1, 1, 1, 2, 5]))
    set_return(log, 13, r.choice([0, 0, 1, 7]))
    for k in (14, 15, 16, 19, 20):
        set_return(log, k, r.getrandbits(32))
    set_return(log, 17, r.choice([0, 1, 50, 99, 100, 0x400000, r.getrandbits(32)]))
    set_return(log, 26, WORD)
    message = r.deal([5, 0x10, 0x10, 0x10, 0x10, 0x14, 0x15, 0x100, 0x100, 0x400, 0x402, 0x402, 0x406, 0xf, 0x16, 0x13, 0xff, 0x101, 0x3ff, 0x401, 0x403, 0x407])
    a = {0x10: r.choice([0x80083] * 8 + [0x80082]), 0x100: r.choice([0xfffffffe, 0x32e, 0x32f, 0x330, 0x32d, 0x331, 0xffffffff])}.get(message, r.choice([CHILD, CHILD, CHILD, 0x7777]))
    b = r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)])
    return Call(0x495da0, [DIALOG, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
