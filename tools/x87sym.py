"""Symbolic replay of x87 code: turns a Ghidra listing into the expression stored by each
FST/FSTP, with the exact association the original computes in.

The game runs with the x87 unit at single precision (Direct3D sets it; the game never asks
it not to), where every x87 operation rounds like an SSE float operation. A C++ port then
matches bit for bit if it evaluates the same expression trees in float. Commutation is
harmless in IEEE arithmetic (a+b == b+a), so only the tree shape has to be kept.

  x87sym.py LISTING START END [NAME=MEMORY ...]

LISTING is "ADDRESS  MNEMONIC OPERANDS" lines (as written by the DumpAsm Ghidra script).
NAME=MEMORY gives memory operands names, e.g. a.x="EBP + ECX*0x4" or t="ESP + 0x24";
float constants in the exe are read and shown as numbers. Unknown operands keep their text.
"""
import re
import struct
import sys
import os

EXE = os.path.join(os.path.dirname(__file__), '..', '.data', 'nocd', 'tp.exe')
FTOL = '0x0067a830'


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

    def mem(self, operand):
        m = re.search(r'\[(.*)\]', operand)
        inner = m.group(1).strip()
        if inner in self.names:
            return self.names[inner]
        if re.fullmatch(r'0x[0-9a-f]+', inner):
            v = rdata_float(int(inner, 16))
            if v is not None:
                return repr(v) + 'f'
        return '[' + inner + ']'

    def operand(self, text):
        text = text.strip()
        if re.fullmatch(r'ST\d', text):
            return ('st', int(text[2]))
        return ('mem', self.mem(text))

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
        elif mnemonic == 'FCHS':
            self.st[0] = ('neg', self.st[0])
        elif base in arith:
            op = arith[base]
            if not ops:                       # FADDP etc.: ST1 = ST1 op ST0, pop
                ops = [('st', 1), ('st', 0)]
            if len(ops) == 1:                 # ST0 = ST0 op operand
                self.st[0] = apply(op, self.st[0], self.value(ops[0]))
            else:                             # dest = dest op src
                d, s = ops
                self.st[d[1]] = apply(op, self.st[d[1]], self.value(s))
            if pop:
                self.st.pop(0)
        else:
            raise ValueError(f'{addr}: unhandled {mnemonic} {operands}')


def infix(e):
    if isinstance(e, str):
        return e
    if e[0] == 'int':
        return f'(float){infix(e[1])}'
    if e[0] == 'neg':
        return f'-{infix(e[1])}'
    return f'({infix(e[1])} {e[0]} {infix(e[2])})'


def replay(lines, start, end, names, memory=None):
    """memory: initial values of named slots that integer moves filled (names it ignores)."""
    cpu = X87(names)
    cpu.memory.update(memory or {})
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
        if not mnem.startswith('F'):
            continue
        ops = re.sub(r'(float|double|dword|qword|tword) ptr ', '', ops)
        cpu.step(hex(addr), mnem, ops)
    return cpu


def main():
    listing, start, end = sys.argv[1], int(sys.argv[2], 16), int(sys.argv[3], 16)
    names = {}
    for arg in sys.argv[4:]:
        k, v = arg.split('=', 1)
        names[v.strip()] = k
    cpu = replay(open(listing).read().splitlines(), start, end, names)
    for addr, dest, e in cpu.stores:
        print(f'{addr} {dest} = {infix(e)}')
    if cpu.st:
        print('left on stack:', [infix(e) for e in cpu.st])


if __name__ == '__main__':
    main()
