# A saved game's header (src/game/world/savegamefile: 0x415f50, 0x416240) against the originals, on a memory stream under the real Stream_Read and Stream_Write: what is written compared by its region, what is read made of random fields, the stream cut short at
# any field now and then; the strings, the park and World_HasRides recorders (the text's and a local park's address not logged), the network order imports COM recorders.
# usage: savegameheader.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, log_region, mask_this, memory_stream, recorder, set_return, this_first, vtable  # noqa: E402,F401

WRITE, READ = 0x415f50, 0x416240
LANG, CTOR, EQUALS, DTOR, PARK, DIFFERS, HAS_RIDES = 1, 2, 3, 4, 5, 6, 7
TO_NET, TO_HOST = 40, 41
TH = {0x41a820: (LANG, 0), 0x5f60b0: (CTOR, 1), 0x5f7980: (EQUALS, 1), 0x5f61e0: (DTOR, 0), 0x418da0: (PARK, 4), 0x5f7e60: (DIFFERS, 1), 0x51ab60: (HAS_RIDES, 2)}
B = Bench([(0x415f50, 0x41623f), (0x416240, 0x4164b8), (0x5f5e20, 0x5f5e80), (0x657cb4, 0x657cc0), (0x5da3c0, 0x5da3c1)], hooks={a: _recorder(k, n) for a, (k, n) in TH.items()})
SELF, STREAM, TABLE, THE_PARK, DATA, NAMES = RAM, RAM + 0x20, RAM + 0x100, RAM + 0x200, RAM + 0x1000, RAM + 0x2000
FIELDS = [1, 0x500, 0x100, 4, 4]


def cut(r, data):
    if r.randrange(3):
        return data
    field = r.randrange(len(FIELDS))
    return data[:sum(FIELDS[:field]) + r.randrange(FIELDS[field])]


def case(r):
    log = log_region()
    op = r.choice([WRITE, READ])
    version = r.getrandbits(32)
    regions = {LOG: log, 0x749870: struct.pack('<I', version), 0x802080: r.bytes(0x20), 0x6fd3fc: struct.pack('<2I', recorder(TO_NET, 1, True), recorder(TO_HOST, 1, True)), 0x7cf83c: r.bytes(4), SELF: r.bytes(0x10), THE_PARK: r.bytes(0x10)}
    park = r.choice([0, THE_PARK])
    set_return(log, PARK, r.choice([0, 0, 1, 2, r.getrandbits(32)]))
    if not park:
        mask_this(log, PARK)
    if op == WRITE:
        mask_this(log, CTOR)
        mask_this(log, EQUALS)
        mask_this(log, DTOR)
        which = r.choice([0, 1, 2, 3, 4, 5, 6, 7])
        regions[TABLE] = struct.pack('<40I', *[(1 | r.getrandbits(24) << 8) if i == which else r.getrandbits(24) << 8 for i in range(40)])
        answers(log, EQUALS, TABLE)
        set_return(log, LANG, r.getrandbits(32))
        rows = bytearray(r.bytes(0x14 * 7))
        for i in range(7):
            struct.pack_into('<I', rows, 0x14 * i + 4, r.randrange(0x281))
            struct.pack_into('<I', rows, 0x14 * i + 8, NAMES)
        regions[0x78a460] = rows
        regions[NAMES] = r.bytes(0x500)
        set_return(log, TO_NET, r.getrandbits(32))
        data = cut(r, r.bytes(sum(FIELDS)))
        regions[DATA] = data + b'\0'
        regions[STREAM] = memory_stream(STREAM, DATA, len(data))
        return Call(WRITE, [SELF, STREAM, park], regions, conv='t', ret='a')
    blob(log, DIFFERS, 0, 0xffffffff)
    blob(log, HAS_RIDES, 0, 0xffffffff)
    set_return(log, DIFFERS, r.choice([0, 0, 0, 0, 0x100, 1, 0x101]))
    set_return(log, HAS_RIDES, r.choice([1, 1, 1, 1, r.getrandbits(32) | 1, 0]))
    set_return(log, TO_HOST, version if r.randrange(6) else version ^ 1)
    data = bytes([r.choice([0, 3, 6, r.getrandbits(8)])]) + r.bytes(0x500 + 0x100 + 4) + struct.pack('<I', r.choice([0, 0, 1, 1, 2, r.getrandbits(32)]))
    data = cut(r, data)
    regions[DATA] = data + b'\0'
    regions[STREAM] = memory_stream(STREAM, DATA, len(data))
    return Call(READ, [SELF, STREAM, r.choice([0, 1, 2, 2, 3]), park], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
