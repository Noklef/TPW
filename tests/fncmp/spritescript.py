# The sprite script VM (src/game/sprite/script: 0x4750c0-0x475f40, 0x476020-0x476a10,
# 0x540b90-0x540c70, 0x541f60) against the original: the VM's globals, a slot table of scripts,
# a block of code (generated, or the game's own scripts) and the sprite folders. SpriteScript_Run
# and the list functions that allocate, free or read the clock call out of range, so the opcodes
# run one at a time here; Kill only ever meets a slot it cannot free.
# usage: spritescript.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, global_heap, heap, put, record_stream  # noqa: E402

B = Bench([(0x475010, 0x476a30), (0x540b90, 0x540d78), (0x541f60, 0x541f70), (0x5da3c0, 0x5da3c1), (0x402d70, 0x403220), (0x4033a0, 0x4033f0),
           (0x5f5660, 0x5f56a0), (0x5f5e20, 0x5f5e90), (0x5f5fa0, 0x5f5fc0)])
SCRIPTS, TABLE, SPRITES, OUT, STREAM, DATA = (RAM + 0x1000 * k for k in range(6))
CLOCK = 0x785970     # the game clock: frozen (+0x38), its time +0x3c, plus +0x48
END = 0x5da3c0       # the END opcode (the empty Debug_Printf)
GLOBALS = 0x7b49f0   # slot table, slots, registers, now, code, ?, steps, ?, step limit, ready, current
FOLDERS = 0x763f88   # gSpriteFolders: 14 x (name, first sprite, count)
GSPRITES = 0x876b48  # gSprites: sprite pointers
CODE_TABLE, CODE_END = 0x74dab8, 0x74f7c0
CODE = CODE_END - 0x400  # generated code, running into the end of the table
NSCRIPTS, STRIDE, NSPRITES = 6, 0x120, 4

OP = {'Goto': 0x476020, 'If': 0x476050, 'EndIf': 0x4762f0, 'Else': 0x476320, 'Gosub': 0x476340, 'Repeat': 0x4763b0,
      'Until': 0x4763d0, 'Yield': 0x4766c0, 'Set': 0x4766d0, 'Add': 0x476750, 'Sub': 0x4767e0, 'Move': 0x476870,
      'SetAnim': 0x4768b0, 'PlayReg': 0x4768d0, 'Play': 0x476940, 'PlayAlways': 0x4769b0, 'Hide': 0x4769f0,
      'StepR9': 0x476a10}


def f32(x):
    return struct.unpack('<I', struct.pack('<f', x))[0]


def nan(r):
    return 0x7f800000 | (r.getrandbits(23) or 1) | r.getrandbits(1) << 31


def snan(r):
    return 0x7f800000 | (r.getrandbits(22) or 1) | r.getrandbits(1) << 31


def small(r):
    return r.choice([0, 1, 2, -1, 5, r.randrange(-20, 20), r.randrange(-300, 300)]) & 0xffffffff


def imm(r):
    return r.choice([small(r), small(r), small(r), 0x7fffffff, 0x80000000, 0x1000001, r.getrandbits(32)])


def freg(r):
    """A float register: mostly whole (so the tests meet equal values), else wide or NaN."""
    return r.choice([f32(float(r.randrange(-20, 20))), f32(float(r.randrange(-20, 20))), f32(r.uniform(-20, 20)),
                     f32(r.uniform(-1e3, 1e3)), f32(16777216.0), f32(16777217.0 - 2), f32(-0.0), f32(3.3e38),
                     f32(-3.3e38), f32(1e-38), f32(1e-45), 0x7f800000, 0xff800000, nan(r), snan(r), r.getrandbits(32)])


def regidx(r):
    return r.choice([r.randrange(1, 7), r.randrange(1, 7), r.randrange(19), r.randrange(19), 0, 9])


def test_code(r):
    return r.choice([r.randrange(1, 9), r.randrange(1, 9), r.randrange(1, 9), 0, 9, r.getrandbits(32)])


def code_word(r):
    return r.choice([OP['If']] * 5 + [OP['EndIf']] * 4 + [OP['Else']] * 2 + list(OP.values()) +
                    [small(r), regidx(r), test_code(r), imm(r)])


