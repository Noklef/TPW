# The objects' lists dialog's rows (src/game/ui/dialog7ca150rows: 0x493cd0-0x4941e0) against the originals: Object_GetDef 10, Window7059f0_AddItem 11 (its texts hashed), FindItemData 12, Object_Rating 13, Field48AsInt 14, HistoryDifference 15, Obj_Diff180_184 16,
# HistoryAverage 17, ObjectControls_GetWord18 18 (its word) and 0x4e2780 22 recorders (their own benches have them); the rest real.
# usage: dialog7ca150rows.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=the addresses in hex)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, blob, log_region, set_return  # noqa: E402

# address: (the end past its last instruction, the texts' mask)
CALLS = {0x493cd0: (0x493e81, 0b11111), 0x493e90: (0x493eab, 0b11111), 0x493eb0: (0x493f26, 0b11111), 0x493f30: (0x49406b, 0b11111), 0x494070: (0x4941d3, 0b111111), 0x4941e0: (0x494243, 0b11)}
OPS = [int(k, 16) for k in os.environ.get('OPS', '').split(',') if k] or list(CALLS)
B = Bench(sorted((a, v[0]) for a, v in CALLS.items()) + [(0x486280, 0x4862a0), (0x519510, 0x519550), (0x4e0df0, 0x4e0dfe), (0x5da3c0, 0x5da3c1)],
          hooks={0x4dd4e0: _recorder(10, 0), 0x66403b: _recorder(11, 4), 0x664d2a: _recorder(12, 1), 0x4e0860: _recorder(13, 1), 0x4dd6d0: _recorder(14, 0), 0x4e1c70: _recorder(15, 0), 0x4e1c60: _recorder(16, 0),
                 0x4e1e30: _recorder(17, 0), 0x4ca8c0: _recorder(18, 1, True), 0x4e2780: _recorder(22, 2)})
THING, FIGURES, LIST, GAME = RAM, RAM + 0x400, RAM + 0x800, 0x31000000


def figures(r):
    """An object: the ring +0x1ac about its bounds, the float +0x44 and the word +0xe, +0x30."""
    o = bytearray(r.bytes(0x400))
    current, count = r.choice([0, 1, 5, 29, 30, 40, -1, -3, r.randrange(50)]), r.choice([0, 1, 5, 29, 30, 31, 40, -1, r.randrange(50)])
    struct.pack_into('<ii', o, 0x224, current, count)
    o[0x22c] = r.choice([0, 0, 1, 2])
    for i in range(30):
        struct.pack_into('<I', o, 0x1ac + 4 * i, r.choice([0, 1, 100, 0xffffffff, 0x80000000, r.getrandbits(32)]))
    struct.pack_into('<f', o, 0x44, r.choice([0.0, 1.5, 99.9, 100.0, 255.0, 256.0, 300.7, -1.5, r.uniform(0, 1000)]))
    return o


def case(r):
    op = r.deal(OPS)
    end, mask = CALLS[op]
    log = log_region()
    blob(log, 11, 0, mask)
    struct.pack_into('<I', log, LOG_BYTES + 4 * 18, 0b1 << 16)   # (the id's upper half: the stack's)
    set_return(log, 11, r.getrandbits(32))
    set_return(log, 12, r.choice([0xffffffff, 0xfffffffe, 0, 1, 7, r.getrandbits(32)]))
    set_return(log, 13, r.choice([0, 1, 50, 100, 0xff, 0x1ff, r.getrandbits(32)]))
    set_return(log, 14, r.choice([0, 1, 50, 100, 0xff, 0x1ff, r.getrandbits(32)]))
    for k in (15, 16, 18):
        set_return(log, k, r.getrandbits(32))
    set_return(log, 17, r.choice([0, 1, 50, 0x32, 0xffffffff, 0xffffff00, 0x7fffffff, 0x80000000, r.getrandbits(32)]))
    thing = figures(r)
    struct.pack_into('<H', thing, 0x30, r.choice([0, 0, 1, 2, 3]))
    regions = {LOG: log, THING: thing, FIGURES: figures(r), LIST: r.bytes(16), 0x7ca14c: struct.pack('<I', LIST), 0x7cf83c: struct.pack('<I', GAME), GAME + 0x1da720: r.bytes(2) + bytes(2)}
    # (the table 0x7cfb90's entries 0-3: a thing's figures or none)
    regions[0x7cfb90] = b''.join(struct.pack('<I', r.choice([FIGURES, FIGURES, 0])) + r.bytes(16) for _ in range(4))
    arguments = [THING]
    if op == 0x493cd0:
        arguments = [THING, LIST, r.choice([FIGURES, FIGURES, 0])]
    return Call(op, arguments, regions, ret='a')


if __name__ == '__main__':
    B.main(case)
