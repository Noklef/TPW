# The text button's window handler (src/game/ui/textbutton: 0x492d80) against the original. The window's default handler +0x118 fncmp's
# logger (its result now and then set), its row (+0xa4, Window_DwordA0 1) a row or 0, its parent +0x60 a window (byte +4 0 or not, its
# handler +0x114 the logger or none) or 0, the message hook [0xfaa5fc] the logger or none; the sound globals as soundplay.py's (the
# manager's +8 a recorder of the command). Messages 0x10006, 0x14 and others.
# FlashStep / FlashHandler (0x48b220, 0x48b2a0): the same, the flash value (+0xa8, Window_DwordA0 2) about 0, 0xdf and -0xdf or any, the
# objects +0x98 / +0x9c recorders or none (the 0x40000 event's colour bytes hashed); the handler's messages 0x11, 0x1e, 0x10001 / 0x10002
# too.
# usage: textbutton.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Handler,FlashStep,FlashHandler)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import CDECL_LOG4, LOG, RAM, Bench, Call, blob, blob_bytes, log_region, put, set_return, vtable  # noqa: E402
from window import RANGES as WINDOW_RANGES  # noqa: E402

CALLS = {'Handler': 0x492d80, 'FlashStep': 0x48b220, 'FlashHandler': 0x48b2a0}
B = Bench([(0x492d80, 0x492df1), (0x485aa0, 0x485abd), (0x51bfc0, 0x51c048), (0x65e705, 0x65e73d), (0x65f66e, 0x65f697),
           (0x66969e, 0x6696b4), (0x65d55a, 0x65d56b), (0x48b220, 0x48b29f), (0x48b2a0, 0x48b310), (0x65c5d5, 0x65c672),
           (0x65e6d5, 0x65e705)] + WINDOW_RANGES)
WINDOW, PARENT, ROW, MANAGER, VT, OBJ, OBJ_VT = RAM + 0x100, RAM + 0x300, RAM + 0x500, RAM + 0x600, RAM + 0x700, RAM + 0x800, RAM + 0x900


def case(r):
    op = r.deal(OPS)
    log = log_region()
    win = bytearray(r.bytes(0x120))
    put(win, 0x60, 'I', r.choice([PARENT, PARENT, PARENT, 0]))
    put(win, 0xa4, 'I', r.choice([ROW, ROW, 0]))
    put(win, 0x118, 'I', CDECL_LOG4)
    parent = bytearray(r.bytes(0x120))
    parent[4] = r.choice([0, 0, 0, 1])
    put(parent, 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
    g = bytearray(r.bytes(0x20))   # 0x802bc0..
    put(g, 8, 'I', r.choice([0, 1, 1, 1]))
    put(g, 0xc, 'I', MANAGER)
    put(g, 0x14, 'I', r.choice([0, 1, 1, 1]))
    blob(log, 2, 1, 0x7f)
    if r.random() < 0.5:
        set_return(log, 2, r.getrandbits(32))
    if r.random() < 0.5:
        set_return(log, 0x3d, r.getrandbits(32))
    regions = {LOG: log, WINDOW: win, PARENT: parent, ROW: r.bytes(8), 0xfaa5fc: struct.pack('<I', r.choice([CDECL_LOG4, 0])),
               0x802bc0: g, MANAGER: struct.pack('<I', VT) + r.bytes(0xc), VT: vtable({2: 2}, 4), 0x803a2c: r.bytes(4)}
    message = r.choice([0x10006, 0x10006, 0x14, 0x14, 0, 0x10005, 0x10007, 0x114, r.getrandbits(32)])
    if op != 'Handler':
        put(win, 0x98, 'I', r.choice([OBJ, OBJ, 0]))
        put(win, 0x9c, 'I', r.choice([OBJ + 0x40, 0]))
        regions[OBJ] = put(put(bytearray(r.bytes(0x80)), 0, 'I', OBJ_VT), 0x40, 'I', OBJ_VT)
        regions[OBJ_VT] = vtable({3: 2})
        blob_bytes(log, 3, 1, 0, (0x10, 0xf, 0x10))
        put(win, 0xa8, 'I', r.choice([0, 0, 1, 0xdf, 0xe0, 0x7fffffe0, -0xdf, -0xe0, -0xe1, -1, -0x80000000, r.getrandbits(32)]) & 0xffffffff)
        if op == 'FlashStep':
            return Call(CALLS[op], [WINDOW], regions, ret='v')
        message = r.choice([0x11, 0x1e, 0x1e, 0x10001, 0x10002, message])
    return Call(CALLS[op], [WINDOW, message, r.getrandbits(32), r.getrandbits(32)], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