def script(r, k, code):
    s = bytearray(r.bytes(STRIDE))
    base = code if r.random() < 0.9 else r.choice([CODE, CODE_TABLE, CODE + 4 * r.randrange(-8, 8)])
    pc = r.choice([r.randrange(0, 0x100), r.randrange(0, 0x100), 0xfc, 0xff, 0x100, 0x102, r.randrange(-4, 0),
                   0x1fe2c310 + r.randrange(-2, 3)])  # past 2^31: the skippers' compare is signed
    if base == CODE_TABLE:
        pc = r.randrange(0, 1870)
    put(s, 4, 'I', k)
    put(s, 8, 'iIiI', pc, r.randrange(0x100), r.randrange(0x100), base)
    put(s, 0x18, 'I', r.choice([1, 1, 2, 3, 4, r.getrandbits(32)]))
    put(s, 0x1c, 'i', r.choice([20, 20, r.randrange(0, 21), 0, 1, 19, r.randrange(-3, 22)]))
    put(s, 0x70, 'iii', *(r.choice([0, 0, 1, 2, 10, 11, -1, r.randrange(-3, 15)]) for _ in range(3)))
    for i in range(19):
        put(s, 0x84 + 4 * i, 'I', freg(r) if 1 <= i <= 6 or i in (8, 9) else r.choice([small(r), small(r), imm(r), freg(r)]))
    put(s, 0xac, 'II', r.randrange(14) if r.random() < 0.95 else r.getrandbits(4), r.choice([r.randrange(8), r.randrange(-3, 12)]) & 0xffffffff)
    put(s, 0xb4, 'I', r.choice([r.randrange(8), r.getrandbits(32)]))
    put(s, 0xc4, 'I', r.getrandbits(32))
    put(s, 0x114, 'I', r.choice([0, 1, r.getrandbits(32)]))
    return s


LIST = ['Run', 'Update', 'ListInit', 'Shutdown', 'Save', 'Load', 'ScriptInit', 'Start', 'Clone']


def list_case(r, op, regions, slot, code_arg, current, nslots, words):
    """The run loop and the list functions that allocate, free, stream or read the clock: the scripts from heap() through the GlobalAlloc
    import (global_heap()), the game clock frozen at a time some scripts are due by (+0x7c), END opcodes among the code."""
    regions[HEAP] = heap()
    regions.update(global_heap())
    clock = bytearray(r.bytes(0x80))
    t, d = r.getrandbits(31), r.choice([0, 0, small(r)])
    put(clock, 0x38, 'III', 1, t, r.getrandbits(32))
    put(clock, 0x48, 'I', d)
    regions[CLOCK] = clock
    now = (t + d) & 0xffffffff
    for k in range(NSCRIPTS):
        put(regions[SCRIPTS], STRIDE * k + 0x7c, 'I', (now + r.choice([-100, -1, 0, 1, 100, r.randrange(-2000, 2000)])) & 0xffffffff)
    if op in ('Run', 'Update'):  # a program of whole instructions, the scripts at their starts
        starts = program(r, words)
        for k in range(NSCRIPTS):
            if r.random() < 0.9:
                put(regions[SCRIPTS], STRIDE * k + 8, 'i', r.choice(starts))
                put(regions[SCRIPTS], STRIDE * k + 0x14, 'I', CODE)
    for _ in range(r.randrange(0, 3)):
        words[r.randrange(0x100)] = END
    regions[CODE] = bytearray(struct.pack('<256I', *words))
    xyz = [freg(r) for _ in range(3)]
    folder, sprite, anim = r.randrange(14), r.choice([r.randrange(8), r.randrange(2)]), r.randrange(5)
    if op == 'Run':
        return Call(0x475010, [r.choice([current, SCRIPTS + STRIDE * r.randrange(NSCRIPTS)])], regions, ret='a')
    if op == 'Update':
        return Call(0x475360, [], regions, ret='a')
    if op == 'ListInit':
        return Call(0x475290, [], regions)
    if op == 'Shutdown':
        return Call(0x4752e0, [], regions)
    if op == 'Save':
        regions[STREAM] = record_stream(STREAM, DATA, r.choice([0x1000, 0x1000, 0x1000, r.randrange(0x200), 8, 12]))
        regions[DATA] = r.bytes(0x1000)
        return Call(0x475650, [STREAM], regions)
    if op == 'Load':
        n = r.choice([1, 2, 5, 12, r.randrange(1, 13), r.randrange(1, 13)])
        table = [0] + [r.choice([0, RAM + 0x3000]) for _ in range(n - 1)]
        data = struct.pack('<3I', 0x53435054 if r.random() < 0.9 else r.getrandbits(32), 0x118 if r.random() < 0.9 else r.choice([0x120, 0]), n)
        data += struct.pack('<%dI' % n, *table)
        for k in range(n):
            if table[k]:
                data += bytes(script(r, k, CODE)[:0x118])
        left = len(data) if r.random() < 0.8 else r.randrange(len(data) + 1)
        regions[STREAM] = record_stream(STREAM, DATA, left)
        regions[DATA] = data + r.bytes(0x10)
        return Call(0x475730, [STREAM], regions, ret='a')
    if op == 'ScriptInit':
        code = r.choice([CODE_TABLE + 4 * r.randrange(1858), CODE_TABLE + 4 * r.randrange(1858), CODE_TABLE, CODE_TABLE - 4, r.getrandbits(32)])
        return Call(0x4758f0, [SCRIPTS + STRIDE * r.randrange(NSCRIPTS), code, folder, sprite, anim] + xyz + [imm(r), imm(r)], regions)
    if op == 'Start':
        return Call(0x475a10, [code_arg, folder, sprite, anim] + xyz, regions, ret='a')
    table = struct.unpack('<12I', regions[TABLE])
    used = [k for k in range(1, nslots) if table[k]]
    if not used and nslots > 1:
        put(regions[TABLE], 4, 'I', SCRIPTS)
        used = [1]
    if used and r.random() < 0.9:
        slot = r.choice(used)
    return Call(0x475cf0, [slot] + xyz + [freg(r)], regions, ret='a')


