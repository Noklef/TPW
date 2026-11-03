# A sound sample's whole life over the real classes (src/game/core/channelvoice, soundstream, soundsample, mp2sample, voicesample,
# streampool and the filters), against the originals, with every block pool's globals (0xfb2068-0xfb20cf, 0xfb26d8-0xfb271b: each list
# empty or one block of RAM, the locks any; the critical sections, interlocked counts, semaphores and timeGetTime recorders) and the heap
# (Mem_Alloc / Mem_Free: its frees counted and hashed) compared, so a block released twice, or into the wrong pool, shows as a differing
# free list or heap. The vtables of the samples, filters and voice are the real ones (driver.cpp's kResolvedVtables).
# - make: the sample factory ChannelVoices_CreateStream (0x6ba610) for a play record (+0 the index, 0 now and then; +4 a resource
#   reference whose resource (+8) is a recorder: slot 6 the header read (the factory's local pointer filled with HDR; its status 0 mostly,
#   negative now and then), 13 streamed or not (mostly not), 9 the offset, 11 a stream (a StreamBase over a RefObject), 15 the other kind's
#   status, 1 / 0 the AddRef / Release of the samples' source; the play's flags 0x8000, 0x800, 0x4000, 0x40, 0x200000 any way, the loops,
#   start and silence about their edges) and a header (+4 the length, 0 now and then; +0x1b the kind: MP2 0x20, voice-coded 0x10, 0x40 or
#   PCM, its bit 2 now and then; the data an MP2 frame header of the game's rates mostly, or any bytes): the samples built in their pools,
#   the filters chained, queried (the MP2 open over the data, the decoder from its pool or the heap) and, when the query fails, closed
#   again. With a voice (the real 0x70a7a8, 0x8c bytes: +4 an old chain or none, +0x2c the counter) or, now and then, none (which the
#   success path faults on, in both).
# - release: ChannelVoice_Release (0x6bb7c0) of a voice holding an old chain: (notify) -> (0x70a6c8) -> (filter) -> a memory, MP2 or
#   voice-coded sample, each part present or not; its blocks back to their pools, the MP2 sample's decoder (a recorder, or none) closed
#   and deleted, the source released.
# - close: SoundStream_Close (0x6c6230) of such a chain's top.
# - read: Stream70a6c8_Read (0x6c74e0) of a 0x70a6c8 stream over a memory sample (its data in RAM, the copier [0xfb2018] the real plain
#   one), read to its end and past: the callback (+0x18, a recorder) fills the stream's +4 (its +0xc) with a second memory sample and
#   then 0, the old ones closed into their pool; the stream then unlinks itself from the voice in front of it (slot 3).
# - life: the voice's read ChannelVoice_Read (0x6bb5e0) over that chain, or a memory sample straight under the voice (its end time +0x1c
#   set now and then): read to the end, the voice then released (ChannelVoice_Release: the chain closed into the pools).
# - stream: make with the resource streamed, the kinds dealt (MP2, voice-coded, PCM) and the status good: 0x6c87c0, 0x6c7f70 and 0x6c7780 at
#   the gate's size too (at make's 15% of streamed resources, a kind's cases are too few for 400).
# - whole: a channel's whole life as a sequence on one memory (fncmp.Call's then): 1-3 real voices of its pool (+0x14), each given a
#   stream by the factory and that stream linked under it (ChannelVoice_SetNext, as 0x6badd0 does) and read now and then; then the
#   channel's teardown (SoundChannel_ReleaseAll or SoundChannel_Dtor; the pool's ChannelVoices_Dtor, the voices' real array delete, with
#   the release of each sample into its pool) and every block pool's Shutdown (the exit's atexit order: the purge the game crashed in).
# - trip: the same, with Sound_Save and Sound_Load before the teardown: a stream written then read back. The mixer's active voices are real
#   (0x70a3e8, flags 0x410: left out of the save, as finished ones), so the load retires them through the real SoundVoice_Command, the sound
#   system's slot 4 and SoundChannelFn70a2d0_Run into ChannelVoice_Release.
# usage: soundlifecycle.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, PREV, RAM, Bench, Call, fill, heap, log_region, memory_stream, recorder, set_return, strings, vtable  # noqa: E402

