# The game's start and its first movies (src/game/core/gamestart: 0x54dcf0, 0x54df20, 0x54e070) against the originals. The strings and the heap real; every other callee and the two imports recorders (the path's text hashed, the movie's found name written and hashed); the options' globals varied.
# usage: gamestart.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob_bytes, fill, global_heap, heap, import_table, log_region, set_return, strings  # noqa: E402
import gamestring  # noqa: E402

LOCALTIME, FREEZE, FOLDERS, AUTH, ADDR, GET0, PARSE, ERROR, KEYS, TIME, SPRINTF, SRAND, SOUND, CAMERA, G802, S8, S0, S4, SC, CATS, PLAY, APPLY, TV, LANG, SHOW, GUI, CLOSE, CLOCK, ADVISOR, DISPLAY, ARCHIVE, MOVIE = range(32)
TICKS, CLIENT = 40, 41
CD = {0x67d8e0: (LOCALTIME, 1), 0x4688b0: (FOLDERS, 1), 0x5d55a0: (AUTH, 0), 0x40f000: (PARSE, 2), 0x40cb80: (KEYS, 0), 0x67d3f0: (TIME, 1), 0x67a650: (SPRINTF, 5), 0x67b5b0: (SRAND, 1), 0x51b660: (SOUND, 2), 0x51b920: (CAMERA, 1), 0x51b3d0: (G802, 0), 0x51bb60: (S8, 1),
      0x51bb80: (S0, 1), 0x51bbc0: (S4, 1), 0x51bba0: (SC, 1), 0x51eae0: (CATS, 0), 0x51bfc0: (PLAY, 6), 0x51bd70: (APPLY, 0), 0x53e920: (TV, 0), 0x5989c0: (ADVISOR, 0), 0x583790: (DISPLAY, 0), 0x5c5110: (ARCHIVE, 2), 0x51b010: (MOVIE, 5)}
TH = {0x402ea0: (FREEZE, 1), 0x4056f0: (ADDR, 0), 0x4056e0: (GET0, 0), 0x405750: (ERROR, 0), 0x41a820: (LANG, 2), 0x587380: (SHOW, 1), 0x4813c0: (GUI, 0), 0x5879c0: (CLOSE, 0), 0x402d70: (CLOCK, 0)}
HOOKS = {a: _recorder(k, n, cdecl=True) for a, (k, n) in CD.items()}
HOOKS.update({a: _recorder(k, n) for a, (k, n) in TH.items()})
B = Bench([(0x54dcf0, 0x54df17), (0x54df20, 0x54e064), (0x54e070, 0x54e2a8), (0x5da3c0, 0x5da3c1), (0x5f5660, 0x5f5690), (0x5f56f0, 0x5f5710)] + gamestring.B_RANGES, hooks=HOOKS)
TM, FOUND_T = RAM, RAM + 0x40
OPS = ['Start', 'Logo', 'Day', 'Day']
# The globals read, each dealt: every dword as a sound level or a flag word.
START_GLOBALS = [0x79d86c, 0x78591c, 0x78592c, 0x785930, 0x785928, 0x803a2c, 0x875030, 0x875034, 0x878128, 0x878d64, 0x879068, 0x87906c]


def case(r):
    op = r.deal(OPS)
    log = log_region()
    table = import_table(log, {'timeGetTime': (TICKS, 0x6fd1d0, 0, 0, 0), 'ClientToScreen': (CLIENT, 0x6fd318, 2, 0, 0)})
    for at, stub in global_heap().items():
        table[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(0x4000, 0), gamestring.LOCALE: bytes(4), 0x7a1a8c: r.bytes(4), 0x7a1930: struct.pack('<I', r.choice([480, 600, 768, 0x4000000, r.getrandbits(32)])), 0x7a130c: r.bytes(4), 0x8782d0: r.bytes(0x20) + b'\0'}
    if op == 'Start':
        for a in START_GLOBALS:
            regions[a] = struct.pack('<I', r.getrandbits(32) if r.random() < 0.7 else r.choice([0, 1, -7 & 0xffffffff, 0x800]))
        regions[0x78d968] = struct.pack('<I', r.choice([0, 0, 1]))
        regions[0x785874] = struct.pack('<I', r.choice([0, 1, 2, 3, 0xffffffff]))
        regions[0x78d8e0] = struct.pack('<I', r.choice([0, 1]))
        regions[0x790ab0] = struct.pack('<B', r.choice([0, 1, 2, 4, 0x10, 8, 0xe9]))
        regions[0x878e84] = r.bytes(1)
        for k in (ADDR, GET0, LANG, CLOCK, AUTH):
            set_return(log, k, r.getrandbits(32))
        set_return(log, PARSE, r.choice([1, 1, 1, 1, 1, 0, 0x100]))
        set_return(log, TICKS, r.getrandbits(32))
        fill(log, TIME, 0, r.bytes(4))
        strings(log, TIME, 0, out=1)
        strings(log, SPRINTF, 0, out=1)
        return Call(0x54dcf0, [], regions, ret='v')
    # The path's text as the archive sees it; the name it finds as the movie gets it.
    set_return(log, ARCHIVE, r.choice([1, 1, 1, 0, 2, 0x101]))
    blob_bytes(log, ARCHIVE, 0, 0xff, inner=(8, 0xffff))
    regions[FOUND_T] = b'found.tgq\0'
    fill(log, ARCHIVE, 1, struct.pack('<5I', gamestring.VT[0][0], 9, FOUND_T, 9, 9))
    strings(log, ARCHIVE, 0, out=1 << 1)
    strings(log, MOVIE, 1)
    blob_bytes(log, CLIENT, 1, 0xff)
    fill(log, CLIENT, 1, struct.pack('<ii', r.randrange(-50, 2000), r.randrange(-50, 2000)), which=1)
    if op == 'Logo':
        return Call(0x54df20, [], regions, ret='v')
    regions[TM] = struct.pack('<4i', 0, 0, 0, r.choice(list(range(1, 32)) + [0, -1, -8, -9]))
    set_return(log, LOCALTIME, TM)
    strings(log, LOCALTIME, 0, out=1)
    strings(log, TIME, 0, out=1)
    fill(log, TIME, 0, r.bytes(4), which=2)
    return Call(0x54e070, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
