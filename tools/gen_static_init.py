"""Generates src/game/core/staticinit.cpp: the C++ static initialisers of tp.exe's CRT table __xc_a..__xc_z (0x727000-0x72e0e8, which
CRT_initterm 0x67fa40 runs from 0x67f90d before WinMain) and the exit thunks they register with CRT_atexit 0x67a7d0.

Each table slot is zero, a hooked function (ported already), a C library function (0x679000-0x6b0000: left as a call to its address) or a
compiler-generated initialiser, generated here:
- the main shape `PUSH text ; MOV ECX,object ; CALL ctor ; PUSH exit ; CALL 0x67a7d0 ; ADD ESP,4 ; RET` (ctor a thiscall of one stack
  argument) is a row of a data table, run by one generic runner;
- an exit thunk `MOV ECX,object ; JMP dtor` (dtor a thiscall of no stack arguments) is the template ExitThunk<object, dtor>;
- anything else is lifted instruction by instruction (registers as locals, the stack as a local frame, x87 through X87Reg, calls through
  AddrCalls' Cd / Th by address, each callee's convention from its hook's declaration). An instruction or a value the lifter cannot
  reproduce exactly (an undefined register or stack slot read, a stack argument a tail call would pass on, an indirect call) refuses
  the function: those are listed in MANUAL and ported by hand in src/game/core/staticinitmanual.cpp.

usage: gen_static_init.py [--check]   (--check: exit 1 when the generated file differs from what is committed)
       gen_static_init.py --stats     (the counts and the refusals)
analyse() is what tests/fncmp/staticinit*.py read: the slots, the generated functions, their callees.
"""
import os
import re
import struct
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import wave  # noqa: E402
import x87emu  # noqa: E402

OUT = os.path.join(ROOT, 'src', 'game', 'core', 'staticinit.cpp')
TABLE, TABLE_END = 0x727000, 0x72e0e8
CLIB_LOW, CLIB_HIGH = 0x679000, 0x6b0000
ATEXIT = 0x67a7d0
EMPTY = 0x5da3c0   # Debug_Printf: empty in the retail build (a RET)
LIBRARY = {0x67acb0: ('Mem_Free', 'C', 1)}   # C library callees (not hooked): called by address
# Import slots called through (re/symbols.csv's imp_ rows): convention and stack arguments. ?..@@QAE@XZ: a thiscall of no arguments.
IMPORTS = {'DeleteCriticalSection': ('DeleteCriticalSection', 'S', 1), 'EnterCriticalSection': ('EnterCriticalSection', 'S', 1),
           'LeaveCriticalSection': ('LeaveCriticalSection', 'S', 1), 'InitializeCriticalSection': ('InitializeCriticalSection', 'S', 1),
           'GlobalMemoryStatus': ('GlobalMemoryStatus', 'S', 1)}


def import_convention(name):
    if name in IMPORTS:
        return IMPORTS[name]
    if name and re.fullmatch(r'\?\?[01]\w+@@QAE@XZ', name):
        return (name, 'T', 0)
    return None


def import_names():
    out = {}
    for line in open(os.path.join(ROOT, 're', 'symbols.csv')):
        m = re.match(r'0x([0-9a-fA-F]+),imp_([^,]+),', line)
        if m:
            out[int(m.group(1), 16)] = m.group(2)
    return out
# Initialisers the lifter refuses, ported by hand (src/game/core/staticinitmanual.cpp: StaticInit_Manual), with the reason.
MANUAL = {0x412b90: 'stores the top byte of the ECX it was entered with (an uninitialised byte)',
          0x583540: 'reads the MEMORYSTATUS GlobalMemoryStatus fills on its stack'}

REG32 = ('EAX', 'ECX', 'EDX', 'EBX', 'ESP', 'EBP', 'ESI', 'EDI')
REG16 = {'AX': 'EAX', 'CX': 'ECX', 'DX': 'EDX', 'BX': 'EBX', 'SI': 'ESI', 'DI': 'EDI', 'BP': 'EBP'}
REG8 = {'AL': ('EAX', 0), 'CL': ('ECX', 0), 'DL': ('EDX', 0), 'BL': ('EBX', 0), 'AH': ('EAX', 1), 'CH': ('ECX', 1), 'DH': ('EDX', 1), 'BH': ('EBX', 1)}
SIZES = {'byte': 1, 'word': 2, 'dword': 4, 'float': 4, 'double': 8, 'qword': 8}
CTYPE = {1: 'u8', 2: 'u16', 4: 'u32'}


class Refuse(Exception):
    pass


