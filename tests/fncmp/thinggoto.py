# ThingGoto_Start (src/game/ui/thinggoto, 0x48cd10) against the original. Recorders (ThingGotoEnv): the message box 0x47f020 10 (agent B's; its record's set bytes hashed: +7 stays stack garbage), Global7c2654_Set 12, FocusCell_Update 13, GameMode_Set 14, GameMode_MapClick 15, Language_WideText 16; the panel's slot 0xb a vtable recorder.
# usage: thinggoto.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob_bytes, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x48cd10, 0x48cddf)], hooks={0x47f020: _recorder(10, 8, cdecl=True), 0x48c940: _recorder(12, 1, cdecl=True), 0x5276d0: _recorder(13, 1, cdecl=True), 0x52f200: _recorder(14, 2, cdecl=True), 0x524960: _recorder(15, 4, cdecl=True), 0x485b00: _recorder(16, 1, cdecl=True)})
PANEL, THING, VT = RAM, RAM + 0x100, RAM + 0x200


def case(r):
    log = log_region()
    blob_bytes(log, 10, 0, 0xff7f)
    set_return(log, 16, r.getrandbits(32))
    thing = bytearray(r.bytes(16))
    thing[5], thing[7] = r.choice([0, 0x7f, 0xff, r.getrandbits(8)]), r.choice([0, 1, 0xff, r.getrandbits(8)])
    regions = {LOG: log, PANEL: put(bytearray(r.bytes(16)), 0, 'I', VT), THING: thing, VT: vtable({0xb: 0}), 0x78d90f: bytes([r.choice([0, 0, 1, r.getrandbits(8)])]), 0x7c2658: struct.pack('<I', THING)}
    return Call(0x48cd10, [PANEL], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