CALLS = {'make': (0x6ba610, 'a'), 'stream': (0x6ba610, 'a'), 'release': (0x6bb7c0, 'b'), 'close': (0x6c6230, 'v'), 'read': (0x6c74e0, 'a'), 'life': (0x6bb5e0, 'a'),
         'whole': (0x6ba610, 'v'), 'trip': (0x6ba610, 'v'), 'runslot1': (0x6b7f00, 'a'), 'destroy': (0x6bbe80, 'v')}
B = Bench([(0x41a830, 0x41a850), (0x4f7c50, 0x4f7c70), (0x5d1750, 0x5d1770), (0x5d60c0, 0x5d60e0), (0x5dc650, 0x5dc670), (0x5f5f10, 0x5f5fc0),
           (0x5f7190, 0x5f72d0), (0x4036a0, 0x4036f0), (0x639100, 0x639120), (0x618d20, 0x618d90), (0x619940, 0x619960), (0x6307c0, 0x630890), (0x6338b0, 0x633910), (0x67a740, 0x67a800),
           (0x67ad90, 0x67af10), (0x67afa0, 0x67b0d0), (0x67dcd0, 0x67ddb0), (0x67efc0, 0x67f1b0), (0x67fa20, 0x67fa50), (0x681740, 0x681970),
           (0x682720, 0x6827d0), (0x6828a0, 0x6830a0), (0x688710, 0x688740), (0x68a170, 0x68a450), (0x68efb0, 0x68f050), (0x6ba390, 0x6ba455), (0x6ba460, 0x6bb870),
           (0x6c3fc0, 0x6c4050), (0x6c5080, 0x6c50c0), (0x6c50f0, 0x6c5140), (0x6c61e0, 0x6d1bd0), (0x6d4cf0, 0x6d6c60), (0x5da3c0, 0x5da3c1),
           # the channel's teardown
           (0x6b76d0, 0x6b774e), (0x6b7410, 0x6b7469), (0x6b9c00, 0x6b9c4d), (0x5f61e0, 0x5f61f3), (0x6b7840, 0x6b7864), (0x6b78e0, 0x6b7904), (0x6c6160, 0x6c6179),
           (0x6c4630, 0x6c4670), (0x6c4410, 0x6c4550), (0x6c47e0, 0x6c4800), (0x6c48b0, 0x6c4900), (0x6bb8d0, 0x6bb922),
           # the save and the load
           (0x51c350, 0x51c580), (0x6b5bd0, 0x6b5c60), (0x6b8b40, 0x6b8b70), (0x6b5c90, 0x6b5cc0), (0x6ba0c0, 0x6ba200), (0x6b6440, 0x6b65a0), (0x6b6930, 0x6b6bb4),
           (0x6b6c30, 0x6b7050), (0x6b7150, 0x6b7210), (0x5f5e20, 0x5f5e90), (0x5f5bf0, 0x5f5c20), (0x6b9260, 0x6b9280), (0x6b9c50, 0x6b9c80), (0x6b9cd0, 0x6b9d00), (0x6c6000, 0x6c60a0),
           (0x47db00, 0x47db30), (0x5ec210, 0x5ec230), (0x5f61a0, 0x5f61f0), (0x5f7980, 0x5f79f0), (0x5f8240, 0x5f8260), (0x632310, 0x632360), (0x67c250, 0x67c2a0),
           (0x6bead0, 0x6bed90), (0x6c0d90, 0x6c1120), (0x6c1240, 0x6c1280), (0x6c15f0, 0x6c1aa0), (0x6c2140, 0x6c2200), (0x6c2860, 0x6c29a0), (0x6c2ca0, 0x6c2d30), (0x6c31a0, 0x6c31f0),
           # the load's stop of a mixer voice
           (0x6b7f00, 0x6b7f18), (0x6bbe80, 0x6bbe88), (0x6b8d60, 0x6b8ec6), (0x6b7e60, 0x6b7ef6), (0x5d1410, 0x5d1420), (0x6bcab0, 0x6bcb50), (0x6b70d0, 0x6b70e0), (0x6b7230, 0x6b7240), (0x6bb950, 0x6bb9e2),
           (0x6baa60, 0x6baaa2), (0x6baad0, 0x6bab18), (0x6bab50, 0x6bab86)])
