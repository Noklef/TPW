# The sound object's channel (src/game/core/soundchannel) against the originals: SoundChannel_Apply hands a functor (a recorder vtable at
# FVT, slot 0 (out, channel)) the channel at CHAN and the out dword at OUT. SoundChannel_GetSettings: a settings record at REC (random
# flags), the channel's info (+0x14), source (+0x18: a recorder vtable, slot 0x13 answering a byte) and EAX buffer (+0x20: its +4 an
# IKsPropertySet, a COM recorder whose Get succeeds or not) each null now and then, [0xfb2058] random.
# The functor 0x70a260's run (0x6b5e40): the channel's source a recorder vtable (slot 1), null now and then (a fault in both).
# The EAX settings' sets (0x6b7910, 0x6b7970): the channel's EAX buffer (its set's Set a COM recorder) or else its source (slot 0x13
# answering a byte, slot 0x14 logged), each null now and then.
# The voices' occlusion (0x6b7b60), the position's set (0x6b79d0), the functor 0x70a278's run (0x6b5e60) and the setters that apply it
# (0x6b90d0, 0x6b9930; the sound object's 0x6b84f0 and 0x6b8b60; the sound commands' runs 0x6b57b0, 0x6b5720, 0x6b5810): see voices_case().
# The voices' pick and play (0x6b9a60, 0x6b9ab0): see play_case().
# The flag 2 switch (0x6b9160): see flag2_case(). The sound object's open (0x6b8480): see objopen_case().
# usage: soundchannel.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import (HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, blob, echo, fb, fill, heap, import_table, log_region, recorder, set_return,  # noqa: E402
                   strings, vtable)

CALLS = {'apply': 0x6b8b40, 'settings': 0x6b7d70, 'fn260': 0x6b5e40, 'eax0b': 0x6b7910, 'eaxall': 0x6b7970, 'occlusion': 0x6b7b60,
         'position': 0x6b79d0, 'fn278': 0x6b5e60, 'mode': 0x6b90d0, 'placement': 0x6b9930, 'voices': 0x6b84f0, 'b4c': 0x6b57b0, 'b3c': 0x6b5720,
         'b54': 0x6b5810, 'objplace': 0x6b8b60, 'voiceocc': 0x6b7fa0, 'clearocc': 0x6b7a70, 'pick': 0x6b9a60, 'play': 0x6b9ab0,
         'setocc': 0x6b8180, 'addocc': 0x6b8230, 'remocc': 0x6b82c0, 'listremove': 0x6c4ba0, 'flag2': 0x6b9160, 'objopen': 0x6b8480, 'occswitch': 0x6b99c0, 'occadd': 0x6b9a10}
SETTERS = ('occlusion', 'voiceocc', 'clearocc', 'position', 'fn278', 'mode', 'placement', 'voices', 'b4c', 'b3c', 'b54', 'objplace', 'setocc', 'addocc', 'remocc', 'listremove', 'occswitch', 'occadd')
B = Bench([(0x6b8b40, 0x6b8b60), (0x6b7d70, 0x6b7e5b), (0x6c4760, 0x6c47a3), (0x6b5e40, 0x6b5e59), (0x6b7910, 0x6b796b),
           (0x6b7970, 0x6b79cb), (0x6c4670, 0x6c46a7), (0x6c46b0, 0x6c475a), (0x6b7b60, 0x6b7d6e), (0x6b79d0, 0x6b7a6b), (0x6b5e60, 0x6b5fb4),
           (0x6c4c80, 0x6c4eff), (0x6c4f00, 0x6c4f54), (0x6b8330, 0x6b8358), (0x6bb7a0, 0x6bb7bd), (0x6bb7c0, 0x6bb856), (0x6babc0, 0x6bac04),
           (0x6bac30, 0x6bac5c), (0x6b7e60, 0x6b7ea5), (0x6c5080, 0x6c5088), (0x6bb240, 0x6bb265), (0x6baa10, 0x6baa20), (0x6d1bc0, 0x6d1bca),
           (0x6b90d0, 0x6b9154), (0x6b9930, 0x6b99ba), (0x6b84f0, 0x6b8569), (0x6b57b0, 0x6b5801), (0x6b5720, 0x6b5739), (0x6b5810, 0x6b5834),
           (0x6b9c50, 0x6b9c6a), (0x6b9c70, 0x6b9c82), (0x6b9c90, 0x6b9ca9), (0x6b9cf0, 0x6b9d02), (0x6b9d30, 0x6b9d43), (0x6b6140, 0x6b6163),
           (0x6b8b60, 0x6b8c63), (0x6b7fa0, 0x6b814e), (0x6b7a70, 0x6b7b5f), (0x6b9a60, 0x6b9aa1), (0x6b9ab0, 0x6b9b26),
           (0x6b8180, 0x6b822d), (0x6b99c0, 0x6b9a05), (0x6b9a10, 0x6b9a5b), (0x6b8230, 0x6b82b1), (0x6c49b0, 0x6c4b93), (0x6c4fd0, 0x6c4fe3), (0x5e2a30, 0x5e2a80), (0x5e2a80, 0x5e2b80),
           (0x67acb0, 0x67acb1), (0x6b82c0, 0x6b832e), (0x6c4ba0, 0x6c4c1e),
           (0x6b9160, 0x6b91ce), (0x6b9660, 0x6b9760), (0x6bead0, 0x6beb03), (0x632150, 0x632380), (0x6b9b30, 0x6b9b94), (0x6b8480, 0x6b84ec)])
