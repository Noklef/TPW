# One instruction of a ride script (src/game/rsse/rsseexec: RSSE_ExecuteInstruction 0x551cb0) against the original, one case per opcode dealt in turn (sub-opcodes of 0x35-0x37 too, words that are no instruction now and
# then). Its calls are cdecl recorders (the driver's RsseExecEnv); RSSE_FetchWord, RSSE_OperandValue, Thing_ByHandle, World_Rand and the one-line 0x409cd0 run as they are. The fixture: a script with its code (the operands
# variables, immediates, labels, strings or anything), variables, stack, limbo and rider slots, object nodes, entries, and three other scripts on the list; the clock and each deadline close together.
# usage: rsseexec.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fill, log_region, put, set_return, strings  # noqa: E402

CALLS = {0x402d70: (31, 0), 0x556ab0: (19, 1), 0x5f5680: (28, 1), 0x5573d0: (2, 5), 0x557970: (3, 5), 0x51ff70: (9, 2), 0x51c2c0: (6, 1), 0x51c300: (7, 1), 0x51bc40: (4, 3), 0x473270: (31, 2), 0x4732a0: (1, 6), 0x473fb0: (2, 4),
         0x4386d0: (1, 3), 0x44b220: (2, 3), 0x44b410: (3, 3), 0x44b4c0: (6, 2), 0x4587b0: (7, 2), 0x458910: (4, 1), 0x458940: (5, 1), 0x4587e0: (8, 2), 0x458890: (1, 4), 0x556af0: (5, 4), 0x556a80: (4, 4), 0x551130: (1, 7),
         0x551290: (5, 3), 0x551560: (3, 4), 0x551320: (3, 6), 0x51bfc0: (2, 6), 0x51e710: (8, 1), 0x55a620: (2, 7), 0x55d3d0: (22, 1), 0x55d830: (20, 2), 0x55d610: (23, 1), 0x55dcf0: (25, 2), 0x55d950: (22, 2), 0x55da70: (24, 2),
         0x55dd20: (24, 1), 0x55dd80: (26, 1), 0x55ddd0: (26, 2), 0x55d9f0: (23, 2), 0x55dd40: (25, 1), 0x55ddf0: (27, 2), 0x55de10: (28, 2), 0x55de30: (27, 1), 0x54aa80: (14, 2), 0x54ab40: (18, 1), 0x544f90: (15, 1), 0x549db0: (13, 2),
         0x544a10: (10, 1), 0x544840: (9, 1), 0x544c80: (13, 1), 0x544e50: (14, 1), 0x544c00: (12, 1), 0x544b50: (11, 1), 0x5452a0: (16, 1), 0x549b80: (17, 1), 0x545100: (12, 2), 0x54ad90: (15, 2), 0x52a490: (10, 2), 0x52a700: (11, 2),
         0x43b050: (1, 1), 0x43b0e0: (1, 2), 0x43b1f0: (2, 1), 0x43b220: (3, 1), 0x43b270: (2, 2), 0x43b2b0: (3, 2), 0x43b2f0: (4, 2), 0x43b330: (5, 2), 0x5587f0: (6, 3), 0x559060: (18, 2), 0x556f40: (1, 8), 0x5571a0: (17, 2),
         0x557110: (20, 1), 0x557160: (6, 4), 0x556de0: (16, 2), 0x55a070: (21, 1), 0x556b90: (1, 5), 0x67d3f0: (29, 1), 0x67d8e0: (30, 1)}
HOOKS = {a: _recorder(k, n, cdecl=True) for a, (k, n) in CALLS.items()}
B = Bench([(0x551cb0, 0x5567d8), (0x557a70, 0x557aa8), (0x5573a0, 0x5573c5), (0x4cd300, 0x4cd331), (0x516330, 0x516370), (0x409cd0, 0x409cdd), (0x5da3c0, 0x5da3c1), (0x67a830, 0x67a860)], hooks=HOOKS)
CTX, CODE, VARS, STACK, LIMBO, RIDERS, ENTRIES, TEXT, PATH, NODES, BLOCK, RIDE, MODEL, THING, TM, STATE, ANY = (RAM, RAM + 0x200, RAM + 0x300, RAM + 0x340, RAM + 0x360, RAM + 0x380, RAM + 0x3c0, RAM + 0x3d0, RAM + 0x3f0, RAM + 0x400,
                                                                                RAM + 0x460, RAM + 0x480, RAM + 0x490, RAM + 0x4b0, RAM + 0x4e0, RAM + 0x4f8, RAM + 0x4fc)
