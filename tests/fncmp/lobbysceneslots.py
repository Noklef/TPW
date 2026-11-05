# The lobby scene's small slots (src/game/frontend/lobbyscene: 0x5d8470, 0x5e1340, 0x5e0460) against the originals: their callees recorders, each with a bench of its own; the weather's pointer at 0x8bcbac varies.
# usage: lobbysceneslots.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_fold, log_region, set_return  # noqa: E402

SIGN, RELEASE, FREE, RELOAD = 22, 23, 24, 25
B = Bench([(0x5d8470, 0x5d8491), (0x5e1340, 0x5e1350), (0x5e0460, 0x5e046b)],
          hooks={0x468420: _recorder(SIGN, 5, cdecl=True), 0x51ea70: _recorder(RELEASE, 0, cdecl=True), 0x57d720: _recorder(FREE, 0), 0x57d850: _recorder(RELOAD, 0)})


def case(r):
    log = log_region()
    log_fold(log)
    for k in (SIGN, FREE, RELOAD):
        set_return(log, k, r.getrandbits(32))
    regions = {LOG: log, RAM: r.bytes(0x40), 0x8bcbac: struct.pack('<I', r.getrandbits(32))}
    address = r.deal([0x5d8470, 0x5e1340, 0x5e0460])
    args = [r.getrandbits(32), r.getrandbits(32)] if address == 0x5d8470 else []
    return Call(address, [RAM] + args, regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
