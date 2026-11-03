# The handlers of the two dialogs on the game object's parts (src/game/ui/gamepartdialogs: Dialog7cc47c_Handler 0x4b86a0, Dialog7cc4b4_Handler 0x4b8b70) against the original. Recorders, as GamePartDialogEnv's:
# Dialog7cca70_Open 10, Class702da0_Push 11, Dialog7cc4b4_SetEffect 12, Dialog7cc4b4_Update 13; each part of [0xf85614] its own object on one vtable of recorders, so the log's this tells the part.
# Window_CallDefault runs on +0x118, fncmp's logger. The message and a kept to the switches' values and their neighbours.
# usage: gamepartdialogs.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CDECL_LOG4, LOG, RAM, Bench, Call, _recorder, log_region, put, vtable  # noqa: E402

B = Bench([(0x4b86a0, 0x4b87c3), (0x4b8b70, 0x4b8c7c), (0x65f66e, 0x65f697)],
          hooks={0x4c2110: _recorder(10, 1, cdecl=True), 0x5d6110: _recorder(11, 2), 0x4b8ee0: _recorder(12, 2, cdecl=True), 0x4b9340: _recorder(13, 0, cdecl=True)})
GAME, PARTS, VT, WINDOW = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x400
HANDLERS = {
    0x4b86a0: (0x14, list(range(0x49d4, 0x49de))),
    0x4b8b70: (0x11, list(range(0x1e0e7, 0x1e0f3))),
}


def case(r):
    op = r.deal(list(HANDLERS))
    extra, ids = HANDLERS[op]
    message = r.choice([0x100] * 8 + [0x14, 0x14, 0x11, 0x11, 0x13, 0x15, 0xff, 0x101, r.getrandbits(32)])
    a = r.choice(ids) if message == 0x100 else r.choice([0, 0, 1, r.getrandbits(32)])
    game = bytearray(r.bytes(0x20))
    parts = bytearray(0x60)
    for k in range(5):
        put(game, 4 * (k + 1), 'I', PARTS + 0x10 * k)
        put(parts, 0x10 * k, 'I', VT)
    regions = {LOG: log_region(), GAME: game, PARTS: parts, VT: vtable({}),
               WINDOW: put(bytearray(r.bytes(0x120)), 0x118, 'I', CDECL_LOG4), 0xf85614: struct.pack('<I', GAME), 0x7cc47c: r.bytes(4), 0x7cc4b4: r.bytes(4)}
    return Call(op, [WINDOW, message, a, r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
