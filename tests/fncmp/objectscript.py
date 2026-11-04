# An object's script loaded (src/game/world/objectscript: 0x4dcf90) against the original. Recorders: sprintf (its name filled), Quickload_Locate (the names logged) and Ride_SetRecordId. Three calls pass a
# word in a dword whose upper half is the stack's or a pointer's, so they are stand-ins as driver.cpp's object_script_*: the two definition lookups and RSSE_LoadScriptFile hash the low word into HASH, and
# Thing_GetHandle copies the object's +0x10.
# usage: objectscript.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, fill, log_region, strings  # noqa: E402

FORMAT, LOCATE, RECORD = 10, 11, 12
OBJECT, HASH, NAME, DEFINITION, TABLE = RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x4000, RAM + 0x5000


def mix(e, *values):
    h = e.u32(HASH)
    for value in values:
        h = (h * 33 ^ value) & 0xffffffff
    e.write(HASH, struct.pack('<I', h))
    return h


def pop(e, n):
    if e.tail:
        ret = e.u32(e.r['ESP'])
        e.r['ESP'] += 4 * n
        e.write(e.r['ESP'], struct.pack('<I', ret))
    else:
        e.r['ESP'] += 4 * n


def stack(e, i):
    return e.u32(e.r['ESP'] + (4 if e.tail else 0) + 4 * i)


def find(k, answer):
    def hook(e):  # thiscall (id, a word): as driver.cpp's object_script_find
        mix(e, k, e.r['ECX'], stack(e, 0) & 0xffff)
        pop(e, 1)
        e.r['EAX'] = answer
    return hook


def handle(e):  # thiscall (&out): as driver.cpp's object_script_handle
    out = stack(e, 0)
    e.write(out, e.read(e.r['ECX'] + 0x10, 2))
    pop(e, 1)
    e.r['EAX'] = out


def load(e):  # cdecl (name, handle, quiet): as driver.cpp's object_script_load
    e.r['EAX'] = mix(e, 3, stack(e, 0), stack(e, 1) & 0xffff, stack(e, 2))


B = Bench([(0x4dcf90, 0x4dd091), (0x5da3c0, 0x5da3c1)],
          hooks={0x412ef0: find(1, NAME), 0x412e90: find(2, DEFINITION), 0x67a650: _recorder(FORMAT, 5, cdecl=True), 0x44a280: _recorder(LOCATE, 3, cdecl=True), 0x50b350: handle, 0x5587f0: load, 0x4685c0: _recorder(RECORD, 2, cdecl=True)})


def text(r):
    return bytes(r.randrange(0x61, 0x7b) for _ in range(r.randrange(1, 12))) + b'\0'


def case(r):
    log = log_region()
    strings(log, FORMAT, 0b1100, 1)
    fill(log, FORMAT, 0, text(r))
    strings(log, LOCATE, 1, 0)
    answers(log, LOCATE, TABLE)
    definition = bytearray(0x2a0)
    definition[0x28c:0x28c + 12] = text(r).ljust(12, b'\0')
    regions = {LOG: log, OBJECT: r.bytes(0x30), HASH: r.bytes(4), NAME: text(r), DEFINITION: definition, TABLE: b''.join(struct.pack('<I', r.deal([0, r.getrandbits(32) | 1])) for _ in range(40)), 0x75b61c: text(r),
               0x785a2c: struct.pack('<I', r.deal([0, 1, 2, r.getrandbits(32)]))}
    return Call(0x4dcf90, [OBJECT], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