OTHERS, OTHER_VARS = RAM + 0x500, RAM + 0x800
NAMES = [0, 3, 7, 10, 14]
OPS = list(range(0x6a)) + [0x35] * 19 + [0x36] * 18 + [0x37] * 9    # (each sub-opcode as often as an opcode)
SUB35, SUB36, SUB37 = list(range(0, 0x14)), list(range(0, 0x13)), list(range(0, 0xa))
LABEL_OPS = (0x1d, 0x1f, 0x20, 0x21, 0x22, 0x23)
STRING_OPS = (0x25, 0x3f, 0x40, 0x5a)
FOURTH_RATE = 0x15


def operand(r):
    k = r.random()
    if k < 0.6:
        return 0x40000000 | r.randrange(16)
    if k < 0.9:
        return r.choice([r.randrange(0, 10), r.randrange(0, 10), r.randrange(0, 4), r.getrandbits(16), 0xffff]) | r.choice([0, 0, r.getrandbits(4) << 16])
    return r.choice([0x20000000 | r.randrange(0, 40), 0x10000000 | r.choice(NAMES), r.getrandbits(32) & 0x8fffffff, 0x50000000 | r.randrange(16)])


def floats(r):
    return [r.choice([0.0, 1.5, -2.25, 300.5, 30000.5, -70000.0, r.randrange(-5000, 5000) / 8]) for _ in range(3)]


def script(r, at, ids):
    other = bytearray(r.bytes(0x100))
    put(other, 0, 'I', OTHERS + 0x100 * (at + 1) if at < 2 else 0)
    put(other, 8, 'i', ids[at])
    put(other, 0x1c, 'I', OTHER_VARS + 0x20 * at)
    put(other, 0x34, 'I', r.choice([TEXT, TEXT, TEXT, 0]))
    put(other, 0x74, 'i', r.choice(NAMES + NAMES + [-1]))
    put(other, 0x8c, 'i', r.choice([0, 2, 8]))
    return other