CHAN, OUT, FUNCTOR, FVT, REC, INFO, SRC, SVT, EAXB, KSET, KVT = (RAM + 0x100 * i for i in range(11))
GLOBAL = 0xfb2058
VOICES, VVT, SND, SNDVT, POOL, NODES, OCCS, VEC, VEC2, OBJECT, MIXER, COMMAND, P1, P2, P3 = (RAM + 0x1000 + 0x200 * i for i in range(15))
PCHAN, PVT, PREC, PVOICE = (RAM + 0x5000 + 0x100 * i for i in range(4))   # (pick / play)
PNODE = RAM + 0x5400   # (the occluder removes' node pointer)
OCCLUDERS, SHIFT, GLOBAL2708 = 0xfb2038, 0x782db8, 0xfb2708   # (the occluders' list, the voices' count at 0xfb2058 its +0x20)
IMPORTS = {'InterlockedDecrement': (41, 0x6fd12c, 1, 0, 0), 'EnterCriticalSection': (42, 0x6fd140, 1, 0, 0),
           'LeaveCriticalSection': (43, 0x6fd130, 1, 0, 0)}
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def case(r):
    name = r.deal(OPS)
    log = log_region()
    if name == 'settings':
        return settings_case(r, log)
    if name in SETTERS:
        return voices_case(r, log, name)
    if name in ('pick', 'play'):
        return play_case(r, log, name)
    if name == 'flag2':
        return flag2_case(r, log)
    if name == 'objopen':
        return objopen_case(r, log)
    if name in ('eax0b', 'eaxall'):
        blob(log, 4, 5, 1 if name == 'eax0b' else 0x3fff)   # (Set's data)
        set_return(log, 4, r.getrandbits(32))
        set_return(log, 0x13, r.choice([0, 1, 0x100, r.getrandbits(32)]))   # (a byte: AL)
        chan = bytearray(r.bytes(0x60))
        struct.pack_into('<II', chan, 0x18, r.choice([SRC] * 6 + [0]), r.getrandbits(32))
        struct.pack_into('<I', chan, 0x20, r.choice([EAXB, 0]))
        regions = {CHAN: chan, OUT: r.bytes(4), REC: r.bytes(0x70), SRC: struct.pack('<I', SVT) + r.bytes(0xc), SVT: vtable({0x14: 2}),
                   EAXB: r.bytes(4) + struct.pack('<I', r.choice([KSET] * 6 + [0])), KSET: struct.pack('<I', KVT) + r.bytes(4),
                   KVT: vtable({}, com={4: 7}), LOG: log}
        return Call(CALLS[name], [CHAN, OUT, REC], regions, conv='t', ret='a')
    if name == 'fn260':
        set_return(log, 1, r.getrandbits(32))
        chan = bytearray(r.bytes(0x60))
        struct.pack_into('<I', chan, 0x18, r.choice([SRC] * 9 + [0]))
        regions = {CHAN: chan, OUT: r.bytes(4), FUNCTOR: struct.pack('<I', 0x70a260), SRC: struct.pack('<I', SVT) + r.bytes(0xc), SVT: vtable({}),
                   LOG: log}
        return Call(CALLS[name], [FUNCTOR, OUT, CHAN], regions, conv='t', ret='a')
    set_return(log, 0, r.getrandbits(32))
    regions = {CHAN: r.bytes(0x60), OUT: r.bytes(4), FUNCTOR: struct.pack('<I', FVT) + r.bytes(8), FVT: vtable({0: 2}), LOG: log}
    return Call(CALLS[name], [r.choice([CHAN, CHAN, 0, r.getrandbits(32)]), OUT, FUNCTOR], regions, conv='t', ret='a')


