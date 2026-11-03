# A held staff member and the world's staff slots (src/game/world/staffheld: Holder_PlaceStaff 0x509af0, World_ClearStaffSlots 0x5083b0) against the originals: Staff_PlaceInCell (k 1) and
# 0x481550 (k 2, its ECX logged) recorders; the handle +0x60 at 0, empty slots and full ones of the table at 0x7cfb90; the slots' bytes +0xa, +0xb 0 or not.
# usage: staffheld.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=place,clear)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

B = Bench([(0x509af0, 0x509b3f), (0x5083b0, 0x5083e7), (0x5da3c0, 0x5da3c1)], hooks={0x505ea0: _recorder(1, 1), 0x481550: _recorder(2, 1)})
OPS = [k for k in os.environ.get('OPS', 'place,clear').split(',') if k]
TABLE = 0x7cfb90


def case(r):
    name = r.deal(OPS)
    log = log_region()
    obj = bytearray(r.bytes(0x400))
    if name == 'clear':
        for i in range(0x20):
            obj[0xb + i * 0x14] = r.choice([0, 0, 1, r.getrandbits(8)])
        return Call(0x5083b0, [RAM, r.choice([0, 0, 1, 0x100, r.getrandbits(32)])], {LOG: log, RAM: obj}, conv='t', ret='v')
    handle = r.choice([0, 1, 2, 0x15e, r.randrange(0x15f)])
    put(obj, 0x60, 'H', handle)
    table = bytearray(r.bytes(0x14 * 0x160))
    if r.random() < 0.3:
        put(table, 0x14 * handle, 'I', 0)
    return Call(0x509af0, [RAM, r.getrandbits(32)], {LOG: log, RAM: obj, TABLE: table}, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