def case(r):
    log = log_region()
    op = r.deal(OPS)
    clock = r.choice([1000, 100000, 0x7fffff00, r.getrandbits(32)])
    code = [operand(r) for _ in range(12)]
    word = 0x80000000 | op
    if r.random() < 0.015:
        word = r.choice([op, 0x40000000 | op, 0x80000000 | r.randrange(0x6a, 0x200), 0x81000000 | op])
    code[0] = word
    if op == 0x35:
        code[1] = r.deal(SUB35)
    elif op == 0x36:
        code[1] = r.deal(SUB36)
    elif op == 0x37:
        code[1] = r.deal(SUB37)
    elif op in LABEL_OPS and r.random() < 0.85:
        code[1] = 0x20000000 | r.randrange(0, 40)
    elif op in STRING_OPS and r.random() < 0.85:
        code[1] = 0x10000000 | r.choice(NAMES)
    elif op in (0xa, 0xb, 0xc, 0x39):
        code[1] = r.randrange(0, 4)
    if op == FOURTH_RATE and r.random() < 0.97:
        code[4] = r.choice([r.randrange(1, 2000), r.randrange(1, 0x7fff)])
    if op == 0x1c and code[2] & 0xffff == 0xffff:
        code[2] = r.getrandbits(15)
    if op == 0x36 and code[1] == 0x11:
        code[2] = r.choice([0, 1, 2])
    start = r.choice([0, 0, 2])
    words = [r.getrandbits(32) for _ in range(start)] + code
    length = len(words)
    k = r.random()
    if k < 0.04:
        length = start + r.randrange(1, 5)
    speed = r.choice([0, 0, 100, -100, -50, 1, r.randrange(-300, 300), r.getrandbits(16) - 0x8000])
    ctx = bytearray(r.bytes(0xf0))
    ids = [r.randrange(1, 100) * 3, r.randrange(100, 200) * 3, r.randrange(200, 300) * 3]
    owner = r.choice([0, 1, 2, 3])
    put(ctx, 0, 'I', OTHERS)
    put(ctx, 8, 'i', r.randrange(1, 1000))
    put(ctx, 0xc, 'i', r.choice(ids + ids + [0]))
    put(ctx, 0x10, 'i', r.choice(ids + ids + [0]))
    put(ctx, 0x18, 'I', CODE)
    put(ctx, 0x1c, 'I', VARS)
    put(ctx, 0x20, 'I', STACK if r.random() < 0.95 else 0)
    put(ctx, 0x24, '2I', LIMBO, RIDERS)
    put(ctx, 0x30, 'I', ENTRIES if r.random() < 0.9 else 0)
    put(ctx, 0x34, '2I', TEXT if r.random() < 0.95 else 0, PATH)
    put(ctx, 0x3c, 'i', start if r.random() < 0.98 else r.choice([-1, -10000, length, length + 3]))
    put(ctx, 0x40, '2i', r.choice([-2, -1, 0, 3, 6, 7, 8, 9]), r.choice([-1, 0, 1, 3, 7, 8]))
    put(ctx, 0x48, 'i', r.choice([0, 0, 1, -1, r.getrandbits(31)]))
    put(ctx, 0x4c, 'i', r.choice([0, 3, 3, 3]))
    put(ctx, 0x50, '2I', length, r.choice([8, 8, 8, 0]))
    put(ctx, 0x58, 'I', r.choice([0, 3, 3, 3]))
    put(ctx, 0x60, 'I', r.randrange(0, 4))
    put(ctx, 0x64, 'I', r.choice([0, 3, 3, 3]))
    put(ctx, 0x6c, '2h', r.choice([0, 2, -1]), r.randrange(-5, 5))
    put(ctx, 0x8c, 'I', r.choice([3, 16, 16]))
    near = r.choice([clock, clock - 1, clock + 1, clock + 5000, clock - 5000])
    put(ctx, 0xa0, 'I', r.choice([0, 0, near & 0xffffffff, r.choice([clock, clock - 1, clock + 1]) & 0xffffffff]))
    put(ctx, 0xa4, 'I', r.choice([0, 0, clock & 0xffffffff, (clock + 1) & 0xffffffff, (clock - 1) & 0xffffffff]))
    if op == 0x12 and r.random() < 0.6:
        code[1], code[2] = r.randrange(0, 10), r.randrange(0, 10)
        words[start + 1], words[start + 2] = code[1], code[2]
    put(ctx, 0xa8, 'I', r.choice([0xffff, 0, ((code[2] & 0xffff) << 16) + (code[1] & 0xffff)]))
    put(ctx, 0xac, 'H', r.randrange(4))
    bank = r.randrange(0, 8)
    state = r.choice([0, 0, bank + 1, r.getrandbits(32), (code[3] + 1) & 0xffffffff])
    put(ctx, 0xbc, 'I', state)
    put(ctx, 0xc0, 'h', speed)
    put(ctx, 0xc8, 'i', owner)
    put(ctx, 0xd0, 'I', r.choice([0, r.getrandbits(32)]))
    put(ctx, 0xe6, 'h', r.choice([0, 1]))
    put(ctx, 0x80, 'I', (clock - r.randrange(0, 6000)) & 0xffffffff)
    put(ctx, 0x84, 'I', r.choice([0, 1000, 3000, 7]))
    put(ctx, 0xc4, 'I', (clock + r.randrange(-3000, 3000)) & 0xffffffff)
    variables = bytearray(r.bytes(0x40))
    for i in range(16):
        if r.random() < 0.6:
            put(variables, 4 * i, 'i', r.choice([0, 1, 2, r.randrange(-300, 300)]))
    stack = bytearray(r.bytes(0x20))
    for i in range(8):
        if r.random() < 0.5:
            put(stack, 4 * i, 'I', 0x20000000 | r.randrange(40))
    limbo = bytearray()
    for i in range(3):
        limbo += struct.pack('<iI', r.choice([0, r.randrange(1, 9), r.randrange(1, 9)]), (clock + r.randrange(-3, 3)) & 0xffffffff)
    riders = bytearray()
    for i in range(3):
        riders += struct.pack('<iiII', r.choice([0, r.randrange(1, 9), r.randrange(1, 9)]), r.randrange(40), (clock + r.randrange(-3, 3)) & 0xffffffff,
                              (clock - r.choice([0, 1, 199, 200, 201, 999, 1000, 1100, 1300, -100, -250])) & 0xffffffff)
    entries = struct.pack('<3I', *[r.choice([0, 1, 2, 3]) for _ in range(3)])
    if r.random() < 0.3:
        entries = struct.pack('<3I', 5, 6, 7)
    text = b'ab\0abc\0ab\0abd\0ab\0'.ljust(0x20, b'\0')
    nodes = bytearray(0x54)
    count = r.randrange(0, 4)
    for i in range(count):
        at = 0x1c * i
        put(nodes, at, '2I', NODES + 0x1c * (i + 1) if i + 1 < count else 0, NODES + 0x1c * (i - 1) if i else 0)
        put(nodes, at + 8, '5I', r.choice([1, 2, 3, 4, 10, 0, 11]), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.randrange(0, 4))
    put(ctx, 0xb0, 'I', NODES if count else 0)
    others = bytearray()
    for at in range(3):
        others += script(r, at, ids)
    other_vars = r.bytes(0x60)
    set_return(log, 1, r.choice([0, 100, 300, 599, 600, 1000, r.getrandbits(32)]))
    set_return(log, 2, r.choice([0xffffffff, r.randrange(0, 5), r.getrandbits(32)]))
    for k in (3, 4, 5, 7, 8, 12, 13, 14, 15, 16, 17, 18, 20, 22, 23, 24, 25, 26, 27):
        set_return(log, k, r.getrandbits(32))
    set_return(log, 6, r.choice(ids + [0]))
    set_return(log, 19, BLOCK)
    set_return(log, 21, r.choice([OTHERS, OTHERS + 0x100, OTHERS + 0x200, 0]))
    set_return(log, 30, TM)
    set_return(log, 31, clock)
    if op in (0x13, 0x14, 0x1a, 0x1b):
        strings(log, 2, 0, out=2)
        fill(log, 2, 1, struct.pack('<I', r.choice([bank, (state - 1) & 0xffffffff, (state - 1) & 0xffffffff])))
    elif op == 0x35:
        strings(log, 1, 0, out=4)
        blob(log, 1, 1, 1)
        fill(log, 1, 2, struct.pack('<3f', *floats(r)))
    elif op in (0x56, 0x58, 0x5d):
        k = 4 if op == 0x5d else 5
        strings(log, k, 0, out=0xe)
        for arg, value in enumerate(floats(r)):
            fill(log, k, arg + 1, struct.pack('<f', value), which=arg)
    elif op == 0x69:
        strings(log, 1, 0, out=2)
    elif 0x61 <= op <= 0x66:
        strings(log, 29, 0, out=1)
        strings(log, 30, 0, out=1)
    elif op in (0x3f, 0x40):
        strings(log, 6, 1)
    rides = struct.pack('<4I', RIDE, RIDE, RIDE, RIDE)
    regions = {LOG: log, CTX: ctx, CODE: b''.join(struct.pack('<I', w & 0xffffffff) for w in words), VARS: variables, STACK: stack, LIMBO: limbo, RIDERS: riders, ENTRIES: entries, TEXT: text, PATH: b'dir/\0\0\0\0', NODES: nodes,
               BLOCK: r.bytes(0x1c), RIDE: struct.pack('<3I', 0, 0, MODEL), MODEL: struct.pack('<4I', 0, 0, 0, 0) + struct.pack('<i', r.choice([0, 90, 180, 359, r.randrange(-720, 720)])), THING: r.bytes(0x30), TM: r.bytes(0x18),
               STATE: struct.pack('<I', r.getrandbits(32)), ANY: bytes(4), OTHERS: others, OTHER_VARS: other_vars, 0x7a4610: rides, 0x7cf6ec: struct.pack('<I', ANY), 0x7cf83c: struct.pack('<I', (STATE - 0x1da708) & 0xffffffff),
               0x7cfb90: struct.pack('<I', THING).ljust(20, b'\0') * 4, 0x803a28: struct.pack('<I', r.getrandbits(32)),
               0x87919c: struct.pack('<2I', r.choice([0, 1]), 0 if op in (0x3f, 0x42, 0x43, 0x44, 0x45) and r.random() < 0.15 else 1) + bytes(0xc) + struct.pack('<2I', OTHERS, r.randrange(0, 50))}
    return Call(0x551cb0, [CTX], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
