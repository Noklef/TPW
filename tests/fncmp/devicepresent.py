# The back surface shown (src/game/render/present: Device_Present, 0x564210) against the original: full screen with Blt once or waited on, Flip waited on, windowed with its two rectangles; answers of 0, the two it waits on, and
# failures. The DirectDraw slots, the hooks and the window calls recorders (the rectangles' bytes hashed), the error's calls recorders. The budgets of 50000000 are not run out here.
# usage: devicepresent.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, blob_bytes, fill, import_table, log_region, recorder, set_return, strings  # noqa: E402

MESSAGE, LOGGED, PUSH, HOOK1, HOOK2, BLT, FLIP, WINDOW, CLIENT = 1, 2, 3, 4, 5, 40, 41, 42, 43
B = Bench([(0x564210, 0x564424)], hooks={0x56a770: _recorder(MESSAGE, 1, cdecl=True), 0x564650: _recorder(LOGGED, 1, cdecl=True), 0x56af20: _recorder(PUSH, 2)})
ENGINE, FRONT, VT, ANSWERS, BACK = RAM, RAM + 0x800, RAM + 0x900, RAM + 0xa00, RAM + 0xb00
WAIT1, WAIT2 = 0x8876021c, 0x887601ae
OPS = ['blt wait', 'blt once', 'flip', 'flip', 'flip off', 'window', 'window']


def rect(r):
    x, y = r.randrange(-100, 700), r.randrange(-100, 500)
    return struct.pack('<4i', x, y, x + r.randrange(0, 900), y + r.randrange(0, 700))


def case(r):
    op = r.deal(OPS)
    log = log_region()
    table = import_table(log, {'GetWindowRect': (WINDOW, 0x6fd390, 2, 0, 1 << 1), 'GetClientRect': (CLIENT, 0x6fd38c, 2, 0, 1 << 1)})
    fill(log, WINDOW, 1, rect(r))
    fill(log, CLIENT, 1, rect(r), which=1)
    set_return(log, MESSAGE, r.getrandbits(32))
    fail = r.choice([0, 0, 0x80000001, 0x887601c2, 5])
    waits = [r.choice([WAIT1, WAIT2]) for _ in range(r.randrange(0, 5))]
    if op == 'blt wait':
        seq = [r.choice([WAIT1, WAIT2, 5]) for _ in range(r.randrange(0, 5))] + [0]
    elif op == 'blt once':
        seq = [r.choice([0, WAIT1, WAIT2, fail])]
    else:
        seq = waits + [fail]
    seq = seq + [0] * (40 - len(seq))
    answers(log, BLT, ANSWERS)
    answers(log, FLIP, ANSWERS)
    if op == 'window':
        shown = r.choice([1, 1, 3])   # (a slot hashes one blob: the other rectangle, on the stack too, is logged as 0)
        blob_bytes(log, BLT, shown, 0xffff)
        strings(log, BLT, 0, out=1 << (4 - shown))
    flags = (r.getrandbits(32) & ~0x8000001) | (1 if op in ('blt wait', 'blt once', 'flip off') else r.choice([0, 1]))
    if op == 'blt wait':
        flags |= 0x8000000
    elif op != 'window' and op != 'flip':
        flags |= 0
    elif r.random() < 0.5:
        flags |= 0x8000000
    engine = bytearray(r.bytes(0x800))
    engine[0x780] = (r.getrandbits(8) & ~1) | (0 if op == 'window' else 1)
    struct.pack_into('<II', engine, 0x10, FRONT, BACK)
    second = r.choice([0, 1, 2])
    struct.pack_into('<I', engine, 0x78c, 1 if op == 'flip off' else (r.choice([0, 7]) if op == 'flip' and flags & 1 == 0 else (second if op == 'flip' else 0)))
    if op == 'flip' and flags & 1 and not struct.unpack_from('<I', engine, 0x78c)[0]:
        struct.pack_into('<I', engine, 0x78c, 3)
    vt = bytearray(0x30)
    struct.pack_into('<I', vt, 0x14, recorder(BLT, 6, True))
    struct.pack_into('<I', vt, 0x2c, recorder(FLIP, 3, True))
    regions = {LOG: log, IMPORT_TABLE: table, ENGINE: engine, FRONT: struct.pack('<I', VT), VT: vt, ANSWERS: struct.pack('<40I', *seq), 0x879cd4: r.bytes(4), 0x8bd508: struct.pack('<I', flags),
               0x8bcbc8: r.bytes(4), 0x8bcbd0: struct.pack('<II', r.randrange(1, 1600), r.randrange(1, 1200)), 0x8bd510: struct.pack('<II', r.choice([0, recorder(HOOK1, 0, cdecl=True)]), r.choice([0, recorder(HOOK2, 0, cdecl=True)])), 0x8bd518: r.bytes(4)}
    return Call(0x564210, [ENGINE], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
