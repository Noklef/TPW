# The panel [0x7c2534]'s two menus built (src/game/ui/panel7c2534menus: 0x48c150, 0x48c600) against the original. Every callee a recorder as driver.cpp's kPanel7c2534MenusEnv; the heap real, failing
# now and then. AddRow answers a new top each call.
# usage: panel7c2534menus.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, answers, heap, log_region, put, set_return, vtable  # noqa: E402

CTOR, HANDLERS, MODE, ATEXIT, WIDE, ROW, ONLINECTOR, LOGGED, ACTIVITY = range(10, 19)
SLOT = 9
B = Bench([(0x48c150, 0x48c5f9), (0x48c600, 0x48c825), (0x67a400, 0x690000), (0x6918f0, 0x691a30), (0x691ba0, 0x691ce0)],
          hooks={0x492e40: _recorder(CTOR, 0), 0x492e80: _recorder(HANDLERS, 2), 0x550ca0: _recorder(MODE, 0), 0x67a7d0: _recorder(ATEXIT, 1, cdecl=True), 0x485b00: _recorder(WIDE, 1, cdecl=True),
                 0x492f60: _recorder(ROW, 7), 0x5b50b0: _recorder(ONLINECTOR, 0), 0x5b6450: _recorder(LOGGED, 0), 0x5b6230: _recorder(ACTIVITY, 0)})
PANEL, ONLINE, TOPS, THING, KVTABLE, HOLDER, MODEWORD = RAM + 0x1000, RAM + 0x1100, RAM + 0x1300, RAM + 0x1400, RAM + 0x1500, RAM + 0x1600, RAM + 0x1700


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x2000, r.choice([0] * 15 + [r.randrange(1, 4)])), PANEL: r.bytes(0x20), KVTABLE: vtable({SLOT: 0})}
    online = bytearray(r.bytes(0x120))
    put(online, 0x108, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    regions[ONLINE] = online
    regions[TOPS] = struct.pack('<40I', *[r.randrange(0, 0x400) for _ in range(40)])
    regions[THING] = struct.pack('<I', KVTABLE)
    regions[HOLDER] = struct.pack('<II', 0, THING)
    regions[MODEWORD] = struct.pack('<I', r.choice([4, 4, 0, 3, r.getrandbits(32)]))
    regions[0x7c2534] = r.bytes(4)
    regions[0xfb34c6] = bytes([r.getrandbits(8)])
    regions[0xfb3b7c] = struct.pack('<I', r.choice([1, 1, 0, 2, r.getrandbits(32)]))
    regions[0x7cf83c] = struct.pack('<I', (MODEWORD - 0x1da738) & 0xffffffff)
    regions[0xf7d88c] = struct.pack('<I', r.choice([ONLINE, ONLINE, 0]))
    regions[0xf85614] = struct.pack('<I', HOLDER)
    set_return(log, CTOR, PANEL)
    set_return(log, ONLINECTOR, ONLINE)
    set_return(log, WIDE, RAM + 0x1800)
    set_return(log, LOGGED, r.choice([0, 1, 0x100, r.getrandbits(32)]))
    set_return(log, ACTIVITY, r.choice([0, 1, 0x100, r.getrandbits(32)]))
    set_return(log, SLOT, r.choice([0, 1, 0x100, r.getrandbits(32)]))
    answers(log, ROW, TOPS)
    return Call(r.choice([0x48c150, 0x48c600]), [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
