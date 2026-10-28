# The followed ride's reverb (src/game/world/ridelist: 0x55a470 RideScripts_SetReverb) against the original, over soundset.py's senders'
# world for 0x51c8d0 (Sound_SetEaxRecord: the sound globals, on and started either 0 now and then, the soundchannel bench's command and
# its EAX record, the kept record 0x803ab4): the camera following a thing (0x790ab0 bit 4, 0x790988) or not, the thing 0x790a44; the ride
# records (0x8791b0, 0-4 linked by +0): their owners +0xc8 the thing or not, parents +0x10 0 mostly, reverbs +0xd8 the sender's index,
# 0, -1 or any; gBalanceLevel's reverb (0x78578c) the index or any.
# usage: scriptreverb.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import RAM, Bench, Call, put  # noqa: E402

_ops = os.environ.pop('OPS', None)   # (soundset.py reads OPS as its senders' addresses)
import soundset  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

B = Bench(sorted(set([(0x55a470, 0x55a4d4), (0x42a4e0, 0x42a4fb)] + soundset.RANGES)))
RECS = RAM + 0x30000


def case(r):
    c = soundset.sender_case(r, 0x51c8d0)
    index = c.args[0]
    regions = dict(c.regions)
    thing = r.choice([1, 0x1234, r.getrandbits(32)])
    n = r.choice([0, 1, 2, 3, 4])
    for k in range(n):
        rec = bytearray(r.bytes(0x100))
        put(rec, 0, 'I', RECS + 0x100 * (k + 1) if k + 1 < n else 0)
        put(rec, 0xc8, 'I', r.choice([thing, thing, 0, thing ^ 1]))
        put(rec, 0x10, 'I', r.choice([0, 0, 0, 1, r.getrandbits(32)]))
        put(rec, 0xd8, 'I', r.choice([index, index, 0, 0xffffffff, r.getrandbits(32)]))
        regions[RECS + 0x100 * k] = rec
    regions.update({0x8791b0: struct.pack('<I', RECS if n else 0), 0x78578c: struct.pack('<I', r.choice([index, index, r.getrandbits(32)])),
                    0x790ab0: bytes([r.getrandbits(8) & ~4 | r.choice([0, 4, 4, 4])]), 0x790988: struct.pack('<I', r.choice([0, 1, 1, 1])),
                    0x790a44: struct.pack('<I', r.choice([thing, thing, thing, 0]))})
    return Call(0x55a470, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
