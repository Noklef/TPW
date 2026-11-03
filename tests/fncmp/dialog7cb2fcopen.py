# The options dialog (src/game/ui/dialog7cb2fcopen: 0x4a2e90 OnSlider, 0x4a3480 OnButton, 0x4a2bf0 Handler, 0x4a3a30 Open) against the originals. Window_FindChild, the options' setters and the CRT's
# copies real (swprintf crt.py's hook); every other callee a recorder as driver.cpp's kDialog7cb2fcOpenEnv. The dialog a tree of every row with its child 0x1d4c9 (their slots 7 and 9 recorders), among
# dialog7cb398.py's decoys. The texts one short wide string (up to 8 units, which fit the stack's 10; the rows without a value 40 too), the wide buffer 0x7ba058 long enough for all the appends.
# The toggle's on / other / select and SetState's arguments are BYTEs (LOG_BYTES): the pushed registers' upper bytes are not the callee's.
# usage: dialog7cb2fcopen.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Slider,Button,Handler,Open)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, log_fold, log_region, put, set_return, vtable  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
from crt import swprintf_hook  # noqa: E402
from dialog7cb398 import KVTABLE, NODE, children, window  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

TEXT, VALUE, TOGGLE, ABC, AC0, AC4, F5996, SEND, ATTACH, DRESS, SLIDER, FOCUS, SLOT3, SLOT4, PARTICLES, F662 = range(10, 26)
STATE, F4092, F4093, FLAGS, SET_FLAGS, RELEASE, BUILD, DEPTH, DEFAULT, F4237, REVERT = 26, 27, 28, 29, 30, 31, 8, 6, 5, 4, 3
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
CALLS = {'Slider': 0x4a2e90, 'Button': 0x4a3480, 'Handler': 0x4a2bf0, 'Open': 0x4a3a30}
B = Bench([(0x4a2bf0, 0x4a33a8), (0x4a3480, 0x4a3900), (0x4a3a30, 0x4a43a3), (0x65d7bc, 0x65d832), (0x67b0d0, 0x67b140), (0x67bed0, 0x67bf0f), (0x423b00, 0x423bb0), (0x423720, 0x423724), (0x423730, 0x423734),
           (0x5b6480, 0x5b6484), (0x619160, 0x619164), (0x662411, 0x662420), (0x5da3c0, 0x5da3c1)],
          hooks={0x67af00: swprintf_hook, 0x485b00: C(TEXT, 1), 0x4a33e0: C(VALUE, 3), 0x4a3960: C(TOGGLE, 6), 0x51bba0: C(ABC, 1), 0x51bb80: C(AC0, 1), 0x51bbc0: C(AC4, 1), 0x5996d0: C(F5996, 1), 0x658d9f: C(SEND, 4),
                 0x485d20: C(ATTACH, 3), 0x4a43b0: C(DRESS, 1), 0x4a4490: C(SLIDER, 2), 0x4862a0: C(FOCUS, 0), 0x65961c: C(SLOT3, 0), 0x659607: C(SLOT4, 0), 0x5eda70: C(PARTICLES, 0), 0x662420: C(F662, 0),
                 0x668bec: _recorder(STATE, 2), 0x4092a0: _recorder(F4092, 2), 0x409300: _recorder(F4093, 0), 0x576400: _recorder(FLAGS, 0), 0x5763f0: _recorder(SET_FLAGS, 1), 0x58dd60: _recorder(RELEASE, 1),
                 0x65fd0e: _recorder(BUILD, 2), 0x65f16b: _recorder(DEPTH, 1), 0x65f66e: _recorder(DEFAULT, 3), 0x4237f0: _recorder(F4237, 0), 0x423ad0: _recorder(REVERT, 0)})
DIALOG, ROWS, ITEMS, DECOYS, WORDS, OWNER, FRAME = 0x33200000, 0x33300000, 0x33400000, 0x33500000, RAM + 0x1000, RAM + 0x2000, RAM + 0x2100
IDS = [0x1d4c1 + i for i in range(0x20)] + [0xffffffff, 0xfffffffe]
TEXT_ROWS = {0x1d4c6, 0x1d4c7, 0x1d4d3}


def tree(r, regions):
    kids = []
    for i, ident in enumerate(IDS):
        row, item = ROWS + i * NODE, ITEMS + i * NODE
        window(r, regions, item, row, 0)
        window(r, regions, row, DIALOG, 0)
        put(regions[row], 0x5c, 'I', children(r, regions, [(item, 0x1d4c9)], row, DECOYS + 0x4000 * (i + 1), [0, 0x1d4c8, 0x1d4ca]))
        kids.append((row, ident))
    window(r, regions, DIALOG, 0, 0)
    put(regions[DIALOG], 0x5c, 'I', children(r, regions, kids, DIALOG, DECOYS, [0, 0x1d4c9]))
    regions[KVTABLE] = vtable({7: 1, 9: 1})


