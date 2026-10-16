"""Interpreter for Ghidra listings of the game's float code, to check reimplementations
against the original instruction stream (never against a reading of it). Nothing from the
exe runs: the listing text is interpreted, and constants are read from the file as data.

The x87 unit runs as the game leaves it, at single precision (Direct3D): every FADD/FSUB/
FMUL/FDIV/FSQRT (and FIADD...) result is the exact result rounded once to a 24-bit
significand, with the register's wide exponent (it becomes a float only when stored). FSIN/FCOS/FPATAN are exempt from precision
control on real hardware; here they are evaluated in double, standing in for 64-bit
extended. Integer flags cover what compiled MSVC 6 code tests.

    emu = Emu([listing paths])
    a = emu.alloc_floats([...])           # input data
    emu.call(0x46fc00, [m, v])           # cdecl call; returns EAX, ST0 is emu.st0()
    emu.read_floats(v, 3)

LISTING lines are "ADDRESS  MNEMONIC OPERANDS" (the DumpAsm Ghidra script). CALLs to
addresses outside the listing go to emu.hooks[address](emu), which must pop its own
x87 arguments and push its result as the original would.
"""
import math
import os
import re
import struct

EXE = os.path.join(os.path.dirname(__file__), '..', '.data', 'nocd', 'tp.exe')
SENTINEL = 0xdead0000
FS_BASE = 0x0fff0000  # FS: addresses (the thread block), in the stack's lowest 64 KB
REGS = ('EAX', 'ECX', 'EDX', 'EBX', 'ESP', 'EBP', 'ESI', 'EDI')
LOW8 = {'AL': 'EAX', 'CL': 'ECX', 'DL': 'EDX', 'BL': 'EBX'}
HIGH8 = {'AH': 'EAX', 'CH': 'ECX', 'DH': 'EDX', 'BH': 'EBX'}
LOW16 = {'AX': 'EAX', 'CX': 'ECX', 'DX': 'EDX', 'BX': 'EBX', 'SI': 'ESI', 'DI': 'EDI', 'BP': 'EBP'}
SIZES = {'byte': 1, 'word': 2, 'dword': 4, 'float': 4, 'qword': 8, 'double': 8, 'extended double': 10}


# The NaN an invalid operation (0/0, inf - inf, sqrt(-1)) makes on x86: sign set.
INDEFINITE = struct.unpack('<d', bytes.fromhex('000000000000f8ff'))[0]


def significand(x):
    return struct.unpack('<Q', struct.pack('<d', x))[0] & ((1 << 52) - 1)


def farith(fn, a, b):
    """fn(a, b) with the x87's NaN results."""
    if math.isnan(a) or math.isnan(b):  # a NaN operand: it (both: the larger significand)
        if math.isnan(a) and math.isnan(b):
            return a if significand(a) >= significand(b) else b
        return a if math.isnan(a) else b
    try:
        r = fn(a, b)
    except ZeroDivisionError:  # x / 0
        return INDEFINITE if a == 0 or math.isnan(a) else math.copysign(math.inf, a) * math.copysign(1, b)
    return INDEFINITE if math.isnan(r) else r


def r24(q):
    """The nonzero rational q rounded once to a 24-bit significand (nearest even) with the
    exponent unbounded: an x87 register at the game's precision, whose exponent range is far
    wider than a float's (it only becomes one, rounding again if subnormal, when stored)."""
    from fractions import Fraction
    sign, m = (-1.0 if q < 0 else 1.0), abs(Fraction(q))
    e = m.numerator.bit_length() - m.denominator.bit_length()
    if Fraction(2) ** e > m:
        e -= 1
    scaled = m * Fraction(2) ** (23 - e)
    n = scaled.numerator // scaled.denominator
    rest = scaled - n
    if rest > Fraction(1, 2) or (rest == Fraction(1, 2) and n & 1):
        n += 1
    return sign * math.ldexp(n, e - 23)


def x87op(fn, a, b):
    """fn(a, b) (fn exact on Fractions as on floats) as the x87 computes it at 24-bit
    precision: the exact result rounded once."""
    from fractions import Fraction
    if math.isfinite(a) and math.isfinite(b):
        try:
            q = fn(Fraction(a), Fraction(b))
        except ZeroDivisionError:
            return farith(fn, a, b)
        return r24(q) if q != 0 else fn(a, b)  # an exact zero: its sign as IEEE has it
    return farith(fn, a, b)


