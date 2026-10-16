"""Wrappers that only set up a call to an already-ported function, for tools/tinygen.py.

wrapper(addr, texts, known, decls) -> a function description or None. `known` maps addresses to the
names symbols.csv gives; `decls` maps a C++ function name to its declaration (return type, calling
convention, parameter types, the header it is in). A wrapper is recognised when its instructions
only load ECX (an address, a field of ECX, an argument), push constants or its own arguments, and
end in CALL or JMP to a function that has a declaration: the C++ then calls that function.
"""
import re

from tinysym import Unsupported, num, off

H = r'(-?0x[0-9a-f]+)'


def parse_decls(headers):
    """{name: dict(ret, conv, params, header)} from the project's headers."""
    out = {}
    pat = re.compile(r'^[ \t]*((?:const\s+)?[A-Za-z_][\w\s\*:<>]*?)\s*__(fastcall|cdecl|stdcall)\s+(\w+)\s*\(([^;{}]*?)\)\s*;', re.M | re.S)
    for path, text in headers:
        text = re.sub(r'//[^\n]*', '', text)
        for m in pat.finditer(text):
            ret, conv, name, params = m.group(1).strip(), m.group(2), m.group(3), m.group(4).strip()
            plist = [p.strip() for p in params.split(',')] if params and params != 'void' else []
            out.setdefault(name, dict(ret=ret, conv=conv, params=plist, header=path))
    return out


def ptype(param):
    """The type of a parameter declaration, without its name."""
    m = re.match(r'^(.*[\*\s])(\w+)$', param.strip())
    return m.group(1).strip() if m else param.strip()


def wrapper(addr, texts, known, decls):
    try:
        return _wrapper(addr, texts, known, decls)
    except Unsupported:
        return None


