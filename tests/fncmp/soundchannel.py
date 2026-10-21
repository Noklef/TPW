# The sound object's channel (src/game/core/soundchannel) against the originals: SoundChannel_Apply hands a functor (a recorder vtable at
# FVT, slot 0 (out, channel)) the channel at CHAN and the out dword at OUT.
# usage: soundchannel.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, log_region, set_return, vtable  # noqa: E402

CALLS = {'apply': 0x6b8b40}
B = Bench([(0x6b8b40, 0x6b8b60)])
CHAN, OUT, FUNCTOR, FVT = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def case(r):
    name = r.deal(OPS)
    log = log_region()
    set_return(log, 0, r.getrandbits(32))
    regions = {CHAN: r.bytes(0x60), OUT: r.bytes(4), FUNCTOR: struct.pack('<I', FVT) + r.bytes(8), FVT: vtable({0: 2}), LOG: log}
    return Call(CALLS[name], [r.choice([CHAN, CHAN, 0, r.getrandbits(32)]), OUT, FUNCTOR], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
