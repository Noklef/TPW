"""Symbolic replay of x87 code: turns a Ghidra listing into the expression stored by each
FST/FSTP, with the exact association the original computes in.

The game runs with the x87 unit at single precision (Direct3D sets it; the game never asks
it not to). A C++ port matches bit for bit if it evaluates the same expression trees with
X87Reg (src/game/math/x87.h), which also keeps the registers' wide exponent. The left
operand of each node is the x87 destination, which decides which of two NaNs survives.

  x87sym.py LISTING START END [NAME=MEMORY ...]

LISTING is "ADDRESS  MNEMONIC OPERANDS" lines (as written by the DumpAsm Ghidra script).
NAME=MEMORY gives memory operands names, e.g. a.x="EBP + ECX*0x4" or t="ESP + 0x24";
float constants in the exe are read and shown as numbers. Unknown operands keep their text,
except ESP-relative ones, which follow PUSH/POP/SUB/ADD ESP from the start address and are
shown relative to the entry ESP: argN for the Nth stack argument, local_N for [entry - N].
General registers follow MOV/ADD/SUB/LEA/XOR from loaded slots, so [EAX + 0x324] after
MOV EAX,[ESP + 0x4]; MOV EAX,[EAX + 0x4] shows as arg0->0x4->0x324. NAME=REG (e.g. this=ECX)
names a register's value on entry. FCOM and friends print as "compare = (st0 cmp other)".
Code is replayed in address order, ignoring jumps, so after a branch rejoins, the stack
and registers are those of the fall-through path.
"""
import re
import struct
import sys
import os

EXE = os.path.join(os.path.dirname(__file__), '..', '.data', 'nocd', 'tp.exe')
FTOL = '0x0067a830'
REG = r'E(?:[A-D]X|[SB]P|[SD]I)'


def rdata_float(va):
    try:
        d = open(EXE, 'rb').read()
        return struct.unpack_from('<f', d, va - 0x401800)[0]
    except (OSError, struct.error):
        return None


