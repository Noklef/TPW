# The user slots' record accessors (src/game/files/userslots: 0x5c7bb0, 0x5c7d00, 0x5c7d20) against the originals: four 24-byte records
# whose dword at +4 (the name's length) is 0, 1, the top bit alone or anything, and each record asked for.
# usage: userslots.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x5c7bb0, 0x5c7bc7), (0x5c7d00, 0x5c7d16), (0x5c7d20, 0x5c7d2e)])
CALLS = {'Count': 0x5c7bb0, 'IfUsed': 0x5c7d00, 'Field14': 0x5c7d20}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def case(r):
    op = OPS[r.deal(range(len(OPS)))]
    obj = bytearray(r.bytes(0x110))
    for i in range(4):
        put(obj, 4 + 24 * i, 'I', r.choice([0, 0, 1, 0x80000000, r.getrandbits(32)]))
    args = [RAM] if op == 'Count' else [RAM, r.randrange(4)]
    return Call(CALLS[op], args, {RAM: obj}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
