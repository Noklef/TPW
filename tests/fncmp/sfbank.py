# The sound resource's length and the SoundFont bank (src/game/core/sfbank) against the originals. LengthMs (0x6d4970): a resource whose
# slot 6 is a recorder (its out and format pointer on the stack logged as 0) answering a pointer to a dword of either sign and filling the
# stack buffer the format pointer points to: +4 any length (small, large, negative), +0x18 a rate (the usual ones, 0, any), +0x1a 8 or
# 0x10 or any, +0x1b any bits. Load (0x6d4aa0) and Unload (0x6d4cd0): a bank at BANK, loaded or not (+0x18 any), its synthesiser sfman.py's
# (SELF, its API recorders, slots 11, 14 and 16 set up as there) or null. Play (0x6d4b50): a voice at VOICE (the program small, at or
# past the synthesiser's count, or any; flags bits 1, 4 and 16 in any mix; +0x5c a level in [0, 1] or any float), the synthesiser's
# messages through midiOutShortMsg as sfman.py's send (mostly an open device answering 0); its preset names (slot 16) with "_1ol" /
# "_2ol" / "_3ol" in them now and then, into a real String (the String code and the heap run on both sides). The constructor (0x6d49f0) and destructors (0x6d4a40, 0x6d4a20):
# the resource's source at SOURCE (its slots 0 and 1 recorders, k 30 and 31) or null; the unload as above.
# usage: sfbank.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, RAM, Bench, Call, blob_bytes, fill, heap, log_region, recorder, set_return, strings, vtable  # noqa: E402

B = Bench([(0x6d4970, 0x6d49e0), (0x6d4aa0, 0x6d4af6), (0x6d4cd0, 0x6d4cef),
           (0x6d3f80, 0x6d3fa1), (0x6d4530, 0x6d4565), (0x6d43c0, 0x6d449c), (0x6d44a0, 0x6d4522),
           (0x6d4b50, 0x6d4cc8), (0x6d45a0, 0x6d46da), (0x6d4570, 0x6d4595), (0x6d4340, 0x6d43b2), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000),
           (0x6d49f0, 0x6d4a15), (0x6d4a20, 0x6d4a3c), (0x6d4a40, 0x6d4a9f), (0x6d4910, 0x6d4964), (0x6338b0, 0x6338c0), (0x6c5450, 0x6c5497)])
BANK, RES, RVT, RESULT, VOICE, SOURCE = RAM + 0x800, RAM + 0x900, RAM + 0xa00, RAM + 0xb00, RAM + 0xc00, RAM + 0xd00
CALLS = {'length': 0x6d4970, 'load': 0x6d4aa0, 'unload': 0x6d4cd0, 'play': 0x6d4b50, 'ctor': 0x6d49f0, 'dtor': 0x6d4a40,
         'deldtor': 0x6d4a20}
TAGS = [b'_1ol', b'_2ol', b'_3ol', b'_1o', b'_ol', b'_1OL']
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def length(r):
    log = log_region()
    strings(log, 6, 0, out=3)
    set_return(log, 6, RESULT)
    fmt = bytearray(r.bytes(0x1c))
    struct.pack_into('<i', fmt, 4, r.choice([r.randrange(0, 0x100000), r.randrange(0, 0x10000), r.getrandbits(32) - (1 << 31), 0]))
    struct.pack_into('<H', fmt, 0x18, r.choice([11025, 22050, 44100, 8000, 0, r.getrandbits(16), r.randrange(1, 4)]))
    fmt[0x1a] = r.choice([8, 0x10, 0x10, r.getrandbits(8)])
    fill(log, 6, 1, bytes(fmt), through=0)
    regions = {RES: struct.pack('<I', RVT) + r.bytes(0x1c), RVT: vtable({6: 3}, size=12), LOG: log,
               RESULT: struct.pack('<I', r.choice([0, 1, 0xffffffff, 0x80004005, r.getrandbits(32)]))}
    return Call(0x6d4970, [RES, r.getrandbits(32)], regions, conv='t', ret='a')


