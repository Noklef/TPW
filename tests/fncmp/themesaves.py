# The theme's save file and a level's start (src/game/core/themesaves: 0x40bf90, 0x40c040, 0x550b30; src/game/core/namedobjects: 0x407e00) against the originals. The strings and the heap real; MultiByteToWideChar and WideCharToMultiByte
# recorders (the narrow text hashed, the wide text written back and hashed where UserSaves gets it); the saves, the theme folder, the local stream's calls and the level's steps recorders. The stream's vtable is not seen by a recorder.
# usage: themesaves.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob_bytes, echo, fill, global_heap, heap, import_table, log_region, mask_this, set_return, strings  # noqa: E402
import gamestring  # noqa: E402

RELOAD, START, LOADDATA, ENVIRONMENT, LOADA, SAVEA, SAVEB, THEME, SCTOR, SDTOR, SCLOSE, OPENPATH = 10, 11, 13, 14, 12, 15, 16, 17, 18, 19, 20, 21
W2M, M2W = 40, 41
SINGLETON = 27
B = Bench([(0x40bf90, 0x40c0f0), (0x407e00, 0x407f17), (0x550b30, 0x550c6f), (0x5c5520, 0x5c5523), (0x424dc0, 0x424e10), (0x424e10, 0x424e6e), (0x5da3c0, 0x5da3c1), (0x5f5660, 0x5f5690), (0x5f56f0, 0x5f5710)] + gamestring.B_RANGES,
          hooks={0x413140: _recorder(RELOAD, 1), 0x5156a0: _recorder(START, 1), 0x407f20: _recorder(LOADDATA, 0), 0x4080e0: _recorder(ENVIRONMENT, 0), 0x5ac5d0: _recorder(LOADA, 1), 0x5ac610: _recorder(SAVEA, 1), 0x5ac630: _recorder(SAVEB, 1),
                 0x5c8890: _recorder(THEME, 1), 0x5f7180: _recorder(SCTOR, 0), 0x5f71e0: _recorder(SDTOR, 0), 0x5f71f0: _recorder(SCLOSE, 0), 0x5f7ed0: _recorder(OPENPATH, 3), 0x5c7420: _recorder(SINGLETON, 0)})
SLOTS, STATUS, SELF, LABEL_T, FOLDER_T, GLOBALS_T = (RAM + a for a in (0, 0x100, 0x200, 0x300, 0x380, 0x400))
OPS = ['Save', 'Load', 'Ensure', 'Start', 'Start']
OWNER = 0x7cf83c


def wide(text):
    return text.encode('utf-16-le')


def case(r):
    op = r.deal(OPS)
    log = log_region()
    table = import_table(log, {'WideCharToMultiByte': (W2M, 0x6fd1dc, 8, 0, 0), 'MultiByteToWideChar': (M2W, 0x6fd174, 6, 1 << 2, 0)})
    for at, stub in global_heap().items():
        table[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(0x4000, 0), gamestring.LOCALE: bytes(4), SLOTS: r.bytes(0x10), STATUS: struct.pack('<ii', r.randrange(0, 5), -r.randrange(1, 5))}
    # The wide text converted, written back; the name as UserSaves sees it.
    units = r.choice([b'x\0\0\0', b'xy\0\0', b'z\0y\0\0\0'])
    set_return(log, M2W, len(units) // 2)
    fill(log, M2W, 4, units)
    for k in (LOADA, SAVEA):
        blob_bytes(log, k, 0, 0xff, inner=(8, 0x3f))
    label = r.choice([b'a', b'ab', b'thm', b''])
    if op in ('Save', 'Load'):
        gamestring.string(r, 0, 0x786b68, LABEL_T, regions, label)
        return Call(0x40bf90 if op == 'Save' else 0x40c040, [], regions, ret='a')
    if op == 'Ensure':
        regions[0xf81ebc] = struct.pack('<I', SLOTS if r.random() < 0.8 else 0)
        set_return(log, SINGLETON, SLOTS)
        n = r.choice([1, 2, 3])
        regions[FOLDER_T] = wide('fold'[:n]) + bytes(2) + r.bytes(0x20)
        fill(log, THEME, 0, struct.pack('<5I', gamestring.VT[1][0], n, FOLDER_T, 8, 8))
        strings(log, THEME, 0, out=1)
        echo(log, THEME, 0)
        for k in (SCTOR, SDTOR, SCLOSE, OPENPATH):
            mask_this(log, k)
        for i, base in enumerate((0x878f18, 0x878a68, 0x877f38)):
            gamestring.string(r, 1, base, GLOBALS_T + 0x60 * i, regions, wide(r.choice(['\\', '.a', 'b', ''])))
        narrow = b'p' * r.randrange(1, 6) + b'\\x'
        set_return(log, W2M, len(narrow) + 1)
        fill(log, W2M, 4, narrow + b'\0', which=1)
        blob_bytes(log, W2M, 2, 0xffffffff)
        strings(log, OPENPATH, 1 << 1, out=1)
        set_return(log, OPENPATH, STATUS + r.choice([0, 4]))
        return Call(0x550b30, [], regions, ret='v')
    regions[OWNER] = struct.pack('<I', r.getrandbits(32))
    set_return(log, RELOAD, r.choice([1, 1, 1, 1, 0, 0x100]))
    set_return(log, START, r.choice([1, 1, 1, 1, 0, 0x100]))
    set_return(log, LOADDATA, r.choice([1, 1, 0, 0x100]))
    set_return(log, ENVIRONMENT, r.choice([1, 0, 0x100]))
    regions[0x788160] = struct.pack('<I', r.choice([0, 1, 7])) + label + b'\0'
    return Call(0x407e00, [RAM + 0x280, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
