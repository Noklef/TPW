# The C library's _output, its helpers, sprintf and the 64 bit division (src/game/core/crtoutput) against the originals: _flsbuf, the locks and _getptd cdecl recorders, the float conversions cdecl recorders in their table, the Win32 calls COM recorders in their import slots.
# Bench limits: no precision over 0x1ff (the digits go past the buffer, onto another frame in each side); no sprintf that fills 2 GB, so its _flsbuf is not reached; a wide %Z's length is not negative (it would read on to a fault).
# usage: crt_output.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, blob, fill, import_table, log_region, recorder, set_return, strings  # noqa: E402

GETPTD, LOCK, UNLOCK, FLSBUF = 2, 7, 8, 22
CFLT, FORCE, CROP = 9, 10, 11
INC, DEC, W2M = 14, 15, 16
B = Bench([(0x67a650, 0x67a6b8), (0x680c00, 0x681480), (0x681590, 0x6815d2), (0x6815e0, 0x681619), (0x681620, 0x68165d), (0x681660, 0x681671), (0x681680, 0x681692), (0x683aa0, 0x683ab2), (0x68c350, 0x68c3b8), (0x68c3c0, 0x68c435),
           (0x687140, 0x6871ac), (0x6871b0, 0x687222), (0x686a00, 0x686a19)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x682460: _recorder(GETPTD, 0, cdecl=True), 0x680ad0: _recorder(FLSBUF, 2, cdecl=True)})