def play(r, log, me, regions):
    import sfman
    count = r.choice([r.randrange(1, 0x80), r.randrange(1, 0x10), r.getrandbits(32) | 1])
    struct.pack_into('<I', me, 0x18, count)
    if r.random() < 0.7:   # (mostly an open device whose messages go through, so the values sent are seen)
        struct.pack_into('<I', me, 0x10, r.getrandbits(32) | 1)
        set_return(log, sfman.SHORT_K, 0)
    voice = bytearray(r.bytes(0x60))
    struct.pack_into('<I', voice, 0, r.choice([r.randrange(0, 0x10), count, count + 1, count + 2, 0, 1, r.getrandbits(32)]))
    flags = r.choice([0x10, 2, 0x10000, 0x10012, 0x12, 0]) | (r.getrandbits(32) & ~0x10012 if r.random() < 0.5 else 0)
    struct.pack_into('<I', voice, 0x14, flags)
    level = r.choice([r.random(), 0.0, 1.0, r.uniform(-4, 4)])
    struct.pack_into('<f', voice, 0x5c, level)
    if r.random() < 0.15:
        voice[0x5c:0x60] = r.bytes(4)
    regions[VOICE] = voice
    sfman.word(log, 16, 0)
    blob_bytes(log, 16, 1, 0xf)
    strings(log, 16, 0, out=4)
    text = r.bytes(r.randrange(0, 12)).replace(b'\0', b'a')
    if r.random() < 0.8:
        at = r.randrange(len(text) + 1)
        text = text[:at] + r.choice(TAGS) + text[at:]
        if r.random() < 0.3:
            text += r.choice(TAGS)
    fill(log, 16, 2, (text + b'\0')[:0x40], 2, through=4)
    set_return(log, 16, r.choice([0, 0, 0, 0, 1, r.getrandbits(32)]))
    regions[HEAP] = heap()
    return Call(0x6d4b50, [BANK, sfman.OUT, VOICE], regions, conv='t', ret='a')


def case(r):
    import sfman
    name = r.deal(OPS)
    if name == 'length':
        return length(r)
    log, me, regions = sfman.env(r)
    bank = bytearray(r.bytes(0x1c))
    struct.pack_into('<I', bank, 0x14, r.choice([sfman.SELF, sfman.SELF, sfman.SELF, 0]))
    struct.pack_into('<I', bank, 0x18, r.choice([0, 0, 1, r.getrandbits(32)]))
    regions[BANK] = bank
    if name == 'unload':
        return Call(0x6d4cd0, [BANK], regions, conv='t', ret='v')
    if name in ('ctor', 'dtor', 'deldtor'):
        struct.pack_into('<I', bank, 0x10, SOURCE if r.random() < 0.8 else 0)
        regions[SOURCE] = struct.pack('<III', SOURCE + 4, recorder(30, 0), recorder(31, 0))
        regions[HEAP] = heap()
        if name == 'ctor':
            return Call(0x6d49f0, [BANK, SOURCE, r.choice([sfman.SELF, 0, r.getrandbits(32)])], regions, conv='t', ret='a')
        if name == 'dtor':
            return Call(0x6d4a40, [BANK], regions, conv='t', ret='v')
        return Call(0x6d4a20, [BANK, r.choice([0, 1, r.getrandbits(32)])], regions, conv='t', ret='a')
    if name == 'play':
        return play(r, log, me, regions)
    sfman.word(log, 16, 0)
    blob_bytes(log, 16, 1, 0xf)
    strings(log, 16, 0, out=4)
    sfman.presets_env(r, log, regions)
    sfman.bank_env(r, log, me, regions)
    return Call(0x6d4aa0, [BANK, sfman.OUT, sfman.PATH, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
