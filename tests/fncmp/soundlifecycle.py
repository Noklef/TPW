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
# usage: soundlifecycle.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, fill, heap, log_region, recorder, set_return, strings, vtable  # noqa: E402

CALLS = {'make': (0x6ba610, 'a'), 'release': (0x6bb7c0, 'b'), 'close': (0x6c6230, 'v'), 'read': (0x6c74e0, 'a'), 'life': (0x6bb5e0, 'a')}
B = Bench([(0x41a830, 0x41a850), (0x4f7c50, 0x4f7c70), (0x5d1750, 0x5d1770), (0x5d60c0, 0x5d60e0), (0x5dc650, 0x5dc670), (0x5f5f10, 0x5f5fc0),
           (0x5f7190, 0x5f72d0), (0x4036a0, 0x4036f0), (0x639100, 0x639120), (0x618d20, 0x618d90), (0x619940, 0x619960), (0x6307c0, 0x630890), (0x6338b0, 0x633910), (0x67a740, 0x67a800),
           (0x67ad90, 0x67af10), (0x67afa0, 0x67b0d0), (0x67dcd0, 0x67ddb0), (0x67efc0, 0x67f1b0), (0x67fa20, 0x67fa50), (0x681740, 0x681970),
           (0x682720, 0x6827d0), (0x6828a0, 0x6830a0), (0x688710, 0x688740), (0x68a170, 0x68a450), (0x68efb0, 0x68f050), (0x6ba460, 0x6bb870),
           (0x6c3fc0, 0x6c4050), (0x6c5080, 0x6c50c0), (0x6c50f0, 0x6c5140), (0x6c61e0, 0x6d1bd0), (0x6d4cf0, 0x6d6c60), (0x5da3c0, 0x5da3c1)])
PLAY, REF, RES, RES_VT, STATUS, VOICE, STREAM, SREF, SREF_VT, CB, CB_VT, COPIER = (RAM + 0x100 * i for i in range(12))
CHAIN = RAM + 0x1000   # (the hand-laid chain's parts: 0x80 each)
FREE = RAM + 0x1800    # (the pools' free blocks: 0x80 each, 13 of them)
HDR, DATA2, BUF, OUT, DEC, DEC_VT = RAM + 0x3000, RAM + 0x3800, RAM + 0x4000, RAM + 0x4800, RAM + 0x4900, RAM + 0x4a00
CB_READ, CB_SEND, DEC_DELETE, DEC_CLOSE = 20, 21, 22, 23   # (the recorders' k apart from the resource's slots)
POOLS_A, POOLS_B, COUNT, COPY_AT, CLOCK, DEC_OUT = 0xfb2068, 0xfb26d8, 0xfb2058, 0xfb2018, 0xfa71e0, 0xfb23d8
# the pools' (list, next offset): POOLS_A's and POOLS_B's (the MP2 decoders' 0x2490 / 0x2c80 blocks never from the free list here)
LISTS_A = [(0xfb2068, 0x10), (0xfb2074, 0x28), (0xfb2080, 0x18), (0xfb208c, 0x10), (0xfb2098, 0x28), (0xfb20a4, 0x14), (0xfb20b0, 0x44),
           (0xfb20bc, 0x28), (0xfb20c8, 0x58)]
LISTS_B = [(0xfb26d8, 0x28), (0xfb26e4, 0x58), (0xfb26f0, 0x14)]
IMPORTS = {'EnterCriticalSection': (40, 0x6fd140, 1), 'LeaveCriticalSection': (41, 0x6fd130, 1), 'InterlockedDecrement': (42, 0x6fd12c, 1),
           'InterlockedIncrement': (43, 0x6fd0c0, 1), 'CreateSemaphoreA': (44, 0x6fd16c, 4), 'CloseHandle': (45, 0x6fd13c, 1),
           'timeGetTime': (46, 0x6fd3ec, 0), 'InitializeCriticalSection': (47, 0x6fd144, 1), 'DeleteCriticalSection': (48, 0x6fd0d0, 1),
           'WaitForSingleObject': (49, 0x6fd128, 2), 'ReleaseSemaphore': (50, 0x6fd170, 3)}
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


def case(r):
    name = r.deal(OPS)
    address, ret = CALLS[name]
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x8000, fail=r.choice([0] * 10 + [1, 2, 3])), COUNT: r.bytes(4), CLOCK: bytes(0x20),
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
    kind = r.choice([0x20, 0x20, 0x20, 0x10, 0x40, 0, 0])
    hdr[0x1b] = kind | r.getrandbits(8) & ~0x70 & (0xff if r.random() < 0.3 else 0x05)
    struct.pack_into('<I', hdr, 4, r.choice([0, r.randrange(0x40, 0x400), r.randrange(0x40, 0x400), r.getrandbits(32)]))
    struct.pack_into('<HBB', hdr, 0x18, r.choice(RATES + [r.getrandbits(16)]), r.choice([8, 0x10, 0x10, r.getrandbits(8)]), hdr[0x1b])
    struct.pack_into('<I', hdr, 0x20, r.choice([0, r.randrange(0x400), r.getrandbits(32)]))
    data = struct.pack('>I', mp2_header(r)) + r.bytes(0x3fc) if kind == 0x20 and r.random() < 0.8 else r.bytes(0x400)
    regions[HDR] = bytes(hdr) + data
    if name == 'make':
        fill(log, 6, 1, struct.pack('<I', HDR))
        strings(log, 6, 0, out=3)   # (the out and the record pointer: the factory's locals)
        strings(log, 15, 0, out=1)
        regions[STATUS] = struct.pack('<II', r.choice([0, 0, 0, 1, 0xffffffff]), r.choice([0, 0xffffffff]))
        set_return(log, 6, STATUS)
        set_return(log, 15, STATUS + 4)
        set_return(log, 13, 0 if r.random() < 0.85 else r.choice([1, r.getrandbits(32)]))
        set_return(log, 9, r.randrange(0x100))
        set_return(log, 11, STREAM)
        play = bytearray(r.bytes(0x60))
        flags = r.getrandbits(32) & ~0x20c840
        for bit in (0x8000, 0x800, 0x4000, 0x40, 0x200000):
            if r.random() < 0.4:
                flags |= bit
        struct.pack_into('<II', play, 0, r.choice([1, 2, 0x100, r.getrandbits(32), 0]) if r.random() < 0.95 else 0, REF if r.random() < 0.97 else 0)
        struct.pack_into('<I', play, 0x14, flags)
        struct.pack_into('<I', play, 0x44, r.choice([0, 1, 2, 3, r.getrandbits(32)]))
        struct.pack_into('<I', play, 0x4c, CB)   # (the 0x70a6c8 stream's callback; +0x50, the notify stream's, any: the factory makes none)
        struct.pack_into('<I', play, 0x54, r.choice([0, 10, r.randrange(1000), r.getrandbits(32)]))
        struct.pack_into('<I', play, 0x58, r.choice([0, 0, 50, r.randrange(1000)]))
        regions[PLAY] = play
        voice = VOICE if r.random() < 0.93 else 0
        regions[VOICE] = voice_obj(r, chain(r, regions, VOICE) if r.random() < 0.3 else 0)
        return Call(address, [r.getrandbits(32), voice, PLAY], regions, conv='t', ret=ret)
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