def program(r, words):
    """words (256) become whole instructions with operands that make sense, END among them; returns where each starts."""
    starts, at, gotos = [], 0, []
    while at < 0x100 - 4:
        name = r.choice(list(OP) + ['End'] + ['Set', 'Add', 'If', 'EndIf', 'Yield'] * 2)
        starts.append(at)
        if name == 'End':
            ops = [END]
        elif name in ('If', 'Until'):
            t = r.randrange(1, 9)
            ops = [OP[name], regidx(r), t, regidx(r) if t <= 4 else imm(r)]
        elif name in ('Set', 'Add', 'Sub'):
            ops = [OP[name], regidx(r), imm(r)]
        elif name == 'Move':
            ops = [OP[name], regidx(r), regidx(r)]
        elif name in ('Goto', 'Gosub'):
            ops = [OP[name], 0]
            gotos.append(at + 1)
        elif name == 'PlayReg':
            ops = [OP[name], regidx(r)]
        elif name in ('SetAnim', 'Play', 'PlayAlways'):
            ops = [OP[name], r.choice([small(r), 0xffffffff, imm(r)])]
        else:
            ops = [OP[name]]
        words[at:at + len(ops)] = ops
        at += len(ops)
    for g in gotos:
        words[g] = CODE + 4 * r.choice(starts) if r.random() < 0.95 else r.choice([CODE - 4, CODE + 4 * r.randrange(0x100)])
    return starts


