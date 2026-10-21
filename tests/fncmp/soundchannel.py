# The sound object's channel (src/game/core/soundchannel) against the originals: SoundChannel_Apply hands a functor (a recorder vtable at
# FVT, slot 0 (out, channel)) the channel at CHAN and the out dword at OUT. SoundChannel_GetSettings: a settings record at REC (random
# flags), the channel's info (+0x14), source (+0x18: a recorder vtable, slot 0x13 answering a byte) and EAX buffer (+0x20: its +4 an
# IKsPropertySet, a COM recorder whose Get succeeds or not) each null now and then, [0xfb2058] random.
# usage: soundchannel.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, fill, log_region, set_return, strings, vtable  # noqa: E402

CALLS = {'apply': 0x6b8b40, 'settings': 0x6b7d70}
B = Bench([(0x6b8b40, 0x6b8b60), (0x6b7d70, 0x6b7e5b), (0x6c4760, 0x6c47a3)])
CHAN, OUT, FUNCTOR, FVT, REC, INFO, SRC, SVT, EAXB, KSET, KVT = (RAM + 0x100 * i for i in range(11))
GLOBAL = 0xfb2058
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def case(r):
    name = r.deal(OPS)
    log = log_region()
    if name == 'settings':
        return settings_case(r, log)
    set_return(log, 0, r.getrandbits(32))
    regions = {CHAN: r.bytes(0x60), OUT: r.bytes(4), FUNCTOR: struct.pack('<I', FVT) + r.bytes(8), FVT: vtable({0: 2}), LOG: log}
    return Call(CALLS[name], [r.choice([CHAN, CHAN, 0, r.getrandbits(32)]), OUT, FUNCTOR], regions, conv='t', ret='a')


def settings_case(r, log):
    strings(log, 3, 0, out=1 << 5 | 1 << 7)   # IKsPropertySet::Get's data and returned-size outs: one stack block
    set_return(log, 0x13, r.choice([0, 1, 0x100, r.getrandbits(32)]))   # (a byte: AL)
    if r.random() < 0.6:
        set_return(log, 3, r.choice([0, 0, 1, 0x80004005, 0x10000]))
    chan = bytearray(r.bytes(0x60))
    struct.pack_into('<II', chan, 0x14, r.choice([INFO, INFO, 0]), r.choice([SRC] * 6 + [0]))
    eax = r.choice([EAXB, 0])
    struct.pack_into('<I', chan, 0x20, eax)
    if not eax and r.random() < 0.5:   # the source's slot 3 (settings) rewrites the record's +0 and asked-for flags (+8; not with the
        # EAX buffer, whose Get shares slot 3)
        fill(log, 3, 0, struct.pack('<III', r.choice([0, 1]), r.getrandbits(32), r.getrandbits(32) & 0x7c1))
    rec = bytearray(r.bytes(0x70))
    for at in (4, 8):
        struct.pack_into('<I', rec, at, r.getrandbits(32) & r.choice([0xffffffff, 0x7c1, 0x300]))
    struct.pack_into('<I', rec, 0, r.choice([0, r.getrandbits(32)]))
    regions = {CHAN: chan, OUT: r.bytes(4), REC: rec, INFO: r.bytes(0x10), SRC: struct.pack('<I', SVT) + r.bytes(0xc), SVT: vtable({3: 1}),
               EAXB: r.bytes(4) + struct.pack('<I', KSET), KSET: struct.pack('<I', KVT) + r.bytes(4), KVT: vtable({}, com={3: 8}),
               GLOBAL: r.bytes(4), LOG: log}
    return Call(0x6b7d70, [CHAN, OUT, REC], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
