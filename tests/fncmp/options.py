# The game options' revert / keep and setters (src/game/core/options: 0x423ad0-0x423ba0) against the
# originals, over junk with +0x94 either sign and values equal to the field or not; the sound settings applied (0x423dd0); the defaults (0x423690);
# the save and load of their sound (0x423e90), display (0x4242c0) and other (0x424560) fields through a record stream; the options file's save
# (0x424820), load (0x424930) and the other fields' load (0x424a60): a file stream (vtable 0x6fd930) of recorders, its open's result, its reads all or part asked for (a version, then
# its bytes again for each field), its writes logged by their first dword, made in the bench heap.
# usage: options.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, Bench, Call, RAM, blob, echo, fill, heap, log_region, put, record_stream, set_return, strings, vtable  # noqa: E402

B = Bench([(0x423690, 0x423714), (0x423ad0, 0x423afc), (0x423b00, 0x423b36), (0x423b40, 0x423b4a), (0x423b60, 0x423b71),
           (0x423b80, 0x423b91), (0x423ba0, 0x423bb1), (0x423dd0, 0x423e89), (0x51b3c0, 0x51b3e0), (0x51bb80, 0x51bc50),
           (0x423e90, 0x424bac), (0x416e10, 0x416e30), (0x424c20, 0x424ca0), (0x4fae10, 0x4fae90), (0x510240, 0x5102c0), (0x5f5e20, 0x5f5e90),
           (0x5da3c0, 0x5da3c1), (0x5f5660, 0x5f5fc0), (0x5f7180, 0x5f7190), (0x67a400, 0x690000)])
OPT = RAM + 0x100
SETS = {0x423b40: 0x30, 0x423b60: 4, 0x423ba0: 8, 0x423b80: 0xc}
OPTIONS = 0x78d8d8   # the game's options (0x423dd0 applies their sound settings)
SERIALIZERS = [0x423e90, 0x4242c0, 0x424560]
STREAM, SINK = RAM + 0x1000, RAM + 0x1100
PATH, NAME = RAM + 0x1200, RAM + 0x1240   # the file's String (its text at +8) and the text
FILE_STREAM_VTABLE = 0x6fd930   # slots: 0 the deleting destructor, +4 close, +8 read, +0xc write, +0x24 open (result, path, mode)
STREAM_SLOTS = {0: 1, 2: 2, 3: 2, 4: 2, 9: 3}


def apply_sound(r):
    """0x423dd0: each of the four sound switches (+0x10, +0x18, +0x20, +0x28 bytes) on, off or junk above the low byte,
    the volumes (+0x14, +0x1c, +0x24, +0x2c) about 0 to 100, either sign and wild; the sound globals and the scale (0x785914) junk."""
    options = bytearray(r.bytes(0x40))
    for at in (0x10, 0x18, 0x20, 0x28):
        put(options, at, 'I', r.choice([0, 1, 1, 0x100, 0xff, r.getrandbits(32)]))
        put(options, at + 4, 'I', r.choice([0, 1, 50, 99, 100, 101, -1, -100, r.randrange(0, 101), r.getrandbits(32)]) & 0xffffffff)
    scale = put(bytearray(r.bytes(8)), 4, 'I', r.choice([0, 50, 100, r.randrange(0, 101), r.getrandbits(32)]))
    return Call(0x423dd0, [], {OPTIONS: options, 0x803ab0: r.bytes(0x40), 0x785910: scale, 0x802ba0: r.bytes(0x10)})


def defaults(r):
    """0x423690: on the game's options or elsewhere; the defaults (0x78591c-0x785934) junk, the switch at 0x785924 zero or not."""
    at = r.choice([OPTIONS, OPT])
    balance = bytearray(r.bytes(0x28))
    put(balance, 0x14, 'I', r.choice([0, 0, 1, 0x100, r.getrandbits(32)]))
    for k in (0x18, 0x1c, 0x20, 0x24):
        put(balance, k, 'I', r.choice([0, 50, 100, r.randrange(0, 101), r.getrandbits(32)]))
    regions = {OPTIONS: r.bytes(0x80), 0x785910: balance, 0x803ab0: r.bytes(0x40), 0x802ba0: r.bytes(0x10)}
    if at == OPT:
        regions[RAM] = r.bytes(0x200)
    return Call(0x423690, [at], regions, conv='t')


