# TqiVideo_DecodeBlock (src/game/gfx/tqivideo, 0x674ab9) against the original, in its registers (conv: DC in EAX, pitch in ECX, out in EDX, dequantiser in ESI, position in EDI, bits in EBP; the position compared, ret 'edi'), with the IDCTs real.
# The streams valid, as the video writes them: random codes walked through the game's tables (x87emu.exe_sections), the end written when a run would pass the block's end (an 8-bit index that wraps and overruns the stack: not a case). Kinds:
# only the end (the DC fill), the 17 bits under 0x20 (the end, unconsumed), few short codes, long codes, escapes of each level form, full blocks; the dequantiser small or any dwords; the pitch either sign.
# usage: tqiblock.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
import x87emu  # noqa: E402
from fncmp import RAM, Bench, Call  # noqa: E402

B = Bench([(0x674ab9, 0x674faf), (0x67673c, 0x67687b), (0x676965, 0x676a6e)])
QUANT, BITS, OUT = RAM, RAM + 0x100, RAM + 0x1000
TOP = 0xfbb9ac


def exe_bytes(va, n):
    for base, data in x87emu.exe_sections():
        if base <= va < base + len(data):
            return data[va - base:va - base + n]
    raise ValueError(hex(va))


TABLES = struct.unpack('<%dI' % ((TOP - 0xfbb69c) // 4), exe_bytes(0xfbb69c, TOP - 0xfbb69c))


def entry(code):
    for least, table, shift in ((0x8000, 0xfbb69c, 13), (0x800, 0xfbb6cc, 9), (0x400, 0xfbb7ac, 7), (0x200, 0xfbb7ac, 5), (0x100, 0xfbb7ec, 4), (0x80, 0xfbb82c, 3), (0x40, 0xfbb86c, 2), (0x20, 0xfbb8ac, 1)):
        if code >= least:
            return TABLES[(table - 0xfbb69c) // 4 + (code >> shift)]
    return None


def number(r, bits):
    return ''.join(r.choice('01') for _ in range(bits))


def stream(r, kind):
    """The bits of one block (a string), as the decoder would walk them."""
    out, index = '', 0x3f
    if kind == 'end':
        return '10'
    if kind == 'zeros':
        return '0' * 12 + number(r, 5)
    steps = {'few': r.randrange(1, 6), 'long': r.randrange(1, 12), 'escape': r.randrange(1, 12), 'full': 64}[kind]
    for _ in range(steps):
        if kind == 'escape' and r.random() < 0.7:
            run = r.randrange(0, min(index, 8) + 1) if index > 0 else None
            if run is None:
                break
            index -= run + 1
            form = r.choice(['low', 'high', 'zero', 'eighty'])
            level = {'low': r.randrange(1, 0x80), 'high': r.randrange(0x81, 0x100), 'zero': 0, 'eighty': 0x80}[form]
            out += '000001' + format(run, '06b') + format(level, '08b') + (number(r, 8) if form in ('zero', 'eighty') else '')
            continue
        zeros = {'few': r.randrange(0, 3), 'long': r.randrange(0, 11), 'full': r.randrange(0, 3)}.get(kind, r.randrange(0, 6))
        bits = '0' * zeros + '1' + number(r, 16 - zeros)
        code = int(bits, 2)
        e = entry(code)
        length, run = e >> 16, e >> 8 & 0xff
        if run >= 0x41 or run == 0 or length == 0 or run > index:  # (the end and the escape have their own ways here)
            continue
        index -= run
        out += bits[:length + 1]
    return out + '10'


def pack(bits):
    bits += '0' * (-len(bits) % 32)
    return struct.pack('>%dI' % (len(bits) // 32), *[int(bits[i:i + 32], 2) for i in range(0, len(bits), 32)])


def le_dwords(data):
    return struct.pack('<%dI' % (len(data) // 4), *struct.unpack('>%dI' % (len(data) // 4), data))


def case(r):
    kind = r.deal(['end', 'zeros', 'few', 'long', 'escape', 'full'])
    start = r.randrange(0, 70)
    bits = number(r, start) + stream(r, kind) + number(r, 96)
    regions = {QUANT: struct.pack('<64I', *[r.choice([r.randrange(1, 256), r.randrange(1, 0x100000), r.getrandbits(32)]) for _ in range(64)])}
    regions[BITS] = le_dwords(pack(bits))
    regions[OUT] = r.bytes(0x1400)
    pitch = r.choice([0x40, 0x40, 0x20, 0x100, 0x140, -0x40])
    dc = r.choice([r.randrange(-2048, 2048), r.randrange(-(1 << 20), 1 << 20), r.getrandbits(32)])
    # (regs: EAX, ECX, EDX, ESI, EDI, EBP)
    return Call(0x674ab9, [dc & 0xffffffff, pitch & 0xffffffff, OUT + 0x400, QUANT, start, BITS], regions, conv='eaxecxedxesiediebp', ret='edi')


if __name__ == '__main__':
    B.main(case)