def play_case(r, log, name):
    """SoundChannel_PickVoice (0x6b9a60) and SoundChannel_PlayRecord (0x6b9ab0): the channel a recorder vtable (slot 6 picks: 0, a voice-like
    address or any; slot 5 any; slot 4 (play's out the record argument's stack slot) writes a status, negative now and then, or not, and
    returns its out; the stack functor {0x70a618, voice} hashed), its flags (+0x28) with bits 3 and 8 any way; a record whose +0x14 has bit
    13 any way; the picked dword 0 or not."""
    set_return(log, 6, r.choice([0, 0, PVOICE, PVOICE, r.getrandbits(32)]))
    set_return(log, 5, r.getrandbits(32))
    strings(log, 4, 0, out=1)
    blob(log, 4, 1, 3)
    if r.random() < 0.75:
        fill(log, 4, 0, struct.pack('<I', r.choice([0, 1, 0x80000000, 0xffffffff, r.getrandbits(32)])))
    echo(log, 4, 0)
    chan = bytearray(r.bytes(0x60))
    struct.pack_into('<I', chan, 0, PVT)
    struct.pack_into('<I', chan, 0x28, r.getrandbits(32) & ~0x108 | r.choice([0, 8, 0x100, 0x108, 0x108]))
    rec = bytearray(r.bytes(0x70))
    struct.pack_into('<I', rec, 0x14, r.getrandbits(32) & ~0x2000 | r.choice([0, 0x2000]))
    regions = {PCHAN: chan, PVT: vtable({4: 2, 5: 2, 6: 2}), PREC: rec, PVOICE: struct.pack('<I', r.choice([0, 0, r.getrandbits(32)])), LOG: log}
    if name == 'pick':
        return Call(CALLS[name], [PCHAN, r.choice([0, PREC + 0x40, r.getrandbits(32)]), PVOICE, PREC], regions, conv='t', ret='a')
    return Call(CALLS[name], [PCHAN, PREC, PVOICE], regions, conv='t', ret='a')


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


F2_CHAN, F2_VT, F2_BANK, F2_RES, F2_RVT, F2_MAP, F2_BLOCK = (RAM + 0x6000 + 0x100 * i for i in range(7))


def flag2_case(r, log):
    """SoundChannel_SetFlag2 (0x6b9160): the channel's flags (+0x28) bits 0 and 1 any way, on 0, 1 or any; its slot 4 a recorder (its out on
    the stack, logged as 0; the functor's vtable hashed). The category singleton made already, its deque B 0-3 banks (null or one whose
    resource's slot 0 is recorder 0) within one block."""
    strings(log, 4, 0, out=1)
    blob(log, 4, 1, 1)
    set_return(log, 4, r.getrandbits(32))
    chan = bytearray(r.bytes(0x60))
    struct.pack_into('<II', chan, 0, F2_VT, r.choice([0, 1, 2, 3, r.getrandbits(32)]))
    struct.pack_into('<I', chan, 0x28, r.choice([0, 1, 2, 3, r.getrandbits(32)]))
    n, j = r.choice([0, 1, 2, 3]), r.randrange(0x3f0)
    category = bytearray(r.bytes(0xd4))
    struct.pack_into('<4I', category, 0x6c, F2_BLOCK, F2_BLOCK + 0x1000, F2_BLOCK + 4 * j, F2_MAP)
    struct.pack_into('<I', category, 0x94, n)
    category[0xd0] = r.choice([1, r.getrandbits(8) | 1])
    regions = {F2_CHAN: chan, F2_VT: vtable({4: 2}), OUT: r.bytes(4), LOG: log, 0xfb1f28: category,
               F2_BANK: r.bytes(8) + struct.pack('<I', F2_RES) + r.bytes(0x34), F2_RES: struct.pack('<I', F2_RVT) + r.bytes(0xc),
               F2_RVT: vtable({}), F2_MAP: struct.pack('<II', F2_BLOCK, F2_BLOCK),
               F2_BLOCK: struct.pack('<1024I', *[r.choice([F2_BANK, F2_BANK, 0]) for _ in range(1024)])}
    return Call(0x6b9160, [F2_CHAN, OUT, r.choice([0, 1, r.getrandbits(32)])], regions, conv='t', ret='a')


