"""Theme Park World sprite scripts: disassemble the script table in tp.exe.

The people and effect sprites (data/esprites.wad, see esp.py) are animated by small scripts
compiled into the exe as threaded code: one table of 32-bit words (0x74dab8-0x74f7c0) in
which each instruction is the address of its opcode function followed by its operands.
SpriteScript_Run (0x475010) calls the opcodes in turn; the C++ port is
src/game/sprite/script.cpp. Scripts start where the game's code points into the table
(SpriteScript_Start 0x475a10 and friends take the address), or through the special-action
table at 0x74f7c8 (Sprite_GetAction).

A script has 19 registers (+0x84 of its 0x118-byte object). r1-r6 are floats, r1-r3 the
position; r7 starts at 0xff, r8 and r9 at 1.0 (STEPR9 steps r9); r10 is the sprite folder
and r11 the sprite in it; r12 is the animation (SETANIM), r13 the frame shown (PLAY) and
r14 the animation's frame count; r16 holds flags. A special action sets r17 and r18 from
the sprite (Sprite_GetAction).

Opcodes (operands: r = register, n = immediate, @ = address in the table):
  GOTO @, GOSUB @ (pushes the return pc; there is no RETURN), IF r test x / ELSE / ENDIF,
  REPEAT / UNTIL r test x, YIELD, SET r n, ADD r n, SUB r n, MOVE r r, SETANIM n,
  PLAY n (shows frame n of the animation; -1 hides), PLAYREG r, PLAYALWAYS n (shows even
  -1) - each sets r13 and yields - HIDE, STEPR9 (r9 += 0.01),
  END (the address of the empty Debug_Printf, 0x5da3c0).
Tests 1-4 compare r with register x (==, !=, >, <), 5-8 with the immediate x; r1-r6 compare
as floats. A failed IF skips to its ELSE or ENDIF; UNTIL loops back to its REPEAT until the
test holds.

usage: spritescript.py [tp.exe] [--check]   (--check: decode and check the table, no listing)
"""
import os
import struct
import sys

EXE = os.path.join(os.path.dirname(__file__), '..', '.data', 'nocd', 'tp.exe')
CODE, CODE_END, ACTIONS = 0x74dab8, 0x74f7c0, 0x74f7c8

# opcode address -> (mnemonic, operand kinds)
OPS = {
    0x476020: ('GOTO', '@'), 0x476050: ('IF', 'rtx'), 0x4762f0: ('ENDIF', ''), 0x476320: ('ELSE', ''),
    0x476340: ('GOSUB', '@'), 0x4763b0: ('REPEAT', ''), 0x4763d0: ('UNTIL', 'rtx'), 0x4766c0: ('YIELD', ''),
    0x4766d0: ('SET', 'rn'), 0x476750: ('ADD', 'rn'), 0x4767e0: ('SUB', 'rn'), 0x476870: ('MOVE', 'rr'),
    0x4768b0: ('SETANIM', 'n'), 0x4768d0: ('PLAYREG', 'r'), 0x476940: ('PLAY', 'n'), 0x4769b0: ('PLAYALWAYS', 'n'),
    0x4769f0: ('HIDE', ''), 0x476a10: ('STEPR9', ''), 0x5da3c0: ('END', ''),
}
REGS = {1: 'x', 2: 'y', 3: 'z', 10: 'folder', 11: 'sprite', 12: 'anim', 13: 'frame', 14: 'frames', 16: 'flags'}
KINDS = {mn: kinds for mn, kinds in OPS.values()}
TESTS = {1: '==', 2: '!=', 3: '>', 4: '<', 5: '==', 6: '!=', 7: '>', 8: '<'}


class Exe:
    def __init__(self, path):
        self.d = open(path, 'rb').read()
        pe = struct.unpack_from('<I', self.d, 0x3c)[0]
        n = struct.unpack_from('<H', self.d, pe + 6)[0]
        opt = struct.unpack_from('<H', self.d, pe + 20)[0]
        base = struct.unpack_from('<I', self.d, pe + 52)[0]
        self.sections = []
        for i in range(n):
            s = pe + 24 + opt + 40 * i
            name = self.d[s:s + 8].rstrip(b'\0').decode()
            vsize, va, rsize, raw = struct.unpack_from('<IIII', self.d, s + 8)
            self.sections.append((name, base + va, raw, min(rsize, vsize)))

    def u32(self, va):
        for _, start, raw, size in self.sections:
            if start <= va < start + size:
                return struct.unpack_from('<I', self.d, raw + va - start)[0]
        raise ValueError('%08x is not in the file' % va)

    def text(self):
        for name, start, raw, size in self.sections:
            if name == '.text':
                return start, self.d[raw:raw + size]
        raise ValueError('no .text')


def reg(i):
    return 'r%d' % i + ('(%s)' % REGS[i] if i in REGS else '')


def imm(v):
    s = v - (1 << 32) if v & 0x80000000 else v
    return str(s) if -100000 < s < 100000 else '0x%x' % v