FILE, POINTER, COUNTS = RAM, RAM + 0x40, RAM + 0x80
ARGS, OUT, FORMAT, STRINGS, WIDE, COUNTED = RAM + 0x100, RAM + 0x1000, RAM + 0x6000, RAM + 0x7000, RAM + 0x7400, RAM + 0x7800
PCTYPE, PTD, A_W2M, A_FLS = RAM + 0x8000, RAM + 0x8400, RAM + 0x8500, RAM + 0x8600
NULLS, WNULLS = RAM + 0x8800, RAM + 0x8810
TEXTS = [b'', b'a', b'hello', b'\xe4\xf6', b'a longer text of some length']
WIDES = [[], [0x41], [0x41, 0x42, 0x43], [0xe4, 0x100, 0x41], [0x7a, 0x7b, 0x7c, 0x7d, 0x7e]]
INTS = [0, 1, 9, 10, 0x2a, 0xff, 0x7fff, 0x8000, 0xffff, 0x10000, 0x7fffffff, 0x80000000, 0xffffffff, 0xfffffffe, 0xffff8000, 0x12345678]
LONGS = [0, 1, 0x100000000, 0xffffffffffffffff, 0x8000000000000000, 0x7fffffffffffffff, 0x123456789abcdef0, 0xfffffffffffffffe, 10000000000]
SIZES = ['', '', '', '', 'h', 'l', 'w', 'I64', 'L', 'F', 'N', 'hl', 'lh']
OPS = [n for n in ('format', 'format', 'format', 'format', 'format', 'format', 'sprintf', 'sprintf', 'write_char', 'write_multi', 'write_string', 'get_int', 'get_short', 'get_int64', 'aulldiv', 'aullrem')
       if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def conversion(r, args):
    """A conversion's text; its arguments added to args, in order."""
    def dword(value):
        args.append(struct.pack('<I', value & 0xffffffff))

    text = '%'
    text += ''.join(r.sample(' #+-0', r.choice([0, 0, 0, 1, 1, 2, 5])))
    width = r.choice(['', '', '', '1', '5', '12', '40', '*', '*'])
    if width == '*':
        dword(r.choice([0, 3, 0x20, -1, -0x10]))
    text += width
    precision = r.choice(['', '', '', '.', '.0', '.1', '.3', '.12', '.100', '.*'])
    if precision == '.*':
        dword(r.choice([0, 2, 0x20, -1, -7]))
    text += precision
    size = r.choice(SIZES)
    text += size
    letter = r.choice('diuoxXpdxcCsSZneEfgGBsu')
    text += letter
    if letter in 'diuoxXp':
        if size == 'I64':
            args.append(struct.pack('<Q', r.choice(LONGS + [r.getrandbits(64)])))
        else:
            dword(r.choice(INTS + [r.getrandbits(32)]))
    elif letter in 'cC':
        dword(r.choice([0x41, 0x7a, 0xe4, 0x100, 0xffff, 0x10041]))
    elif letter in 'sS':
        wide = size in ('l', 'w', 'hl', 'lh') or (letter == 'S' and size not in ('h',))
        dword(r.choice([0] + [(WIDE if wide else STRINGS) + 0x40 * i for i in range(5)]))
    elif letter == 'Z':
        wide = 'w' in size
        dword(r.choice([0, COUNTED + 0x40] + [COUNTED + 8 * r.randrange(5 if wide else 7)]))
    elif letter == 'n':
        dword(COUNTS + 4 * r.randrange(0x10))
    elif letter in 'eEfgG':
        args.append(struct.pack('<d', r.choice([0.0, 1.5, -2.25, 1e100])))
    return text


def build_format(r, args):
    out = bytearray()
    for _ in range(r.choice([1, 2, 3, 5, 8, 12])):
        kind = r.random()
        if kind < 0.6:
            out += conversion(r, args).encode()
        elif kind < 0.8:
            out += r.choice([b'plain', b' ', b'!', b'\x81A', b'\x81\x82', b'\xe4z', b'x=', b'\x7f', b'\x1f', b'y'])
        else:   # (not conversions: the state machine's other ways; a lead byte with its trail, or the next conversion's % goes and its arguments shift)
            out += r.choice([b'%%', b'%!', b'%y', b'%D', b'%I', b'%I6', b'%hI6d', b'%5%', b'%.%', b'%-%', b'%h%', b'%A', b'%H', b'%\x81A', b'%-\x1f', b'%5\xe4y'])
    if r.random() < 0.05:
        out += b'\x81'   # (a lead byte at the end: its trail the terminator)
    return bytes(out)


def state(r, log, regions):
    """The C library's state, the strings and the recorders' answers."""
    set_return(log, GETPTD, PTD)
    regions[PTD] = bytes(0x80)
    answers(log, FLSBUF, A_FLS)
    regions[A_FLS] = struct.pack('<40I', *[0xffffffff if r.random() < 0.15 else r.choice([0x41, 0xff, 0]) for _ in range(40)])
    answers(log, W2M, A_W2M)
    regions[A_W2M] = struct.pack('<40I', *[r.choice([1, 1, 1, 2, 0]) for _ in range(40)])
    strings(log, W2M, 0, 0b10010100)
    fill(log, W2M, 4, b'\xc1\xc2')
    fill(log, W2M, 7, struct.pack('<I', r.choice([0, 0, 0, 1])), which=1)
    table = bytearray(0x200)
    for c in (0x81, 0x82, 0xe4):
        table[c * 2 + 1] = 0x80
    table[0x41 * 2 + 1] = 0x7f
    regions[PCTYPE] = table
    regions[0x77ea90] = struct.pack('<I', PCTYPE)
    regions[0xfb1438] = struct.pack('<3I', 0x11, 0x22, r.choice([0, 0, 1])) + bytes(0xc) + struct.pack('<I', 0x4e4)
    regions[0xfb3ca4] = struct.pack('<2I', r.choice([0, 0, 1]), 0)
    regions[0x77eca0] = struct.pack('<8I', recorder(CFLT, 5, cdecl=True), recorder(CROP, 1, cdecl=True), 0, recorder(FORCE, 1, cdecl=True), 0, 0, NULLS, WNULLS)
    regions[NULLS] = b'(null)\0\0' + bytes(8) + struct.pack('<7H', 0x28, 0x6e, 0x75, 0x6c, 0x6c, 0x29, 0)
    # A float conversion's text, in turn; cropzeros and forcdecpt change it.
    blob(log, CFLT, 0, 0b11)
    strings(log, CFLT, 0, 0b10)
    fill(log, CFLT, 1, b'-1.50\0\0\0' + b'0.5\0\0\0\0\0' + b'\0' * 8 + b'1234567\0' + b'-\0\0\0\0\0\0\0', which=2, records=5)
    strings(log, CROP, 1)
    strings(log, FORCE, 1)
    fill(log, CROP, 0, b'9.5\0', which=3)
    fill(log, FORCE, 0, b'-8.\0', which=4)
    data = bytearray(0x400)
    for i, text in enumerate(TEXTS):
        data[0x40 * i:0x40 * i + len(text)] = text
    regions[STRINGS] = data
    wide = bytearray(0x400)
    for i, units in enumerate(WIDES):
        struct.pack_into('<%dH' % len(units), wide, 0x40 * i, *units)
    regions[WIDE] = wide
    counted = bytearray(0x80)
    for i, (length, room, at) in enumerate([(2, 4, WIDE + 0x40 * 2), (4, 4, WIDE + 0x40 * 2), (6, 8, WIDE + 0x40 * 3), (0, 0, 0), (1, 1, WIDE + 0x40), (-2, 0, STRINGS + 0x80), (-0x8000, 0, STRINGS + 0x80)]):
        struct.pack_into('<hhI', counted, i * 8, length, room, at)
    struct.pack_into('<hhI', counted, 0x40, 3, 3, 0)
    regions[COUNTED] = counted
    regions[COUNTS] = r.bytes(0x40)


def make_file(r, regions):
    regions[FILE] = struct.pack('<8I', OUT, r.choice([0x7fffffff, 0x7fffffff, 0x7fffffff, 0, 1, 5, 0x10, 0x28]), OUT, 0x42, 0, 0, 0, 0)
    regions[OUT] = bytes(0x4000)


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, IMPORT_TABLE: import_table(log, {'inc': (INC, 0x6fd0c0, 1, 0, 0), 'dec': (DEC, 0x6fd12c, 1, 0, 0), 'w2m': (W2M, 0x6fd1dc, 8, 0, 0)})}
    state(r, log, regions)
    make_file(r, regions)
    if name in ('format', 'sprintf'):
        args = []
        text = build_format(r, args)
        regions[FORMAT] = text + bytes(0x10)
        if name == 'format':
            regions[ARGS] = b''.join(args) + bytes(0x100)
            return Call(0x680c00, [FILE, FORMAT, ARGS], regions, conv='c', ret='a')
        words = struct.unpack('<%dI' % (sum(map(len, args)) // 4), b''.join(args))
        return Call(0x67a650, [OUT, FORMAT, *words, 0, 0, 0, 0], regions, conv='c', ret='a')
    if name == 'write_char':
        return Call(0x681590, [r.choice([0x41, 0xff, -1, 0, 0x141]), FILE, COUNTS], regions, conv='c', ret='a')
    if name == 'write_multi':
        return Call(0x6815e0, [r.choice([0x20, 0x30, 0xe4]), r.choice([-5, -1, 0, 1, 2, 3, 0x10, 0x28]), FILE, COUNTS], regions, conv='c', ret='a')
    if name == 'write_string':
        return Call(0x681620, [STRINGS + 0x40 * r.randrange(5), r.choice([-5, -1, 0, 1, 2, 5, 0x1c]), FILE, COUNTS], regions, conv='c', ret='a')
    if name in ('get_int', 'get_short', 'get_int64'):
        regions[ARGS] = r.bytes(0x10)
        regions[POINTER] = struct.pack('<I', ARGS + r.choice([0, 4, 8]))
        address, ret = {'get_int': (0x681660, 'a'), 'get_short': (0x681680, 'w'), 'get_int64': (0x683aa0, 'i')}[name]
        return Call(address, [POINTER], regions, conv='c', ret=ret)
    value = r.choice(LONGS + [r.getrandbits(64), r.getrandbits(32), r.getrandbits(48)])
    divisor = r.choice([1, 2, 8, 10, 16, 0xffffffff, 0x100000000, 0x100000001, 0xffffffffffffffff, r.getrandbits(32) | 1, r.getrandbits(40) | 1, r.getrandbits(64) | 1])
    return Call(0x68c350 if name == 'aulldiv' else 0x68c3c0, [value & 0xffffffff, value >> 32, divisor & 0xffffffff, divisor >> 32], regions, conv='s', ret='i')


if __name__ == '__main__':
    B.main(case)