def objopen_case(r, log):
    """SoundObject_Open (0x6b8480): the object's slots 9 and 7 recorders 20 and 21, the channel's slot 7 recorder 7 (filling its out, value's
    stack slot, with a status, negative now and then, and returning that out); the category singleton made already, the globals around it."""
    for k in (7, 21):
        strings(log, k, 0, out=1)
    echo(log, 7, 0)
    fill(log, 7, 0, struct.pack('<I', r.choice([0, 1, 0x80000000, 0xffffffff, r.getrandbits(32)])), which=0)
    set_return(log, 20, r.getrandbits(32))
    set_return(log, 21, r.getrandbits(32))
    obj = bytearray(r.bytes(0x40))
    struct.pack_into('<I', obj, 0, F2_VT)
    struct.pack_into('<I', obj, 0x24, F2_CHAN)
    chan = bytearray(r.bytes(0x70))
    struct.pack_into('<I', chan, 0, F2_RVT)
    table = [recorder(40 + i, 0) for i in range(16)]
    table[9], table[7] = recorder(20, 0), recorder(21, 2)
    globs = bytearray(r.bytes(0xdc))
    globs[0xd4] = r.choice([1, r.getrandbits(8) | 1])
    regions = {OBJECT: obj, F2_CHAN: chan, F2_VT: struct.pack('<16I', *table), F2_RVT: vtable({7: 3}), OUT: r.bytes(4), LOG: log, 0xfb1f24: globs}
    return Call(0x6b8480, [OBJECT, OUT, r.choice([0, 1, 4, r.getrandbits(32)])], regions, conv='t', ret='a')


