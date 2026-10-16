"""A small symbolic evaluator for straight-line leaf functions, for tools/tinygen.py.

symbolic(addr, texts) -> a function description (the dict tinygen writes out), or None when the
function is not simple enough. It follows the registers through MOV, LEA and XOR, loads and stores
(through ECX or another register, the stack arguments, or fixed addresses), the debug build's
stack frame (PUSH EBP; MOV EBP,ESP; [PUSH ECX; MOV [EBP-4],ECX] ... POP EBP) and RET [n], then says
what the function does as C++: its stores in order and the value left in EAX. Values are read when
the C++ uses them, so a load that a store separates from its use is refused.
"""
import re

R32 = ('EAX', 'ECX', 'EDX', 'EBX', 'ESI', 'EDI')
R16 = {'AX': 'EAX', 'CX': 'ECX', 'DX': 'EDX', 'BX': 'EBX'}
R8 = {'AL': 'EAX', 'CL': 'ECX', 'DL': 'EDX', 'BL': 'EBX'}
CTYPE = {32: 'u32', 16: 'u16', 8: 'u8'}
H = r'(-?0x[0-9a-f]+)'


class Unsupported(Exception):
    pass


def num(s):
    return int(s, 16) if s else 0


def reg_of(name):
    if name in R32:
        return name, 32
    if name in R16:
        return R16[name], 16
    if name in R8:
        return R8[name], 8
    raise Unsupported(name)


def is_reg(name):
    return name in R32 or name in R16 or name in R8


def off(k):
    return '(u32)%d' % k if k < 0 else '0x%x' % k


# Values: ('this',) ('arg', i) ('imm', n) ('ld', base, k, size, epoch) ('gl', addr, size, epoch) ('addr', base, k)
def ptr(v):
    t = v[0]
    if t == 'this':
        return 'o'
    if t == 'arg':
        return 'reinterpret_cast<u8 *>((uintptr_t)v%d)' % v[1]
    if t == 'ld':
        return 'at<u8 *>(%s, %s)' % (ptr(v[1]), off(v[2]))
    if t == 'addr':
        return '%s %s 0x%x' % (ptr(v[1]), '-' if v[2] < 0 else '+', abs(v[2]))
    raise Unsupported('pointer ' + t)


def expr(v, size):
    t = v[0]
    if t == 'imm':
        return '0x%x' % (v[1] & ((1 << size) - 1))
    if t == 'arg':
        return 'v%d' % v[1] if size == 32 else '(%s)v%d' % (CTYPE[size], v[1])
    if t == 'this':
        return '(u32)(uintptr_t)o'
    if t == 'ld':
        if v[3] < size:
            raise Unsupported('widening load')
        return 'at<%s>(%s, %s)' % (CTYPE[v[3]], ptr(v[1]), off(v[2]))
    if t == 'gl':
        if v[2] < size:
            raise Unsupported('widening load')
        return 'global<%s>(0x%08x)' % (CTYPE[v[2]], v[1])
    if t == 'addr':
        return '(u32)(uintptr_t)(%s)' % ptr(v)
    raise Unsupported(t)


def loads_of(v):
    """The loads inside a value (for the epoch check)."""
    out = []
    if v[0] in ('ld', 'gl'):
        out.append(v)
    if v[0] in ('ld', 'addr'):
        out += loads_of(v[1])
    return out


def mentions(v, what):
    if v is None:
        return False
    if v[0] == what:
        return True
    return v[0] in ('ld', 'addr') and mentions(v[1], what)


def symbolic(addr, texts):
    try:
        return _symbolic(addr, texts)
    except Unsupported:
        return None


