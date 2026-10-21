# The sound resource's length and the SoundFont bank (src/game/core/sfbank) against the originals. LengthMs (0x6d4970): a resource whose
# slot 6 is a recorder (its out and format pointer on the stack logged as 0) answering a pointer to a dword of either sign and filling the
# stack buffer the format pointer points to: +4 any length (small, large, negative), +0x18 a rate (the usual ones, 0, any), +0x1a 8 or
# 0x10 or any, +0x1b any bits. Load (0x6d4aa0) and Unload (0x6d4cd0): a bank at BANK, loaded or not (+0x18 any), its synthesiser sfman.py's
# (SELF, its API recorders, slots 11, 14 and 16 set up as there) or null.
# usage: sfbank.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import LOG, RAM, Bench, Call, blob_bytes, fill, log_region, set_return, strings, vtable  # noqa: E402

B = Bench([(0x6d4970, 0x6d49e0), (0x6d4aa0, 0x6d4af6), (0x6d4cd0, 0x6d4cef),
           (0x6d3f80, 0x6d3fa1), (0x6d4530, 0x6d4565), (0x67c250, 0x67c2a0), (0x6d43c0, 0x6d449c), (0x6d44a0, 0x6d4522)])
BANK, RES, RVT, RESULT = RAM + 0x800, RAM + 0x900, RAM + 0xa00, RAM + 0xb00
CALLS = {'length': 0x6d4970, 'load': 0x6d4aa0, 'unload': 0x6d4cd0}
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
    sfman.word(log, 16, 0)
    blob_bytes(log, 16, 1, 0xf)
    strings(log, 16, 0, out=4)
    sfman.presets_env(r, log, regions)
    sfman.bank_env(r, log, me, regions)
    return Call(0x6d4aa0, [BANK, sfman.OUT, sfman.PATH, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