def voices_case(r, log, name):
    """SoundChannel_UpdateOcclusion (0x6b7b60), SoundChannel_SetPosition (0x6b79d0) and the functor 0x70a278's run (0x6b5e60): a channel (its
    position small integers) with 0-3 voices (channelvoice.py's layout; +0x10 / +0x14 flags with bits 0 and 5 any way, +0x28 0, 0.5, NaNs
    (quiet and signalling) or any; a sound or null) in a pool, its source a recorder vtable (slot 0x1e fills a voice's position, small integers; slot 0x22 (recorder 6) a status,
    negative now and then; slot 0x13 a byte; slot 10 ReleaseVoice's status), an EAX buffer now and then, 0-3 occluders at 0xfb2038 near the
    voices (soundoccluders.py's layout); a settings record with random flags (+4, +0x4c), +0x50 a vector or null, +0x54 a vector (+0x58,
    +0x5c, +0x68 random). The setters: the channel's flags (+0x28) random; a sound object (+0x1c a count, the same as the argument now
    and then; +0x20 a mixer or null, +0x24 the channel or null now and then) and a command of three random values. The placement: three
    vectors, the object's own (the channel's scale +0x2c 1.0) or one of them changed now and then, else random. The occlusion switch
    (0x6b8180): the occluders' switch and radius as they were or changed, NaNs on either side; the channel's switch over it (0x6b99c0) with
    on 0, 1 or any. The channel's add (0x6b9a10): as the add's, the node's out at PNODE."""
    def ints(k=6):
        return fb(float(r.randint(-k, k)))

    def status():
        return b''.join(struct.pack('<I', r.choice([0, 1, 0x80000000, 0xffffffff, r.getrandbits(32)])) for _ in range(4))

    def vec():
        return struct.pack('<3I', *[r.choice([ints(), r.f(), r.f(), 0x7fc00000, 0]) for _ in range(3)])

    for k in (0x10, 0x13):
        set_return(log, k, r.choice([0, 1, 0x100, r.getrandbits(32)]))   # (0x13: a byte, AL)
    set_return(log, 11, r.getrandbits(32))
    set_return(log, 41, r.getrandbits(32))
    strings(log, 0x1e, 0, out=5)   # (a local out, the position on the stack)
    strings(log, 0x1d, 0, out=1)   # (0x6b7fa0's slot 0x1d: an out on the stack)
    fill(log, 0x1e, 2, b''.join(struct.pack('<3I', ints(), r.getrandbits(32), ints()) for _ in range(4)), which=0, records=4)
    for k in (5, 0x10, 0x15, 0x19, 0x14):
        strings(log, k, 0, out=1)
    blob(log, 0x12, 1, 0xfff)   # (the EAX properties on the stack)
    blob(log, 7, 1, 7)   # (the scaled copy on the stack: slot 0x23's recorder)
    for k, which in ((6, 1), (10, 2)):   # (6: slot 0x22's recorder)
        strings(log, k, 0, out=1)
        fill(log, k, 0, status(), which=which, records=4)
        echo(log, k, 0)
    n = r.choice([0, 1, 1, 2, 3])
    voices = bytearray()
    for _ in range(max(n, 1)):
        v = bytearray(r.bytes(0x8c))
        struct.pack_into('<II', v, 0, VVT, r.choice([SND, SND, SND, 0]))
        for at in (0x10, 0x14):
            struct.pack_into('<I', v, at, r.getrandbits(32) & ~0x21 | r.choice([0x20, 0x20, 0x21, 0, 1]))
        struct.pack_into('<II', v, 0x28, r.choice([0, 0, fb(0.5), 0x7fc00000, 0xffc00000, 0x7f800001, fb(r.random()), r.getrandbits(32)]),
                         r.choice([r.getrandbits(32), r.getrandbits(8)]))
        voices += v
    pool = struct.pack('<IIII', r.choice([0, 1, 2, 4, r.getrandbits(32)]), r.choice([VOICES] * 8 + [0]), n, CHAN)
    chan = bytearray(r.bytes(0x60))
    struct.pack_into('<3I', chan, 8, ints(), r.getrandbits(32), ints())
    struct.pack_into('<III', chan, 0x14, POOL, r.choice([SRC] * 12 + [0]), r.getrandbits(32))
    struct.pack_into('<I', chan, 0x20, r.choice([EAXB, 0, 0]))
    struct.pack_into('<I', chan, 0x28, r.getrandbits(32) & r.choice([0xffffffff, 0x83, 0xc0, 0x80]))
    m = r.choice([0, 1, 2, 3])
    occs = b''.join(struct.pack('<4I', ints(), ints(), r.getrandbits(32), fb(float(r.randint(0, 6)))) for _ in range(max(m, 1)))
    nodes = b''.join(struct.pack('<II', OCCS + 0x10 * i, NODES + 8 * (i + 1) if i + 1 < m else 0) for i in range(max(m, 1)))
    occluders = bytearray(r.bytes(0x24))
    struct.pack_into('<I', occluders, 4, NODES if m else r.choice([0, NODES]))
    struct.pack_into('<I', occluders, 0xc, m if m else r.choice([0, 1]))
    struct.pack_into('<I', occluders, 0x14, r.choice([0, 0, fb(1.0), fb(3.0)]))
    rec = bytearray(r.bytes(0x70))
    struct.pack_into('<I', rec, 4, r.getrandbits(32) & r.choice([0xffffffff, 0x840, 0x800, 0]))
    struct.pack_into('<I', rec, 0x40, r.choice([0, 1, 2, 3, r.getrandbits(32)]))
    flags = r.getrandbits(32) & r.choice([0xffffffff, 0x2f4, 0x2fc, 0x274]) if r.random() < 0.9 else 0
    if flags & 0x60 == 0x60 and (struct.unpack_from('<I', chan, 0x20)[0] or r.random() < 0.5):   # (one blob mask for both of the EAX buffer's sets: one of them)
        flags &= ~r.choice([0x20, 0x40])
    struct.pack_into('<I', rec, 0x4c, flags)
    struct.pack_into('<II', rec, 0x50, r.choice([VEC, 0]), VEC2)
    blob(log, 4, 5, 1 if flags & 0x60 == 0x20 else 0x3fff)   # (IKsPropertySet::Set's data)
    set_return(log, 4, r.getrandbits(32))
    svt = vtable({5: 2, 10: 2, 0x10: 2, 0x12: 2, 0x14: 2, 0x15: 3, 0x19: 2, 0x1d: 4, 0x1e: 3}, size=36)
    struct.pack_into('<II', svt, 0x88, recorder(6, 5), recorder(7, 3))   # (slots 0x22 and 0x23: recorders 6 and 7, past the 32 a vtable() has)
    regions = {CHAN: chan, OUT: r.bytes(4), REC: rec, SRC: struct.pack('<I', SVT) + r.bytes(0xc), SVT: svt,
               EAXB: r.bytes(4) + struct.pack('<I', r.choice([KSET] * 6 + [0])), KSET: struct.pack('<I', KVT) + r.bytes(4), KVT: vtable({}, com={4: 7}),
               VOICES: voices, VVT: vtable({11: 1}), SND: struct.pack('<I', SNDVT) + r.bytes(0xc), SNDVT: vtable({}), POOL: pool, NODES: nodes,
               OCCS: occs, VEC: vec(), VEC2: vec(), OCCLUDERS: occluders, SHIFT: bytes([r.choice([0, 1, 4, r.getrandbits(8)])]) + r.bytes(3),
               GLOBAL2708: r.bytes(4), FUNCTOR: struct.pack('<II', 0x70a278, REC), IMPORT_TABLE: import_table(log, IMPORTS), LOG: log}
    if name == 'objplace':
        vectors = [struct.pack('<3I', *[r.choice([ints(), r.f(), 0x7fc00000]) for _ in range(3)]) for _ in range(3)]
        obj = bytearray(r.bytes(0x40))
        if r.random() < 0.6:
            struct.pack_into('<f', chan, 0x2c, 1.0)
            for at, v in zip((0x28, 4, 0x10), vectors):
                obj[at:at + 12] = v
            if r.random() < 0.5:
                k = r.randrange(3)
                vectors[k] = vectors[k][:4 * (k % 3)] + struct.pack('<I', ints()) + vectors[k][4 * (k % 3) + 4:]
        else:
            struct.pack_into('<I', chan, 0x2c, r.choice([fb(1.0), fb(0.5), r.f(), r.getrandbits(32)]))
        struct.pack_into('<III', obj, 0x1c, r.getrandbits(32), r.choice([MIXER, 0]), r.choice([CHAN] * 8 + [0]))
        strings(log, 7, 0, out=1)   # (slot 0x23's out: here the placement's local)
        regions.update({CHAN: chan, OBJECT: obj, MIXER: r.bytes(0x54), P1: vectors[0], P2: vectors[1], P3: vectors[2]})
        return Call(0x6b8b60, [OBJECT, OUT, P1, P2, P3], regions, conv='t', ret='a')
    if name in ('voices', 'b4c', 'b3c', 'b54'):
        count = r.choice([0, 1, 2, 3, 4, 0x1e, 0x1f, r.getrandbits(32)])
        obj = bytearray(r.bytes(0x30))
        struct.pack_into('<III', obj, 0x1c, r.choice([count, count, r.getrandbits(32)]), r.choice([MIXER, MIXER, 0]), CHAN)
        regions.update({OBJECT: obj, MIXER: r.bytes(0x54), COMMAND: struct.pack('<I', r.getrandbits(32)) + struct.pack('<III', count,
                        r.choice([0, 1, r.getrandbits(32)]), r.choice([0, 1, r.getrandbits(32)]))})
        if name == 'voices':
            return Call(0x6b84f0, [OBJECT, OUT, count], regions, conv='t', ret='a')
        return Call(CALLS[name], [COMMAND, OUT, OBJECT], regions, conv='t', ret='a')
    if name == 'mode':
        return Call(0x6b90d0, [CHAN, OUT] + [r.choice([0, 1, 0x100, r.getrandbits(32)]) for _ in range(3)], regions, conv='t', ret='a')
    if name == 'placement':
        return Call(0x6b9930, [CHAN, OUT, r.choice([VEC] * 4 + [0]), VEC2, r.getrandbits(32)], regions, conv='t', ret='a')
    if name == 'occlusion':
        return Call(0x6b7b60, [CHAN, r.choice([0, 1, 0x100, r.getrandbits(32)])], regions, conv='t', ret='v')
    if name == 'position':
        return Call(0x6b79d0, [CHAN, OUT, REC], regions, conv='t', ret='a')
    if name in ('setocc', 'occswitch'):   # (the switch at +0x18 and the radius at +0x14 as they were or not; NaNs; the byte argument's high bytes random)
        was = r.choice([0, 1, r.getrandbits(8)])
        occluders[0x18] = was
        on = r.choice([was, was, 0, 1, r.getrandbits(8)])
        old = struct.unpack_from('<I', occluders, 0x14)[0]
        radius = r.choice([old, old, old, 0, fb(1.0), fb(3.0), 0x7fc00000, 0xffc00000, 0x7f800001, 0x80000000, r.getrandbits(32)])
        if r.random() < 0.15:
            struct.pack_into('<I', occluders, 0x14, r.choice([0x7fc00000, 0x7f800001]))
        if name == 'occswitch':
            return Call(0x6b99c0, [CHAN, OUT, r.choice([0, 1, r.getrandbits(32)]), radius], regions, conv='t', ret='a')
        return Call(0x6b8180, [CHAN, OUT, r.getrandbits(24) << 8 | on, radius], regions, conv='t', ret='a')
    if name in ('addocc', 'remocc', 'listremove', 'occadd'):   # (the occluders' list rebuilt with the list's own nodes {occluder, next, prev} and
        # tail; the switch on or off; the EAX buffer or the source's slot 0x13. The add: the new one's radius equal to one there, between,
        # NaN or random. The removes: one of the nodes (its occluder null now and then), or none; the count 0 now and then)
        m = r.choice([0, 1, 2, 3])
        radii = sorted((float(r.randint(0, 6)) for _ in range(m)), reverse=True)
        occs = b''.join(struct.pack('<4I', ints(), ints(), r.getrandbits(32), fb(radii[i])) for i in range(m)) or bytes(0x10)
        nodes = bytearray(0xc * 4)
        for i in range(m):
            struct.pack_into('<3I', nodes, 0xc * i, OCCS + 0x10 * i, NODES + 0xc * (i + 1) if i + 1 < m else 0, NODES + 0xc * (i - 1) if i else 0)
        struct.pack_into('<3I', occluders, 4, NODES if m else 0, NODES + 0xc * (m - 1) if m else 0, m)
        occluders[0x18] = r.choice([0, 1, r.getrandbits(8)])
        if name not in ('addocc', 'occadd'):
            pick = NODES + 0xc * r.randrange(m) if m and r.random() < 0.9 else r.choice([0, 0, NODES])
            if pick and r.random() < 0.15:
                struct.pack_into('<I', nodes, pick - NODES, 0)
            if r.random() < 0.1:
                struct.pack_into('<I', occluders, 0xc, 0)
            regions.update({OCCS: occs, NODES: nodes, HEAP: heap(), PNODE: struct.pack('<I', pick)})
            return Call(CALLS[name], [OCCLUDERS if name == 'listremove' else CHAN, OUT, PNODE], regions, conv='t', ret='a')
        radius = r.choice([fb(float(r.randint(0, 7))), fb(radii[0]) if m else 0, fb(2.5), 0x7fc00000, r.getrandbits(32)])
        regions.update({OCCS: occs, NODES: nodes, HEAP: heap(fail=r.choice([0, 0, 0, 1]))})
        if name == 'occadd':
            regions[PNODE] = r.bytes(4)
            return Call(0x6b9a10, [CHAN, OUT, ints(), r.getrandbits(32), ints(), radius, PNODE], regions, conv='t', ret='a')
        return Call(0x6b8230, [CHAN, ints(), r.getrandbits(32), ints(), radius], regions, conv='t', ret='a')
    if name == 'clearocc':
        return Call(0x6b7a70, [CHAN, r.choice([0, 1, 0x100, r.getrandbits(32)])], regions, conv='t', ret='v')
    if name == 'voiceocc':   # (one voice: the record's +0xc flags, its +0x30 the position or null)
        struct.pack_into('<I', rec, 0xc, r.choice([r.getrandbits(32), 0x820, r.getrandbits(32) | 0x820, r.getrandbits(32) | 0x820, 0x800, 0x20]))
        struct.pack_into('<I', rec, 0x30, r.choice([VEC2] * 6 + [0]))
        if r.random() < 0.8:   # (the position near the occluders, as slot 0x1e's: small integers)
            regions[VEC2] = struct.pack('<3I', ints(), r.getrandbits(32), ints())
        k = r.randrange(max(n, 1))
        voice = VOICES + 0x8c * k
        struct.pack_into('<I', voices, 0x8c * k + 0x10, r.getrandbits(32) & ~3 | r.choice([0, 1, 2, 3]))   # (flag 1 and its neighbour)
        if r.random() < 0.3:
            struct.pack_into('<I', occluders, 0xc, 0)   # (no occluders: not occluded)
        return Call(0x6b7fa0, [CHAN, r.getrandbits(32), voice, REC], regions, conv='t', ret='v')
    return Call(0x6b5e60, [FUNCTOR, OUT, CHAN], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
