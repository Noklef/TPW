# The dialog [0x7ca150]'s list's handler (src/game/ui/dialog7ca150list: Dialog7ca150_ListHandler 0x4955e0) against the original. The objects' buffers +0x1a8 random and CircBufU32_30_Sum, _Get and _Count real; recorders (driver.cpp's kDialog7ca150ListEnv, in its order from 10): the texts hashed by the tab's own mask, the items' data from fills, the owner's handle from a return.
# usage: dialog7ca150list.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fill, log_region, put, set_return, strings  # noqa: E402

B = Bench([(0x4955e0, 0x4959ad), (0x495c20, 0x495ce4), (0x495cf0, 0x495d3f), (0x495d40, 0x495d56), (0x5da3c0, 0x5da3c1)],
          hooks={0x65ef90: _recorder(10, 2), 0x65f024: _recorder(11, 1), 0x664c71: _recorder(12, 2), 0x6644ea: _recorder(13, 5), 0x4e0df0: _recorder(14, 1), 0x4e0860: _recorder(15, 1), 0x4dd6d0: _recorder(16, 0), 0x4e1c70: _recorder(17, 0), 0x4e1c60: _recorder(18, 0), 0x4e1e30: _recorder(19, 0), 0x65f66e: _recorder(20, 3)})
WINDOW, WORD, OBJECTS = RAM + 0x100, RAM + 0x300, RAM + 0x1000
SIZE = 0x400


def buffer(r, obj, at):
    for k in range(30):
        put(obj, at + 4 + 4 * k, 'I', r.choice([0, 1, 100, 12345, 0xffffffff, 0x1000001, r.getrandbits(32), r.randrange(1000)]))
    put(obj, at + 0x7c, 'i', r.choice([-1, 0, 1, 5, 20, 29]))
    put(obj, at + 0x80, 'i', r.choice([0, 1, 5, 30, 30, 30]))
    put(obj, at + 0x84, 'B', r.choice([0, 1]))


def case(r):
    log = log_region()
    objects = bytearray(r.bytes(SIZE * 3))
    for k in range(3):
        put(objects, SIZE * k + 0x44, 'f', r.choice([0.0, 1.0, 50.5, 99.9, 255.0, 256.0, 1000.5, -1.5, r.uniform(0, 300)]))
        buffer(r, objects, SIZE * k + 0x1a8)
    tab = r.deal([0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 4])
    table = bytearray(r.bytes(20 * 4))
    for k in range(4):
        put(table, 20 * k, 'I', OBJECTS + SIZE * r.randrange(3))
    regions = {LOG: log, OBJECTS: objects, 0x7ca154: struct.pack('<I', tab), 0x7cfb90: table, WORD: struct.pack('<I', r.choice([0, 0, 1, 2, 3]) | r.choice([0, 0x7f000000]))}
    message = r.choice([0x10] * 8 + [0x15, 0x15, 0xf, 0x11, 0x14, 0x16, 0x100])
    a = r.choice([0x80083] * 9 + [0x80082])
    window = bytearray(r.bytes(0x180))
    put(window, 0x150, 'i', r.choice([0, 1, 2, 3, 5, -1]))
    regions[WINDOW] = window
    fill(log, 12, 1, b''.join(struct.pack('<I', OBJECTS + SIZE * r.randrange(3)) for _ in range(8)), records=8)
    strings(log, 12, 0, 1 << 1)
    strings(log, 14, 0, 1 << 0)
    blob(log, 13, 1, 0x3f if tab == 2 else 0x1f)
    set_return(log, 10, r.choice([0, 1, 0x10000, 0x10001]))
    set_return(log, 14, WORD)
    set_return(log, 15, r.getrandbits(32))
    set_return(log, 16, r.getrandbits(32))
    for k in (17, 18, 19, 20):
        set_return(log, k, r.choice([0, 1, 100, 0x400000, r.getrandbits(32)]))
    return Call(0x4955e0, [r.choice([WINDOW] * 15 + [0]), message, a, r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