def value(r):
    return r.choice([-1, -5, -0x80000000, 0, 1, 50, 99, 100, r.randrange(1000000)]) & 0xffffffff


def case(r):
    log = log_region()
    log_fold(log)   # (Open makes more calls than the log's 40 entries)
    regions = {LOG: log}
    tree(r, regions)
    op = r.deal(OPS)
    put(log, LOG_BYTES + 4 * TOGGLE, 'I', 0x1c)   # (on, other, select)
    put(log, LOG_BYTES + 4 * STATE, 'I', 3)
    options = bytearray(r.bytes(0x100))
    put(options, 0, 'I', r.deal([0, 0, 1, 1, r.getrandbits(32)]))
    for at in (4, 8):
        put(options, at, 'I', r.deal([0, 1, 2, 3, r.getrandbits(32)]))
    put(options, 0xc, 'I', r.deal([0, 1, r.getrandbits(32)]))
    for at in (0x14, 0x1c, 0x24, 0x2c, 0x30):
        put(options, at, 'I', value(r))
    for at in (0x10, 0x18, 0x20, 0x28, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a):
        options[at] = r.choice([0, 0, 1, 1, r.getrandbits(8)])
    regions[0x78d8d8] = options
    regions[0x879a24] = struct.pack('<I', OWNER)
    regions[OWNER] = put(bytearray(r.bytes(0x800)), 0x788, 'i', r.deal([0, 1, 2, 3, -1]))
    regions[0xf82884] = struct.pack('<I', r.deal([0, FRAME, FRAME]))
    regions[FRAME] = r.bytes(8)
    regions[0x7cb2ac] = r.bytes(4)
    regions[0x87a6d0] = r.bytes(4)
    regions[0x8c4b44] = r.bytes(4)
    regions[0xfaa5d8] = r.bytes(4)
    regions[0x786ba4] = struct.pack('<I', r.deal([0, 0, 1, 1, 2]))
    regions[0x7cb2ec] = bytes([r.choice([0, 0, 1, r.getrandbits(8)])])
    regions[0x7cb2f4] = r.bytes(16)
    set_return(log, FLAGS, r.getrandbits(32))
    set_return(log, BUILD, DIALOG)
    set_return(log, DEFAULT, r.getrandbits(32))
    set_return(log, 7, r.deal([0, 0, 1, 0x10000, 0x10001]))
    # the text and the buffer
    args, ident = [], 0
    if op == 'Slider':
        ident = r.deal([0x1d4d5, 0x1d4d6, 0x1d4d8, 0x1d4da, 0x1d4dc, 0x1d4de, 0x1d4e0, 0x1d4d7, 0x1d4e1, 0])
        args = [ident, value(r)]
    elif op == 'Button':
        ident = r.deal(list(range(0x1d4c1, 0x1d4c8)) + [0x1d4d2] * 6 + [0x1d4d3, 0x1d4d7, 0x1d4d9, 0x1d4db, 0x1d4dd, 0x1d4c0, 0x1d4c8, 0x1d4d4, 0x1d4d8, 0x1d4de, 0x1d4e0])
        args = [ident, r.deal([0, 1, 1, 2, r.getrandbits(32)])]
    elif op == 'Handler':
        message = r.deal([0x14, 0x100, 0x100, 0x100, 0x800, 0x801, 0x801, 0x801, 0x802, 0x13, 0xff, 0x101, 0x7ff, 0x803, r.getrandbits(32)])
        a = r.deal(list(range(0x1d4d7, 0x1d4e2)) + [0xffffffff, 0xfffffffe, 0x1d4c1, 0x1d4d2])
        args = [RAM + 0x3000, message, a, value(r)]
    units = r.choice([0, 1, 4, 8])
    if ident in TEXT_ROWS and r.random() < 0.4:
        units = 40
    regions[WORDS] = b''.join(r.randrange(0x20, 0x7f).to_bytes(2, 'little') for _ in range(units)) + b'\0\0'
    set_return(log, TEXT, WORDS)
    first = b''.join(r.randrange(0x20, 0x7f).to_bytes(2, 'little') for _ in range(r.choice([0, 1, 4, 30])))
    regions[0x7ba058] = first + b'\0\0' + r.bytes(0x1000)
    regions[0x7cb2fc] = struct.pack('<II', DIALOG if op != 'Open' or r.random() < 0.05 else 0, r.getrandbits(32))
    return Call(CALLS[op], args, regions, ret='a' if op == 'Handler' else 'v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
