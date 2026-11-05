# The front-end frame's message handler (src/game/frontend/frame: 0x5d5dd0, 0x5d58b0) against the original: the messages it looks at with their wparams, others; the frame at 0xf82884 or none; the frame's list of
# 0 to 3 objects (slot 5 a recorder with the message). DialogF82884_ShowChild (0x4bbbd0) and Window_CallDefault (0x65f66e) are recorders, each with a bench of its own.
# usage: framemessage.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob_bytes, log_region, put, set_return, vtable  # noqa: E402

SLOT, DEFAULT, SHOW, INSTANCE, LOGGED_IN, OPEN, WIDE, BOX = 5, 6, 7, 8, 9, 10, 11, 12
os.environ['FNCMP_PAINT_STACK'] = '1'   # (the driver's: the online button's record is a stack object, each of its slots must be written)
B = Bench([(0x5d58b0, 0x5d58d7), (0x5d5dd0, 0x5d5ed5), (0x5d58e0, 0x5d596a)],
          hooks={0x4bbbd0: _recorder(SHOW, 0, cdecl=True), 0x65f66e: _recorder(DEFAULT, 3), 0x48b390: _recorder(INSTANCE, 0, cdecl=True), 0x5b6450: _recorder(LOGGED_IN, 0), 0x4bc710: _recorder(OPEN, 0, cdecl=True),
                 0x485b00: _recorder(WIDE, 1, cdecl=True), 0x47f020: _recorder(BOX, 8, cdecl=True)})
FRAME, OWNER, NODES, OBJECTS, VT = RAM, RAM + 0x80, RAM + 0x100, RAM + 0x200, RAM + 0x300
WINDOW = 0x12345678
MESSAGES = [0x11, 0x14, 0x22, 0x10003, 0x10004, 0x10005]


def case(r):
    log = log_region()
    set_return(log, DEFAULT, r.getrandbits(32))
    n = r.choice([0, 1, 2, 3])
    nodes = bytearray(r.bytes(0x40))
    for i in range(n):
        put(nodes, 8 * i, '2I', OBJECTS + 4 * i, NODES + 8 * (i + 1) if i + 1 < n else 0)
    frame = bytearray(r.bytes(0x40))
    put(frame, 0x18, 'I', OWNER)
    put(frame, 0x1c, 'I', r.choice([0, 7, r.getrandbits(32)]))
    regions = {LOG: log, FRAME: frame, OWNER: struct.pack('<II', r.getrandbits(32), NODES if n else 0), NODES: nodes, OBJECTS: struct.pack('<3I', VT, VT, VT), VT: vtable({SLOT: 3}),
               0xf82884: struct.pack('<I', r.choice([0, FRAME, FRAME, FRAME]))}
    message = r.choice(MESSAGES + MESSAGES + [0, 0x10, 0x12, 0x15, 0x21, 0x23, 0x10002, 0x10006, 0xffffffff, r.getrandbits(32)])
    wparam = r.choice([0, 1, 2, 3, 0xffffffff, r.getrandbits(32)])
    op = r.deal(['handler', 'proc', 'online'])
    if op == 'online':   # (0x5d58e0: the message box's record on the stack, its byte 7 unwritten)
        set_return(log, INSTANCE, r.getrandbits(32))
        set_return(log, LOGGED_IN, r.choice([0, 0, 1, r.getrandbits(32)]))
        set_return(log, WIDE, r.getrandbits(32))
        blob_bytes(log, BOX, 0, 0xff7f)
        return Call(0x5d58e0, [WINDOW, r.choice([0x10006, 0x10006, 0x10006, message]), wparam, r.getrandbits(32)], regions, ret='a')
    if op == 'proc':   # (0x5d58b0: the same on the frame at 0xf82884, or 0)
        return Call(0x5d58b0, [WINDOW, message, wparam, r.getrandbits(32)], regions, ret='a')
    return Call(0x5d5dd0, [FRAME, WINDOW, message, wparam, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
