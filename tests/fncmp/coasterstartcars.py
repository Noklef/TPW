# A coaster's first cars placed (src/game/coaster/startcars: 0x435570) against the original: one for a type with flag 0x20 (its part from the model, at the model's holder), else two (each found by key in the coaster's holder), a part or an object missing, tools set or not, the front end kept or not. The callees are recorders.
# usage: coasterstartcars.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, fill, log_region, put, set_return, strings  # noqa: E402

POSITION, FIND, ENTRY, CAR, TOOLS, FRONT, STATE = 1, 2, 3, 4, 5, 6, 7
B = Bench([(0x435570, 0x435710), (0x5da3c0, 0x5da3c1)],
          hooks={0x435710: _recorder(POSITION, 2, cdecl=True), 0x44b220: _recorder(FIND, 3, cdecl=True), 0x432c10: _recorder(ENTRY, 6, cdecl=True), 0x434060: _recorder(CAR, 9, cdecl=True), 0x442880: _recorder(TOOLS, 1, cdecl=True), 0x498780: _recorder(FRONT, 0, cdecl=True), 0x435730: _recorder(STATE, 2, cdecl=True)})
COASTER, TYPE, MODEL, P1, P2, HOLDER, A, C, OBJECTS, LINK, ANSWERS = (RAM + a for a in (0, 0x80, 0x100, 0x280, 0x2c0, 0x340, 0x380, 0x400, 0x600, 0x800, 0x880))
BB = RAM + 0x3c0
OPS = ['one', 'one', 'one none', 'one zero', 'two', 'two', 'two', 'first none', 'second none', 'first zero', 'second zero', 'tools', 'quiet']


def case(r):
    op = r.deal(OPS)
    log = log_region()
    one = op.startswith('one')
    coaster = bytearray(r.bytes(0x60))
    index = r.randrange(0, 8)
    put(coaster, 4, 'I', TYPE)
    put(coaster, 0x10, 'I', index)
    put(coaster, 0x54, 'I', MODEL)
    kind = r.getrandbits(32) & ~0x20 | (0x20 if one else 0)
    model = bytearray(r.bytes(0x110))
    other = r.randrange(0, 8)
    put(model, 0x70, 'I', other)
    put(model, 0x100, 'I', P1)
    p1 = bytearray(r.bytes(0x20))
    put(p1, 0x18, 'I', P2)
    p2 = bytearray(r.bytes(0x30))
    part = r.randrange(0, 4)
    put(p2, 0x2c, 'i', -1 if op == 'one none' else part)
    holder = bytearray(r.bytes(0x10))
    put(holder, 8, 'I', A)
    a = bytearray(r.bytes(0x30))
    put(a, 0x28, 'I', BB)
    b = bytearray(r.bytes(8))
    put(b, 4, 'I', C)
    second = r.randrange(0, 4)
    c = bytearray(r.bytes(0x14 * 4 + 8))
    for i in range(4):
        put(c, 0x14 * i + 4, 'I', OBJECTS + 0x40 * i)
    if op == 'one zero':
        put(c, 0x14 * part + 4, 'I', 0)
    elif op == 'first zero':
        put(c, 0x14 * part + 4, 'I', 0)
        second = (part + 1) % 4
    elif op == 'second zero':
        put(c, 0x14 * second + 4, 'I', 0)
        part = (second + 1) % 4
    finds = bytearray(r.bytes(0xa0))
    put(finds, 0, 'i', -1 if op == 'first none' else part)
    put(finds, 4, 'i', -1 if op == 'second none' else second)
    answers(log, FIND, ANSWERS)
    strings(log, POSITION, 0, 2)
    fill(log, POSITION, 1, r.bytes(12))
    if op not in ('one none', 'first none'):   # (there the position is what the stack held)
        blob(log, ENTRY, 5, 7)
    else:
        strings(log, ENTRY, 0, 1 << 5)
    set_return(log, ENTRY, LINK)
    table = bytearray(r.bytes(0x20))
    put(table, 4 * (other if one else index), 'I', HOLDER)   # (the holder at the index that one reads; the other index's dword is random)
    regions = {LOG: log, COASTER: coaster, TYPE: struct.pack('<I', kind), MODEL: model, P1: p1, P2: p2, HOLDER: holder, A: a, BB: b, C: c, OBJECTS: r.bytes(0x100), LINK: r.bytes(0x30), ANSWERS: finds,
               0x790fec: struct.pack('<I', COASTER), 0x7a4610: table, 0x785a2c: struct.pack('<I', r.choice([1, 0x100]) if op == 'tools' else r.choice([0, 0, 1]))}
    return Call(0x435570, [r.choice([1, 0x100]) if op == 'quiet' else r.choice([0, 0, 1])], regions, conv='c')


if __name__ == '__main__':
    B.main(case)
