# The panel's handler of the window [0x7cc61c] (src/game/ui/dialog7cc61c: Dialog7cc61c_PanelHandler, 0x4bb720) against the original. Language_Font 10, Window_NotifyC000 11, Online_Instance 12, Dialog7cc61c_FitText 13
# recorders (driver.cpp's kDialog7cc61cPanelEnv: their own benches have them); slot 1 and slot 9 vtable recorders; Window_CallDefault runs on +0x118, fncmp's logger.
# usage: dialog7cc61cpanel.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CDECL_LOG4, LOG, RAM, Bench, Call, _recorder, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x4bb720, 0x4bb7fb), (0x65f66e, 0x65f697)],
          hooks={0x485a70: _recorder(10, 1, cdecl=True), 0x65c672: _recorder(11, 2), 0x48b390: _recorder(12, 0, cdecl=True), 0x4bb460: _recorder(13, 0, cdecl=True)})
WINDOW, HOLDER, FONT, VT = (RAM + 0x1000 * k for k in range(4))


def case(r):
    log = log_region()
    set_return(log, 10, FONT)
    set_return(log, 12, HOLDER)
    set_return(log, 1, r.choice([12, 16, 1, 0xffff0010, 0x10010, 0, r.getrandbits(32)]))
    message = r.choice([0x800] * 4 + [0x101] * 4 + [0x100, 0x7ff, 0x801, r.getrandbits(32)])
    a = r.choice([0x5b82, 0x5b82, 0x5b83, 0x5b81, r.getrandbits(32)])
    b = r.choice([1, 1, 0, 2, -1 & 0xffffffff, 3, r.getrandbits(32)])
    window = put(put(bytearray(r.bytes(0x120)), 0, 'I', VT), 0x118, 'I', CDECL_LOG4)
    regions = {LOG: log, WINDOW: window, HOLDER: r.bytes(0x100), FONT: put(bytearray(16), 0, 'I', VT), VT: vtable({1: 0, 9: 1}), 0x7cc620: bytes([r.choice([0, 1, r.randrange(256)])]),
               0x758870: r.bytes(4), 0xfaa5c0: struct.pack('<h', r.choice([480, 600, 768, 1, -1, r.randrange(1, 0x7fff)]))}
    return Call(0x4bb720, [WINDOW, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