def hooks_and_conventions():
    """{address: (name, kind, stack arguments)} of the hooked functions: kind 'T' thiscall (a __fastcall port), 'C' cdecl, 'S' stdcall,
    None when the declaration is not found."""
    rows = re.findall(r'\{"([A-Za-z0-9_]+)",\s*0x([0-9a-fA-F]+),', open(os.path.join(ROOT, 'src', 'hook', 'hooks.cpp')).read())
    decls = {}
    for d, _, files in os.walk(os.path.join(ROOT, 'src', 'game')):
        for f in files:
            if f.endswith('.h'):
                text = re.sub(r'//[^\n]*', '', open(os.path.join(d, f), errors='replace').read())
                for m in re.finditer(r'([A-Za-z_][\w \t\*:<>,]*?)\b(__cdecl|__stdcall|__fastcall|__thiscall)?\s*\*?\s*\b([A-Za-z0-9_]+)\s*\(([^()]*(?:\([^()]*\)[^()]*)*)\)\s*;', text):
                    decls.setdefault(m.group(3), (m.group(2) or '', m.group(4)))
    out = {}
    for name, addr in rows:
        conv, params = decls.get(name, (None, ''))
        ps = [p for p in (q.strip() for q in re.split(r',(?![^()]*\))', params)) if p and p != 'void']
        kind = None if conv is None else 'T' if conv == '__fastcall' else 'S' if conv == '__stdcall' else 'C' if conv in ('', '__cdecl') else None
        n = None if kind is None else max(len(ps) - 2, 0) if kind == 'T' else len(ps)
        out[int(addr, 16)] = (name, kind, n)
    return out