BASE = RAM + 0x100000   # (above the sound category's fixture, which trip's names need)
PLAY, REF, RES, RES_VT, STATUS, VOICE, STREAM, SREF, SREF_VT, CB, CB_VT, COPIER = (BASE + 0x100 * i for i in range(12))
CHAIN = BASE + 0x1000   # (the hand-laid chain's parts: 0x80 each)
FREE = BASE + 0x1800    # (the pools' free blocks: 0x80 each, 13 of them)
HDR, DATA2, BUF, OUT, DEC, DEC_VT = BASE + 0x3000, BASE + 0x3800, BASE + 0x4000, BASE + 0x4800, BASE + 0x4900, BASE + 0x4a00
ARR, CH, POOL, EVENTS, EVT, HANDLES, SRC2, SRC2VT, HOST, CO, COVT = BASE + 0x5000, *(BASE + 0x5200 + 0x80 * i for i in range(10))
OBJ, MGRH, MIX, PAGE, SAVE, SAVED, SAVEBUF, FAKE = BASE + 0x5800, BASE + 0x5880, BASE + 0x5900, BASE + 0x6000, BASE + 0x6400, BASE + 0x6480, BASE + 0x6800, BASE + 0x6500
CB_READ, CB_SEND, DEC_DELETE, DEC_CLOSE = 20, 21, 22, 23   # (the recorders' k apart from the resource's slots)
SHUTDOWNS = [0x6c8f30, 0x6c90c0, 0x6c6370, 0x6c68d0, 0x6c6c00, 0x6c7330, 0x6c79f0, 0x6c7e40, 0x6c6600, 0x6c7650, 0x6c8690, 0x6c8190, 0x6c8cd0, 0x6c7120]
POOLS_A, POOLS_B, COUNT, COPY_AT, CLOCK, DEC_OUT = 0xfb2068, 0xfb26d8, 0xfb2058, 0xfb2018, 0xfa71e0, 0xfb23d8
# the pools' (list, next offset): POOLS_A's and POOLS_B's (the MP2 decoders' 0x2490 / 0x2c80 blocks never from the free list here)
LISTS_A = [(0xfb2068, 0x10), (0xfb2074, 0x28), (0xfb2080, 0x18), (0xfb208c, 0x10), (0xfb2098, 0x28), (0xfb20a4, 0x14), (0xfb20b0, 0x44),
           (0xfb20bc, 0x28), (0xfb20c8, 0x58)]
LISTS_B = [(0xfb26d8, 0x28), (0xfb26e4, 0x58), (0xfb26f0, 0x14)]
IMPORTS = {'EnterCriticalSection': (40, 0x6fd140, 1), 'LeaveCriticalSection': (41, 0x6fd130, 1), 'InterlockedDecrement': (42, 0x6fd12c, 1),
           'InterlockedIncrement': (43, 0x6fd0c0, 1), 'CreateSemaphoreA': (44, 0x6fd16c, 4), 'CloseHandle': (45, 0x6fd13c, 1),
           'timeGetTime': (46, 0x6fd3ec, 0), 'InitializeCriticalSection': (47, 0x6fd144, 1), 'DeleteCriticalSection': (48, 0x6fd0d0, 1),
           'WaitForSingleObject': (49, 0x6fd128, 2), 'ReleaseSemaphore': (50, 0x6fd170, 3), 'SetEvent': (51, 0x6fd124, 1), 'ResetEvent': (52, 0x6fd168, 1),
           'InterlockedExchange': (53, 0x6fd200, 2)}
SVT_READ, SVT_SEEK, SVT_TELL = 25, 26, 28   # (the streamed samples' stream: its vtable's read, seek and tell recorders)
RATES = [44100, 48000, 32000, 22050, 24000, 16000, 11025, 12000, 8000]
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def mp2_header(r):
    """An MP2 frame header (big-endian dword) of the game's standards, layers, bitrates and rates; any dword now and then."""
    if r.random() < 0.1:
        return r.getrandbits(32)
    standard, index = r.choice([(0x180000, 0), (0x100000, 3), (0, 6)]), r.randrange(3)
    h = 0xffe00000 | r.getrandbits(21)
    return h & ~0x1efc00 | standard[0] | r.randrange(1, 4) << 17 | r.randrange(1, 15) << 12 | index << 10


