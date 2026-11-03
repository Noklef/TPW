# The objects' lists dialog's page's handler (src/game/ui/dialog7ca150: 0x495360 Dialog7ca150_Handler) against the original: dialogs.py's camera world (WindowAt10_CentreHandler's; the
# camera's own path, as the handle pushed carries a stack pointer's high word), the window's default handler +0x118 fncmp's logger and its children a list (window7059f0.py's items()) with the
# id asked for and a decoy; the items' data the thing or the window, 0x400's any. The page [0x7ca150] a logger window. The tabs' 0x494ec0, 0x494c70, 0x494250, 0x495110, the language text
# 0x485b00, HoverLabel_Attach 0x485d20 and ThingPanel_Show 0x486920 cdecl recorders 24-30 (their own benches have them; driver.cpp's kDialog7ca150Env).
# usage: dialog7ca150handler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialogs  # noqa: E402
import window7059f0  # noqa: E402
from fncmp import CDECL_LOG4, LOG, RAM, Bench, Call, _recorder, log_region, put, set_return  # noqa: E402
from window import RANGES, WINDOW  # noqa: E402

CDECL = {0x494ec0: (24, 1), 0x494c70: (25, 1), 0x494250: (26, 1), 0x495110: (27, 1), 0x485b00: (28, 1), 0x485d20: (29, 3), 0x486920: (30, 1)}
B = Bench(sorted(set(RANGES + [(0x495360, 0x4955d8), (0x42aab0, 0x42ab30), (0x4867b0, 0x486870), (0x50b350, 0x50b370), (0x5da3c0, 0x5da3c1), (0x65f66e, 0x65f6e1), (0x663a93, 0x663c11),
                               (0x664c71, 0x664cb9), (0x659710, 0x659740)])),
          hooks={address: _recorder(k, n, cdecl=True) for address, (k, n) in CDECL.items()})
LIST, DECOY, LINKS, PAGE = RAM + 0x4000, RAM + 0x4200, RAM + 0x4400, RAM + 0x5000
ID = 0x77
DECK = [(0x101, 0x12c4ba + i) for i in range(4)] * 3 + [(0x101, 0x12c4b9), (0x101, 0x12c4be)] + [(0x400, ID)] * 8 + [(0x400, 0x78), (0x400, 0x79)] + \
       [(0x402, ID)] * 8 + [(0x402, 0x78), (0x402, 0x79)] + [(0x406, 0)] * 5 + [(0x100, ID), (0x401, ID), (0x403, ID), (0x405, 0), (0x407, 0), (0x15, 0)]


def case(r):
    message, a = r.deal(DECK)
    log = log_region()
    regions = {LOG: log}
    dialogs.centre_case(r, regions, log)   # (the camera's: a logger window at WINDOW, its surroundings, the thing [0x7c2658])
    set_return(log, 28, r.getrandbits(32))
    thing = struct.unpack('<I', regions[0x7c2658])[0]
    regions[thing] = struct.pack('<H', r.randrange(6)) + r.bytes(2)
    win = bytearray(regions[WINDOW])
    put(win, 0, 'H', r.randrange(6))   # (the handle when the item has no data: the slot is left the window)
    put(win, 0x5c, 'I', r.choice([LIST, DECOY, DECOY, 0]) if r.random() < 0.1 else DECOY)
    regions[WINDOW] = win
    decoy, lst = bytearray(r.bytes(0x170)), bytearray(r.bytes(0x170))
    put(decoy, 0x8c, 'I', 0x78)
    put(decoy, 0x54, 'I', LIST)
    put(lst, 0x8c, 'I', ID)
    put(lst, 0x54, 'I', 0)
    idx = window7059f0.items(r, lst, regions, base=LINKS)
    links = bytearray(regions[LINKS])
    for i in idx:
        put(links, 4 * (i + 1), 'I', r.choice([thing] * 3 + [WINDOW] + ([r.getrandbits(32), 0] if message == 0x400 else [])))
    regions[LINKS] = links
    put(lst, 0x150, 'i', len(idx))
    put(lst, 0x160, 'i', r.choice(idx + [idx[0], -1]))
    page = bytearray(r.bytes(0x120))
    page[4] = r.choice([0, 0, 0, 1])
    put(page, 0x114, 'I', CDECL_LOG4)
    regions.update({DECOY: decoy, LIST: lst, PAGE: page, 0x7ca150: struct.pack('<I', PAGE), 0xfaa5fc: struct.pack('<I', 0),
                    0x7ca154: struct.pack('<I', r.choice([0, 1, 2, 3] * 2 + [4, r.getrandbits(32)])), 0x750d5c: r.bytes(16)})
    if message in (0x400, 0x402):
        b = r.choice([0, 0, 1, 2, len(idx) - 1, len(idx), r.randrange(len(idx) + 1), -1])
    else:
        b = r.choice([1, 1, 1, 1, 0, 2, r.getrandbits(32)])
    return Call(0x495360, [WINDOW, message, a, b & 0xffffffff], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
