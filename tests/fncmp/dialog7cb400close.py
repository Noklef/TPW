# The dialog [0x7cb400]'s end (src/game/ui/dialog7cb400close: Dialog7cb400_Commit 0x4a6b80, Dialog7cb400_Close 0x4a6a50, Dialog7cb400_Handler 0x4a6d00; src/game/files/userslots: UserSlots_Instance 0x48bcd0) against the
# originals. Window_FindChild, Window_DwordA0, Fn_65d55a, Fn_66a281 and the wide string real, the heap heap()'s; the rest recorders from 10 (driver.cpp's kDialog7cb400CloseEnv, kUserSlotsEnv), the wide string told to 0x5c7f40 hashed by its
# length and its first 15 bytes (blob_bytes).
# usage: dialog7cb400close.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from dialog7cb400 import kids  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, blob_bytes, heap, log_region, put, set_return  # noqa: E402

KEYS = {0x658d9f: _recorder(10, 4, cdecl=True), 0x65e59b: _recorder(11, 1), 0x65d9dd: _recorder(12, 1), 0x40cf60: _recorder(13, 0, cdecl=True), 0x5d6060: _recorder(14, 0), 0x550ca0: _recorder(15, 0),
        0x67a7d0: _recorder(16, 1, cdecl=True), 0x409410: _recorder(17, 0, cdecl=True), 0x5afc30: _recorder(18, 0), 0x5c8a10: _recorder(19, 0), 0x5d6110: _recorder(20, 2), 0x5e1fa0: _recorder(21, 1),
        0x5e2010: _recorder(22, 0), 0x4b90f0: _recorder(23, 0, cdecl=True), 0x47ee90: _recorder(24, 4, cdecl=True), 0x5c7f40: _recorder(25, 3), 0x5c83b0: _recorder(26, 1), 0x5c7420: _recorder(27, 0)}
B = Bench([(0x48bcd0, 0x48bd3d), (0x4a6a50, 0x4a6b77), (0x4a6b80, 0x4a6cfc), (0x4a6d00, 0x4a6e3a), (0x5f6980, 0x5f6a30), (0x5f6a60, 0x5f6a90), (0x5f7800, 0x5f7840), (0x65d55a, 0x65d57b), (0x65d7bc, 0x65d832), (0x65e705, 0x65e73d),
           (0x66a281, 0x66a2a5), (0x67acd0, 0x67ad00), (0x67bf10, 0x67bf50)], hooks=KEYS)
DIALOG, SLOTS, MAIN, SCENE, KIDS = RAM + 0x100, RAM + 0x300, RAM + 0x600, RAM + 0x700, RAM + 0x1000
TEXTS = ['', 'a', 'name', ' ', '    ', 'ab  ', ' ab', ' a b ', 'seven77', 'seven7 ', 'x' * 20 + '   ', ' ' * 30 + 'y', ' ' * 31]
HANDLER_CASES = [(0x14, 0, 0), (0x100, 0xfffffffe, 0), (0x100, 0xffffffff, 0), (0x100, 0xffffffff, 0), (0x100, 0xffffffff, 0), (0x100, 0, 0), (0x101, 0x70d, 1), (0x101, 0x70e, 1), (0x101, 0x70d, 0), (0x101, 0x70d, 2),
                 (0x802, 0, 0), (0x804, 0, 0), (0x803, 0, 0), (0x801, 0, 0), (0x13, 0, 0), (0x102, 0, 0), (0x15, 0, 0), (0xff, 0, 0)]


def world(r, log):
    """The windows and globals: the dialog's child 0x70c, the user slots, the scene queue, the main window."""
    regions = {LOG: log}
    first, top = kids(r, regions, KIDS, (0x70c, 0x70b))
    dialog = bytearray(r.bytes(0x200))
    put(dialog, 0x5c, 'I', first)
    put(dialog, 0x60, 'I', r.getrandbits(32))
    put(regions[top[0x70c]], 0x138, 'I', r.choice([0x70d, 0x70e, 0x70d, r.getrandbits(32)]))
    regions[DIALOG] = dialog
    slots = bytearray(r.bytes(0x110))
    put(slots, 0x60, 'i', r.choice([-1, -1, 0, 1, r.getrandbits(31)]))
    regions[SLOTS] = slots
    regions[MAIN] = struct.pack('<2I', r.getrandbits(32), r.getrandbits(32))
    regions[SCENE] = bytearray(r.bytes(0x20))
    regions[0xf82884] = struct.pack('<I', MAIN)
    regions[0xf85614] = struct.pack('<I', SCENE)
    regions[0xfb34c0] = bytes([r.getrandbits(8) & ~1 | r.choice([0, 1, 1])])
    regions[0xfb3b7c] = struct.pack('<I', r.choice([2, 2, 0, 1, 3]))
    regions[HEAP] = heap()
    state = bytearray(0x4c)
    text = r.choice(TEXTS)
    put(state, 0, '%ds' % (2 * len(text)), text.encode('utf-16le'))
    put(state, 0x40, 'I', DIALOG)
    put(state, 0x44, 'I', r.choice([0, 1, r.getrandbits(32) | 1]))
    put(state, 0x48, 'I', DIALOG)
    regions[0x7cb3b8] = state
    regions[0xf81ebc] = struct.pack('<I', r.choice([SLOTS, SLOTS, SLOTS, 0]))
    set_return(log, 27, SLOTS)   # (0x5c7420's answer)
    blob_bytes(log, 25, 1, 0xf0, (8, 0x7fff))
    return regions


def case(r):
    op = r.deal(['Instance', 'Commit', 'Close', 'Handler', 'Handler'])
    log = log_region()
    regions = world(r, log)
    if op == 'Instance':
        regions[HEAP] = heap(fail=r.choice([0, 0, 1]))
        return Call(0x48bcd0, [], regions, ret='a')
    if op == 'Commit':
        regions[HEAP] = heap(fail=r.choice([0, 0, 0, 2]))   # (the slots' block, the second)
        return Call(0x4a6b80, [r.getrandbits(32)], regions, ret='b')
    if op == 'Close':
        return Call(0x4a6a50, [r.choice([0, 0, 0, 1, r.getrandbits(32) | 1])], regions, ret='v')
    message, a, b = r.deal(HANDLER_CASES)
    if a == 0 and message not in (0x14, 0x100):
        a = r.getrandbits(32)
    return Call(0x4a6d00, [DIALOG, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
