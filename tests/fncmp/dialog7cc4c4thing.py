# Dialog7cc4c4_ShowThing (0x4b9980) and Dialog7cc4b4_SetFlag (0x4b9840) (src/game/ui/dialogs) against the original. Window_FindChild real over a list of children, each with the one vtable: slot 9 and slot 5 recorder 10 (ShowThing's text pointer, SetFlag's word),
# Window_SetShown recorder 11 (driver.cpp's kDialog7cc4b4FlagEnv), the text formatter the log stub with its number argument hashed (text_args()). The original's SetFlag pushes ESI with the byte in its low word and the caller's upper half: logged as words.
# usage: dialog7cc4c4thing.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=ShowThing,SetFlag)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _cdecl_log4, _recorder, log_region, put, recorder, text_args, vtable  # noqa: E402

B = Bench([(0x4b9980, 0x4b9a6b), (0x4b9840, 0x4b98ad), (0x65d7bc, 0x65d7d0), (0x65d7d0, 0x65d840)], hooks={0x6acd60: _cdecl_log4, 0x65d9dd: _recorder(11, 1)})
WINDOW, THING, VT, KIDS = RAM + 0x100, RAM + 0x300, RAM + 0x500, RAM + 0x1000


def windows(r, ids):
    """The dialog with the children `ids`, shuffled, with some others among them."""
    ids = list(ids) + [r.getrandbits(32) & 0xfffffff0 for _ in range(r.randrange(3))]
    r.shuffle(ids)
    kids = bytearray()
    for k, kid_id in enumerate(ids):
        kid = bytearray(r.bytes(0x100))
        put(kid, 0, 'I', VT)
        put(kid, 0x8c, 'I', kid_id)
        put(kid, 0x54, 'I', 0 if k == len(ids) - 1 else KIDS + 0x100 * (k + 1))
        kids += kid
    return {KIDS: bytes(kids), WINDOW: bytes(put(bytearray(r.bytes(0x100)), 0x5c, 'I', KIDS)), VT: put(put(vtable({}, size=0x10), 0x24, 'I', recorder(10, 1)), 0x14, 'I', recorder(10, 1))}


def show_case(r):
    log = log_region()
    text_args(log, 1)
    thing = r.choice([THING, THING, THING, 0])
    regions = windows(r, [0x16c11, 0x16c12])
    regions.update({LOG: log, THING: r.bytes(0x60), 0x7cc4c4: struct.pack('<I', r.choice([WINDOW, WINDOW, WINDOW, WINDOW, 0])), 0x7ba058: r.bytes(0x20), 0x78a374: r.bytes(4), 0x78ba94: r.bytes(4),
                    0x7cc4e4: struct.pack('<I', r.choice([THING, 0, 0, r.getrandbits(32)]))})
    return Call(0x4b9980, [thing], regions, ret='v')


def flag_case(r):
    log = log_region()
    for k in (10, 11):
        struct.pack_into('<I', log, LOG_BYTES + 4 * k, 1 << 16)
    regions = windows(r, [0x1e0f0, 0x1e0f1, 0x1e0ea, 0x1e0ec])
    regions.update({LOG: log, 0x7cc4b4: struct.pack('<I', r.choice([WINDOW, WINDOW, WINDOW, WINDOW, 0]))})
    return Call(0x4b9840, [r.choice([0, 1, 2, 0x100, 0xff, 0x80, r.getrandbits(32)])], regions, ret='v')


CALLS = {'ShowThing': show_case, 'SetFlag': flag_case}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def case(r):
    return CALLS[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)
