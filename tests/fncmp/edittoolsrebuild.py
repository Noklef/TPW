# The coaster rebuilt by the tools (src/game/coaster/edittools: 0x447d30 EditTools_RebuildCurrent, 0x447d60 EditTools_RebuildIndexed) against the original:
# the coaster at [0x790fec] or in the table 0x790bd0, its bit 0 at +0, Coaster_Rebuild a recorder (k 3), the globals 0x792218 and 0x790fe4.
# usage: edittoolsrebuild.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

REBUILD = 3
B = Bench([(0x447d30, 0x447d52), (0x447d60, 0x447d8c)], hooks={0x43eb70: _recorder(REBUILD, 1, cdecl=True)})
COASTER = RAM


def case(r):
    me = bytearray(r.bytes(0x80))
    me[0] = r.getrandbits(8)
    index = r.randrange(8)
    table = bytearray(r.bytes(0x20))
    put(table, index * 4, 'I', r.choice([0, COASTER, COASTER, COASTER]))
    regions = {LOG: log_region(), COASTER: me, 0x790bd0: table, 0x790fec: struct.pack('<I', COASTER), 0x792218: r.bytes(4), 0x790fe4: r.bytes(4)}
    if r.randrange(2):
        return Call(0x447d30, [], regions, ret='v')
    return Call(0x447d60, [index], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
