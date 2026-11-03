# Panel7cc218_Refresh (src/game/ui/panel7cc218, 0x4ae940) against the original. Recorders (Panel7cc218RefreshEnv): Thing_NameBuffer7ba058 10, Panel7cc218_ShowRide 11, Panel7cc218_UpdateButtons 12; the panel's and its +0x20's slots vtable recorders.
# usage: panel7cc218refresh.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x4ae940, 0x4ae97d)], hooks={0x486280: _recorder(10, 1, cdecl=True), 0x4af030: _recorder(11, 0), 0x4ad4e0: _recorder(12, 0)})
PANEL, OBJECT, VT = RAM, RAM + 0x100, RAM + 0x200


def case(r):
    log = log_region()
    set_return(log, 10, r.getrandbits(32))
    panel = put(put(bytearray(r.bytes(0x40)), 0, 'I', VT), 0x20, 'I', OBJECT)
    regions = {LOG: log, PANEL: panel, OBJECT: put(bytearray(r.bytes(0x10)), 0, 'I', VT), VT: vtable({9: 1}), 0x7c2658: r.bytes(4)}
    return Call(0x4ae940, [PANEL], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
