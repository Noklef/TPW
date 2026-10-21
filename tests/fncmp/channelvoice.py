# A sound channel's voices (src/game/core/channelvoice) against the originals: voices of 0x8c bytes (a recorder vtable: slot 11 the
# release's value; +4 a sound (a recorder vtable, slot 2) or null, +0x2c an id, +0x74 the critical section), the imports
# (Enter/LeaveCriticalSection, InterlockedIncrement/Decrement) recorders; the count at 0xfb2058. The pool (+0 capacity, +4 an array of
# 0-4 voices or null, +8 the count in use (0 now and then, the first voice still released), +0xc a channel whose source's slot 10 (a
# recorder) writes a status through its out (negative now and then) and returns the out; the shift at 0x782db8 any byte). The pool's
# count from the settings' +0x40 (below, at or above the capacity). SoundChannel_ReleaseVoice (soundchannel, 0x6b7e60) is benched here
# ('releasevoice'): a voice or null. ChannelVoice_SetFlag: the byte on over random upper bytes, any mask.
# usage: channelvoice.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, echo, fill, import_table, log_region, set_return, strings, vtable  # noqa: E402

# name: (address, convention, returns)
CALLS = {'reset': (0x6c5060, 'c', 'v'), 'inc': (0x6c5070, 't', 'a'), 'dec': (0x6c5080, 't', 'a'), 'sound': (0x6bb7a0, 't', 'a'),
         'release': (0x6bb7c0, 't', 'b'), 'releaseall': (0x6babc0, 't', 'v'), 'setcount': (0x6bac30, 't', 'v'),
         'releasevoice': (0x6b7e60, 't', 'a'), 'setflag': (0x6b8330, 't', 'v')}
RANGES = [(0x6c5060, 0x6c506b), (0x6c5070, 0x6c5078), (0x6c5080, 0x6c5088), (0x6bb7a0, 0x6bb7bd), (0x6bb7c0, 0x6bb856), (0x6babc0, 0x6bac04),
          (0x6bac30, 0x6bac5c), (0x6b7e60, 0x6b7ea5), (0x6b8330, 0x6b8358)]
B = Bench(RANGES)
VOICES, VVT, SND, SVT, POOL, CHAN, SRC, SRCVT, SETS, OUT = (RAM + 0x400 * i for i in range(10))
COUNT, SHIFT = 0xfb2058, 0x782db8
IMPORTS = {'InterlockedIncrement': (40, 0x6fd0c0, 1, 0, 0), 'InterlockedDecrement': (41, 0x6fd12c, 1, 0, 0),
           'EnterCriticalSection': (42, 0x6fd140, 1, 0, 0), 'LeaveCriticalSection': (43, 0x6fd130, 1, 0, 0)}
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def voices(r, n):
    out = bytearray()
    for _ in range(n):
        v = bytearray(r.bytes(0x8c))
        struct.pack_into('<II', v, 0, VVT, r.choice([SND, SND, 0]))
        struct.pack_into('<I', v, 0x2c, r.choice([r.getrandbits(32), r.getrandbits(8)]))
        out += v
    return out


def common(r, log, n):
    set_return(log, 11, r.getrandbits(32))
    set_return(log, 2, r.getrandbits(32))
    for k in (40, 41):
        set_return(log, k, r.getrandbits(32))
    strings(log, 10, 0, out=1)   # (the source's out: the voice argument's own stack slot)
    fill(log, 10, 0, b''.join(struct.pack('<I', r.choice([0, 1, 0x80000000, 0xffffffff, r.getrandbits(32)])) for _ in range(4)), records=4)
    echo(log, 10, 0)   # (slot 10 returns its out)
    chan = bytearray(r.bytes(0x40))
    struct.pack_into('<I', chan, 0x18, SRC)
    return {VOICES: voices(r, n), VVT: vtable({11: 1}), SND: struct.pack('<I', SVT) + r.bytes(0xc), SVT: vtable({}), CHAN: chan,
            SRC: struct.pack('<I', SRCVT) + r.bytes(0xc), SRCVT: vtable({10: 2}), COUNT: r.bytes(4), SHIFT: bytes([r.getrandbits(8)]) + r.bytes(3),
            OUT: r.bytes(4), IMPORT_TABLE: import_table(log, IMPORTS), LOG: log}


def case(r):
    name = r.deal(OPS)
    address, conv, ret = CALLS[name]
    log = log_region()
    n = r.choice([0, 1, 2, 3, 4])
    regions = common(r, log, max(n, 1))
    if name == 'releasevoice':
        return Call(address, [CHAN, OUT, r.choice([VOICES] * 6 + [0]), r.getrandbits(32)], regions, conv=conv, ret=ret)
    if name == 'setflag':   # (the byte on over random upper bytes)
        return Call(address, [VOICES, r.choice([0, 1, 0x100, r.getrandbits(32)]), r.getrandbits(32)], regions, conv=conv, ret=ret)
    if name == 'reset':
        return Call(address, [], regions, conv=conv, ret=ret)
    if name in ('inc', 'dec'):
        return Call(address, [r.choice([COUNT, OUT])], regions, conv=conv, ret=ret)
    if name in ('sound', 'release'):
        return Call(address, [VOICES] + ([r.getrandbits(32)] if name == 'release' else []), regions, conv=conv, ret=ret)
    pool = struct.pack('<IIII', r.choice([0, 1, 2, 4, 8, r.getrandbits(32)]), r.choice([VOICES] * 6 + [0]), n, CHAN)
    sets = bytearray(r.bytes(0x50))
    struct.pack_into('<I', sets, 0x40, r.choice([0, 1, 2, 3, 4, 8, r.getrandbits(32)]))
    regions.update({POOL: pool, SETS: sets})
    return Call(address, [POOL] + ([SETS] if name == 'setcount' else []), regions, conv=conv, ret=ret)


if __name__ == '__main__':
    B.main(case)