def serialize(r):
    """0x423e90 / 0x4242c0 / 0x424560: the game's options or others, saved to or loaded from a record stream with room for all, part or none
    (loading, the bytes it has random); saving any non-zero value; the sound globals as apply_sound's."""
    at = r.deal(SERIALIZERS)
    obj = r.choice([OPTIONS, OPT])
    left = r.choice([0x100, 0x100, 0x100, 0, 1, 4, 8, 0xc, 0x10, 0x14, 0x18, 0x20, 0x22, 0x24, r.randrange(0x30)])
    regions = {OPTIONS: r.bytes(0x90), STREAM: record_stream(STREAM, SINK, left), SINK: r.bytes(0x100), 0x803ab0: r.bytes(0x40),
               0x785910: put(bytearray(r.bytes(8)), 4, 'I', r.choice([0, 50, 100, r.randrange(0, 101), r.getrandbits(32)])), 0x802ba0: r.bytes(0x10)}
    if obj == OPT:
        regions[RAM] = r.bytes(0x200)
    return Call(at, [obj, STREAM, r.choice([0, 0, 1, 1, r.getrandbits(32)]), r.getrandbits(32)], regions, conv='t', ret='a')   # (the third unused)


def options_file(r):
    """0x424820 / 0x424930 / 0x424a60: the game's options or others; the open's result 0, negative or positive; reads of all asked for (mostly) or a count,
    filled with a version (0, 1, 2 or junk) and its bytes for each field; writes of all asked for (mostly) or a count; the sound globals as apply_sound's."""
    at = r.deal([0x424820, 0x424930, 0x424a60])
    obj = r.choice([OPTIONS, OPT])
    log = log_region()
    fill(log, 9, 0, struct.pack('<i', r.choice([0, 0, 0, 0, -1, 1, -0x80000000])), 1)
    strings(log, 9, 2, 1)   # (the open: the path as text, the result's address 0)
    fill(log, 2, 0, struct.pack('<I', r.choice([1, 1, 1, 0, 2, 2, 3, r.getrandbits(32)])))
    strings(log, 2, 0, 1)   # (the reads' buffers: a stack one first)
    blob(log, 3, 0, 1)   # (the writes' buffers: their first dword)
    for slot in (2, 3):
        if r.random() < 0.8:
            echo(log, slot, 1)
        else:
            set_return(log, slot, r.choice([0, 4, 3, 8, -1]))
    name = r.choice([b'options.cfg', b'C:\\tp\\x.cfg', b''])
    regions = {OPTIONS: r.bytes(0x90), HEAP: heap(0x200), LOG: log, FILE_STREAM_VTABLE: vtable(STREAM_SLOTS), PATH: struct.pack('<5I', 0, len(name), NAME, 0x20, 0),
               NAME: name + b'\0' + r.bytes(8), 0x803ab0: r.bytes(0x40), 0x785910: r.bytes(0x28), 0x802ba0: r.bytes(0x10)}
    if obj == OPT:
        regions[RAM] = r.bytes(0x200)
    return Call(at, [obj, PATH], regions, conv='t', ret='a')


def case(r):
    if r.random() < 0.25:
        return options_file(r)
    if r.random() < 0.3:
        return serialize(r)
    if r.random() < 0.2:
        return defaults(r)
    if r.random() < 0.25:
        return apply_sound(r)
    mem = put(r.bytes(0x400), 0x194, 'i', r.choice([-2, -1, 0, 1, 2, r.getrandbits(31), -r.getrandbits(31)]))
    at = r.choice([0x423ad0, 0x423b00] + list(SETS))
    if at in SETS:
        cur = struct.unpack_from('<I', mem, 0x100 + SETS[at])[0]
        return Call(at, [OPT, r.choice([cur, r.getrandbits(32)])], {RAM: mem}, conv='t')
    return Call(at, [OPT], {RAM: mem}, conv='t')


if __name__ == '__main__':
    B.main(case)
