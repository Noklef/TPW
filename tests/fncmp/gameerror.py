# The error report's index of a library code (src/game/core/gameerror: 0x405b90) against the original: the codes of the table
# (0xb0754000-0xb0754018) and the three after it (0xb0754100-0xb0754102), their neighbours, and any other dword; the report set (0x405710) with
# no text or texts of up to 0x60 bytes over junk.
# usage: gameerror.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x405b90, 0x405cb0), (0x405710, 0x405742), (0x67ae00, 0x67af00)])
TEXT = RAM + 0x100


def case(r):
    if r.deal(['index', 'set']) == 'set':
        text = bytes(r.randrange(1, 256) for _ in range(r.choice([0, 1, 3, 4, 0x4f, 0x50, 0x51, r.randrange(0x60)])))
        regions = {RAM: r.bytes(0x60), TEXT: text + b'\0' + r.bytes(8)}
        return Call(0x405710, [RAM, r.getrandbits(32), r.getrandbits(32), r.choice([TEXT, TEXT, 0])], regions, conv='t')
    code = r.choice([0xb0754000 + r.randrange(0x19), 0xb0754000 + r.randrange(0x19), 0xb0754100 + r.randrange(3), 0xb0754000, 0xb0754018, 0xb0753fff, 0xb0754019,
                     0xb07540ff, 0xb0754103, 0xb0754000 + r.randrange(-0x40, 0x140), 0, 0x7fffffff, 0x80000000, 0xffffffff, r.getrandbits(32)])
    return Call(0x405b90, [r.choice([RAM, 0]), code & 0xffffffff], {RAM: r.bytes(0x10)}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
