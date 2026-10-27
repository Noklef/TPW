# The EAX 2.0 wrappers (src/game/core/soundeax) against the originals: a buffer at BUF whose +4 is an IKsPropertySet at SET (a COM
# recorder vtable; Get answers 0 or a failure now and then, else its hash); the stack block (data and returned size) is logged as 0. The
# buffer's sets (one property, its value; all, the properties at PROPS reordered): the set's +4 null now and then, Set logged with its data
# hashed and answering anything. The listener's set: an object at OBJ whose slot 4 (a recorder) fills its out with a buffer at BUFFER (or null); the buffer's QueryInterface
# (a COM recorder) filling its out with SET and answering 0 or a failure; Set's data logged as a hash of its 13 dwords.
# The buffer's open (0x6c4550) and release (0x6c4630), the channel's free of it (0x6b7840) and open (0x6b7750): see open_case().
# usage: soundeax.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, blob, blob_bytes, fill, heap, log_region, recorder, set_return, strings, vtable  # noqa: E402

CALLS = {'query': 0x6c4760, 'listener': 0x6d6cf0, 'set0b': 0x6c4670, 'setall': 0x6c46b0, 'open': 0x6c4550, 'release': 0x6c4630,
         'chaneax': 0x6b7840, 'chanopen': 0x6b7750}
B = Bench([(0x6c4760, 0x6c47a3), (0x6d6cf0, 0x6d6db0), (0x6c4670, 0x6c46a7), (0x6c46b0, 0x6c475a), (0x6c4550, 0x6c462d), (0x6c4630, 0x6c466f),
           (0x6d3a80, 0x6d3b04), (0x6b7840, 0x6b7864), (0x6b7750, 0x6b7839), (0x6c4040, 0x6c418b)])
BUF, OUT, SET, SVT = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300
OBJ, OVT, BUFFER, BVT, PROPS = RAM + 0x400, RAM + 0x500, RAM + 0x600, RAM + 0x700, RAM + 0x800
SLOT_K, QUERY_K, RELEASE_K, SET_K = 20, 21, 22, 23
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def case(r):
    name = r.deal(OPS)
    log = log_region()
    if name == 'listener':
        return listener(r, log)
    if name in ('open', 'release', 'chaneax', 'chanopen'):
        return open_case(r, log, name)
    if name in ('set0b', 'setall'):
        blob(log, SET_K, 5, 1 if name == 'set0b' else 0x3fff)
        set_return(log, SET_K, r.choice([0, 0x80004005, r.getrandbits(32)]))
        regions = {BUF: r.bytes(4) + struct.pack('<I', r.choice([SET] * 6 + [0])) + r.bytes(8), OUT: r.bytes(4),
                   SET: struct.pack('<I', SVT) + r.bytes(0xc), SVT: table({4: (SET_K, 7, True)}), PROPS: r.bytes(0x34), LOG: log}
        return Call(CALLS[name], [BUF, OUT, r.getrandbits(32) if name == 'set0b' else PROPS], regions, conv='t', ret='a')
    strings(log, 3, 0, out=1 << 5 | 1 << 7)   # Get's data and returned-size outs: one stack block
    if r.random() < 0.5:
        set_return(log, 3, r.choice([0, 0, 1, 0x80004005, 0x80070057, 0x80000000, 0x10000]))   # (failures with a low word of 0 too)
    regions = {BUF: r.bytes(4) + struct.pack('<I', SET) + r.bytes(8), OUT: r.bytes(4), SET: struct.pack('<I', SVT) + r.bytes(0xc),
               SVT: vtable({}, com={3: 8}), LOG: log}
    return Call(CALLS[name], [BUF, OUT, r.getrandbits(32)], regions, conv='t', ret='a')


def table(slots):
    """A vtable of 8 slots whose slots {slot: (k, arguments, com)} call recorder k (the others recorders 56-63, never called)."""
    return b''.join(struct.pack('<I', recorder(*slots[i]) if i in slots else recorder(56 + i, 0)) for i in range(8))


def listener(r, log):
    fill(log, SLOT_K, 1, struct.pack('<I', r.choice([BUFFER, BUFFER, BUFFER, 0])))
    strings(log, SLOT_K, 0, out=1 << 1)
    fill(log, QUERY_K, 2, struct.pack('<I', SET), which=1)
    strings(log, QUERY_K, 0, out=1 << 2)
    set_return(log, QUERY_K, r.choice([0, 0, 0, 1, 0x80004002, 0x80000000]))
    blob(log, SET_K, 5, 0x1fff)
    set_return(log, SET_K, r.getrandbits(32))
    set_return(log, RELEASE_K, r.getrandbits(32))
    regions = {OBJ: struct.pack('<I', OVT) + r.bytes(0xc), OVT: table({4: (SLOT_K, 2)}), BUFFER: struct.pack('<I', BVT) + r.bytes(0xc),
               BVT: table({0: (QUERY_K, 3, True)}), SET: struct.pack('<I', SVT) + r.bytes(0xc),
               SVT: table({2: (RELEASE_K, 1, True), 4: (SET_K, 7, True)}), PROPS: r.bytes(0x30), LOG: log}
    return Call(CALLS['listener'], [OBJ, r.getrandbits(32), PROPS], regions, conv='t', ret='v')


SOUND, SOUND_VT, D3, D3_VT, DSB, DSB_VT, KS, KS_VT = (RAM + 0x900 + 0x40 * i for i in range(8))
CHANNEL = RAM + 0xb00
CREATE_K, DSB_QI_K, D3_QI_K, SUPPORT_K, D3_REL_K, KS_REL_K, DSB_REL_K = 24, 25, 26, 27, 28, 29, 30
SOURCE, SOURCE_VT = RAM + 0xc00, RAM + 0xc40
SOURCE_K, ADDREF_K, SOUND_REL_K, GETCAPS_K = 31, 32, 33, 34
STATICS = 0xfb201c   # (Obj_CacheStatics's max and free 3D streaming buffers, then its done byte)


