# The lobby's sky and camera set up (src/game/frontend/lobbysky: 0x5d8b50) against the original. The string real; the sky's load and turn, the light slot and the look-at recorders. The renderer's flags and fog vary.
# usage: lobbysky.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, global_heap, heap, log_fold, log_region, set_return, strings  # noqa: E402

SKY, TURN, LIGHT, LOOK, DOME = range(5)
B = Bench([(0x5d8b50, 0x5d8ca6), (0x5d8d70, 0x5d8dd6)] + gamestring.B_RANGES,
          hooks={0x5852b0: _recorder(SKY, 1), 0x585690: _recorder(TURN, 3), 0x5d92e0: _recorder(LIGHT, 4), 0x5d8de0: _recorder(LOOK, 0), 0x5856c0: _recorder(DOME, 0)})


def case(r):
    log = log_region()
    log_fold(log)
    imports = bytearray(0x600)
    for at, stub in global_heap().items():
        imports[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    strings(log, SKY, 1)
    regions = {LOG: log, IMPORT_TABLE: imports, HEAP: heap(0x1000, 0), RAM: r.bytes(0x80)}
    for address, size in ((0x8bd47c, 4), (0x8bd490, 8), (0x879dd8, 0x10)):
        regions[address] = r.bytes(size)
    regions[0x8bcbc8] = struct.pack('<I', r.choice([0, 0xffffffff, r.getrandbits(32), r.getrandbits(32)]))
    if r.deal(['enter', 'leave']) == 'leave':
        set_return(log, DOME, r.getrandbits(32))
        return Call(0x5d8d70, [RAM], regions, conv='t', ret='a')
    return Call(0x5d8b50, [RAM], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