def decode(exe):
    """[(address, mnemonic or None, operand words)]: instructions, and lone data words."""
    words = [exe.u32(a) for a in range(CODE, CODE_END, 4)]
    out, i = [], 0
    while i < len(words):
        a = CODE + 4 * i
        if words[i] in OPS:
            mn, kinds = OPS[words[i]]
            out.append((a, mn, words[i + 1:i + 1 + len(kinds)]))
            i += 1 + len(kinds)
        else:
            out.append((a, None, [words[i]]))
            i += 1
    return out


def entries(exe, ins):
    """{table address: [code addresses that refer to it]}, the special-action scripts, and
    the number of byte matches dropped. .text is scanned for any 32-bit value in the table
    (push imm32, mov reg, imm32, ...); a match that is not the start of an instruction or
    of the table is a coincidence inside other code (e.g. a call's rel32)."""
    start, text = exe.text()
    starts = {a for a, mn, _ in ins if mn} | {CODE}
    refs, dropped = {}, 0
    for i in range(len(text) - 3):
        v = struct.unpack_from('<I', text, i)[0]
        if CODE <= v < CODE_END:
            if v in starts:
                refs.setdefault(v, []).append(start + i)
            else:
                dropped += 1
    actions = [exe.u32(ACTIONS + 4 * k) for k in range(4)]
    return refs, actions, dropped


def operands(kinds, ops):
    if kinds == 'rtx':
        r, t, x = ops
        if t in (1, 2, 3, 4):
            return '%s %s %s' % (reg(r), TESTS[t], reg(x))
        if t in (5, 6, 7, 8):
            return '%s %s %s' % (reg(r), TESTS[t], imm(x))
        return '%s ?%d %s' % (reg(r), t, imm(x))
    parts = []
    for k, v in zip(kinds, ops):
        parts.append(reg(v) if k == 'r' else 'L%06x' % v if k == '@' else imm(v))
    return ', '.join(parts)


def listing(exe):
    ins = decode(exe)
    refs, actions, _ = entries(exe, ins)
    targets = {ops[0] for a, mn, ops in ins if mn in ('GOTO', 'GOSUB')}
    lines, depth = [], 0
    for a, mn, ops in ins:
        if a in refs or a in actions or a in targets:
            notes = []
            if a in refs:
                notes.append('used at ' + ', '.join('%06x' % r for r in refs[a]))
            if a in actions:
                notes.append('special action %d' % actions.index(a))
            if a == CODE:
                notes.insert(0, 'table base: pcs count words from here')
            lines.append('')
            lines.append('L%06x:%s' % (a, ('  ; ' + '; '.join(notes)) if notes else ''))
            depth = 0
        if mn is None:
            lines.append('  %06x  .word %s' % (a, imm(ops[0])))
            continue
        if mn in ('ENDIF', 'ELSE', 'UNTIL'):
            depth = max(depth - 1, 0)
        text = mn + (' ' + operands(KINDS[mn], ops) if ops else '')
        lines.append('  %06x  %s%s' % (a, '  ' * depth, text))
        if mn in ('IF', 'ELSE', 'REPEAT'):
            depth += 1
    return lines


def check(exe):
    """Decode the table and check it is consistent: every jump and special action lands on
    an instruction, IF/ELSE/ENDIF and REPEAT/UNTIL nest within each script, and the only
    words that are not instructions are the zeros between scripts (and the table's header)."""
    ins = decode(exe)
    refs, actions, dropped = entries(exe, ins)
    starts = {a for a, mn, _ in ins if mn}
    problems = []
    for a, mn, ops in ins:
        if mn in ('GOTO', 'GOSUB') and ops[0] not in starts:
            problems.append('%06x: %s to %06x, not an instruction' % (a, mn, ops[0]))
        if mn is None and ops[0] != 0 and a != CODE:
            problems.append('%06x: data word %s' % (a, imm(ops[0])))
    problems += ['special action %d at %06x, not an instruction' % (k, t)
                 for k, t in enumerate(actions) if t not in starts]
    stack = []
    for a, mn, ops in ins:
        if mn is None:  # between scripts
            problems += ['%06x: %s not closed' % (b, m) for b, m in stack]
            stack = []
        elif mn in ('IF', 'REPEAT'):
            stack.append((a, mn))
        elif mn == 'ELSE' and not (stack and stack[-1][1] == 'IF'):
            problems.append('%06x: ELSE outside an IF' % a)
        elif mn in ('ENDIF', 'UNTIL'):
            want = 'IF' if mn == 'ENDIF' else 'REPEAT'
            if stack and stack[-1][1] == want:
                stack.pop()
            else:
                problems.append('%06x: %s without its %s' % (a, mn, want))
    count = sum(1 for _, mn, _ in ins if mn)
    jumps = sum(1 for _, mn, _ in ins if mn in ('GOTO', 'GOSUB'))
    print('%d instructions, %d entry points from code, %d special actions, %d jumps; '
          '%d coincidental byte matches dropped' % (count, len(refs), len(actions), jumps, dropped))
    for p in problems:
        print('  ' + p)
    print('%d problems' % len(problems))
    return 1 if problems else 0


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    exe = Exe(args[0] if args else EXE)
    if '--check' in sys.argv:
        sys.exit(check(exe))
    print('\n'.join(listing(exe)))


if __name__ == '__main__':
    main()