def table_slots():
    for base, data in x87emu.exe_sections():
        if base <= TABLE < base + len(data):
            return list(struct.unpack_from('<%dI' % ((TABLE_END - TABLE) // 4), data, TABLE - base))
    raise SystemExit('no table')


_IDX = None


def function_body(start, starts=frozenset()):
    """(address, instruction) of the function at start, in address order: what its jumps reach. A JMP to one of starts (other functions)
    or more than 0x1000 bytes away is a tail call."""
    global _IDX
    if _IDX is None:
        _IDX = wave.index()
    lines, out, todo = wave.asm_lines(), {}, [start]
    while todo:
        va = todo.pop()
        if va in out:
            continue
        if va not in _IDX:
            raise Refuse('%x not in the listing' % va)
        i = _IDX[va]
        while True:
            va, ins = int(lines[i][:8], 16), lines[i][8:].strip()
            if va in out:
                break
            out[va] = ins
            i += 1
            while not re.match(r'[0-9a-f]{8}  ', lines[i]):
                i += 1
            m = re.fullmatch(r'J\w+ 0x([0-9a-f]+)', ins)
            if m:
                t = int(m.group(1), 16)
                if not ins.startswith('JMP'):
                    todo.append(t)
                elif t not in starts and abs(t - start) < 0x1000:
                    todo.append(t)
            if ins.startswith('RET') or ins.startswith('JMP'):
                break
        if len(out) > 2000:
            raise Refuse('no end')
    return sorted(out.items())


def imm(s):
    s = s.strip()
    if re.fullmatch(r'-?0x[0-9a-fA-F]+|-?\d+', s):
        return int(s, 0) & 0xffffffff
    return None


def hx(v):
    return '0x%xu' % (v & 0xffffffff)


class Lifter:
    """One function lifted to C++ statements. Static state: ESP relative to the entry (0: the return address), the defined bytes of each
    register and of the frame, the x87 stack as names of X87Reg locals."""

    def __init__(self, gen, address, body):
        self.gen, self.address, self.body = gen, address, body
        self.lines, self.esp, self.low = [], 0, 0
        self.defined = {r: 0 for r in REG32}
        self.defined['ESP'] = 0xf
        self.frame = set()      # defined frame bytes (entry-relative offsets)
        self.fpu, self.fpus = [], 0
        self.used = set()       # register locals the code names
        self.flag_at = None     # (line index, flag statement) of the last flag-setting instruction
        self.labels, self.states, self.need, self.label_states = set(), {}, set(), {}
        self.callees = []       # (address, kind, n) of each call
        self.thunk_slots = {}   # frame offset -> exit thunk address pushed there

    # --- operands
    def mem(self, s):
        m = re.fullmatch(r'(?:(byte|word|dword|float|double|qword) ptr )?(?:[A-Z]S:)?\[(.*)\]', s.strip())
        if not m:
            return None
        size = SIZES.get(m.group(1)) if m.group(1) else None
        regs, disp = [], 0
        for t in m.group(2).split(' + '):
            t = t.strip()
            if imm(t) is not None:
                disp += int(t, 0)
            elif '*' in t:
                r, k = t.split('*')
                regs.append((r, int(k, 0)))
            else:
                regs.append((t, 1))
        return size, regs, disp, m.group(1)

    def address_expr(self, regs, disp):
        if any(r == 'ESP' for r, _ in regs):
            if regs != [('ESP', 1)]:
                raise Refuse('ESP indexed')
            off = self.esp + disp
            if off >= 0:
                raise Refuse('reads the caller\'s stack')
            return ('frame', off)
        parts = []
        for r, k in regs:
            if r not in REG32:
                raise Refuse('address register ' + r)
            self.need_reg(r, 0xf)
            parts.append(r.lower() if k == 1 else '%s * %d' % (r.lower(), k))
        if disp or not parts:
            parts.append(hx(disp))
        return ('abs', ' + '.join(parts))

    def mem_ref(self, ctype, regs, disp):
        a = self.address_expr(regs, disp)
        if a[0] == 'frame':
            return 'M<%s>(sp - 0x%x)' % (ctype, -a[1]), a[1]
        return 'M<%s>(%s)' % (ctype, a[1]), None

    def need_reg(self, r, mask):
        if self.defined[r] & mask != mask:
            raise Refuse('reads an undefined %s' % r)
        self.used.add(r)

    def reg_size(self, s):
        s = s.strip()
        if s in REG32:
            return 4
        if s in REG16:
            return 2
        if s in REG8:
            return 1
        return None

    def read(self, s, size=None):
        """A C++ expression of the operand's value (size bytes)."""
        s = s.strip()
        v = imm(s)
        if v is not None:
            return hx(v & ((1 << (8 * size)) - 1) if size else v)
        if s in REG32:
            self.need_reg(s, 0xf)
            return s.lower()
        if s in REG16:
            self.need_reg(REG16[s], 0x3)
            return '(u16)%s' % REG16[s].lower()
        if s in REG8:
            r, b = REG8[s]
            self.need_reg(r, 1 << b)
            return '(u8)%s' % r.lower() if b == 0 else '(u8)(%s >> 8)' % r.lower()
        m = self.mem(s)
        if m:
            msize, regs, disp, _ = m
            size = msize or size
            ref, off = self.mem_ref(CTYPE[size], regs, disp)
            if off is not None and not all(off + i in self.frame for i in range(size)):
                raise Refuse('reads an undefined stack slot')
            return ref
        raise Refuse('operand ' + s)

    def write(self, s, value, size, defined=True):
        s = s.strip()
        if s in REG32:
            self.lines.append('%s = %s;' % (s.lower(), value))
            self.defined[s] = 0xf if defined else 0
            self.used.add(s)
            return
        if s in REG16:
            r = REG16[s]
            self.lines.append('%s = (%s & 0xffff0000u) | (u16)(%s);' % (r.lower(), r.lower(), value))
            self.defined[r] |= 0x3
            self.used.add(r)
            return
        if s in REG8:
            r, b = REG8[s]
            if b == 0:
                self.lines.append('%s = (%s & 0xffffff00u) | (u8)(%s);' % (r.lower(), r.lower(), value))
            else:
                self.lines.append('%s = (%s & 0xffff00ffu) | (u32)(u8)(%s) << 8;' % (r.lower(), r.lower(), value))
            self.defined[r] |= 1 << b
            self.used.add(r)
            return
        m = self.mem(s)
        if m:
            msize, regs, disp, _ = m
            size = msize or size
            ref, off = self.mem_ref(CTYPE[size], regs, disp)
            if off is not None:
                for i in range(size):
                    self.frame.add(off + i)
                self.thunk_slots.pop(off, None)
            self.lines.append('%s = %s;' % (ref, value))
            return
        raise Refuse('destination ' + s)

    def size_of(self, a, b):
        for s in (a, b):
            n = self.reg_size(s)
            if n:
                return n
            m = self.mem(s)
            if m and m[0]:
                return m[0]
        raise Refuse('operand size')

    def flag(self, expr):
        self.flag_at = (len(self.lines), 'zf = (%s) == 0;' % expr)

    # --- stack
    def push(self, value, defined=True, thunk=None):
        self.esp -= 4
        self.low = min(self.low, self.esp)
        self.thunk_slots.pop(self.esp, None)
        if defined:
            self.lines.append('M<u32>(sp - 0x%x) = %s;' % (-self.esp, value))
            self.frame |= set(range(self.esp, self.esp + 4))
            if thunk:
                self.thunk_slots[self.esp] = thunk
        else:
            self.frame -= set(range(self.esp, self.esp + 4))

    def slot(self, i):
        """The i-th stack argument at a call (0: the top)."""
        off = self.esp + 4 * i
        if off >= 0:
            raise Refuse('a call reads the caller\'s stack')
        if not all(off + j in self.frame for j in range(4)):
            raise Refuse('passes an undefined stack slot')
        return 'M<u32>(sp - 0x%x)' % -off, off

    # --- calls
    def call(self, target, tail, slot=None):
        """A call of target (an address, or with slot an import slot read live), or a tail jump to it."""
        if self.fpu:
            raise Refuse('a call with the x87 stack in use')
        if target == EMPTY:   # (Debug_Printf: a RET)
            self.lines.append('// Debug_Printf 0x%06x: empty' % EMPTY)
            if tail:
                self.lines.append('return;')
            return
        if slot is None and target in self.gen.generated and tail and self.esp == 0:
            self.callees.append((target, 'G', 0))
            self.lines.append('StaticInit_RunAt(0x%06x);   // (tail)' % target)
            self.lines.append('return;')
            return
        info = import_convention(self.gen.import_names.get(slot)) if slot is not None else self.gen.hooks.get(target) or LIBRARY.get(target)
        if info is None:
            raise Refuse('calls %x, not hooked' % target if slot is None else 'calls the import %s' % self.gen.import_names.get(slot))
        name, kind, n = info
        if kind not in ('T', 'C', 'S'):
            raise Refuse('callee %s convention %s' % (name, kind))
        address = '0x%08x' % target if slot is None else 'M<u32>(0x%08x)' % slot
        args, offs = [], []
        for i in range(n):
            a, off = self.slot(i)
            if off in self.thunk_slots and target != ATEXIT:
                raise Refuse('an exit thunk passed to %x' % target)
            args.append(a)
            offs.append(off)
        if tail and n:
            raise Refuse('a tail call to %s with stack arguments' % name)
        if kind == 'T':
            self.need_reg('ECX', 0xf)
            call = 'g_calls.Th(%s, %s)' % (address, ', '.join(['ecx'] + args))
        else:
            call = 'g_calls.%s(%s%s)' % ('Cd' if kind == 'C' else 'St', address, ''.join(', ' + a for a in args))
        if target == ATEXIT and offs[0] in self.thunk_slots:
            self.gen.registered.add(self.thunk_slots[offs[0]])
        self.callees.append((target if slot is None else ('import', slot), kind, n))
        if tail:
            self.lines.append('%s;   // %s (tail)' % (call, name))
            self.lines.append('return;')
            return
        self.lines.append('eax = %s;   // %s' % (call, name))
        self.used.add('EAX')
        self.defined['EAX'] = 0xf
        self.defined['ECX'] = self.defined['EDX'] = 0
        if kind in ('T', 'S'):
            self.esp += 4 * n
            self.frame = {o for o in self.frame if o >= self.esp}
            self.thunk_slots = {k: v for k, v in self.thunk_slots.items() if k >= self.esp}
        else:   # (a cdecl callee owns its argument slots: never read again)
            self.frame -= set(range(self.esp, self.esp + 4 * n))

    # --- the instructions
    def state(self):
        return (self.esp, len(self.fpu), dict(self.defined), frozenset(self.frame), dict(self.thunk_slots))

    def lift(self):
        targets, inside = set(), {a for a, _ in self.body}
        for va, ins in self.body:
            m = re.fullmatch(r'J\w+ 0x([0-9a-f]+)', ins)
            if m and int(m.group(1), 16) in inside:
                targets.add(int(m.group(1), 16))
        if self.body and re.match(r'J(?!MP)', self.body[0][1]):
            raise Refuse('starts with a conditional jump (a mis-split)')
        live = True
        for va, ins in self.body:
            if va in targets:
                self.label_states[va] = None
                if va in self.states:
                    st = self.states[va]
                    if live:
                        if st[:2] != (self.esp, len(self.fpu)):
                            raise Refuse('the stack differs at a join')
                        for r in REG32:
                            self.defined[r] &= st[2][r]
                        self.frame &= st[3]
                        self.thunk_slots = {k: v for k, v in self.thunk_slots.items() if st[4].get(k) == v}
                    else:
                        self.esp, fl, self.defined, frame, self.thunk_slots = st[0], st[1], dict(st[2]), set(st[3]), dict(st[4])
                        self.frame = frame
                        if fl:
                            raise Refuse('x87 stack at a label')
                    live = True
                self.lines.append('L_%06x:' % va)
                self.labels.add(va)
                self.label_states[va] = self.state()
            if not live:
                raise Refuse('unreachable code at %x' % va)
            live = self.step(va, ins, targets)
        if live:
            raise Refuse('falls off its end')
        if self.need - self.labels:
            raise Refuse('a jump to outside the body')
        return self.lines

    def step(self, va, ins, targets):
        mn, _, rest = ins.partition(' ')
        ops = [o.strip() for o in re.split(r',(?![^\[]*\])', rest)] if rest else []
        if mn == 'MOV':
            size = self.size_of(ops[0], ops[1])
            src = self.read(ops[1], size)
            self.write(ops[0], src, size)
        elif mn in ('MOVZX', 'MOVSX'):
            m = self.mem(ops[1])
            size = m[0] if m else self.reg_size(ops[1])
            src = self.read(ops[1], size)
            cast = {('MOVZX', 1): '(u8)', ('MOVZX', 2): '(u16)', ('MOVSX', 1): '(u32)(s32)(s8)', ('MOVSX', 2): '(u32)(s32)(s16)'}[(mn, size)]
            self.write(ops[0], cast + src, 4)
        elif mn == 'LEA':
            m = self.mem(ops[1])
            a = self.address_expr(m[1], m[2])
            self.write(ops[0], 'sp - 0x%x' % -a[1] if a[0] == 'frame' else a[1], 4)
        elif mn == 'XOR' and ops[0] == ops[1] and self.reg_size(ops[0]):
            size = self.reg_size(ops[0])
            self.write(ops[0], '0', size)
            self.flag('0')
        elif mn == 'OR' and imm(ops[1]) == 0xffffffff and ops[0] in REG32:
            self.write(ops[0], '0xffffffffu', 4)
            self.flag('1')
        elif mn in ('OR', 'AND', 'XOR', 'ADD', 'SUB') and not (ops[0] == 'ESP'):
            size = self.size_of(ops[0], ops[1])
            op = {'OR': '|', 'AND': '&', 'XOR': '^', 'ADD': '+', 'SUB': '-'}[mn]
            a, b = self.read(ops[0], size), self.read(ops[1], size)
            self.write(ops[0], '%s %s %s' % (a, op, b), size)
            self.flag(self.read(ops[0], size))
        elif mn in ('ADD', 'SUB') and ops[0] == 'ESP':
            n = imm(ops[1])
            if n is None or n & 3:
                raise Refuse('ESP by ' + ops[1])
            self.esp += n if mn == 'ADD' else -n
            self.low = min(self.low, self.esp)
            if mn == 'ADD':
                self.frame = {o for o in self.frame if o >= self.esp}
                self.thunk_slots = {k: v for k, v in self.thunk_slots.items() if k >= self.esp}
            else:
                self.frame -= set(range(self.esp, self.esp + n))
        elif mn in ('DEC', 'INC'):
            size = self.reg_size(ops[0]) or self.mem(ops[0])[0]
            a = self.read(ops[0], size)
            self.write(ops[0], '%s %s 1' % (a, '-' if mn == 'DEC' else '+'), size)
            self.flag(self.read(ops[0], size))
        elif mn == 'TEST':
            size = self.size_of(ops[0], ops[1])
            self.flag('%s & %s' % (self.read(ops[0], size), self.read(ops[1], size)))
        elif mn == 'CMP':
            size = self.size_of(ops[0], ops[1])
            self.flag('%s - %s' % (self.read(ops[0], size), self.read(ops[1], size)))
            self.flag_at = (self.flag_at[0], 'zf = (%s) == (%s);' % (self.read(ops[0], size), self.read(ops[1], size)))
        elif mn in ('JZ', 'JNZ', 'JE', 'JNE'):
            if self.flag_at is None:
                raise Refuse('a jump on unknown flags')
            at, text = self.flag_at
            self.lines.insert(at, text)
            self.flag_at = None
            self.used.add('ZF')
            target = int(ops[0], 16)
            self.jump(target)
            self.lines.append('if (%szf) goto L_%06x;' % ('' if mn in ('JZ', 'JE') else '!', target))
        elif mn == 'PUSH':
            if ops[0] in REG32 and self.defined[ops[0]] != 0xf:
                self.push(None, defined=False)
            else:
                v = imm(ops[0])
                if v is not None and v in self.gen.thunk_addresses:
                    self.push('Fn(%s)' % self.gen.thunk_name(v), thunk=v)
                else:
                    self.push(self.read(ops[0], 4))
        elif mn == 'POP':
            off = self.esp
            defined = all(off + i in self.frame for i in range(4))
            self.write(ops[0], 'M<u32>(sp - 0x%x)' % -off if defined else '0', 4, defined=defined)
            self.esp += 4
            self.frame -= set(range(off, off + 4))
        elif mn == 'CALL':
            t = imm(ops[0])
            m = re.fullmatch(r'dword ptr \[0x([0-9a-f]+)\]', ops[0])
            if m:
                self.call(None, tail=False, slot=int(m.group(1), 16))
            elif t is None:
                raise Refuse('an indirect call: ' + ins)
            else:
                self.call(t, tail=False)
        elif mn == 'JMP':
            t = imm(ops[0])
            m = re.fullmatch(r'dword ptr \[0x([0-9a-f]+)\]', ops[0])
            if m:
                if self.esp != 0:
                    raise Refuse('a tail call with the stack moved')
                self.call(None, tail=True, slot=int(m.group(1), 16))
                return False
            if t is None:
                raise Refuse('an indirect jump')
            if t in {a for a, _ in self.body}:
                self.jump(t)
                self.lines.append('goto L_%06x;' % t)
                return False
            if self.esp != 0:
                raise Refuse('a tail call with the stack moved')
            self.call(t, tail=True)
            return False
        elif mn == 'RET':
            if ops:
                raise Refuse('RET n')
            if self.esp != 0 or self.fpu:
                raise Refuse('returns with the stack moved')
            self.lines.append('return;')
            return False
        elif mn == 'STOSD.REP':
            for r in ('ECX', 'EDI', 'EAX'):
                self.need_reg(r, 0xf)
            self.lines.append('for (; ecx; --ecx, edi += 4) M<u32>(edi) = eax;')
        elif mn == 'NOP':
            pass
        elif mn.startswith('F'):
            self.x87(mn, ops)
        else:
            raise Refuse('instruction ' + ins)
        return True

    def jump(self, target):
        self.need.add(target)
        st = self.state()
        if target in self.labels:   # (a loop: what the code after the label assumes must hold on the way back)
            ls = self.label_states[target]
            if ls[:2] != st[:2] or any(ls[2][r] & ~st[2][r] for r in REG32) or not ls[3] <= st[3] or any(st[4].get(k) != v for k, v in ls[4].items()):
                raise Refuse('a loop whose state differs on the way back')
        elif target in self.states:
            if self.states[target][:2] != st[:2]:
                raise Refuse('the stack differs at a join')
        else:
            self.states[target] = st
        if self.fpu:
            raise Refuse('a jump with the x87 stack in use')

    def x87(self, mn, ops):
        def operand(s):
            m = self.mem(s)
            if not m or m[3] not in ('float', 'double'):
                raise Refuse('x87 operand ' + s)
            ref, off = self.mem_ref(m[3], m[1], m[2])
            if off is not None and not all(off + i in self.frame for i in range(SIZES[m[3]])):
                raise Refuse('x87 reads an undefined stack slot')
            return m[3], ref

        def st(i):
            if i >= len(self.fpu):
                raise Refuse('x87 stack underflow')
            return self.fpu[-1 - i]
        if mn == 'FLD':
            kind, ref = operand(ops[0])
            name = 'x%d' % self.fpus
            self.fpus += 1
            self.lines.append('X87Reg %s = %s;' % (name, ref) if kind == 'double' else 'X87Reg %s = X87Reg(%s);' % (name, ref))
            self.fpu.append(name)
        elif mn in ('FADD', 'FSUB', 'FSUBR', 'FMUL', 'FDIV', 'FDIVR') and len(ops) == 1:
            kind, ref = operand(ops[0])
            a = st(0)
            if kind == 'float':   # (a float operand is a memory operand: X87Reg's rule for a signalling NaN there)
                e = {'FADD': '%s + %s', 'FSUB': '%s - %s', 'FMUL': '%s * %s', 'FDIV': '%s / %s', 'FSUBR': '%s - %s', 'FDIVR': '%s / %s'}[mn]
                e = e % ((ref, a) if mn in ('FSUBR', 'FDIVR') else (a, ref))
            elif mn in ('FADD', 'FSUB', 'FSUBR'):   # (X87_Add rounds the exact sum of any two doubles once)
                e = {'FADD': '%s + X87Reg(%s)' % (a, ref), 'FSUB': '%s - X87Reg(%s)' % (a, ref), 'FSUBR': 'X87Reg(%s) - %s' % (ref, a)}[mn]
            elif mn == 'FMUL':   # (the exact product of a 24-bit register and a 53-bit double rounded once)
                e = 'X87Reg(X87_MulDouble(%s.v, %s))' % (a, ref)
            else:
                raise Refuse('x87 %s of a double' % mn)
            self.lines.append('%s = %s;' % (a, e))
        elif mn in ('FADDP', 'FSUBP', 'FSUBRP', 'FMULP', 'FDIVP', 'FDIVRP') and (not ops or ops == ['ST1', 'ST0'] or ops == ['ST1']):
            a, b = st(1), st(0)   # st1 = st1 op st0, pop
            e = {'FADDP': '%s + %s' % (a, b), 'FSUBP': '%s - %s' % (a, b), 'FMULP': '%s * %s' % (a, b), 'FDIVP': '%s / %s' % (a, b),
                 'FSUBRP': '%s - %s' % (b, a), 'FDIVRP': '%s / %s' % (b, a)}[mn]
            self.lines.append('%s = %s;' % (a, e))
            self.fpu.pop()
        elif mn == 'FSQRT':
            self.lines.append('%s = X87_Sqrt(%s);' % (st(0), st(0)))
        elif mn == 'FXCH' and (not ops or ops == ['ST1']):
            self.fpu[-1], self.fpu[-2] = st(1), st(0)
        elif mn in ('FSTP', 'FST'):
            kind, ref = operand(ops[0])
            self.lines.append('%s = %s;' % (ref, '(float)%s' % st(0) if kind == 'float' else '%s.v' % st(0)))
            if mn == 'FSTP':
                self.fpu.pop()
        else:
            raise Refuse('x87 ' + mn + ' ' + ','.join(ops))


class Gen:
    def __init__(self):
        self.hooks = hooks_and_conventions()
        self.slots = table_slots()
        self.registered = set()
        self.kinds = {}       # address -> 'hooked' | 'clib' | 'main' | 'lifted' | 'manual'
        self.main = []        # (address, text, object, ctor, exit)
        self.lifted = {}      # address -> (C++ lines, used registers, frame size, x87 locals)
        self.callees = {}     # address -> [(callee, kind, n)] of each generated initialiser / thunk
        self.refused = {}     # address -> reason
        self.thunks = {}      # exit thunk address -> ('template', object, dtor) | ('lifted',)
        self.thunk_addresses = set()
        self.bodies = {}
        self.generated = set()
        self.import_names = import_names()

    def thunk_name(self, address):
        t = self.thunks[address]
        return 'ExitThunk<0x%x, 0x%x>' % (t[1], t[2]) if t[0] == 'template' else 'Exit_%06x' % address

    def run(self):
        inits = []
        for v in self.slots:
            if not v or v in self.kinds:
                continue
            if v in self.hooks:
                self.kinds[v] = 'hooked'
            elif CLIB_LOW <= v < CLIB_HIGH:
                self.kinds[v] = 'clib'
            else:
                inits.append(v)
        self.generated = set(inits)
        starts = frozenset(self.generated) | frozenset(self.hooks)
        # the exit thunks: the last address the initialisers push before a CALL of atexit (not hooked: those are ports already)
        for v in inits:
            try:
                body = function_body(v, starts)
            except Refuse as r:
                self.refused[v] = str(r)
                continue
            self.bodies[v] = body
            pushed = None
            for _, ins in body:
                m = re.fullmatch(r'PUSH 0x([0-9a-f]+)', ins)
                if m:
                    pushed = int(m.group(1), 16)
                elif ins == 'CALL 0x%08x' % ATEXIT:
                    if pushed and pushed not in self.hooks:
                        self.thunk_addresses.add(pushed)
                    pushed = None
                elif ins.startswith('CALL') or ins.startswith('PUSH'):
                    pushed = None
        for t in sorted(self.thunk_addresses):
            body = function_body(t, starts)
            self.bodies[t] = body
            ins = [i for _, i in body]
            m0 = re.fullmatch(r'MOV ECX,0x([0-9a-f]+)', ins[0]) if len(ins) == 2 else None
            m1 = re.fullmatch(r'JMP 0x([0-9a-f]+)', ins[1]) if len(ins) == 2 else None
            if m0 and m1 and self.hooks.get(int(m1.group(1), 16), (0, 0, 0))[1:] == ('T', 0):
                self.thunks[t] = ('template', int(m0.group(1), 16), int(m1.group(1), 16))
                self.callees[t] = [(int(m1.group(1), 16), 'T', 0)]
            else:
                self.thunks[t] = ('lifted',)
        for t in sorted(self.thunk_addresses):
            if self.thunks[t][0] == 'lifted':
                self.lift(t)
        for v in inits:
            if v in self.refused:
                continue
            body = [i for _, i in self.bodies[v]]
            m = [re.fullmatch(p, i) for p, i in zip((r'PUSH 0x([0-9a-f]+)', r'MOV ECX,0x([0-9a-f]+)', r'CALL 0x([0-9a-f]+)', r'PUSH 0x([0-9a-f]+)',
                                                      'CALL 0x%08x' % ATEXIT, r'ADD ESP,0x4', 'RET'), body)] if len(body) == 7 else []
            if m and all(m) and self.hooks.get(int(m[2].group(1), 16), (0, 0, 0))[1:] == ('T', 1) and int(m[3].group(1), 16) in self.thunks:
                text, obj, ctor, ex = (int(m[i].group(1), 16) for i in (0, 1, 2, 3))
                self.main.append((v, text, obj, ctor, ex))
                self.kinds[v] = 'main'
                self.callees[v] = [(ctor, 'T', 1), (ATEXIT, 'C', 1)]
                self.registered.add(ex)
            else:
                self.lift(v)
        for v in inits:
            if v not in self.kinds:
                self.kinds[v] = 'manual' if v in MANUAL else 'refused'
        return self

    def lift(self, v):
        try:
            lf = Lifter(self, v, self.bodies[v])
            lines = lf.lift()
        except Refuse as r:
            self.refused[v] = str(r)
            return
        self.lifted[v] = (lines, lf.used, -lf.low, lf.fpus)
        self.callees[v] = lf.callees
        if v not in self.thunk_addresses:
            self.kinds[v] = 'lifted'

    # --- output
    def function(self, name, v):
        lines, used, frame, _ = self.lifted[v]
        out = ['void __cdecl %s() {   // 0x%06x' % (name, v)]
        regs = [r.lower() for r in REG32 if r in used and r != 'ESP']
        if regs:
            out.append('    u32 %s;' % ', '.join('%s = 0' % r for r in regs))
        if 'ZF' in used:
            out.append('    bool zf = false;')
        if frame:
            out.append('    alignas(4) u8 frame[0x%x];' % frame)
            out.append('    const u32 sp = (u32)(uintptr_t)(frame + sizeof frame);')
        for line in lines:
            out.append(('' if line.startswith('L_') else '    ') + line)
        out.append('}')
        return out

    def cpp(self):
        o = ['// GENERATED by tools/gen_static_init.py from tp.exe\'s C++ initialiser table (0x727000-0x72e0e8) and _re/text.asm: do not edit.',
             '// The compiler-generated static initialisers and the exit thunks they register with atexit. Main shape: a row of kMain run by',
             '// RunMain; exit thunks MOV ECX,object ; JMP dtor: ExitThunk<object, dtor>; the rest lifted instruction by instruction.',
             '#include "staticinit.h"',
             '',
             '#include <algorithm>',
             '#include <cstdint>',
             '',
             '#include "addrcalls.h"',
             '#include "../math/x87.h"',
             '',
             'namespace {',
             '',
             'AddrCalls g_calls;',
             '',
             'template <typename T> T &M(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }',
             'u32 Fn(void(__cdecl *fn)()) { return (u32)(uintptr_t)fn; }',
             '',
             '// MOV ECX,Object ; JMP Dtor (a thiscall of no stack arguments).',
             'template <u32 Object, u32 Dtor> void __cdecl ExitThunk() { g_calls.Th(Dtor, Object); }',
             '']
        lifted_thunks = [t for t in sorted(self.thunks) if self.thunks[t][0] == 'lifted' and t in self.lifted]
        for t in lifted_thunks:
            o += self.function('Exit_%06x' % t, t)
        o.append('')
        o.append('// PUSH text ; MOV ECX,object ; CALL ctor ; PUSH exit ; CALL atexit 0x67a7d0 ; ADD ESP,4 ; RET')
        o.append('struct MainInit {')
        o.append('    u32 address, text, object, ctor;')
        o.append('    void(__cdecl *exit)();')
        o.append('};')
        o.append('const MainInit kMain[] = {')
        for v, text, obj, ctor, ex in self.main:
            o.append('    {0x%06x, 0x%06x, 0x%06x, 0x%06x, &%s},' % (v, text, obj, ctor, self.thunk_name(ex)))
        o.append('};')
        o.append('')
        o.append('void RunMain(const MainInit &e) {')
        o.append('    g_calls.Th(e.ctor, e.object, e.text);')
        o.append('    g_calls.Cd(0x%08x, Fn(e.exit));' % ATEXIT)
        o.append('}')
        o.append('')
        inits = [v for v in sorted(self.lifted) if self.kinds.get(v) == 'lifted']
        for v in inits:
            o += self.function('Init_%06x' % v, v)
        o.append('')
        o.append('struct Fn0 {')
        o.append('    u32 address;')
        o.append('    void(__cdecl *fn)();')
        o.append('};')
        o.append('const Fn0 kLifted[] = {   // (and the ones by hand)')
        for v in sorted(inits + [m for m in MANUAL if self.kinds.get(m) == 'manual']):
            o.append('    {0x%06x, &Init_%06x},' % (v, v) if v in self.lifted else '    {0x%06x, &StaticInit_%06x},   // by hand: %s' % (v, v, MANUAL[v]))
        o.append('};')
        o.append('const Fn0 kExits[] = {')
        for t in sorted(self.thunks):
            if self.thunks[t][0] == 'template' or t in self.lifted:
                o.append('    {0x%06x, &%s},' % (t, self.thunk_name(t)))
        o.append('};')
        o.append('')
        o.append('template <typename T, size_t N> const T *Find(const T (&table)[N], u32 address) {')
        o.append('    const T *const at = std::lower_bound(table, table + N, address, [](const T &e, u32 a) { return e.address < a; });')
        o.append('    return at != table + N && at->address == address ? at : nullptr;')
        o.append('}')
        o.append('')
        o.append('}  // namespace')
        o.append('')
        o.append('extern "C" {')
        o.append('')
        o.append('bool __cdecl StaticInit_Has(u32 address) { return Find(kMain, address) || Find(kLifted, address); }')
        o.append('')
        o.append('bool __cdecl StaticInit_RunAt(u32 address) {')
        o.append('    if (const MainInit *const e = Find(kMain, address)) {')
        o.append('        RunMain(*e);')
        o.append('        return true;')
        o.append('    }')
        o.append('    if (const Fn0 *const e = Find(kLifted, address)) {')
        o.append('        e->fn();')
        o.append('        return true;')
        o.append('    }')
        o.append('    return false;')
        o.append('}')
        o.append('')
        o.append('void __cdecl StaticInit_Run() {')
        o.append('    for (u32 slot = 0x%06x; slot < 0x%06x; slot += 4) {   // (CRT_initterm over __xc_a..__xc_z, read live)' % (TABLE, TABLE_END))
        o.append('        const u32 entry = M<u32>(slot);')
        o.append('        if (entry && !StaticInit_RunAt(entry)) g_calls.Cd(entry);   // (a port\'s or the C library\'s address)')
        o.append('    }')
        o.append('}')
        o.append('')
        o.append('void(__cdecl *__cdecl StaticInit_Exit(u32 address))() {')
        o.append('    const Fn0 *const e = Find(kExits, address);')
        o.append('    return e ? e->fn : nullptr;')
        o.append('}')
        o.append('')
        o.append('u32 __cdecl StaticInit_ExitAddress(void(__cdecl *fn)()) {')
        o.append('    for (const Fn0 &e : kExits)')
        o.append('        if (e.fn == fn) return e.address;')
        o.append('    return 0;')
        o.append('}')
        o.append('')
        o.append('void __cdecl StaticInit_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }')
        o.append('')
        o.append('}  // extern "C"')
        o.append('')
        o.append('const AddrCalls &StaticInit_Calls() { return g_calls; }')
        return '\r\n'.join(o) + '\r\n'


_GEN = None


def analyse():
    global _GEN
    if _GEN is None:
        _GEN = Gen().run()
    return _GEN


def main():
    g = analyse()
    from collections import Counter
    if '--stats' in sys.argv:
        print(Counter(g.kinds.values()), 'slots', len(g.slots), 'zero', g.slots.count(0))
        print('thunks', Counter(t[0] for t in g.thunks.values()), 'refused thunks', len([t for t in g.thunks if t in g.refused]))
        reasons = Counter(r for r in g.refused.values())
        for r, n in reasons.most_common():
            print(n, r, [hex(a) for a, x in g.refused.items() if x == r][:6])
        print('clib', ' '.join('%x' % v for v in g.slots if g.kinds.get(v) == 'clib'))
        return
    text = g.cpp()
    if '--check' in sys.argv:
        old = open(OUT, 'rb').read().decode() if os.path.exists(OUT) else ''
        sys.exit(0 if old == text else 1)
    with open(OUT, 'wb') as f:
        f.write(text.encode())
    print('wrote', OUT, len(g.main), 'main,', len([v for v in g.kinds if g.kinds[v] == 'lifted']), 'lifted,', len(g.refused), 'refused')


if __name__ == '__main__':
    main()