def _symbolic(addr, texts):
    regs = {'ECX': (('this',), 32)}
    stores = []   # (size, base or None, k, value)
    state = dict(frame=False, args=0)
    local = {}

    def read(name):
        reg, size = reg_of(name)
        if reg not in regs:
            raise Unsupported('read of %s' % name)
        v, vs = regs[reg]
        if size > vs:
            raise Unsupported('wide read of a narrow value')
        return v

    def write(name, v):
        reg, size = reg_of(name)
        # a narrow write leaves the register's upper bits as they were; the function's callers are taken to read
        # only what it gave them (the result then has the narrow type)
        regs[reg] = (v, size)

    def stack(base, k):
        if base == 'EBP' and k == -4 and -4 in local:
            return local[-4]
        if (base == 'EBP') != state['frame']:
            raise Unsupported('stack base')
        first = 8 if base == 'EBP' else 4
        if k < first or (k - first) % 4:
            raise Unsupported('stack slot')
        i = (k - first) // 4
        if i > 7:
            raise Unsupported('far stack slot')
        state['args'] = max(state['args'], i + 1)
        return ('arg', i)

    def load(text, size):
        m = re.match(r'^(?:w|b)?\[(ESP|EBP) \+ %s\]$' % H, text)
        if m:
            return stack(m.group(1), num(m.group(2)))
        m = re.match(r'^(?:w|b)?\[(0x[0-9a-f]+)\]$', text)
        if m:
            return ('gl', num(m.group(1)), size, len(stores))
        m = re.match(r'^(?:w|b)?\[(E[A-Z]{2})(?: \+ %s)?\]$' % H, text)
        if m and m.group(1) in R32:
            return ('ld', read(m.group(1)), num(m.group(2)), size, len(stores))
        raise Unsupported('operand ' + text)

    n = len(texts)
    i = 0
    ret_pop = None
    while i < n:
        t = texts[i]
        i += 1
        if t == 'PUSH EBP':
            if i != 1 or texts[1:2] != ['MOV EBP,ESP']:
                raise Unsupported('prologue')
            state['frame'] = True
            i += 1
            if texts[i:i + 2] == ['PUSH ECX', 'MOV [EBP + -0x4],ECX']:
                local[-4] = regs['ECX'][0]
                i += 2
            continue
        if t in ('MOV ESP,EBP', 'POP EBP'):
            continue
        m = re.match(r'^RET(?: (0x[0-9a-f]+))?$', t)
        if m:
            if i != n:
                raise Unsupported('code after RET')
            ret_pop = num(m.group(1))
            break
        m = re.match(r'^XOR (\w+),(\w+)$', t)
        if m and m.group(1) == m.group(2) and is_reg(m.group(1)):
            write(m.group(1), ('imm', 0))
            continue
        m = re.match(r'^LEA (E[A-Z]{2}),\[(E[A-Z]{2})(?: \+ %s)?\]$' % H, t)
        if m and m.group(2) in R32:
            write(m.group(1), ('addr', read(m.group(2)), num(m.group(3))))
            continue
        m = re.match(r'^MOV (\w+),(\w+)$', t)
        if m and is_reg(m.group(1)) and is_reg(m.group(2)):
            write(m.group(1), read(m.group(2)))
            continue
        m = re.match(r'^MOV (\w+),(.+)$', t)
        if m and is_reg(m.group(1)):
            size = reg_of(m.group(1))[1]
            src = m.group(2)
            im = re.match(r'^%s$' % H, src)
            write(m.group(1), ('imm', num(im.group(1))) if im else load(src, size))
            continue
        m = re.match(r'^MOV (w|b)?\[(.+?)\],(.+)$', t)
        if m:
            size = 16 if m.group(1) == 'w' else 8 if m.group(1) == 'b' else 32
            dest, src = m.group(2), m.group(3)
            if re.match(r'^%s$' % H, src):
                value = ('imm', num(src))
            elif is_reg(src):
                value = read(src)
                if reg_of(src)[1] != size:
                    raise Unsupported('store width')
            else:
                raise Unsupported('store source')
            gm = re.match(r'^(0x[0-9a-f]+)$', dest)
            if gm:
                base, k = None, num(gm.group(1))
            else:
                dm = re.match(r'^(E[A-Z]{2})(?: \+ %s)?$' % H, dest)
                if not dm or dm.group(1) not in R32 or dm.group(1) == 'EBP' or dm.group(1) == 'ESP':
                    raise Unsupported('store dest')
                base, k = read(dm.group(1)), num(dm.group(2))
            # values are read when the C++ runs the store, so nothing loaded before an earlier store may be used
            for part in ([value] + ([base] if base else [])):
                if any(l[-1] != len(stores) for l in loads_of(part)):
                    raise Unsupported('load across a store')
            stores.append((size, base, k, value))
            continue
        raise Unsupported(t)
    if ret_pop is None or ret_pop % 4:
        raise Unsupported('return')
    result = regs.get('EAX') if 'EAX' in regs else None
    if result is not None and result[0] == ('this',) and False:
        pass
    if not stores and result is None:
        raise Unsupported('does nothing')
    if result is not None:
        if any(l[-1] != len(stores) for l in loads_of(result[0])):
            raise Unsupported('load across a store')
    vals = [s[3] for s in stores] + [s[1] for s in stores if s[1]] + ([result[0]] if result else [])
    uses_this = any(mentions(v, 'this') for v in vals)
    popped = ret_pop // 4
    args = state['args']
    for v in vals:
        for j in range(8):
            if mentions(v, 'arg'):
                pass
    conv = 't' if uses_this else 's' if popped else 'c'
    if popped and args > popped:
        raise Unsupported('arguments beyond the pop')
    if conv == 'c' and (args or popped):
        pass
    nargs = max(popped, args)
    if not popped and conv != 'c':
        pass
    body = []
    for size, base, k, value in stores:
        if base is None:
            body.append('global<%s>(0x%08x) = %s;' % (CTYPE[size], k, expr(value, size)))
        else:
            body.append('at<%s>(%s, %s) = %s;' % (CTYPE[size], ptr(base), off(k), expr(value, size)))
    ret = 'v'
    if result is not None:
        v, size = result
        if v[0] == 'addr':
            ret = 'p'
            body.append('return %s;' % ptr(v))
        else:
            ret = {32: 'a', 16: 'w', 8: 'b'}[size]
            body.append('return %s;' % expr(v, size))
    globs = [k for size, base, k, value in stores if base is None]
    for v in vals:
        for l in loads_of(v):
            if l[0] == 'gl':
                globs.append(l[1])
    parts = []
    for size, base, k, value in stores:
        parts.append('%s%s = %s' % ('' if base is None else '+', hex(k) if base is None or k >= 0 else '-' + hex(-k),
                                    {'arg': 'argument', 'imm': hex(value[1]) if value[0] == 'imm' else '', 'this': 'the object'}.get(value[0], 'a value')))
    doc = 'stores %s' % ', '.join(parts) if parts else 'returns %s' % ('a constant' if result and result[0][0] == 'imm' else 'a field or global')
    return dict(name='Fn_%06x' % addr, conv=conv, nargs=nargs, ret=ret, doc=doc, body='\n    '.join(body), globals=globs)
