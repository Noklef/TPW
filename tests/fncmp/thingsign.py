# The thing panels' sign slot (src/game/ui/thingsign: 0x497060, 0x4997f0, 0x4ad7f0, 0x4afb70, 0x4b0fa0, 0x4b64a0) against the originals. Recorders (ThingSignEnv): Object_GetDef 10, Sign_PlaceOverRect 12 (the rectangle's words hashed); GuiRect and the window's saved rectangle real. The window or the shown thing absent now and then.
# usage: thingsign.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob_bytes, log_region, put, set_return  # noqa: E402

CALLS = [0x497060, 0x4997f0, 0x4ad7f0, 0x4afb70, 0x4b0fa0, 0x4b64a0]
B = Bench([(a, a + 0x96) for a in CALLS] + [(0x659d00, 0x659d53), (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aaa0), (0x66bff7, 0x66c01b), (0x65d4f6, 0x65d520)],
          hooks={0x4dd4e0: _recorder(10, 0), 0x486410: _recorder(12, 3, cdecl=True)})
SELF, WINDOW, THING, DEF = (RAM + 0x1000 * k for k in range(4))


def case(r):
    log = log_region()
    blob_bytes(log, 12, 0, 0xff0)
    set_return(log, 10, DEF)
    self_ = put(bytearray(r.bytes(0x40)), 0x18, 'I', WINDOW if r.random() < 0.95 else 0)
    window = put(bytearray(r.bytes(0x100)), 0xe0, 'h', r.choice([0, 1, -5, 100, r.getrandbits(15)]))
    regions = {LOG: log, SELF: self_, WINDOW: window, THING: r.bytes(16), DEF: r.bytes(0x500), 0x7c2658: struct.pack('<I', THING if r.random() < 0.95 else 0)}
    return Call(r.deal(CALLS), [SELF], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
