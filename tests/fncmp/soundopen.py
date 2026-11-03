# A sound channel's open (SoundChannel_Open 0x6b7510) and reopen (SoundChannel_Reopen 0x6b7650, src/game/core/soundchannel) against the originals. All they call is a
# recorder (SoundChannelOpenEnv): the settings' 3D voices (its stack copy of the settings hashed, its +0x40 and +0x44 filled in), the pool's constructor (answering any
# pool), the pool's start, the source maker, the EAX and synthesiser opens and frees, the apply (the functor's first word, and in the reopen its settings, logged). What
# has an out answers it with a status per call, non-negative or negative, or answers another status word. The heap fails now and then (a null pool still started); the
# source +0x18's slot 19 answers AL any; a null source faults in both now and then.
# usage: soundopen.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=open,reopen)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, blob, echo, fill, heap, log_region, set_return, strings, vtable  # noqa: E402

CHOOSE, CTOR, START, MAKE, OPEN_EAX, FREE_EAX, OPEN_SYNTH, FREE_SYNTH, APPLY = range(1, 10)
B = Bench([(0x6b7510, 0x6b764b), (0x6b7650, 0x6b76c4)],
          hooks={0x6b7470: _recorder(CHOOSE, 1), 0x6ba230: _recorder(CTOR, 2), 0x6ba2c0: _recorder(START, 2), 0x6c4190: _recorder(MAKE, 4, com=True),
                 0x6b7750: _recorder(OPEN_EAX, 1), 0x6b7840: _recorder(FREE_EAX, 0), 0x6b7870: _recorder(OPEN_SYNTH, 1), 0x6b78e0: _recorder(FREE_SYNTH, 0),
                 0x6b8b40: _recorder(APPLY, 2)})
CHANNEL, OUT, SETTINGS, SOURCE, VT, STATUS, POOL = (RAM + 0x100 * i for i in range(7))
HAPPY = [0, 0, 0, 1, 0x7fffffff]
SAD = [0xffffffff, 0x80000000, 0x88780078]
SKIP_K = 19   # (the source's slot 19, offset 0x4c)


def answer(r, log, k, which, regions):
    """Slot k's out (its first argument, a local) gets a status, mostly non-negative."""
    strings(log, k, 0, out=1)
    if r.random() < 0.9:
        echo(log, k, 0)
        fill(log, k, 0, struct.pack('<I', r.choice(HAPPY if r.random() < 0.75 else SAD)), which=which)
    else:
        set_return(log, k, STATUS)


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x40, r.choice([0, 0, 0, 0, 1])), SETTINGS: r.bytes(0x70), VT: vtable({0: 1}), SOURCE: struct.pack('<I', VT) + r.bytes(0xc),
               STATUS: struct.pack('<I', r.choice(HAPPY + SAD)), OUT: r.bytes(4), POOL: r.bytes(0x14)}
    channel = bytearray(r.bytes(0x40))
    struct.pack_into('<I', channel, 0x18, SOURCE if r.random() < 0.97 else 0)
    regions[CHANNEL] = channel
    set_return(log, SKIP_K, r.choice([0, 0, 0, 0x100, 1, 0xff, r.getrandbits(32)]))
    answer(r, log, START, 0, regions)
    answer(r, log, MAKE, 1, regions)
    answer(r, log, OPEN_EAX, 2, regions)
    answer(r, log, OPEN_SYNTH, 3, regions)
    set_return(log, CTOR, POOL)
    if name == 'open':
        blob(log, CHOOSE, 0, (1 << 28) - 1)
        fill(log, CHOOSE, 0, struct.pack('<2I', r.randrange(8), r.randrange(8)), which=4, offset=0x40)
        blob(log, MAKE, 2, (1 << 28) - 1)
        blob(log, APPLY, 1, 1)
        fill(log, APPLY, 0, struct.pack('<I', r.getrandbits(32)), which=5)
        return Call(0x6b7510, [CHANNEL, OUT, SETTINGS], regions, conv='t', ret='a')
    strings(log, APPLY, 0, out=1)
    blob(log, APPLY, 1, 3)
    return Call(0x6b7650, [CHANNEL, OUT, SETTINGS], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', 'open,open,reopen').split(',') if k]

if __name__ == '__main__':
    B.main(case)
