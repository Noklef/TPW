# The handler of the guests' list (src/game/ui/dialog7ca13chandler: 0x493230) against the original. Window_FindChild among decoys and the CRT (_alldiv, __ftol) real; the others recorders as driver.cpp's kDialog7ca13cHandlerEnv.
# usage: dialog7ca13chandler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from dialog7cb400 import kids  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, fill, log_region, put, set_return, strings  # noqa: E402

SEND, NTH, TIME, SPAN, TEXTS, TEXT, ATTACH, CANCEL, CREATE, OPEN168, OPEN150, HANDLE, CAMERA, SHOW, KEYS = range(10, 25)
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x493230, 0x49352d), (0x65d7bc, 0x65d840), (0x67a400, 0x690000)],
          hooks={0x658d9f: C(SEND, 4), 0x664c71: _recorder(NTH, 2), 0x4fd950: _recorder(TIME, 0), 0x4f8730: _recorder(SPAN, 2), 0x6644ea: _recorder(TEXTS, 5), 0x485b00: C(TEXT, 1), 0x485d20: C(ATTACH, 3), 0x486b40: C(CANCEL, 1),
                 0x4a52a0: C(CREATE, 0), 0x496620: C(OPEN168, 2), 0x495aa0: C(OPEN150, 2), 0x50b350: _recorder(HANDLE, 1), 0x4867b0: C(CAMERA, 1), 0x4b79b0: _recorder(SHOW, 1), 0x488ba0: C(KEYS, 4)})
WINDOW, DIALOG, CLOCK, TIMES, SPANS, WORD = (RAM + 0x1000 * k for k in range(1, 7))
GUESTS = RAM + 0x8000
KIDS1, KIDS2 = RAM + 0x10000, RAM + 0x14000
HOUR = 36000000000
FLOATS = [0.0, 0.5, 1.0, 50.0, 99.5, 100.0, 127.9, 255.0, 256.0, 300.7, -1.5, -300.0, 1e10, 3e9, float('inf'), float('nan')]


def case(r):
    log = log_region()
    regions = {LOG: log, CLOCK: r.bytes(0x400)}
    regions[0x7cf83c] = struct.pack('<I', CLOCK)
    first, top = kids(r, regions, KIDS1, (0x1e497,))
    regions[DIALOG] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', first)
    put(regions[top[0x1e497]], 0x150, 'i', r.deal([0, 1, 2, 3, 4, -1]))
    first, _ = kids(r, regions, KIDS2, (0x77, 0x1e497))
    regions[WINDOW] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', first)
    regions[0x7ca13c] = struct.pack('<I', r.deal([DIALOG, DIALOG, DIALOG, 0]))
    regions[0x7508bc] = r.bytes(4)
    data = b''
    for k in range(4):
        guest = bytearray(r.bytes(0x400))
        put(guest, 0x30, 'I', r.deal([0, 1, 2, 0x100, 0xffffffff]))
        put(guest, 0x19c, 'f', r.deal(FLOATS))
        regions[GUESTS + 0x400 * k] = guest
        data += struct.pack('<I', GUESTS + 0x400 * k)
    fill(log, NTH, 1, data, records=4)
    strings(log, NTH, 0, 1 << 1)
    regions[TIMES] = r.bytes(160)
    answers(log, TIME, TIMES)
    strings(log, SPAN, 0, 1 << 0)
    regions[SPANS] = struct.pack('<q', r.deal([0, HOUR - 1, HOUR, HOUR + 1, 7 * HOUR, -HOUR, -1, 1 << 62, -(1 << 63), r.getrandbits(63)]))
    set_return(log, SPAN, SPANS)
    blob(log, TEXTS, 1, 0x3f)
    regions[WORD] = r.bytes(4)
    set_return(log, HANDLE, WORD)
    strings(log, HANDLE, 0, 1 << 0)
    set_return(log, TEXT, r.getrandbits(32))
    set_return(log, KEYS, r.getrandbits(32))
    message = r.deal([5, 0x10, 0x10, 0x10, 0x10, 0x14, 0x15, 0x100, 0x100, 0x100, 0x100, 0x100, 0x400, 0x400, 0x402, 0x402, 0x402, 0x406, 0x401, 0x403, 0x405, 0xff, r.getrandbits(32)])
    if message == 0x10:
        a, b = r.deal([0x80083, 0x80083, 0x80083, 0x80084]), r.getrandbits(32)
    elif message == 0x100:
        a, b = r.deal([0xfffffffe, 0x1e499, 0x1e49a, 0x1e49b, 0x1e498, 0x1e49c, 0xffffffff]), r.getrandbits(32)
    else:
        a, b = r.deal([0x77, 0x77, 0x1e497, 0x1234]), r.deal([0, 1, 2, 3, 0xffffffff])
    return Call(0x493230, [WINDOW, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
