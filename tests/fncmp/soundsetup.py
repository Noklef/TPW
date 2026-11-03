# A sound channel's set-ups (SoundChannel_Setup 0x6b8f90, SoundChannel_SetupWithReverb 0x6c5c90, SoundChannel_Refresh 0x6b91d0, src/game/core/soundchannel) against the originals. The open, the query and the reopen are
# recorders (SoundChannelSetupEnv), the channel's slots 0 and 0xd and its sub-object's slot 5 too. The settings record on the stack is hashed at each call at the dwords Init_6b9c50
# sets (the rest is stack garbage). The slot 0xd and the query write the record's +4 now and then (the flags the channel takes); no write: the +4 the code left. The open and the
# query answer a status through their out or another status word.
# usage: soundsetup.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=setup,refresh,reverb)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, echo, fill, log_region, set_return, strings, vtable  # noqa: E402

OPEN, QUERY, REOPEN, REVERB = 1, 2, 3, 4
SLOT_RECORD, SLOT_SUB = 13, 5
B = Bench([(0x6b8f90, 0x6b90c8), (0x6b91d0, 0x6b9217), (0x6b9c50, 0x6b9cc2), (0x6c5c90, 0x6c5cd1), (0x6b9850, 0x6b98c5), (0x6b5740, 0x6b5762), (0x6b5a70, 0x6b5a8c)],
          hooks={0x6b7510: _recorder(OPEN, 2), 0x6b7d70: _recorder(QUERY, 2), 0x6b7650: _recorder(REOPEN, 2), 0x6c5d20: _recorder(REVERB, 0)})
CHANNEL, OUT, VT, SUB_VT, STATUS, VALUE, COMMAND, TARGET = (RAM + 0x100 * i for i in range(8))
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
    if name in ('word44', 'word0', 'cmd44', 'cmd0'):   # (the record's asked-for word and its answer flags: what the query fills, so that nothing unset is read)
        status(r, log, QUERY, 2)
        blob(log, QUERY, 1, FIELDS)
        word44 = name.endswith('44')
        flags = r.choice([0, 0x100, 0x1, 0x101, r.getrandbits(32)])
        fill(log, QUERY, 1, struct.pack('<I', r.getrandbits(32)), which=3, offset=0x44 if word44 else 0)
        fill(log, QUERY, 1, struct.pack('<I', flags), which=4, offset=0x4c)
        regions[VALUE] = r.bytes(4)
        if name.startswith('word'):
            return Call(0x6b9060 if word44 else 0x6b9850, [CHANNEL, OUT, VALUE], regions, conv='t', ret='a')
        regions[COMMAND] = struct.pack('<II', r.getrandbits(32), VALUE)
        regions[TARGET] = r.bytes(0x24) + struct.pack('<I', CHANNEL)
        return Call(0x6b5740 if word44 else 0x6b5a70, [COMMAND, OUT, TARGET], regions, conv='t', ret='a')
    if name == 'refresh':
        blob(log, REOPEN, 1, FIELDS)
        return Call(0x6b91d0, [CHANNEL, OUT, r.getrandbits(32)], regions, conv='t', ret='a')
    status(r, log, OPEN, 1)
    blob(log, OPEN, 1, FIELDS)
    status(r, log, QUERY, 2)
    blob(log, QUERY, 1, FIELDS)
    if r.random() < 0.6:
        fill(log, QUERY, 1, struct.pack('<I', r.choice(FLAGS)), which=3, offset=4)
    address = 0x6c5c90 if name == 'reverb' else 0x6b8f90   # (the reverb's: the real set-up, then SndReverb_Load's recorder)
    return Call(address, [CHANNEL, OUT, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', 'setup,setup,refresh,reverb,word44,word0,cmd44,cmd0').split(',') if k]

if __name__ == '__main__':
    B.main(case)