class X87:
    def __init__(self, names):
        self.st = []          # st[0] is ST0
        self.names = names
        self.stores = []      # (address, destination, expression)
        self.memory = {}      # values stored earlier, by operand name
        self.esp = 0          # ESP relative to its value at the start address
        self.regs = {}        # general registers holding a known value (integer copies)

    def written(self, reg):
        """reg gets a new value: forget it, and what was stored through it as a base."""
        self.regs.pop(reg, None)
        for k in [k for k in self.memory if re.search(r'\b' + reg + r'\b', k)]:
            del self.memory[k]

    def pointer(self, reg):
        """(symbol, offset) when reg holds a known slot's value plus offset, else None."""
        v = self.regs.get(reg)
        return v if v and isinstance(v[0], str) else None

    def mem(self, operand):
        m = re.search(r'\[(.*)\]', operand)
        inner = m.group(1).strip()
        if inner in self.names:
            return self.names[inner]
        sp = re.fullmatch(r'ESP(?: ([+-]) (0x[0-9a-f]+))?', inner)
        if sp:
            k = self.esp + (-1 if sp.group(1) == '-' else 1) * int(sp.group(2) or '0', 16)
            return f'arg{(k - 4) // 4}' if k >= 4 and k % 4 == 0 else f'local_{-k:x}' if k < 0 else f'[entry+{k:x}]'
        based = re.fullmatch(r'(' + REG + r')(?: ([+-]) (0x[0-9a-f]+))?', inner)
        if based and self.pointer(based.group(1)):
            sym, off = self.pointer(based.group(1))
            off += (-1 if based.group(2) == '-' else 1) * int(based.group(3) or '0', 16)
            name = f'{sym}->{off:#x}' if off >= 0 else f'{sym}->-{-off:#x}'
            return self.names.get(name, name)

        if re.fullmatch(r'0x[0-9a-f]+', inner):
            v = rdata_float(int(inner, 16))
            if v is not None:
                return repr(v) + 'f'
        return '[' + inner + ']'

    def operand(self, text):
        text = text.strip()
        if re.fullmatch(r'ST\d', text):
            return ('st', int(text[2]))
        return ('mem', self.mem(text)) if '[' in text else ('reg', text)

    def value(self, op):
        return self.st[op[1]] if op[0] == 'st' else self.memory.get(op[1], op[1])

    def step(self, addr, mnemonic, operands):
        ops = [self.operand(o) for o in operands.split(',')] if operands else []
        arith = {'FADD': '+', 'FSUB': '-', 'FMUL': '*', 'FDIV': '/', 'FSUBR': 'r-', 'FDIVR': 'r/'}

        def apply(op, a, b):  # a op b, with the reversed forms swapping
            return ('-', b, a) if op == 'r-' else ('/', b, a) if op == 'r/' else (op, a, b)

        base = mnemonic.rstrip('P') if mnemonic not in ('FSTP',) else mnemonic
        pop = mnemonic.endswith('P') and mnemonic not in ('FSTP',)
        if mnemonic in ('FLD', 'FILD'):
            v = self.value(ops[0])
            if mnemonic == 'FILD':
                v = ('int', v)
            self.st.insert(0, v)
        elif mnemonic in ('FLD1', 'FLDZ'):
            self.st.insert(0, '1.0f' if mnemonic == 'FLD1' else '0.0f')
        elif mnemonic in ('FST', 'FSTP'):
            if ops[0][0] == 'st':
                self.st[ops[0][1]] = self.st[0]
            else:
                self.stores.append((addr, ops[0][1], self.st[0]))
                self.memory[ops[0][1]] = self.st[0]
            if mnemonic == 'FSTP':
                self.st.pop(0)
        elif mnemonic == 'FXCH':
            i = ops[0][1] if ops else 1
            self.st[0], self.st[i] = self.st[i], self.st[0]
        elif mnemonic in ('FCHS', 'FABS', 'FSQRT', 'FSIN', 'FCOS'):
            self.st[0] = ({'FCHS': 'neg'}.get(mnemonic, mnemonic[1:].lower()), self.st[0])
        elif base in arith:
            op = arith[base]
            if not ops:                       # FADDP etc.: ST1 = ST1 op ST0, pop
                ops = [('st', 1), ('st', 0)]
            if pop and len(ops) == 1:         # FMULP STi: STi = STi op ST0, pop
                ops = [ops[0], ('st', 0)]
            if len(ops) == 1:                 # ST0 = ST0 op operand
                self.st[0] = apply(op, self.st[0], self.value(ops[0]))
            else:                             # dest = dest op src
                d, s = ops
                self.st[d[1]] = apply(op, self.st[d[1]], self.value(s))
            if pop:
                self.st.pop(0)
        elif mnemonic in ('FCOM', 'FCOMP', 'FCOMPP', 'FUCOM', 'FUCOMP', 'FUCOMPP', 'FTST'):
            other = '0.0f' if mnemonic == 'FTST' else self.value(ops[0]) if ops else self.st[1]
            self.stores.append((addr, 'compare', ('cmp', self.st[0], other)))
            for _ in range(mnemonic.count('P')):
                self.st.pop(0)
        elif mnemonic in ('FNSTSW', 'FSTSW', 'FLDCW', 'FNSTCW', 'FSTCW', 'FWAIT', 'FNCLEX'):
            pass
        else:
            raise ValueError(f'{addr}: unhandled {mnemonic} {operands}')


def infix(e):
    if isinstance(e, str):
        return e
    if e[0] == 'int':
        return f'(float){infix(e[1])}'
    if e[0] == 'neg':
        return f'-{infix(e[1])}'
    if len(e) == 2:
        return f'{e[0]}({infix(e[1])})'
    return f'({infix(e[1])} {e[0]} {infix(e[2])})'


