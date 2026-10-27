# The MP2 decoder object (src/game/core/mp2decoder) against the originals: the buffer clear (0x6cc740, fixed point 0xfb2704 or not), the
# output setup (0x6ccb10) and the two opens (0x6d18e0, 0x6d1990) over a decoder whose header fields are a stream's (the game's rates and
# bitrates, layer I-III, one or two channels; random dwords now and then, so rates below 100 fault in both), half rate (0xfb2708) now and
# then; the opens' own slots 2 (close) and 5 (header: 0 mostly, else failing) are recorders, as is the stream's slot 2 (seek). The rewind
# (0x6d1a50), the close (0x6d1a80, open or not), the stream decoder's seek (0x6cc780) and the 64-bit muldiv (0x6d1980; products past 32
# bits, quotients that overflow and divisors of 0 now and then: those fault in both). The MMX probe (0x6d1810) and the static initializer that
# keeps its answer in 0xfb2704 (0x6d1800) over a fake CPU (fncmp.cpu_region()) with the MMX bit or not. The frame's decode (0x6d17b0):
# a layer I frame case of mp2layer1.py's or a layer II one of mp2layer2.py's, the layer byte (+0x2442) 1, 2, 0, 3 or any otherwise, the
# decoder's slot 4 a recorder (0 mostly).
# usage: mp2decoder.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CPU, LOG, RAM, Bench, Call, cpu_region, log_region, set_return, vtable  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import mp2layer1  # noqa: E402
import mp2layer2  # noqa: E402

CALLS = {'clear': 0x6cc740, 'setup': 0x6ccb10, 'rewind': 0x6d1a50, 'muldiv': 0x6d1980, 'seek': 0x6cc780, 'close': 0x6d1a80,
         'open_buffer': 0x6d18e0, 'open_stream': 0x6d1990, 'hasmmx': 0x6d1810, 'initfixed': 0x6d1800, 'decode': 0x6d17b0}
B = Bench([(0x6cc740, 0x6cc7a1), (0x6ccb10, 0x6ccc13), (0x6d18e0, 0x6d1aa4), (0x6d1800, 0x6d18dd), (0x5d60c0, 0x5d60c3), (0x5da3c0, 0x5da3c1),
           (0x6d17b0, 0x6d17f0), (0x6ce5a0, 0x6d17a4), (0x6ccc20, 0x6cd218), (0x6cd220, 0x6cde4d), (0x6cdf50, 0x6ce594), (0x6c9270, 0x6c957d),
           (0x6c9580, 0x6ca574), (0x6ca580, 0x6cc735), (0x6d62c0, 0x6d6c4c)])
DECODER, VT_SELF, STREAM, VT_STREAM, BUF = RAM, RAM + 0x4000, RAM + 0x4100, RAM + 0x4200, RAM + 0x5000
VT_FRAME = RAM + 0x9000   # (the decode's own vtable: past the layer cases' frame and output)
FIXED, HALF = 0xfb2704, 0xfb2708
RATES = [44100, 48000, 32000, 22050, 24000, 16000, 11025, 12000, 8000]
BITRATES = [8, 32, 64, 128, 160, 256, 320, 384, 448, 44100]
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def i32(r):
    return r.getrandbits(32)


def decoder(r):
    d = bytearray(r.bytes(0x2c80))
    wild = r.random() < 0.1
    struct.pack_into('<I', d, 0, VT_SELF)
    struct.pack_into('<I', d, 0x2424, i32(r) if wild and r.random() < 0.5 else r.choice(RATES))
    struct.pack_into('<I', d, 0x2420, i32(r) if wild else r.choice(BITRATES))
    struct.pack_into('<I', d, 0x244c, i32(r) if wild else r.choice([1, 2]))
    d[0x2442] = r.choice([1, 1, 2, 3, r.getrandbits(8)])
    d[0x243c] = r.choice([0, 1, r.getrandbits(8)])
    struct.pack_into('<I', d, 0x2c7c, STREAM)
    return d


def case(r):
    name = r.deal(OPS)
    log = log_region()
    set_return(log, 5, r.choice([0, 0, 0, 0, 1, 0xffffffff]))
    set_return(log, 2, i32(r))
    regions = {DECODER: decoder(r), VT_SELF: vtable({2: 0, 5: 0}), STREAM: struct.pack('<I', VT_STREAM), VT_STREAM: vtable({2: 1}),
               LOG: log, FIXED: struct.pack('<I', r.choice([0, 1, i32(r)])), HALF: struct.pack('<I', r.choice([0, 0, 1, i32(r)]))}
    addr = CALLS[name]
    if name == 'decode':
        layer = r.choice([1, 1, 2, 2, 0, 3, r.getrandbits(8)])
        call = mp2layer2.case_frame(r) if layer == 2 or (layer not in (0, 1) and r.random() < 0.5) else mp2layer1.case(r)
        d = bytearray(call.regions[DECODER])
        struct.pack_into('<I', d, 0, VT_FRAME)
        d[0x2442] = layer
        set_return(log, 4, r.choice([0, 0, 0, 0, 1, i32(r)]))
        call.regions.update({DECODER: bytes(d), VT_FRAME: vtable({4: 0}), LOG: log})
        return Call(addr, call.args, call.regions, conv='t', ret='a')
    if name in ('hasmmx', 'initfixed'):   # (the fake CPU: leaf 1's EDX with bit 23, the MMX bit, or not)
        edx = i32(r) | 0x800000 if r.random() < 0.5 else i32(r) & ~0x800000
        leaves = [tuple(i32(r) for _ in range(4)), (i32(r), i32(r), i32(r), edx)] + [tuple(i32(r) for _ in range(4))] * 2
        regs = {CPU: cpu_region(1, leaves), FIXED: regions[FIXED], DECODER: bytes(4)}
        return Call(addr, [], regs, conv='c', ret='a' if name == 'hasmmx' else 'v')
    if name == 'muldiv':
        if r.random() < 0.85:
            a, b = r.randrange(-1 << 20, 1 << 20), r.randrange(-1 << 10, 1 << 10)
            c = r.choice([1, -1, r.randrange(1, 1 << 20), -r.randrange(1, 1 << 20)])
        else:
            a, b, c = i32(r), i32(r), r.choice([0, 1, -1, i32(r)])
        return Call(addr, [a & 0xffffffff, b & 0xffffffff, c & 0xffffffff], {DECODER: bytes(4)}, conv='c', ret='a')
    if name == 'seek':
        return Call(addr, [DECODER, i32(r)], regions, conv='t', ret='a')
    if name == 'open_buffer':
        regions[BUF] = r.bytes(0x10)
        return Call(addr, [DECODER, BUF, r.choice([r.randrange(0, 1 << 22), i32(r)])], regions, conv='t', ret='a')
    if name == 'open_stream':
        return Call(addr, [DECODER, STREAM, r.choice([r.randrange(0, 1 << 22), i32(r)])], regions, conv='t', ret='a')
    return Call(addr, [DECODER], regions, conv='t', ret='v' if name == 'clear' else 'a')


if __name__ == '__main__':
    B.main(case)
