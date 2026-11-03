# The sound object's small slots (src/game/core/soundobjectslots) against the originals. What they hand on to is recorders (SoundObjectSlotsEnv): SoundUpdate_Apply 20, ChannelVoices_Holds 21
# (its low byte 0 with high bits, or not; writing its out or not), ChannelVoices_PlayRecord 22 (0 or not; the same), ChannelVoices_PlayNotify 23, ChannelVoice_Mix 24; the object's slot 4
# a vtable recorder, the stack functor logged as a hash of its four dwords. The constant slots' floats compared in st0.
# usage: soundobjectslots.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fill, log_region, put, set_return, strings, vtable  # noqa: E402

B = Bench([(0x6b7f20, 0x6b7f79), (0x6b7f80, 0x6b7f95), (0x6b8360, 0x6b837a), (0x6c5630, 0x6c5684), (0x6bb4f0, 0x6bb506), (0x6b9ba0, 0x6b9ba9), (0x6b9bb0, 0x6b9bc4), (0x6ba1f0, 0x6ba1f9)],
          hooks={0x6c5690: _recorder(20, 3), 0x6bab50: _recorder(21, 2), 0x6badd0: _recorder(22, 3), 0x6bae60: _recorder(23, 2), 0x6bb510: _recorder(24, 2)})
OBJ, OUT, VT = RAM, RAM + 0x100, RAM + 0x200
ALL = ['play', 'notify', 'byId', 'byHandle', 'zero', 'one', 'flag', 'mix']
OPS = [k for k in os.environ.get('OPS', ','.join(ALL)).split(',') if k]


def case(r):
    name = r.deal(OPS)
    log = log_region()
    obj = bytearray(r.bytes(0x40))
    put(obj, 0, 'I', VT)
    regions = {LOG: log, OBJ: obj, OUT: r.bytes(4), VT: vtable({4: 2})}
    a, b = r.getrandbits(32), r.getrandbits(32)
    if name == 'play':
        set_return(log, 22, r.choice([0, 0, 1, 0x100, r.getrandbits(32)]))
        if r.random() < 0.7:
            fill(log, 22, 2, r.bytes(4))
        strings(log, 22, 0, out=0b100)
        strings(log, 4, 0, out=1)
        blob(log, 4, 1, 0xf)
        return Call(0x6b7f20, [OBJ, a, b], regions, conv='t', ret='a')
    if name == 'notify':
        return Call(0x6b7f80, [OBJ, a, b], regions, conv='t', ret='a')
    if name == 'byId':
        return Call(0x6b8360, [OBJ, OUT, a], regions, conv='t', ret='a')
    if name == 'byHandle':
        channel = RAM + 0x300
        regions[channel] = r.bytes(0x40)
        put(obj, 4, 'I', r.choice([0, 0, 1, r.getrandbits(32), r.getrandbits(32)]))
        set_return(log, 21, r.choice([0, 0x100, 0xffffff00, 1, 0x101, r.getrandbits(32)]))
        if r.random() < 0.7:
            fill(log, 21, 1, r.bytes(4))
        blob(log, 21, 0, 1)
        strings(log, 21, 0, out=0b10)
        return Call(0x6c5630, [OBJ, OUT, channel], regions, conv='t', ret='a')
    if name == 'zero':
        return Call(0x6b9ba0, [OBJ, a], regions, conv='t', ret='d')
    if name == 'one':
        return Call(0x6ba1f0, [OBJ, a], regions, conv='t', ret='d')
    if name == 'flag':
        return Call(0x6b9bb0, [OBJ], regions, conv='t', ret='d')
    return Call(0x6bb4f0, [a, b, r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