def _wrapper(addr, texts, known, decls):
    ecx = 'o'            # C++ for the object pointer ECX holds
    ecx_is_this = True
    eax = None
    stack = []           # pushed values, first pushed first
    args_used = 0
    pre_stores = []
    state = dict(args=0)

    def arg(i):
        state['args'] = max(state['args'], i + 1)
        return 'v%d' % i

    n = len(texts)
    i = 0
    tail = None
    ctor = texts[:2] == ['PUSH ESI', 'MOV ESI,ECX']   # PUSH ESI; MOV ESI,ECX; CALL base; MOV [ESI..],vtable; MOV EAX,ESI; POP ESI; RET
    if ctor:
        i = 2
    while i < n:
        t = texts[i]
        i += 1
        m = re.match(r'^MOV EAX,dword ptr \[ESP \+ 0x([0-9a-f]+)\]$', t) or re.match(r'^MOV EAX,\[ESP \+ 0x([0-9a-f]+)\]$', t)
        if m:
            k = num('0x' + m.group(1))
            if k < 4 or k % 4:
                raise Unsupported('slot')
            eax = arg(k // 4 - 1)
            continue
        m = re.match(r'^MOV ECX,(?:dword ptr )?\[ESP \+ 0x([0-9a-f]+)\]$', t)
        if m:
            k = num('0x' + m.group(1))
            if k < 4 or k % 4:
                raise Unsupported('slot')
            ecx, ecx_is_this = 'reinterpret_cast<u8 *>((uintptr_t)%s)' % arg(k // 4 - 1), False
            continue
        m = re.match(r'^MOV ECX,(0x[0-9a-f]+)$', t)
        if m and num(m.group(1)) >= 0x400000:
            ecx, ecx_is_this = 'reinterpret_cast<u8 *>(0x%08x)' % num(m.group(1)), False
            continue
        m = re.match(r'^MOV ECX,(?:dword ptr )?\[ECX(?: \+ %s)?\]$' % H, t)
        if m:
            ecx = 'at<u8 *>(%s, %s)' % (ecx, off(num(m.group(1))))
            continue
        m = re.match(r'^(?:LEA ECX,\[ECX(?: \+ %s)?\]|ADD ECX,%s)$' % (H, H), t)
        if m and m.group(1) is None and m.group(2) is not None:
            m = None
            k = num(re.match(r'^ADD ECX,%s$' % H, t).group(1))
            ecx = '(%s) + 0x%x' % (ecx, k)
            continue
        if m:
            k = num(m.group(1))
            ecx = '(%s) %s 0x%x' % (ecx, '-' if k < 0 else '+', abs(k))
            continue
        m = re.match(r'^MOV (?:dword ptr )?\[ECX(?: \+ %s)?\],(-?0x[0-9a-f]+)$' % H, t)
        if m and ecx == 'o' and not ctor:
            pre_stores.append('at<u32>(o, %s) = 0x%x;' % (off(num(m.group(1) or '0')), num(m.group(2)) & 0xffffffff))
            continue
        m = re.match(r'^PUSH (-?0x[0-9a-f]+)$', t)
        if m:
            stack.append('0x%x' % (num(m.group(1)) & 0xffffffff))
            continue
        if t == 'PUSH EAX' and eax is not None:
            stack.append(eax)
            continue
        m = re.match(r'^(CALL|JMP) 0x([0-9a-f]+)$', t)
        if m:
            tail = (m.group(1), int(m.group(2), 16), i)
            break
        raise Unsupported(t)
    if tail is None:
        raise Unsupported('no call')
    kind, target, after = tail
    name = known.get(target)
    d = decls.get(name) if name else None
    if not d:
        raise Unsupported('callee')
    rest = texts[after:]
    ret_pop = 0
    post_stores = []
    if ctor:
        if kind != 'CALL' or ecx != 'o' or stack or state['args'] or eax is not None:
            raise Unsupported('ctor shape')
        while rest:
            m = re.match(r'^MOV (?:dword ptr )?\[ESI(?: \+ %s)?\],(-?0x[0-9a-f]+)$' % H, rest[0])
            if not m:
                break
            post_stores.append('at<u32>(o, %s) = 0x%x;' % (off(num(m.group(1) or '0')), num(m.group(2)) & 0xffffffff))
            rest = rest[1:]
        if rest != ['MOV EAX,ESI', 'POP ESI', 'RET']:
            raise Unsupported('ctor tail')
        rest = ['RET']
    if kind == 'CALL':
        if rest and re.match(r'^ADD ESP,0x[0-9a-f]+$', rest[0]):
            rest = rest[1:]
        if len(rest) != 1:
            raise Unsupported('after the call')
        m = re.match(r'^RET(?: (0x[0-9a-f]+))?$', rest[0])
        if not m:
            raise Unsupported('ret')
        ret_pop = num(m.group(1)) // 4 if m.group(1) else 0
    else:
        if rest:
            raise Unsupported('after the jump')
    params = d['params']
    this_call = d['conv'] == 'fastcall'
    # the arguments the callee takes after the object (and the unused edx slot)
    callee_args = params[2:] if this_call else params
    if this_call and not params:
        raise Unsupported('thiscall without the object')
    if kind == 'JMP':
        # the wrapper passes its own stack arguments through: as many as the callee takes
        passed = ['v%d' % j for j in range(len(callee_args))]
        if stack:
            raise Unsupported('push before a jump')
        nargs = len(callee_args)
        call_args = passed
    else:
        if len(stack) != len(callee_args):
            raise Unsupported('argument count')
        call_args = list(reversed(stack))
        nargs = max(ret_pop, state['args'])
    # the wrapper takes the object (thiscall) only when the callee does and the object is what it passes on
    wrapper_uses_this = this_call and re.search(r'\bo\b', ecx) is not None
    if wrapper_uses_this:
        conv = 't'
    else:
        popped_by_wrapper = ret_pop if kind == 'CALL' else (nargs if d['conv'] != 'cdecl' else 0)
        conv = 's' if popped_by_wrapper else 'c'
    # C++ call
    casts = []
    for k, a in enumerate(call_args):
        pt = ptype(callee_args[k])
        if pt.endswith('*'):
            casts.append('reinterpret_cast<%s>(%s)' % (pt, a if a.startswith('0x') else '(uintptr_t)' + a))
        else:
            casts.append(a)
    if this_call:
        first = ptype(params[0])
        first_arg = 'reinterpret_cast<%s>(%s)' % (first, ecx) if first.endswith('*') else ecx
        call = '%s(%s)' % (name, ', '.join([first_arg] + (['nullptr'] if len(params) > 1 else []) + casts))
    else:
        call = '%s(%s)' % (name, ', '.join(casts))
    ret = d['ret']
    if ret not in ('void', 'u32', 's32', 'u8', 'u16', 'u8 *', 'const u8 *') and not ret.endswith('*'):
        raise Unsupported('return type')
    rk = {'void': 'v', 'u32': 'a', 's32': 'a', 'u8': 'b', 'u16': 'w'}.get(ret, 'p' if ret.endswith('*') else 'a')
    body = ('return ' if ret != 'void' else '') + call + ';'
    if pre_stores:
        body = ' '.join(pre_stores) + ' ' + body
    if ctor:
        ret, rk = 'u8 *', 'p'
        body = call + '; ' + ' '.join(post_stores) + (' ' if post_stores else '') + 'return o;'
    return dict(name='Wrap_%s_%06x' % (name, addr), conv=conv, nargs=nargs, ret=rk, rettype=ret, doc='calls %s' % name, body=body, callee=target,
                includes=[d['header']], globals=[])