def sample_obj(r, kind, data, length):
    """A sample's block: kind 'mem' (0x70a6f8), 'mp2' (0x70ae30) or 'voice' (0x70ad60), over data."""
    o = bytearray(r.bytes(0x60))
    vt = {'mem': 0x70a6f8, 'mp2': 0x70ae30, 'voice': 0x70ad60}[kind]
    struct.pack_into('<4I', o, 0, vt, 0, 0, r.getrandbits(32))
    pos = r.choice([0, 0, length // 2, length, r.randrange(length + 1)])
    struct.pack_into('<5I', o, 0x10, data, pos, length, 0, r.getrandbits(32))
    o[0x20] = r.choice([0x21, 0x22, 0x41, 0x42, 0x2a, 0x52])   # (the format byte: 8 / 16-bit, mono / stereo, the rate bits)
    struct.pack_into('<I', o, 0x24, r.choice([RES, RES, 0]))
    if kind == 'mp2':
        struct.pack_into('<3I', o, 0x2c, r.getrandbits(1), r.randrange(0x400), DEC if r.random() < 0.5 else 0)
        o[0x3c] = r.getrandbits(1)
    elif kind == 'voice':
        struct.pack_into('<I', o, 0x2c, r.choice([0, HEAP + 0x10]))
        struct.pack_into('<I', o, 0x38, r.choice([0, HEAP + 0x20]))
    return o


def chain(r, regions, prev):
    """A hand-laid chain at CHAIN: some of (notify, 0x70a6c8, filter) in front of a sample; returns its top."""
    parts = []
    if r.random() < 0.4:
        parts.append(('notify', 0x70a748))
    if r.random() < 0.5:
        parts.append(('wrap', 0x70a6c8))
    if r.random() < 0.4:
        parts.append(('filter', 0x70a678))
    kind = r.choice(['mem', 'mem', 'mp2', 'mp2', 'voice'])
    addrs = [CHAIN + 0x80 * i for i in range(len(parts) + 1)]
    for i, (name, vt) in enumerate(parts):
        o = bytearray(r.bytes(0x40))
        struct.pack_into('<4I', o, 0, vt, addrs[i + 1], addrs[i - 1] if i else prev, r.getrandbits(32))
        struct.pack_into('<I', o, 0x18, CB if name != 'filter' else r.randrange(6))
        regions[addrs[i]] = o
    s = sample_obj(r, kind, HDR + 0x28, r.randrange(0x40, 0x200))
    struct.pack_into('<I', s, 8, addrs[-2] if parts else prev)
    regions[addrs[-1]] = s
    return addrs[0]


def voice_obj(r, nxt):
    v = bytearray(r.bytes(0xa0))
    struct.pack_into('<II', v, 0, 0x70a7a8, nxt)
    struct.pack_into('<I', v, 0x10, r.getrandbits(32))
    struct.pack_into('<I', v, 0x2c, r.choice([0, 5, r.getrandbits(32)]))
    return v


def play_record(r, flags_any=True):
    play = bytearray(r.bytes(0x60))
    flags = r.getrandbits(32) & ~0x20c840
    for bit in (0x8000, 0x800, 0x4000, 0x40, 0x200000):
        if r.random() < 0.4:
            flags |= bit
    struct.pack_into('<II', play, 0, r.choice([1, 2, 0x100, r.getrandbits(32), 0]) if r.random() < 0.95 or not flags_any else 0, REF if r.random() < 0.97 or not flags_any else 0)
    struct.pack_into('<I', play, 0x14, flags)
    struct.pack_into('<I', play, 0x44, r.choice([0, 1, 2, 3, r.getrandbits(32)]))
    struct.pack_into('<I', play, 0x4c, CB)   # (the 0x70a6c8 stream's callback; +0x50, the notify stream's, any: the factory makes none)
    struct.pack_into('<I', play, 0x54, r.choice([0, 10, r.randrange(1000), r.getrandbits(32)]))
    struct.pack_into('<I', play, 0x58, r.choice([0, 0, 50, r.randrange(1000)]))
    return play


def channel_life(r, regions, log, name, kind_streamed):
    """whole / trip: the regions of a channel with its real voices, and the sequence up to the teardown; returns (the first call's args, the rest of the steps)."""
    streamed = kind_streamed[1]
    if streamed:   # (a read over a stream answering nothing would not end, in both)
        set_return(log, SVT_READ, r.choice([0x40, 0x400]))
    set_return(log, 13, r.choice([1, r.getrandbits(32) | 1]) if streamed else 0)
    n = r.choice([1, 2, 2, 3])
    voices = [ARR + 4 + 0x8c * i for i in range(n)]
    arr = struct.pack('<I', n)
    for _ in voices:
        v = voice_obj(r, 0)[:0x8c]
        struct.pack_into('<I', v, 0x64, HEAP + 0x40)   # (its buffer: only freed)
        arr += bytes(v)
    regions[ARR] = arr
    pool = struct.pack('<5I', r.getrandbits(32), voices[0], n, CH, EVENTS)
    events = bytearray(r.bytes(0x18))
    struct.pack_into('<4I', events, 0, EVT, r.getrandbits(32), r.getrandbits(32), r.randrange(4))
    struct.pack_into('<II', events, 0x10, HANDLES, r.getrandbits(32) & ~2)
    ch = bytearray(r.bytes(0x70))
    struct.pack_into('<I', ch, 0, 0x70a5d8)
    struct.pack_into('<5I', ch, 0x10, r.getrandbits(32), POOL, SRC2 if name == 'trip' else r.choice([SRC2, SRC2, 0]), 0, 0)   # (the load's stop faults without it)
    ch[0x28] = ch[0x28] & ~3 | r.choice([0, 1, 2])
    co = lambda: r.choice([CO, 0])
    play = play_record(r, False)
    struct.pack_into('<I', play, 0x44, r.choice([0, 1, 2, 3]))   # (a read of many loops over a stream answering nothing would not end, in both)
    regions.update({POOL: pool, EVENTS: events, EVT: vtable({0: 1}), HANDLES: r.bytes(0x10), CH: ch, SRC2: struct.pack('<I', SRC2VT) + r.bytes(0xc),
                    SRC2VT: vtable({13: 1, 0: 1, 10: 2}), CO: struct.pack('<I', COVT) + r.bytes(0xc), COVT: vtable({}, com={2: 1}),
                    0xfb2028: bytes([r.getrandbits(8) | 1]) + r.bytes(7) + struct.pack('<2I', co(), co()) + r.bytes(8), 0x782db8: bytes([16]) + r.bytes(3),
                    PLAY: play, BUF: r.bytes(0x800)})
    set_return(log, 10, r.choice([STATUS, STATUS, STATUS + 4]))   # (the source's slot 10: the voice's release, by its status)
    strings(log, 13, 0, out=1)
    strings(log, 10, 0, out=1)
    steps = [(0x6bb730, [voices[0], PREV], 't')]
    for i, v in enumerate(voices):
        if i:
            steps += [(0x6ba610, [CH, v, PLAY], 't'), (0x6bb730, [v, PREV], 't')]
        if r.random() < 0.3 and kind_streamed[0] != 0x10:   # (a voice-coded sample over these bytes may answer nothing for ever, in both; voicesample.py reads them)
            steps.append((0x6bb5e0, [v, BUF, r.choice([0, 4, 0x40, r.randrange(0x200)]), OUT, r.getrandbits(32)], 't'))
    if name == 'trip':
        steps += trip(r, regions, log, voices)
    steps.append((r.choice([0x6b76d0, 0x6b9c00]), [CH], 't'))
    steps += [(a, [], 'c') for a in SHUTDOWNS]
    return [CH, voices[0], PLAY], steps


def trip(r, regions, log, voices):
    """Sound_Save then Sound_Load over the mixer's real voices of a page, finished ones (left out of the save, retired by the load)."""
    import soundsave as ss
    import soundcategory as sc
    loc, _ = ss.located(r)
    # (the names' walk within the deque's first block: a walk past it into a block the fixture has not faults in both, and thousands of names take long; soundsave.py has those)
    while not struct.unpack_from('<I', loc[sc.SINGLETON], 0x44)[0] <= struct.unpack_from('<I', loc[sc.SINGLETON], 0x54)[0] < struct.unpack_from('<I', loc[sc.SINGLETON], 0x40)[0] or struct.unpack_from('<I', loc[sc.SINGLETON], 0x64)[0] > 0x40:
        loc, _ = ss.located(r)
    for k in loc:
        if k != LOG and k != HEAP:
            regions.setdefault(k, loc[k])
    k = r.choice([1, 1, 2])
    k = min(k, len(voices))
    page = bytearray(r.bytes(0xc + 0x50 * 3))
    struct.pack_into('<I', page, 8, 0)
    addrs = [PAGE + 0xc + 0x50 * i for i in range(k)]
    for i, a in enumerate(addrs):
        e = a - PAGE
        struct.pack_into('<I', page, e, 0x70a3e8)
        # the channel voice's handle: its index above the shift (16), the id the factory gave its stream (+0x2c, one more than the voice's), or another
        vid = struct.unpack_from('<I', regions[ARR], 4 + 0x8c * i + 0x2c)[0]
        struct.pack_into('<I', page, e + 0x24, i << 16 | (vid + 1 & 0xffff if r.random() < 0.9 else r.getrandbits(16)))
        struct.pack_into('<I', page, e + 0x2c, i)
        struct.pack_into('<I', page, e + 0x30, r.getrandbits(32) & ~0x2000 | 0x410)
        struct.pack_into('<II', page, e + 0x34, addrs[i + 1] if i + 1 < k else 0, addrs[i - 1] if i else 0)
        struct.pack_into('<I', page, e + 0x4c, 0)
        struct.pack_into('<I', page, e + 0x28, 0)   # (its dtor would call that object)
    mixer = bytearray(r.bytes(0x54))
    struct.pack_into('<4I', mixer, 0x1c, PAGE, 0, addrs[0], 1)
    obj = bytearray(r.bytes(0x40))
    struct.pack_into('<II', obj, 0x20, MIX, CH)
    g = bytearray(r.bytes(0x20))
    struct.pack_into('<II', g, 8, 1, MGRH)
    struct.pack_into('<I', g, 0x14, 1)
    save = memory_stream(SAVE, SAVEBUF, 0x4000)
    regions.update({PAGE: page, MIX: mixer, OBJ: obj, 0x802bc0: g, 0x803aa0: r.bytes(0x60), MGRH: struct.pack('<II', r.getrandbits(32), OBJ), SAVE: save,
                    SAVED: memory_stream(SAVED, SAVEBUF, 0x4000), SAVEBUF: r.bytes(0x4000), 0xfb1f24: struct.pack('<I', CH)})
    return [(0x51c350, [SAVE], 'c'), (0x51c400, [SAVED], 'c')]


def case(r):
    name = r.deal(OPS)
    address, ret = CALLS[name]
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x8000, fail=r.choice([0] * 10 + [1, 2, 3]) if name not in ('whole', 'trip') else 0), COUNT: r.bytes(4), CLOCK: bytes(0x20),
               DEC_OUT: bytes(0x300), BUF: r.bytes(0x800 if name == 'read' else 4), OUT: r.bytes(4)}
    for k, slot, n in IMPORTS.values():
        regions[slot] = struct.pack('<I', recorder(k, n, True))
    set_return(log, 46, r.getrandbits(32))
    set_return(log, 49, r.choice([0, 0, 0, 0x102]))
    set_return(log, SVT_READ, r.choice([0, 4, 0x40, 0x400, r.randrange(0x800)]))
    strings(log, SVT_READ, 0, out=1)   # (the MP2 stream decoder's open reads its header into a local)
    set_return(log, SVT_SEEK, r.choice([0, r.randrange(0x100), 0xffffffff]))
    set_return(log, SVT_TELL, r.choice([0, r.randrange(0x100)]))
    # the pools: each list empty or one free block of RAM (its next 0)
    pa, pb = bytearray(r.bytes(0x68)), bytearray(r.bytes(0x44))
    for i, (at, nxt) in enumerate(LISTS_A + LISTS_B):
        free = 0
        if r.random() < 0.3:
            free = FREE + 0x80 * i
            b = bytearray(r.bytes(0x80))
            struct.pack_into('<I', b, nxt, 0)
            regions[free] = b
        buf, base = (pa, POOLS_A) if at < POOLS_B else (pb, POOLS_B)
        struct.pack_into('<I', buf, at - base, free)
    struct.pack_into('<4I', pb, 0x2c, r.choice([0, 0, 1]), r.choice([0, 0, 1]), 0, r.getrandbits(32))   # (0xfb2704 MMX, 0xfb2708 half rate, no decoder blocks)
    struct.pack_into('<II', pb, 0x3c, 0, r.getrandbits(32))
    regions[POOLS_A], regions[POOLS_B] = pa, pb
    # the resource (the samples' source too) and its reference
    regions[REF] = struct.pack('<3I', r.getrandbits(32), r.getrandbits(32), RES)
    regions[RES] = struct.pack('<I', RES_VT) + r.bytes(0xc)
    regions[RES_VT] = vtable({0: 0, 1: 0, 6: 3, 9: 1, 11: 0, 13: 0, 15: 2})
    regions[DEC] = struct.pack('<I', DEC_VT) + r.bytes(0x3c)   # (the hand-laid MP2 samples' decoder: slot 2 the close, 0 the delete)
    regions[DEC_VT] = struct.pack('<4I', recorder(DEC_DELETE, 1), recorder(24, 0), recorder(DEC_CLOSE, 0), recorder(24, 0))
    regions[CB] = struct.pack('<I', CB_VT) + r.bytes(0xc)   # (the callback: slot 0 the 0x70a6c8 stream's next source, 1 the notify's send)
    regions[CB_VT] = struct.pack('<8I', recorder(CB_READ, 2), recorder(CB_SEND, 3), *[recorder(24, 0)] * 6)
    regions[STREAM] = struct.pack('<II', STREAM + 0x40, SREF) + r.bytes(0x38) + struct.pack(
        '<8I', *[{2: recorder(SVT_READ, 2), 4: recorder(SVT_SEEK, 2), 6: recorder(SVT_TELL, 0)}.get(i, recorder(24, 0)) for i in range(8)])
    regions[SREF] = struct.pack('<3I', SREF_VT, 0, r.getrandbits(32))
    # (the stream's source, a RefObject: slot 0 its delete, 1 the read, 3 the seek, 5 the tell (the windows' StreamBase calls), the rest)
    sref = {0: recorder(27, 1), 1: recorder(SVT_READ, 2), 2: recorder(31, 2), 3: recorder(SVT_SEEK, 2), 5: recorder(SVT_TELL, 0),
            6: recorder(31, 2), 8: recorder(31, 1)}
    regions[SREF_VT] = struct.pack('<16I', *[sref.get(k, recorder(31, 0)) for k in range(16)])
    regions[COPIER] = struct.pack('<I', 0x70aa34)
    regions[COPY_AT] = struct.pack('<I', COPIER)
    hdr = bytearray(r.bytes(0x28))
    dealt = None
    if name == 'stream':
        dealt = (r.deal([0x20, 0x10, 0]), 1)
    elif name in ('whole', 'trip'):
        dealt = r.deal([(0x20, 0), (0x20, 1), (0x10, 0), (0x10, 1), (0, 0), (0, 1)])
    kind = dealt[0] if dealt else r.choice([0x20, 0x20, 0x20, 0x10, 0x40, 0, 0])
    hdr[0x1b] = kind | r.getrandbits(8) & ~0x70 & (0xff if r.random() < 0.3 else 0x05)
    struct.pack_into('<I', hdr, 4, r.randrange(0x40, 0x400) if dealt else r.choice([0, r.randrange(0x40, 0x400), r.randrange(0x40, 0x400), r.getrandbits(32)]))
    struct.pack_into('<HBB', hdr, 0x18, r.choice(RATES + [r.getrandbits(16)]), r.choice([8, 0x10, 0x10, r.getrandbits(8)]), hdr[0x1b])
    struct.pack_into('<I', hdr, 0x20, r.choice([0, r.randrange(0x400), r.getrandbits(32)]))
    data = struct.pack('>I', mp2_header(r)) + r.bytes(0x3fc) if kind == 0x20 and r.random() < 0.8 else r.bytes(0x400)
    regions[HDR] = bytes(hdr) + data
    if name in ('make', 'stream', 'whole', 'trip'):
        fill(log, 6, 1, struct.pack('<I', HDR))
        strings(log, 6, 0, out=3)   # (the out and the record pointer: the factory's locals)
        strings(log, 15, 0, out=1)
        regions[STATUS] = struct.pack('<II', 0 if dealt else r.choice([0, 0, 0, 1, 0xffffffff]), r.choice([0, 0xffffffff]))
        set_return(log, 6, STATUS)
        set_return(log, 15, STATUS + 4)
        set_return(log, 13, r.choice([1, r.getrandbits(32) | 1]) if name == 'stream' else 0 if r.random() < 0.85 else r.choice([1, r.getrandbits(32)]))
        set_return(log, 9, r.randrange(0x100))
        set_return(log, 11, STREAM)
        if name in ('whole', 'trip'):
            args, steps = channel_life(r, regions, log, name, dealt)
            return Call(address, args, regions, conv='t', ret=ret, then=steps)
        play = play_record(r)
        regions[PLAY] = play
        voice = VOICE if r.random() < 0.93 else 0
        regions[VOICE] = voice_obj(r, chain(r, regions, VOICE) if r.random() < 0.3 else 0)
        return Call(address, [r.getrandbits(32), voice, PLAY], regions, conv='t', ret=ret)
    if name == 'runslot1':   # (the functor a recorder pair)
        regions[FAKE] = struct.pack('<4I', FAKE + 0x10, 0, 0, 0) + struct.pack('<2I', recorder(24, 0), recorder(29, 2))
        return Call(address, [CH, OUT, FAKE], regions, conv='t', ret=ret)
    if name == 'destroy':   # (slot 2 a recorder of its one argument)
        regions[FAKE] = struct.pack('<4I', FAKE + 0x10, 0, 0, 0) + struct.pack('<3I', recorder(24, 0), recorder(24, 0), recorder(29, 1))
        return Call(address, [FAKE], regions, conv='t', ret=ret)
    if name == 'release':
        regions[VOICE] = voice_obj(r, chain(r, regions, VOICE) if r.random() < 0.9 else 0)
        return Call(address, [VOICE, r.choice([0, 1, r.getrandbits(32)])], regions, conv='t', ret=ret)
    if name == 'close':
        top = chain(r, regions, VOICE)
        regions[VOICE] = voice_obj(r, top)
        return Call(address, [top], regions, conv='t', ret=ret)
    # read: VOICE -> WRAP -> sample 1; the callback gives sample 2, then 0
    wrap, s1, s2 = CHAIN, CHAIN + 0x80, CHAIN + 0x100
    w = bytearray(r.bytes(0x40))
    struct.pack_into('<5I', w, 0, 0x70a6c8, s1 if r.random() < 0.95 else 0, VOICE, wrap + 4, r.getrandbits(32))
    struct.pack_into('<I', w, 0x18, CB)
    regions[wrap] = w
    l1, l2 = r.randrange(1, 0x300), r.randrange(1, 0x300)
    for at, data, length in ((s1, HDR + 0x28, l1), (s2, DATA2, l2)):
        s = sample_obj(r, 'mem', data, length)
        struct.pack_into('<I', s, 8, wrap)
        struct.pack_into('<I', s, 0x1c, 0)
        regions[at] = s
    regions[DATA2] = r.bytes(0x300)
    n = r.randrange(1, 3)
    fill(log, CB_READ, 0, struct.pack('<2I', *([s2, 0] if n == 2 else [0, 0])), records=2)
    count = r.choice([0, 1, r.randrange(0x40), l1, l1 + 1, l1 + l2, r.randrange(0x800)])
    if name == 'life':   # (the voice's read: through the 0x70a6c8 stream, or straight from sample 1, ended now and then (+0x1c))
        direct = r.random() < 0.3
        if direct:
            struct.pack_into('<I', regions[s1], 8, VOICE)
            struct.pack_into('<I', regions[s1], 0x1c, r.choice([0, 0, 1, r.getrandbits(32)]))
        regions[VOICE] = voice_obj(r, s1 if direct else wrap)
        return Call(address, [VOICE, BUF, min(count, 0x800), OUT, r.getrandbits(32)], regions, conv='t', ret=ret)
    regions[VOICE] = voice_obj(r, wrap)
    return Call(address, [wrap, BUF, count, OUT, r.choice([1, 1, 0, r.getrandbits(32)])], regions, conv='t', ret=ret)


if __name__ == '__main__':
    B.main(case)
