# The sound module's start (Sound_Start 0x51b660, src/game/core/soundset) against the original. The manager's maker and its command post are recorders (SoundStartEnv): the maker answers
# a status through its out or another status word and writes the manager; the post's functor is hashed, its out (a stack slot) logged as 0. The first argument now and then is the
# on flag's address, so that the maker's fill through it changes that flag, which the code reads again after the first post. The sound globals are compared.
# usage: soundstart.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, echo, fill, log_region, set_return, strings  # noqa: E402

MAKE, APPLY = 1, 2
B = Bench([(0x51b660, 0x51b7c0), (0x5da3c0, 0x5da3c1)], hooks={0x6b5d00: _recorder(MAKE, 3, cdecl=True), 0x6b5bd0: _recorder(APPLY, 2)})
FLAGS = 0x802bc0
ON = 0x802bc8
STATUS = RAM
VALUES = [0, 0, 0, 0, 1, 0x7fffffff, 0xffffffff, 0x80000000, 0x88780078]


def query(r):
    """Sound_QueryWord0 0x51b770: the post answers through the functor's +4 and a status word (the word always written: nothing unset read)."""
    log = log_region()
    strings(log, APPLY, 0, out=1)
    blob(log, APPLY, 1, 1)
    echo(log, APPLY, 0)
    fill(log, APPLY, 0, struct.pack('<I', r.choice(VALUES)), which=0)
    fill(log, APPLY, 1, r.bytes(4), which=1, through=4)
    flags = bytearray(r.bytes(0x20))
    struct.pack_into('<I', flags, 8, r.choice([0, 1, 1, r.getrandbits(32)]))
    struct.pack_into('<I', flags, 0x14, r.choice([0, 1, 1, r.getrandbits(32)]))
    return Call(0x51b770, [], {LOG: log, FLAGS: flags}, conv='c', ret='a')


def case(r):
    if r.random() < 0.3:
        return query(r)
    log = log_region()
    strings(log, MAKE, 0, out=1)
    if r.random() < 0.85:
        echo(log, MAKE, 0)
        fill(log, MAKE, 0, struct.pack('<I', r.choice(VALUES)), which=0)
    else:
        set_return(log, MAKE, STATUS)
    if r.random() < 0.8:
        fill(log, MAKE, 2, r.bytes(4), which=1)
    a = r.getrandbits(32)
    if r.random() < 0.3:
        a = ON
        fill(log, MAKE, 1, struct.pack('<I', 0), which=2)
    strings(log, APPLY, 0, out=1)
    blob(log, APPLY, 1, 3)
    set_return(log, APPLY, r.getrandbits(32))
    flags = bytearray(r.bytes(0x20))
    struct.pack_into('<I', flags, 0x14, r.choice([0, 0, 0, 1, r.getrandbits(32)]))   # (0x802bd4, started)
    regions = {LOG: log, FLAGS: flags, 0x803a00: r.bytes(0x100), STATUS: struct.pack('<I', r.choice(VALUES))}
    return Call(0x51b660, [a, r.choice([0, 1, 1, 0x400, r.getrandbits(32) | 1])], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