def open_case(r, log, name):
    """The buffer's +0 / +4 / +8 each its object (a COM recorder vtable: QueryInterface, Release, QuerySupport) or null. open: the sound (or
    null) whose CreateSoundBuffer logs the description and its format as a hash and fills the buffer (+8), each QueryInterface filling its
    out, QuerySupport the support (the rate's slot, logged as 0); each answering 0 or a failure now and then."""
    def answer(k, values):
        set_return(log, k, r.choice(values))
    for k in (D3_REL_K, KS_REL_K, DSB_REL_K):
        answer(k, [0, 1, r.getrandbits(32)])
    objects = [D3, KS, DSB] if name == 'open' and r.random() < 0.3 else [r.choice([o, o, 0]) for o in (D3, KS, DSB)]
    regions = {BUF: struct.pack('<3I', *objects) + r.bytes(4), OUT: r.bytes(4), LOG: log,
               D3: struct.pack('<I', D3_VT) + r.bytes(0xc), D3_VT: table({0: (D3_QI_K, 3, True), 2: (D3_REL_K, 1, True)}),
               KS: struct.pack('<I', KS_VT) + r.bytes(0xc), KS_VT: table({2: (KS_REL_K, 1, True), 5: (SUPPORT_K, 4, True)}),
               DSB: struct.pack('<I', DSB_VT) + r.bytes(0xc), DSB_VT: table({0: (DSB_QI_K, 3, True), 2: (DSB_REL_K, 1, True)})}
    if name == 'release':
        return Call(CALLS[name], [BUF], regions, conv='t', ret='v')
    if name == 'chanopen':   # (as chaneax's, then the source +0x18's slot 3 filling its stack out with SOUND or null; the sound's AddRef,
        # Release and GetCaps (the caps' max 3D streaming buffers small) COM recorders, the cached caps done or not; the new buffer opened as open's)
        fill(log, SOURCE_K, 0, struct.pack('<I', r.choice([SOUND] * 5 + [0])), which=0)
        strings(log, SOURCE_K, 0, out=1)
        set_return(log, SOURCE_K, r.getrandbits(32))
        caps = bytearray(r.bytes(0x40))
        struct.pack_into('<I', caps, 0x30, r.choice([0, 1, 4, 30, r.getrandbits(32)]))
        fill(log, GETCAPS_K, 1, bytes(caps), which=1, offset=4)
        answer(GETCAPS_K, [0, 0, 0, 1, 0x88780078])
        for k in (ADDREF_K, SOUND_REL_K):
            answer(k, [0, 1, r.getrandbits(32)])
        statics = struct.pack('<2I', r.choice([0, 1, 1, 4, r.getrandbits(32)]), r.getrandbits(32)) + bytes([r.choice([0, 1, 1, r.getrandbits(8)])]) + r.bytes(3)
        source = r.bytes(0xc) + struct.pack('<I', r.choice([22050, 44100, 11025, r.getrandbits(32)]))
        regions.update({HEAP: heap(0x200), CHANNEL: r.bytes(0x18) + struct.pack('<II', SOURCE, r.getrandbits(32)) + struct.pack('<I', r.choice([BUF, BUF, 0])) + r.bytes(0xc),
                        SOURCE: struct.pack('<I', SOURCE_VT) + source[4:], SOURCE_VT: table({3: (SOURCE_K, 1)}), STATICS: statics})
    if name == 'chaneax':   # (a channel whose +0x20 is the buffer or null; the buffer freed to the heap)
        regions.update({HEAP: heap(0x100), CHANNEL: r.bytes(0x20) + struct.pack('<I', r.choice([BUF, BUF, 0])) + r.bytes(0xc)})
        return Call(CALLS[name], [CHANNEL], regions, conv='t', ret='v')
    blob_bytes(log, CREATE_K, 1, 0xffff, inner=(0x10, 0x3ffff))
    fill(log, CREATE_K, 2, struct.pack('<I', DSB), which=2)
    answer(CREATE_K, [0, 0, 0, 1, 0x88780078, 0x80000000])
    fill(log, DSB_QI_K, 2, struct.pack('<I', D3), which=3)
    answer(DSB_QI_K, [0, 0, 0, 1, 0x80004002])
    fill(log, D3_QI_K, 2, struct.pack('<I', KS), which=4)
    answer(D3_QI_K, [0, 0, 0, 1, 0x80004002])
    strings(log, SUPPORT_K, 0, out=1 << 3)
    if r.random() < 0.8:   # (else the support's out left as it was: 0, set before the query)
        fill(log, SUPPORT_K, 3, struct.pack('<I', r.choice([1, 3, 0, 2, r.getrandbits(32)])), which=5)
    answer(SUPPORT_K, [0, 0, 0, 1, 0x80004001])
    regions[SOUND], regions[SOUND_VT] = struct.pack('<I', SOUND_VT) + r.bytes(0xc), table({3: (CREATE_K, 4, True)})
    if name == 'chanopen':
        regions[SOUND_VT] = table({1: (ADDREF_K, 1, True), 2: (SOUND_REL_K, 1, True), 3: (CREATE_K, 4, True), 4: (GETCAPS_K, 2, True)})
        return Call(CALLS[name], [CHANNEL, OUT], regions, conv='t', ret='a')
    rate = r.choice([22050, 44100, 11025, r.getrandbits(16), r.getrandbits(32)])
    return Call(CALLS[name], [BUF, OUT, r.choice([SOUND] * 9 + [0]), rate], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
