# The park view's handler (src/game/ui/window7cb2achandler: Window7cb2ac_Handler, 0x4881a0) against the original. Every callee a recorder (driver.cpp's kWindow7cb2acHandlerEnv) but the
# mouse modes' holder and the drag's new mode (focus.py's MODES, the heap) and the mode [0x7b05d8]'s slot 9 (a vtable of recorders).
# usage: window7cb2achandler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, _recorder, heap, log_region, put, set_return, vtable  # noqa: E402
from focus import MODES  # noqa: E402

# (thiscall 12-18, cdecl 19-28, stdcall 29: driver.cpp's kWindow7cb2acHandlerEnv)
CAPTURE, RELEASE, FIRE_DOWN, FIRE_UP, CHEATS_ON, CHEATS_CTOR, DEFAULT = range(12, 19)
CLOCK, CAMERA, GAME_MODE, HIDE, UNLIGHT, HELP, MODIFIERS, BUTTON, SHOWN = range(19, 28)   # (atexit: _cdecl_log, as the modes' statics)
MOVED = 29

B = Bench(sorted(set(MODES + [(0x4881a0, 0x4889ce), (0x46c6b0, 0x46c6cc)])),
          hooks={0x65e549: _recorder(CAPTURE, 0), 0x65e56e: _recorder(RELEASE, 0), 0x40c900: _recorder(FIRE_DOWN, 2), 0x40c990: _recorder(FIRE_UP, 2), 0x405560: _recorder(CHEATS_ON, 0),
                 0x4053b0: _recorder(CHEATS_CTOR, 0), 0x65f66e: _recorder(DEFAULT, 3), 0x65968e: _recorder(CLOCK, 0, cdecl=True), 0x42a760: _recorder(CAMERA, 3, cdecl=True),
                 0x52f200: _recorder(GAME_MODE, 2, cdecl=True), 0x4989d0: _recorder(HIDE, 0, cdecl=True), 0x469470: _recorder(UNLIGHT, 0, cdecl=True), 0x486d90: _recorder(HELP, 1, cdecl=True),
                 0x46b420: _recorder(MODIFIERS, 0, cdecl=True), 0x46c210: _recorder(BUTTON, 5, cdecl=True), 0x4879d0: _recorder(SHOWN, 0, cdecl=True), 0x46c2a0: _recorder(MOVED, 2, com=True), 0x67a7d0: _cdecl_log,
                 0x4814f0: _recorder(9, 1)})   # (the online mode's 0x46c6b0 calls it on 0x7b51f0: natively the thiscall recorder 9)
MODE, MODE_VT, HOLDER, WINDOW = RAM + 0x4000, RAM + 0x4100, RAM + 0x4200, RAM
OPS = ['Off', 'Off', 'Timer', 'Timer', 'Move', 'Move', 'Press', 'Release', 'Release', 'Destroy', 'Clear', 'Enter', 'Leave', 'Up', 'Down', 'Down', 'KeyDown', 'KeyUp', 'Any']


def case(r):
    log = log_region()
    op = r.deal(OPS)
    start, x0, y0 = r.getrandbits(32), r.getrandbits(16), r.getrandbits(16)
    set_return(log, CLOCK, (start + r.deal([0, 0xc8, 0xc9, 0x1000, 0xfffffff0, 0x80000000])) & 0xffffffff)
    set_return(log, DEFAULT, r.getrandbits(32))
    set_return(log, MODIFIERS, r.getrandbits(32))
    set_return(log, SHOWN, r.deal([0, 1, 0x100]))
    set_return(log, CHEATS_ON, r.deal([0, 1, 0x100]))
    set_return(log, FIRE_DOWN, r.deal([0, 0, 1]))
    set_return(log, FIRE_UP, r.deal([0, 0, 1]))
    near = lambda v: (v + r.deal([0, 8, -8, 9, -9])) & 0xffff  # noqa: E731
    message, a, b = {'Off': (r.deal([0x10005, 0x10004, 0x1e, 0x14]), r.deal([1, 1, 0, 2]), r.getrandbits(32)), 'Timer': (0x1e, r.getrandbits(32), r.getrandbits(32)),
                     'Move': (0x10003, near(x0) << 16 | near(y0), r.getrandbits(32)), 'Press': (0x10005, 1, r.getrandbits(32)), 'Release': (0x10004, 1, r.getrandbits(32)),
                     'Destroy': (0x14, 0, 0), 'Clear': (0x15, 0, 0), 'Enter': (0x10001, 0, 0), 'Leave': (0x10002, 0, 0), 'Up': (0x10004, r.deal([0, 2, 0x20]), r.getrandbits(32)),
                     'Down': (0x10005, r.deal([0, 0, 2, 0x20, 1]), r.getrandbits(32)), 'KeyDown': (0x1000a, r.getrandbits(32), r.getrandbits(32)),
                     'KeyUp': (0x1000b, r.getrandbits(32), r.getrandbits(32)), 'Any': (r.deal([0x12, 0x1f, 0x10000, 0x10006, 0x1000c]), r.getrandbits(32), r.getrandbits(32))}[op]
    bytes_ = lambda n: bytearray(r.bytes(n))  # noqa: E731
    flags = bytes_(0x14)                                  # 0x7c24f4..0x7c2504
    put(flags, 8, 'I', r.deal([0, 1, 1]))                 # 0x7c24fc
    put(flags, 0xc, 'I', r.deal([0, 1, 1]))               # 0x7c2500, the drag
    put(flags, 0x10, 'I', start)                          # 0x7c2504
    mouse = bytes_(0x20)
    put(mouse, 0xc, 'I', r.deal([MODE, MODE, MODE, 0]))  # 0x7b05d8
    put(mouse, 0x1c, 'I', r.deal([0, HOLDER, HOLDER]))   # 0x7b05e8
    scales = bytes_(0x88)
    for at in (0, 4, 0x80, 0x84):                         # 0xfaa540 / 0xfaa544 (the cursor), 0xfaa5c0 / 0xfaa5c4
        put(scales, at, 'h', r.deal([640, 800, 1024, -640, 480, 600, 768, r.getrandbits(15)]))
    regions = {LOG: log, HEAP: heap(0x1000, r.deal([0, 0, 0, 0, 1, 2])), MODE_VT: vtable({0: 1, 5: 2}), MODE: put(bytearray(r.bytes(0x10)), 0, 'I', MODE_VT), HOLDER: b'\x00' * 4,
               0x74fc2c: bytes_(4), 0x78d911: bytes([0 if op == 'Off' else r.deal([0, 1, 1, 2])]), 0x7c24c8: struct.pack('<I', r.deal([0, 0, RAM + 0x800])), 0x7c24f4: flags,
               0x7b5344: struct.pack('<H', x0), 0x7b9774: struct.pack('<H', y0), 0x7a1c7c: struct.pack('<I', r.deal([0, 1])), 0x7b05cc: mouse, 0x7cb2ac: bytes_(4),
               0x787c14: bytes_(4), 0x787a60: bytes_(4), 0x78721c: bytes_(4), 0x78733c: bytes_(4), 0xfb34c8: bytes([r.deal([0, 1, 0xfe, 0xff])]), 0xfaa540: scales}
    return Call(0x4881a0, [WINDOW, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