def f32(x):
    try:
        return struct.unpack('<f', struct.pack('<f', x))[0]
    except OverflowError:
        return math.copysign(math.inf, x)


def exe_sections():
    """(va, bytes) of the exe's sections, for constants."""
    d = open(EXE, 'rb').read()
    pe = struct.unpack_from('<I', d, 0x3c)[0]
    n = struct.unpack_from('<H', d, pe + 6)[0]
    opt = struct.unpack_from('<H', d, pe + 20)[0]
    base = struct.unpack_from('<I', d, pe + 52)[0]
    out = []
    for i in range(n):
        s = pe + 24 + opt + 40 * i
        vsize, va, rsize, raw = struct.unpack_from('<IIII', d, s + 8)
        out.append((base + va, d[raw:raw + min(rsize, vsize)]))
    return out


class MemFault(Exception):
    """An access outside what a strict emulator allows (see Emu.ranges)."""


class Emu:
    def __init__(self, listings):
        self.code = {}
        for path in listings:
            for line in open(path).read().splitlines():
                m = re.match(r'([0-9a-f]{8})\s+(\S+)\s*(.*)', line.strip())
                if m:
                    self.code[int(m.group(1), 16)] = (m.group(2), m.group(3).strip())
        self.order = sorted(self.code)
        self.next = {a: b for a, b in zip(self.order, self.order[1:])}
        self.sections = exe_sections()
        self.mem = {}
        self.ranges = None  # strict: [(lo, hi, writable)] plus 64 KB chunks (self.chunks) may be touched
        self.chunks = set()
        self.hooks = {}
        self.tail = False  # a hook was reached by a jump (the return address is at [ESP]) rather than a call (the first argument is)
        self.heap = 0x20000000
        self.reset()

    def reset(self):
        self.r = {k: 0 for k in REGS}
        self.r['ESP'] = 0x10000000
        self.st = []
        self.c0 = self.c2 = self.c3 = False
        self.cw = 0x007f  # the game's (single precision); FLDCW keeps it, arithmetic stays at 24 bits
        self.zf = self.sf = self.cf = self.of = self.pf = False
        self.df = False  # the direction flag (STD / CLD): string instructions step down when set

    # memory: written bytes first, then the exe image, else zero
    def dstep(self, n):
        return -n if self.df else n

    def allowed(self, a, write):
        if a >> 16 in self.chunks:
            return True
        return any(lo <= a < hi and (w or not write) for lo, hi, w in self.ranges)

    def rb(self, a):
        a &= 0xffffffff
        if a in self.mem:
            return self.mem[a]
        if self.ranges is not None and not self.allowed(a, False):
            raise MemFault('read of %08x' % a)
        for va, data in self.sections:
            if va <= a < va + len(data):
                return data[a - va]
        return 0

    def read(self, a, n):
        return bytes(self.rb(a + i) for i in range(n))

    def write(self, a, data):
        for i, b in enumerate(data):
            at = (a + i) & 0xffffffff
            if self.ranges is not None and at not in self.mem and not self.allowed(at, True):
                raise MemFault('write of %08x' % at)
            self.mem[at] = b

    def u32(self, a):
        return struct.unpack('<I', self.read(a, 4))[0]

    def alloc(self, n):
        a = self.heap
        self.heap += (n + 15) & ~15
        return a

    def alloc_floats(self, values):
        a = self.alloc(4 * len(values))
        self.write(a, struct.pack('<%df' % len(values), *values))
        return a

    def read_floats(self, a, n):
        return list(struct.unpack('<%df' % n, self.read(a, 4 * n)))

    # operands
    def ea(self, text):
        total = 0
        for term in re.split(r'\s*\+\s*', text.strip()):
            m = re.fullmatch(r'(E[A-Z]{2})\*0x([0-9a-f]+)', term)
            if m:
                total += self.r[m.group(1)] * int(m.group(2), 16)
            elif term in self.r:
                total += self.r[term]
            else:
                total += int(term, 16) if not term.startswith('-') else -int(term[1:], 16)
        return total & 0xffffffff

    def mem_operand(self, op):
        m = re.fullmatch(r'(?:(byte|word|dword|float|qword|double|extended double|tword) ptr )?(?:(\w+):)?\[(.*)\]', op)
        if not m:
            return None
        base = FS_BASE if m.group(2) == 'FS' else 0  # the thread block (FS:[0], the SEH chain)
        return (base + self.ea(m.group(3))) & 0xffffffff, m.group(1) or 'dword'

    def get(self, op):
        if op in self.r:
            return self.r[op]
        if op in LOW8:
            return self.r[LOW8[op]] & 0xff
        if op in HIGH8:
            return (self.r[HIGH8[op]] >> 8) & 0xff
        if op in LOW16:
            return self.r[LOW16[op]] & 0xffff
        mo = self.mem_operand(op)
        if mo:
            a, kind = mo
            return int.from_bytes(self.read(a, SIZES.get(kind, 4)), 'little')
        return int(op, 16) & 0xffffffff if not op.startswith('-') else (-int(op[1:], 16)) & 0xffffffff

    def size(self, op):
        if op in LOW8 or op in HIGH8:
            return 1
        if op in LOW16:
            return 2
        mo = self.mem_operand(op)
        return SIZES.get(mo[1], 4) if mo else 4

    def put(self, op, v):
        v &= 0xffffffff
        if op in self.r:
            self.r[op] = v
        elif op in LOW8:
            self.r[LOW8[op]] = (self.r[LOW8[op]] & ~0xff) | (v & 0xff)
        elif op in HIGH8:
            self.r[HIGH8[op]] = (self.r[HIGH8[op]] & ~0xff00) | ((v & 0xff) << 8)
        elif op in LOW16:
            self.r[LOW16[op]] = (self.r[LOW16[op]] & ~0xffff) | (v & 0xffff)
        else:
            a, kind = self.mem_operand(op)
            self.write(a, v.to_bytes(SIZES.get(kind, 4), 'little'))

    def flags(self, res, bits, cf=False, of=False):
        mask = (1 << bits) - 1
        res &= mask
        self.zf, self.sf, self.cf, self.of = res == 0, bool(res >> (bits - 1)), cf, of
        self.pf = bin(res & 0xff).count('1') % 2 == 0
        return res

    def push(self, v):
        self.r['ESP'] = (self.r['ESP'] - 4) & 0xffffffff
        self.write(self.r['ESP'], struct.pack('<I', v & 0xffffffff))

    def pop(self):
        v = self.u32(self.r['ESP'])
        self.r['ESP'] = (self.r['ESP'] + 4) & 0xffffffff
        return v

    # x87
    def fload(self, op):
        if re.fullmatch(r'ST\d', op):
            return self.st[int(op[2])]
        a, kind = self.mem_operand(op)
        # a signalling NaN loads quiet (invalid operation, masked)
        if kind == 'float':
            b = struct.unpack('<I', self.read(a, 4))[0]
            if b & 0x7f800000 == 0x7f800000 and b & 0x7fffff:
                b |= 0x400000
            return struct.unpack('<f', struct.pack('<I', b))[0]
        if kind == 'double':
            b = struct.unpack('<Q', self.read(a, 8))[0]
            if b >> 52 & 0x7ff == 0x7ff and b & (1 << 52) - 1:
                b |= 1 << 51
            return struct.unpack('<d', struct.pack('<Q', b))[0]
        if kind == 'extended double':
            b = self.read(a, 10)
            mant, se = struct.unpack('<QH', b)
            if se & 0x7fff == 0x7fff:
                return math.copysign(math.inf if mant << 1 == 0 else math.nan, -1 if se >> 15 else 1)
            return math.ldexp(mant, (se & 0x7fff) - 16383 - 63) * (-1 if se >> 15 else 1)
        raise ValueError(op)

    def fstore(self, op, v):
        if re.fullmatch(r'ST\d', op):
            self.st[int(op[2])] = v
            return
        a, kind = self.mem_operand(op)
        if kind == 'float':
            self.write(a, struct.pack('<f', f32(v)))
        elif kind == 'double':
            self.write(a, struct.pack('<d', v))
        else:
            raise ValueError(op)

    def compare(self, a, b):
        if math.isnan(a) or math.isnan(b):
            self.c0 = self.c2 = self.c3 = True
        else:
            self.c0, self.c2, self.c3 = a < b, False, a == b

    def cond(self, j):
        z, s, c, o = self.zf, self.sf, self.cf, self.of
        return {'JZ': z, 'JE': z, 'JNZ': not z, 'JNE': not z, 'JC': c, 'JB': c, 'JNC': not c, 'JAE': not c,
                'JBE': c or z, 'JA': not (c or z), 'JL': s != o, 'JGE': s == o, 'JLE': z or s != o,
                'JG': not z and s == o, 'JS': s, 'JNS': not s, 'JP': self.pf, 'JNP': not self.pf}[j]

    def call(self, addr, args, limit=1_000_000):
        """cdecl call with 32-bit args (use fbits() for floats); returns EAX."""
        self.reset()
        for a in reversed(args):
            self.push(a)
        self.push(SENTINEL)
        pc = addr
        for _ in range(limit):
            if pc == SENTINEL:
                return self.r['EAX']
            pc = self.step(pc)
        raise RuntimeError('step limit')

    def st0(self):
        return self.st[0]

    def step(self, pc):
        if pc in self.hooks:  # a tail call (jmp), into the listing or not: the hook sees the return address at [ESP]
            self.tail = True
            self.hooks[pc](self)
            return self.pop()
        if pc not in self.code:
            raise KeyError('no listing for %08x' % pc)
        mn, ops = self.code[pc]
        o = [x.strip() for x in re.split(r',(?![^\[]*\])', ops)] if ops else []
        if len(o) == 2 and mn not in ('MOVZX', 'MOVSX'):  # Ghidra leaves the size off [m] when a register implies it
            for i in (0, 1):
                if '[' in o[i] and ' ptr ' not in o[i]:
                    if o[1 - i] in LOW8 or o[1 - i] in HIGH8:
                        o[i] = 'byte ptr ' + o[i]
                    elif o[1 - i] in LOW16:
                        o[i] = 'word ptr ' + o[i]
        nxt = self.next.get(pc)
        arith = {'FADD': lambda a, b: x87op(lambda x, y: x + y, a, b),
                 'FSUB': lambda a, b: x87op(lambda x, y: x - y, a, b),
                 'FMUL': lambda a, b: x87op(lambda x, y: x * y, a, b),
                 'FDIV': lambda a, b: x87op(lambda x, y: x / y, a, b),
                 'FSUBR': lambda a, b: x87op(lambda x, y: x - y, b, a),
                 'FDIVR': lambda a, b: x87op(lambda x, y: x / y, b, a)}
        base = mn[:-1] if mn.endswith('P') and mn[:-1] in arith else mn
        if base in arith:
            fn = arith[base]
            if not o:
                o = ['ST1', 'ST0']
            if len(o) == 1 and mn != base:  # FMULP STi: STi = STi op ST0, pop
                o = [o[0], 'ST0']
            if len(o) == 1:
                self.st[0] = fn(self.st[0], self.fload(o[0]))
            else:
                d = int(o[0][2])
                self.st[d] = fn(self.st[d], self.fload(o[1]))
            if mn != base:
                self.st.pop(0)
        elif mn in ('FIADD', 'FISUB', 'FISUBR', 'FIMUL', 'FIDIV', 'FIDIVR'):
            a, kind = self.mem_operand(o[0])
            n = int.from_bytes(self.read(a, SIZES.get(kind, 4)), 'little', signed=True)
            fn = {'FIADD': lambda p, q: p + q, 'FISUB': lambda p, q: p - q, 'FISUBR': lambda p, q: q - p,
                  'FIMUL': lambda p, q: p * q, 'FIDIV': lambda p, q: p / q, 'FIDIVR': lambda p, q: q / p}[mn]
            self.st[0] = x87op(fn, self.st[0], float(n))
        elif mn == 'FLD':
            self.st.insert(0, self.fload(o[0]))
        elif mn == 'FILD':
            a, kind = self.mem_operand(o[0])
            n = SIZES.get(kind, 4) if kind != 'dword' else 4
            self.st.insert(0, float(int.from_bytes(self.read(a, n), 'little', signed=True)))
        elif mn in ('FLD1', 'FLDZ', 'FLDPI'):
            self.st.insert(0, {'FLD1': 1.0, 'FLDZ': 0.0, 'FLDPI': math.pi}[mn])
        elif mn in ('FST', 'FSTP'):
            self.fstore(o[0], self.st[0])
            if mn == 'FSTP':
                self.st.pop(0)
        elif mn in ('FIST', 'FISTP'):
            v = self.st[0]
            i = round(v) if math.isfinite(v) and -2 ** 31 <= round(v) < 2 ** 31 else -2 ** 31
            self.put(o[0], i)
            if mn == 'FISTP':
                self.st.pop(0)
        elif mn == 'FXCH':
            i = int(o[0][2]) if o else 1
            self.st[0], self.st[i] = self.st[i], self.st[0]
        elif mn == 'FCHS':
            self.st[0] = -self.st[0]
        elif mn == 'FABS':
            self.st[0] = abs(self.st[0])
        elif mn == 'FSQRT':
            v = self.st[0]
            self.st[0] = v if math.isnan(v) or v == 0 or v == math.inf else r24(math.sqrt(v)) if v > 0 else INDEFINITE
        elif mn in ('FSIN', 'FCOS'):
            v = self.st[0]
            # infinity: invalid, the indefinite; beyond 2^63: C2 set, st0 left as it is
            self.c2 = not math.isnan(v) and math.inf > abs(v) >= 2.0 ** 63
            if math.isinf(v):
                self.st[0] = INDEFINITE
            elif not math.isnan(v) and not self.c2:
                self.st[0] = (math.sin if mn == 'FSIN' else math.cos)(v)
        elif mn == 'FPATAN':
            x, y = self.st.pop(0), self.st[0]
            # a NaN operand comes through (Python's atan2 would give its own NaN)
            self.st[0] = farith(math.atan2, y, x)
        elif mn in ('FCOM', 'FCOMP', 'FCOMPP'):
            self.compare(self.st[0], self.fload(o[0]) if o else self.st[1])
            for _ in range({'FCOM': 0, 'FCOMP': 1, 'FCOMPP': 2}[mn]):
                self.st.pop(0)
        elif mn in ('FSTCW', 'FNSTCW'):
            self.put(o[0], self.cw)
        elif mn == 'FLDCW':
            self.cw = self.get(o[0])
        elif mn in ('FNSTSW', 'FSTSW'):
            self.put(o[0], (self.c0 << 8) | (self.c2 << 10) | (self.c3 << 14))
        elif mn in ('MOV', 'MOVZX'):  # get() is already zero-extended
            self.put(o[0], self.get(o[1]))
        elif mn == 'BSWAP':
            self.put(o[0], int.from_bytes(self.get(o[0]).to_bytes(4, 'little'), 'big'))
        elif mn == 'MOVSX':
            v, n = self.get(o[1]), self.size(o[1])
            self.put(o[0], v - (1 << (8 * n)) if v >> (8 * n - 1) else v)
        elif mn == 'LEA':
            self.put(o[0], self.ea(re.search(r'\[(.*)\]', o[1]).group(1)))
        elif mn in ('ADD', 'SUB', 'CMP', 'ADC', 'SBB'):
            a, b, bits = self.get(o[0]), self.get(o[1]), 8 * self.size(o[0])
            mask = (1 << bits) - 1
            b &= mask
            carry = int(mn in ('ADC', 'SBB') and self.cf)
            if mn in ('ADD', 'ADC'):
                res = a + b + carry
                sa, sb, sr = a >> (bits - 1), b >> (bits - 1), (res >> (bits - 1)) & 1
                res = self.flags(res, bits, res > mask, sa == sb and sr != sa)
            else:
                res = a - b - carry
                sa, sb, sr = a >> (bits - 1), b >> (bits - 1), (res >> (bits - 1)) & 1
                res = self.flags(res, bits, a < b + carry, sa != sb and sr != sa)
            if mn != 'CMP':
                self.put(o[0], res)
        elif mn in ('NEG', 'NOT'):
            a, bits = self.get(o[0]), 8 * self.size(o[0])
            if mn == 'NEG':
                res = self.flags(-a, bits, a != 0, a == 1 << (bits - 1))
            else:
                res = ~a & ((1 << bits) - 1)  # NOT leaves the flags
            self.put(o[0], res)
        elif mn in ('INC', 'DEC'):
            a, cf, bits = self.get(o[0]), self.cf, 8 * self.size(o[0])
            res = self.flags(a + (1 if mn == 'INC' else -1), bits, cf,
                             a == ((1 << (bits - 1)) - 1 if mn == 'INC' else 1 << (bits - 1)))
            self.put(o[0], res)
        elif mn in ('AND', 'OR', 'XOR', 'TEST'):
            a, b, bits = self.get(o[0]), self.get(o[1]), 8 * self.size(o[0])
            res = self.flags({'AND': a & b, 'TEST': a & b, 'OR': a | b, 'XOR': a ^ b}[mn], bits)
            if mn != 'TEST':
                self.put(o[0], res)
        elif mn in ('SHL', 'SAR', 'SHR'):  # CF = the last bit shifted out; a count of 0 changes nothing
            a, n, bits = self.get(o[0]), self.get(o[1]) & 31, 8 * self.size(o[0])
            if n:
                if mn == 'SAR':
                    a = a - (1 << bits) if a >> (bits - 1) else a
                res = a << n if mn == 'SHL' else a >> n
                cf = bool((a << n) >> bits & 1) if mn == 'SHL' else bool(a >> (n - 1) & 1)
                self.put(o[0], self.flags(res, bits, cf))
        elif mn in ('ROR', 'ROL'):  # of the operand's size; CF = the last bit rotated round, SF/ZF kept
            a, n, bits = self.get(o[0]), self.get(o[1]) & 31, 8 * self.size(o[0])
            if n:
                m, mask = n % bits, (1 << bits) - 1
                res = a if not m else ((a >> m) | (a << (bits - m)) if mn == 'ROR' else (a << m) | (a >> (bits - m))) & mask
                self.cf = bool(res >> (bits - 1)) if mn == 'ROR' else bool(res & 1)
                self.put(o[0], res)
        elif mn == 'MOVAPS' and len(o) == 2 and o[0] == o[1]:  # a register moved onto itself: the compiler's alignment filler
            pass
        elif mn == 'CDQ':
            self.r['EDX'] = 0xffffffff if self.r['EAX'] >> 31 else 0
        elif mn in ('MUL', 'IMUL', 'DIV', 'IDIV') and len(o) == 1:  # with EDX:EAX
            signed = lambda x, bits: x - (1 << bits) if x >> (bits - 1) else x
            v = self.get(o[0])
            if mn in ('MUL', 'IMUL'):
                full = signed(self.r['EAX'], 32) * signed(v, 32) if mn == 'IMUL' else self.r['EAX'] * v
                full &= (1 << 64) - 1
                self.r['EAX'], self.r['EDX'] = full & 0xffffffff, full >> 32
            else:
                n = self.r['EDX'] << 32 | self.r['EAX']
                if mn == 'IDIV':
                    n, v = signed(n, 64), signed(v, 32)
                if v == 0:
                    raise ZeroDivisionError('%08x: divide by zero' % pc)
                q = abs(n) // abs(v) * (1 if (n < 0) == (v < 0) else -1)
                self.r['EAX'], self.r['EDX'] = q & 0xffffffff, (n - q * v) & 0xffffffff
        elif mn == 'IMUL' and len(o) >= 2:  # IMUL r, r/m (, imm): low 32 bits, CF = OF = overflow
            signed = lambda x: x - (1 << 32) if x >> 31 else x
            a, b = (self.get(o[0]), self.get(o[1])) if len(o) == 2 else (self.get(o[1]), self.get(o[2]))
            full = signed(a) * signed(b)
            self.put(o[0], full)
            self.cf = self.of = full != signed(full & 0xffffffff)
        elif mn == 'PUSH':
            self.push(self.get(o[0]))
        elif mn == 'POP':
            self.put(o[0], self.pop())
        elif mn == 'STOSB.REP':
            while self.r['ECX']:
                self.write(self.r['EDI'], bytes([self.r['EAX'] & 0xff]))
                self.r['EDI'] = (self.r['EDI'] + self.dstep(1)) & 0xffffffff
                self.r['ECX'] -= 1
        elif mn == 'SCASB.REPNE':  # the flags are those of the last CMP AL,[EDI] (none when ECX is 0)
            while self.r['ECX']:
                b, al = self.rb(self.r['EDI']), self.r['EAX'] & 0xff
                self.r['EDI'] = (self.r['EDI'] + self.dstep(1)) & 0xffffffff
                self.r['ECX'] -= 1
                self.zf, self.cf, self.sf = al == b, al < b, bool((al - b) & 0x80)
                if self.zf:
                    break
        elif mn == 'STOSW.REP':
            while self.r['ECX']:
                self.write(self.r['EDI'], struct.pack('<H', self.r['EAX'] & 0xffff))
                self.r['EDI'] = (self.r['EDI'] + self.dstep(2)) & 0xffffffff
                self.r['ECX'] -= 1
        elif mn == 'CMPSB.REPE':  # the flags are those of the last CMP [ESI],[EDI] (none when ECX is 0)
            while self.r['ECX']:
                a, b = self.rb(self.r['ESI']), self.rb(self.r['EDI'])
                self.r['ESI'] = (self.r['ESI'] + self.dstep(1)) & 0xffffffff
                self.r['EDI'] = (self.r['EDI'] + self.dstep(1)) & 0xffffffff
                self.r['ECX'] -= 1
                self.flags(a - b, 8, a < b, bool(((a ^ b) & (a ^ (a - b))) & 0x80))
                if not self.zf:
                    break
        elif mn in ('STD', 'CLD'):
            self.df = mn == 'STD'
        elif mn == 'XCHG':
            a, b = self.get(o[0]), self.get(o[1])
            self.put(o[0], b)
            self.put(o[1], a)
        elif mn == 'LEAVE':
            self.r['ESP'] = self.r['EBP']
            self.r['EBP'] = self.pop()
        elif mn == 'JECXZ':
            return int(o[0], 16) if self.r['ECX'] == 0 else nxt
        elif mn == 'MOVSB.REP':
            while self.r['ECX']:
                self.write(self.r['EDI'], self.read(self.r['ESI'], 1))
                self.r['ESI'] = (self.r['ESI'] + self.dstep(1)) & 0xffffffff
                self.r['EDI'] = (self.r['EDI'] + self.dstep(1)) & 0xffffffff
                self.r['ECX'] -= 1
        elif mn in ('MOVSB', 'MOVSW', 'MOVSD'):  # one element, no REP
            n = {'MOVSB': 1, 'MOVSW': 2, 'MOVSD': 4}[mn]
            self.write(self.r['EDI'], self.read(self.r['ESI'], n))
            self.r['ESI'] = (self.r['ESI'] + self.dstep(n)) & 0xffffffff
            self.r['EDI'] = (self.r['EDI'] + self.dstep(n)) & 0xffffffff
        elif mn in ('MOVSD.REP', 'STOSD.REP'):
            while self.r['ECX']:
                if mn == 'MOVSD.REP':
                    self.write(self.r['EDI'], self.read(self.r['ESI'], 4))
                    self.r['ESI'] = (self.r['ESI'] + self.dstep(4)) & 0xffffffff
                else:
                    self.write(self.r['EDI'], struct.pack('<I', self.r['EAX']))
                self.r['EDI'] = (self.r['EDI'] + self.dstep(4)) & 0xffffffff
                self.r['ECX'] -= 1
        elif mn == 'JMP':  # direct, or through a register or a jump table
            return self.get(o[0])
        elif mn.startswith('SET'):
            self.put(o[0], int(self.cond('J' + mn[3:])))
        elif mn.startswith('J'):
            return int(o[0], 16) if self.cond(mn) else nxt
        elif mn == 'CALL':
            target = self.get(o[0])
            if target in self.hooks:
                self.tail = False
                self.hooks[target](self)
                return nxt
            self.push(nxt)
            return target
        elif mn == 'RET':
            ret = self.pop()
            self.r['ESP'] = (self.r['ESP'] + (int(o[0], 16) if o else 0)) & 0xffffffff
            return ret
        else:
            raise ValueError('%08x: unhandled %s %s' % (pc, mn, ops))
        return nxt


def fbits(x):
    return struct.unpack('<I', struct.pack('<f', x))[0]
