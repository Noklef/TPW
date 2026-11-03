# A list item's texts set (src/game/ui/window7059f0texts: Window7059f0_SetItemTexts 0x6644ea, WString_AssignTextOf 0x665f10, WString_AssignText 0x665f80) against the originals. Window7059f0_ColumnBottom and the text's length real, recorders (driver.cpp's kWindow7059f0TextsEnv): 0x4051d0 10, _NthItem 11, _RefreshRows 12.
# usage: window7059f0texts.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return  # noqa: E402

B = Bench([(0x6644ea, 0x664605), (0x665f10, 0x665f27), (0x665f60, 0x665f71), (0x665f80, 0x665fa4), (0x6636f6, 0x663718), (0x67acd0, 0x67acee)], hooks={0x4051d0: _recorder(10, 2), 0x663ba2: _recorder(11, 1), 0x663324: _recorder(12, 0)})
WINDOW, ITEMS, COLUMNS, TEXTS, CHARS, STRINGS = RAM + 0x100, RAM + 0x400, RAM + 0x800, RAM + 0xa00, RAM + 0xc00, RAM + 0x1000


def text(r, regions):
    chars = [r.randrange(1, 0x10000) for _ in range(r.choice([0, 1, 2, 5, 31, 40]))]
    regions[CHARS] = bytes(r.bytes(2)) + struct.pack('<%dH' % (len(chars) + 1), *chars, 0) + r.bytes(6)
    return CHARS + 2


def case(r):
    log = log_region()
    regions = {LOG: log}
    set_return(log, 10, r.getrandbits(32))
    op = r.deal([0, 0, 0, 0, 1, 2])
    if op:
        return Call(0x665f10 if op == 1 else 0x665f80, [STRINGS, text(r, regions)], regions, conv='t', ret='a')
    text(r, regions)
    count = r.choice([0, 1, 2, 3, 4, 6, -1])
    window = bytearray(r.bytes(0x180))
    put(window, 0x144, 'I', r.choice([ITEMS] * 8 + [0]))
    put(window, 0x150, 'i', r.choice([5, 5, 5, 0, 3, -1]))
    put(window, 0x164, 'h', count)
    put(window, 0x16c, 'I', COLUMNS)
    regions[WINDOW] = window
    cols = bytearray(r.bytes(0xc * 8))
    for c in range(8):
        put(cols, 0xc * c + 0xa, 'h', r.choice([0, 0, 0, 1, -1]))
    regions[COLUMNS] = cols
    regions[ITEMS] = b''.join(struct.pack('<I', r.choice([STRINGS + 0x10 * r.randrange(4)] * 3 + [0, r.getrandbits(32)])) for _ in range(0x40))
    regions[TEXTS] = b''.join(struct.pack('<I', r.choice([CHARS + 2] * 4 + [0])) for _ in range(8))
    set_return(log, 11, r.choice([0, 1, 4, 8, 12, -1, -8]) & 0xffffffff)
    mask = r.choice([0, 1, 3, 5, 0xffffffff, r.getrandbits(32), r.getrandbits(32) & 0xf])
    return Call(0x6644ea, [WINDOW, r.choice([0, 1, 4, 5, 7, -1 & 0xffffffff]), r.choice([TEXTS] * 9 + [0]), mask, r.choice([0, 1, 2]), r.getrandbits(32)], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
