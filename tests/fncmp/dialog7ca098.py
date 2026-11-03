# The message box's action and handler (src/game/ui/dialog7ca098: 0x486b40, 0x48e0f0, 0x48e2f0) against the originals. Window_FindChild and Window_DwordA0 real over dialog7cb400.py's children (0x10d32, 0x10d30, 0x10d2f, 0x10d2e among decoys; the first missing now and then: nothing else is checked), the game tables' accessors real, the other calls recorders numbered as kDialog7ca098Env from 10 (the last three 7, 8, 9: a recorder's slots end at 31).
# The buttons' slot 5 (vtable VT) recorder 5, [0x7ca090]'s too. The box's dwords 3, 4 the kinds, RunAction's jump table at 0x48e284 in the data read from the exe.
# usage: dialog7ca098.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Action,Handler,Cancel)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialog7cb400  # noqa: E402
from dialog7cb400 import VT, kids  # noqa: E402
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, blob_bytes, log_region, put, recorder, set_return, vtable  # noqa: E402

B = Bench([(0x486b40, 0x486b58), (0x48e0f0, 0x48e284), (0x48e284, 0x48e2e8), (0x48e2f0, 0x48e603), (0x65d7bc, 0x65d840), (0x65e705, 0x65e72d), (0x5194d0, 0x519510), (0x519610, 0x519650), (0x5da3c0, 0x5da3c1)],
          hooks={0x4867b0: _recorder(10, 1, cdecl=True), 0x4868d0: _recorder(11, 1, cdecl=True), 0x4acc70: _recorder(12, 2, cdecl=True), 0x49bdd0: _recorder(13, 1, cdecl=True), 0x49fb30: _recorder(14, 0, cdecl=True), 0x4aa480: _recorder(15, 0, cdecl=True), 0x4a52a0: _recorder(16, 0, cdecl=True),
                 0x4b2750: _recorder(17, 0, cdecl=True), 0x4b7b00: _recorder(18, 0, cdecl=True), 0x498d80: _recorder(19, 0, cdecl=True), 0x4dfe30: _recorder(20, 1), 0x4d2df0: _recorder(21, 0), 0x4d2ec0: _recorder(22, 0), 0x65f5b7: _recorder(23, 1), 0x48e610: _recorder(24, 2, cdecl=True),
                 0x65d070: _recorder(25, 1), 0x659bc9: _recorder(26, 1, cdecl=True), 0x65f16b: _recorder(27, 1), 0x65e72d: _recorder(28, 2), 0x65e897: _recorder(29, 0), 0x59aa70: _recorder(30, 1), 0x658d9f: _recorder(31, 4, cdecl=True), 0x658ccc: _recorder(7, 4, cdecl=True), 0x4a25f0: _recorder(8, 1, cdecl=True),
                 0x65f66e: _recorder(9, 3)})
WINDOW, BOX, GAME, OPEN, TABLE = RAM + 0x100, RAM + 0x20000, RAM + 0x30000, RAM + 0x31000, RAM + 0x32000
KIDS = RAM + 0x1000


def world(r, regions):
    regions[0x7cfb90] = b''.join(struct.pack('<I', TABLE + i) + r.bytes(16) for i in range(0x10))
    regions[0x7cf83c] = struct.pack('<I', GAME)
    index = bytearray(r.bytes(0x100))
    for at in (0x1c, 0x20, 0x36):   # (the tables' index words of the accessors)
        put(index, at, 'H', r.randrange(0x10))
    regions[GAME + 0x1da700] = bytes(index)


def box(r, regions, at, kinds=None):
    w = bytearray(r.bytes(0x200))
    put(w, 0, 'I', VT)
    for i in range(16):
        put(w, 0xa0 + 4 * i, 'I', r.choice([0, 1, 2, r.getrandbits(32)]))
    if kinds:
        put(w, 0xa0 + 12, 'I', kinds[0])
        put(w, 0xa0 + 16, 'I', kinds[1])
    regions[at] = w
    return w


def kind_of(r):
    return r.deal(list(range(0, 0x1e)))


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log}
    world(r, regions)
    for k in (20, 23, 26, 27, 28, 29, 31, 7, 9):
        set_return(log, k, r.getrandbits(32))
    blob_bytes(log, 11, 0, 3)   # (a pointer to b's low word)
    struct.pack_into('<I', log, LOG_BYTES + 4 * 27, 1 << 16)   # (the depth a word over ECX's stale upper half)
    if op == 'Cancel':
        return Call(0x486b40, [r.getrandbits(32)], regions, ret='v')
    if op == 'Action':
        return Call(0x48e0f0, [kind_of(r), r.randrange(0x10) | r.getrandbits(16) << 16, r.randrange(0x10) | r.getrandbits(16) << 16], regions, ret='a')
    regions[VT] = put(put(vtable({}, size=0x10), 0x14, 'I', recorder(5, 1)), 0x24, 'I', recorder(6, 1))
    regions[OPEN] = put(bytearray(r.bytes(0x40)), 0, 'I', VT)
    kinds = [r.choice([3, 4, 9, 0xd, 0x12, 0x17, 0x19, 0x1a, 0x1b, 0x1b, 0x1c]) for _ in range(2)]
    box(r, regions, WINDOW, kinds)
    the_box = box(r, regions, BOX, kinds)
    ids = [0x10d30, 0x10d2f, 0x10d2e] + ([0x10d32] if r.random() < 0.9 else [])
    first, _ = kids(r, regions, KIDS, ids)
    put(the_box, 0x5c, 'I', first)
    regions[0x7ca098] = struct.pack('<I', BOX)
    regions[0x7ca090] = struct.pack('<I', r.choice([0, OPEN, OPEN, OPEN]))
    regions[0x7ca094] = struct.pack('<I', r.choice([0, OPEN, r.getrandbits(32)]))
    regions[0x7cb2c4] = r.bytes(4)
    message = r.deal([0x1d, 0x1d, 0x1001d9, 0x1001d9, 0x1001d9, 0x1001d8, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x1c, 0x101, 0x1001d7, 0x1001da, r.getrandbits(32)])
    a = r.choice([1, 0, 0x10d30, 0x10d2f, 0x10d2e, 0x10d31, 0x10d2d, r.getrandbits(32)]) if message != 0x100 else r.choice([0x10d30, 0x10d30, 0x10d2f, 0x10d2f, 0x10d2e, 0x10d2e, 0x10d31, 0x10d2d])
    return Call(0x48e2f0, [BOX if r.random() < 0.5 else WINDOW, message, a, r.getrandbits(32)], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', 'Action,Action,Handler,Handler,Handler,Cancel').split(',') if k]

if __name__ == '__main__':
    B.main(case)
