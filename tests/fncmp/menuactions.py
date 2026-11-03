# The UI's one-call entries (src/game/ui/menuactions) against the originals: every call a recorder numbered as MenuActionsEnv: 16-31, then 0-12 (their own benches have them); the objects' slots vtable recorders 0-15, the callback [0x7cca4c] recorder 45.
# usage: menuactions.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=the addresses in hex)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, log_region, recorder, set_return, vtable  # noqa: E402

D = {0x4acc70: (16, 2), 0x49bdd0: (17, 1), 0x495aa0: (18, 2), 0x496620: (19, 2), 0x493530: (20, 1), 0x4a52a0: (21, 0), 0x49c970: (22, 1), 0x48ee50: (23, 1), 0x493800: (24, 1), 0x658d9f: (28, 4), 0x4a2590: (29, 1), 0x486b00: (30, 1), 0x4a25f0: (31, 1), 0x486b40: (0, 1),
     0x48c940: (4, 1), 0x5276d0: (5, 1), 0x52f200: (6, 2), 0x524960: (7, 4), 0x488ba0: (11, 4)}
T = {0x508380: (25, 1), 0x506830: (26, 1), 0x506860: (27, 0), 0x4df300: (1, 0), 0x4df390: (2, 0), 0x4dfe30: (3, 1), 0x5d6060: (8, 0), 0x48caf0: (9, 0), 0x48cbe0: (10, 0), 0x65e705: (12, 1)}
# address: (the end past its last instruction, its convention, the number of stack arguments, the answer kept)
CALLS = {0x4816e0: (0x4816f3, 'c', 0, 'v'), 0x481700: (0x48170b, 'c', 0, 'v'), 0x481710: (0x481715, 'c', 0, 'v'), 0x481720: (0x48172d, 'c', 0, 'v'), 0x481730: (0x48173d, 'c', 0, 'v'), 0x481740: (0x48174b, 'c', 0, 'v'),
         0x481550: (0x481560, 's', 1, 'v'), 0x4816a0: (0x4816b0, 's', 1, 'v'), 0x4937e0: (0x4937f3, 'c', 1, 'a'), 0x4c9e40: (0x4c9e53, 's', 1, 'a'), 0x4b60d0: (0x4b60e7, 's', 1, 'v'), 0x4b60f0: (0x4b6102, 'c', 0, 'a'),
         0x4b8510: (0x4b851d, 'c', 0, 'a'), 0x4c4470: (0x4c447d, 'c', 0, 'a'), 0x4a4580: (0x4a4595, 'c', 0, 'v'), 0x4bf8e0: (0x4bf8f5, 'c', 0, 'v'), 0x4b9c40: (0x4b9c55, 'c', 0, 'v'), 0x4bc640: (0x4bc654, 'c', 2, 'a'),
         0x4b9f60: (0x4b9f6c, 's', 2, 'a'), 0x4b4650: (0x4b4661, 't', 0, 'a'), 0x4d0690: (0x4d06a7, 't', 0, 'a'), 0x498bb0: (0x498bce, 'c', 0, 'v'), 0x498bd0: (0x498be8, 'c', 0, 'v'), 0x48ccf0: (0x48cd0e, 't', 1, 'v'),
         0x48ce50: (0x48ce75, 't', 1, 'v'), 0x48cde0: (0x48ce42, 'c', 1, 'a'), 0x48d200: (0x48d28b, 'c', 4, 'a'), 0x4c07d0: (0x4c0825, 'c', 1, 'v'), 0x48cf90: (0x48cf9f, 'c', 0, 'a')}
OPS = [int(k, 16) for k in os.environ.get('OPS', '').split(',') if k] or list(CALLS)
hooks = {a: _recorder(k, n, cdecl=True) for a, (k, n) in D.items()}
hooks.update({a: _recorder(k, n) for a, (k, n) in T.items()})
hooks[0x4e2780] = _recorder(22, 2)   # (0x48cf90's real Thing_NameBuffer7ba058: the name's copy, Dialog7ca210Env's)
B = Bench(sorted((a, v[0]) for a, v in CALLS.items()) + [(0x486280, 0x4862a0)], hooks=hooks)
OBJECT, VT, PART8, PART10, GAME, PANEL, THING, WINDOW = RAM, RAM + 0x400, RAM + 0x500, RAM + 0x540, RAM + 0x600, RAM + 0x700, RAM + 0x800, RAM + 0x900


def case(r):
    op = r.deal(OPS)
    end, conv, count, ret = CALLS[op]
    log = log_region()
    regions = {LOG: log, VT: vtable({}, size=16)}
    struct.pack_into('<I', log, LOG_BYTES + 4 * 16, 0b10 << 16)   # (Dialog7cc1f8_Open's selected: a word on the stack in 0x4816e0)
    for k in (10, 11, 25, 26, 27, 29, 28):
        set_return(log, k, r.getrandbits(32))
    set_return(log, 12, PANEL)
    regions[OBJECT] = struct.pack('<I', VT) + r.bytes(0x1fc)
    regions[PANEL] = struct.pack('<I', VT) + r.bytes(0x3c)
    regions[PART8] = struct.pack('<I', VT) + r.bytes(4)
    regions[PART10] = struct.pack('<I', VT) + r.bytes(4)
    game = bytearray(r.bytes(0x20))
    struct.pack_into('<I', game, 8, PART8)
    struct.pack_into('<I', game, 0x10, PART10)
    regions[GAME] = game
    regions[0xf85614] = struct.pack('<I', GAME)
    regions[WINDOW] = r.bytes(8)
    regions[0xf82884] = struct.pack('<I', WINDOW)
    regions[0x7c2658] = struct.pack('<I', r.choice([THING, THING, THING, 0]))
    regions[THING] = bytes(5) + bytes([r.getrandbits(8)]) + bytes(1) + bytes([r.choice([0, 1, 0x7f, 0x80, 0xff, r.getrandbits(8)])])
    regions[0x80239c] = r.bytes(4)
    for at in (0x7cb2fc, 0x7cc8bc, 0x7cc4c4, 0x7cc684, 0x7cc65c, 0x7cc504, 0x7ca1e0, 0x7cc984, 0x7cca50):
        regions[at] = r.bytes(4)
    regions[0x7cca4c] = struct.pack('<I', recorder(13, 2, cdecl=True))
    arguments = [r.choice([0, 1, r.getrandbits(32)]) for _ in range(count)]
    if op == 0x48cde0:
        arguments = [THING]
    elif op == 0x48d200:
        arguments = [WINDOW, r.choice([0x14, 0x14, 0x1000a, 0x1000a, 0x1000a, 0x1000b, r.getrandbits(32)]), r.choice([0x2600, 0x2600, 0x2800, 0x2800, 0x2700, r.getrandbits(32)]), r.getrandbits(32)]
    elif op == 0x48ce50:
        arguments = [r.choice([0, 1, 0x100, 0x12300, r.getrandbits(32)])]
    if conv == 't':
        arguments = [PANEL if op in (0x48ccf0, 0x48ce50) else OBJECT] + arguments
    return Call(op, arguments, regions, conv=conv, ret=ret)


if __name__ == '__main__':
    B.main(case)
