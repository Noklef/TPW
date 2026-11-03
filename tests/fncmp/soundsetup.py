# A sound channel's set-ups (SoundChannel_Setup 0x6b8f90, SoundChannel_Refresh 0x6b91d0, src/game/core/soundchannel) against the originals. The open, the query and the reopen are
# recorders (SoundChannelSetupEnv), the channel's slots 0 and 0xd and its sub-object's slot 5 too. The settings record on the stack is hashed at each call at the dwords Init_6b9c50
# sets (the rest is stack garbage). The slot 0xd and the query write the record's +4 now and then (the flags the channel takes); no write: the +4 the code left. The open and the
# query answer a status through their out or another status word.
# usage: soundsetup.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=setup,refresh)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, echo, fill, log_region, set_return, strings, vtable  # noqa: E402

OPEN, QUERY, REOPEN = 1, 2, 3
SLOT_RECORD, SLOT_SUB = 13, 5
B = Bench([(0x6b8f90, 0x6b905e), (0x6b91d0, 0x6b9217), (0x6b9c50, 0x6b9cc2)],
          hooks={0x6b7510: _recorder(OPEN, 2), 0x6b7d70: _recorder(QUERY, 2), 0x6b7650: _recorder(REOPEN, 2)})
CHANNEL, OUT, VT, SUB_VT, STATUS = (RAM + 0x100 * i for i in range(5))
FIELDS = sum(1 << (off // 4) for off in (0, 4, 0x40, 0x48, 0x4c, 0x68))
VALUES = [0, 0, 0, 1, 0x7fffffff, 0xffffffff, 0x80000000, 0x88780078]
FLAGS = [0, 0x100, 0x200, 0x300, 0xfffffcff, 0xffffffff]


def status(r, log, k, which):
    """Slot k's out (its first argument, a local) gets a status, or the call answers another status word."""
    strings(log, k, 0, out=1)
    if r.random() < 0.85:
        echo(log, k, 0)
        fill(log, k, 0, struct.pack('<I', r.choice(VALUES)), which=which)
    else:
        set_return(log, k, STATUS)


def case(r):
    name = r.deal(OPS)
    log = log_region()
    channel = bytearray(r.bytes(0x80))
    struct.pack_into('<I', channel, 0, VT)
    struct.pack_into('<I', channel, 0x30, SUB_VT)
    regions = {LOG: log, CHANNEL: channel, OUT: r.bytes(4), VT: vtable({SLOT_RECORD: 3}), SUB_VT: vtable({SLOT_SUB: 1}), STATUS: struct.pack('<I', r.choice(VALUES))}
    strings(log, SLOT_RECORD, 0, out=1)
    blob(log, SLOT_RECORD, 1, FIELDS)
    if r.random() < 0.5:
        fill(log, SLOT_RECORD, 1, struct.pack('<I', r.choice(FLAGS)), which=0, offset=4)
    if name == 'refresh':
        blob(log, REOPEN, 1, FIELDS)
        return Call(0x6b91d0, [CHANNEL, OUT, r.getrandbits(32)], regions, conv='t', ret='a')
    status(r, log, OPEN, 1)
    blob(log, OPEN, 1, FIELDS)
    status(r, log, QUERY, 2)
    blob(log, QUERY, 1, FIELDS)
    if r.random() < 0.6:
        fill(log, QUERY, 1, struct.pack('<I', r.choice(FLAGS)), which=3, offset=4)
    return Call(0x6b8f90, [CHANNEL, OUT, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', 'setup,setup,refresh').split(',') if k]

if __name__ == '__main__':
    B.main(case)