def case(r):
    op = r.deal(OPS)
    code = CODE
    words = [code_word(r) for _ in range(0x100)]
    scripts = bytearray()
    for k in range(NSCRIPTS):
        scripts += script(r, k + 1, code)
    nslots = r.choice([1, 2, 4, 8, 12, r.randrange(1, 13)])
    slots = [0] + [r.choice([0] + [SCRIPTS + STRIDE * k for k in range(NSCRIPTS)] * 3) for _ in range(11)]
    current = SCRIPTS + STRIDE * r.randrange(NSCRIPTS)
    g = bytearray(r.bytes(0x2c))
    regs = current + 0x84 if r.random() < 0.95 else SCRIPTS + STRIDE * r.randrange(NSCRIPTS) + 0x84
    ready = r.choice([1, 1, 1, 1, 0, r.getrandbits(32)])
    put(g, 0, 'IiIIII', TABLE, nslots, regs, r.getrandbits(32), code if r.random() < 0.95 else CODE_TABLE, r.getrandbits(32))
    put(g, 0x18, 'IIIII', r.getrandbits(32), r.getrandbits(32), r.choice([50, 1, 0]), ready, current)
    folders = bytearray(r.bytes(14 * 12))
    for f in range(14):
        put(folders, 12 * f + 4, 'Ii', r.randrange(40), r.choice([1, 2, 5, 8, r.randrange(1, 20), -3, -1]))
    sprites = bytearray()
    for _ in range(NSPRITES):
        s = bytearray(r.bytes(0x260))
        for a in range(4):  # the actions: code entry + 1, anim + 1, r17, r18
            put(s, 0x20e + 4 * a, 'BBBB', r.choice([1, 2, 3, 0, r.randrange(256)]), r.randrange(6), r.randrange(256), r.randrange(256))
        sprites += s
    gs = struct.pack('<64I', *(SPRITES + 0x260 * r.randrange(NSPRITES) for _ in range(64)))
    regions = {SCRIPTS: scripts, TABLE: bytearray(struct.pack('<12I', *slots)), SPRITES: sprites,
               OUT: bytearray(r.bytes(0x10)), GLOBALS: g, CODE: bytearray(struct.pack('<256I', *words)),
               FOLDERS: folders, GSPRITES: bytearray(gs)}
    slot = r.choice([r.randrange(1, nslots + 1), r.randrange(1, nslots + 1), 0, -1, nslots, r.randrange(13)])
    code_arg = r.choice([0, 1, 2, 3, CODE_TABLE + 4 * r.randrange(1858), CODE_TABLE + 4 * r.randrange(1858), 4, -1 & 0xffffffff])
    if op in LIST:
        return list_case(r, op, regions, slot, code_arg, current, nslots, words)
    if op in OP:
        s = regions[SCRIPTS]
        pc = struct.unpack_from('<i', s, current - SCRIPTS + 8)[0]
        if r.random() < 0.8 and 0 <= pc < 0x100 - 3:  # operands that make sense
            if op in ('If', 'Until'):  # r, test, r (1-4) or n (5-8)
                t = r.randrange(1, 9)
                ops = [regidx(r), t, regidx(r) if t <= 4 else imm(r)]
                if r.random() < 0.5:  # the immediate equal to the register
                    v = struct.unpack_from('<f', s, current - SCRIPTS + 0x84 + 4 * ops[0])[0]
                    if t > 4 and v == v and abs(v) < 2e9:
                        ops[2] = int(v) & 0xffffffff
            elif op in ('Set', 'Add', 'Sub'):
                ops = [regidx(r), imm(r)]
            elif op == 'Move':
                ops = [regidx(r), regidx(r)]
            elif op in ('Goto', 'Gosub'):
                ops = [r.choice([CODE + 4 * r.randrange(0x100), CODE_TABLE + 4 * r.randrange(1858), CODE - 4, r.getrandbits(32)])]
            elif op == 'PlayReg':
                ops = [regidx(r)]
            else:
                ops = [r.choice([small(r), 0xffffffff, imm(r)])]
            words[pc:pc + len(ops)] = ops
            regions[CODE] = bytearray(struct.pack('<256I', *words))
        return Call(OP[op], [], regions)
    if op == 'Push':
        return Call(0x475230, [imm(r)], regions)
    if op == 'Pop':
        return Call(0x475260, [], regions, ret='a')
    if op == 'SkipBlock':
        return Call(0x4750c0, [], regions)
    if op == 'SkipToElse':
        return Call(0x475130, [], regions)
    if op == 'SetPosition':
        return Call(0x475480, [slot, freg(r), freg(r), freg(r), imm(r)], regions)
    if op == 'GetPosition':
        return Call(0x4754e0, [slot] + [r.choice([OUT + 4 * k, 0]) for k in range(4)], regions)
    if op == 'Kill':  # never a script it would free (that calls out of range)
        if ready and 0 < slot < nslots and slots[slot]:
            slot = r.choice([0, -1, nslots, 12])
        return Call(0x475550, [slot], regions)
    if op == 'SetReg':
        return Call(0x475590, [slot, r.choice([regidx(r), 18, 19, r.getrandbits(32)]), imm(r)], regions)
    if op == 'SetRegF':
        return Call(0x4755d0, [slot, r.choice([regidx(r), 18, 19, r.getrandbits(32)]), freg(r)], regions)
    if op == 'SetPeriod':
        return Call(0x475610, [slot, imm(r)], regions)
    if op == 'Jump':
        return Call(0x475b80, [slot, code_arg], regions)
    if op == 'IsAt':
        return Call(0x475c50, [slot, code_arg], regions, ret='a')
    if op == 'CopyFrom':  # both slots in use mostly (an empty or out-of-range one is a null copy: crash in both)
        table = struct.unpack('<12I', regions[TABLE])
        used = [k for k in range(1, nslots) if table[k]]
        src = r.choice([r.randrange(1, nslots + 1), 0, r.randrange(13)])
        if used and r.random() < 0.8:
            slot, src = r.choice(used), r.choice(used)
        return Call(0x475e10, [slot, src], regions)
    if op == 'FindShown':
        return Call(0x475ef0, [r.choice([SCRIPTS + STRIDE * r.randrange(NSCRIPTS) + 0xd0, SCRIPTS + 0xd0, 0, OUT])], regions, ret='a')
    if op == 'WrapSprites':
        return Call(0x475f40, [], regions)
    if op == 'Show':
        return Call(0x540b90, [current], regions)
    if op == 'FrameCount':
        return Call(0x540c60, [SPRITES + 0x260 * r.randrange(NSPRITES), r.randrange(5)], regions, conv='t', ret='a')
    if op == 'GetAction':
        return Call(0x540c70, [SPRITES + 0x260 * r.randrange(NSPRITES), r.choice([0, 1, 2, 3, 4, r.getrandbits(32)]),
                               OUT, OUT + 4, OUT + 8, OUT + 12], regions, conv='t')
    return Call(0x541f60, [r.randrange(14)], regions, ret='a')


OPS_ALL = list(OP) + ['Push', 'Pop', 'SkipBlock', 'SkipToElse', 'SetPosition', 'GetPosition', 'Kill', 'SetReg', 'SetRegF',
                      'SetPeriod', 'Jump', 'IsAt', 'CopyFrom', 'FindShown', 'WrapSprites', 'Show', 'FrameCount',
                      'GetAction', 'FolderCount'] + LIST
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