def replay(lines, start, end, names, memory=None, regs=None):
    """memory: initial values of named slots that integer moves filled (names it ignores);
    regs: {REG: name} for registers holding a named value on entry."""
    cpu = X87(names)
    cpu.memory.update(memory or {})
    cpu.regs.update({r: (n, 0) for r, n in (regs or {}).items()})
    for line in lines:
        m = re.match(r'([0-9a-f]{8})\s+(\S+)\s*(.*)', line.strip())
        if not m:
            continue
        addr, mnem, ops = int(m.group(1), 16), m.group(2), m.group(3)
        if not start <= addr < end:
            continue
        if mnem == 'CALL' and ops.strip() == FTOL:  # __ftol pops ST0 (truncated into EAX)
            cpu.st.pop(0)
            continue
        o = ops.strip()
        m32 = re.fullmatch(r'(' + REG + r'),(?:dword ptr )?(\[.*\])', o)
        if mnem == 'MOV' and m32:  # register <- memory: a float's bits, or a pointer
            v = cpu.value(('mem', cpu.mem(m32.group(2))))
            cpu.written(m32.group(1))
            cpu.regs[m32.group(1)] = (v, 0)
            continue
        m32 = re.fullmatch(r'(?:dword ptr )?(\[.*\]),(' + REG + r')', o)
        if mnem == 'MOV' and m32:  # memory <- register
            v = cpu.regs.get(m32.group(2))
            cpu.memory[cpu.mem(m32.group(1))] = (v[0] if not v[1] else f'({v[0]} + {v[1]:#x})') if v else m32.group(2)
            continue
        rr = re.fullmatch(r'(' + REG + r'),(' + REG + r'|0x[0-9a-f]+|\d+)', o)
        if rr and rr.group(1) != 'ESP' and mnem in ('MOV', 'ADD', 'SUB', 'XOR'):
            d, s = rr.groups()
            known = cpu.pointer(d) if mnem in ('ADD', 'SUB') else None
            imm = None if re.fullmatch(REG, s) else int(s, 0)
            cpu.written(d)
            if mnem == 'MOV':  # copy, or a constant
                if imm is None and s in cpu.regs:
                    cpu.regs[d] = cpu.regs[s]
                elif imm is not None:
                    cpu.regs[d] = (f'{imm:#x}', 0)
            elif mnem == 'XOR' and s == d:
                cpu.regs[d] = ('0', 0)
            elif known and imm is not None:  # pointer arithmetic
                cpu.regs[d] = (known[0], known[1] + (imm if mnem == 'ADD' else -imm))
            continue
        lea = re.fullmatch(r'(' + REG + r'),\[(' + REG + r')(?: ([+-]) (0x[0-9a-f]+))?\]', o)
        if mnem == 'LEA' and lea and lea.group(1) != 'ESP':
            d, s = lea.group(1), lea.group(2)
            disp = (-1 if lea.group(3) == '-' else 1) * int(lea.group(4) or '0', 16)
            if s == 'ESP':  # the address of a stack slot
                src = ('&' + cpu.mem('[ESP + %#x]' % disp) if disp >= 0 else None, 0)
            else:
                p = cpu.pointer(s)
                src = (p[0], p[1] + disp) if p else (None, 0)
            cpu.written(d)
            if src[0]:
                cpu.regs[d] = src
            continue
        m32 = re.match(r'(' + REG + r')\b', o)
        if m32 and not mnem.startswith('F') and mnem not in ('CMP', 'TEST', 'PUSH'):
            cpu.written(m32.group(1))  # any other write to a register forgets it
        if mnem in ('PUSH', 'POP'):
            cpu.esp += -4 if mnem == 'PUSH' else 4
            continue
        sp = re.fullmatch(r'ESP,(0x[0-9a-f]+|\d+)', ops.strip())
        if mnem in ('SUB', 'ADD') and sp:
            cpu.esp += (-1 if mnem == 'SUB' else 1) * int(sp.group(1), 0)
            continue
        if not mnem.startswith('F'):
            continue
        ops = re.sub(r'(float|double|dword|qword|tword) ptr ', '', ops)
        cpu.step(hex(addr), mnem, ops)
    return cpu


def main():
    listing, start, end = sys.argv[1], int(sys.argv[2], 16), int(sys.argv[3], 16)
    names, regs = {}, {}
    for arg in sys.argv[4:]:
        k, v = arg.split('=', 1)
        if re.fullmatch(REG, v.strip()):
            regs[v.strip()] = k
        else:
            names[v.strip()] = k
    cpu = replay(open(listing).read().splitlines(), start, end, names, regs=regs)
    for addr, dest, e in cpu.stores:
        print(f'{addr} {dest} = {infix(e)}')
    if cpu.st:
        print('left on stack:', [infix(e) for e in cpu.st])


if __name__ == '__main__':
    main()
