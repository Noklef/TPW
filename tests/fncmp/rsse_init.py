# RSSE_Init (0x551600) and the script event name lookup (0x468600) of src/game/rsse/rsse_vm against the originals: the opcode name table (0x765280) pointing at strings
# of every length and sign (and empty strings ending it), the initialised flag set or clear, globals of random content; scripts with twelve event tables of up to three
# entries, keys that match (or not) and outputs given or null.
# usage: rsse_init.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Init,Event)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

CALLS = {'Init': 0x551600, 'Event': 0x468600}
B = Bench([(0x551600, 0x5516a3), (0x468600, 0x468708), (0x5da3c0, 0x5da3c1)])
STRINGS, SCRIPT, KEYS, TABLES, OUT = RAM + 0x100, RAM + 0x1000, RAM + 0x1400, RAM + 0x1500, RAM + 0x1800


def init_case(r):
    n = r.choice([0, 1, 2, 3, 6, 12])
    text = bytearray()
    table = bytearray(4 * (n + 2))
    for k in range(n):
        s = bytes(r.choice([1, 2, 7, 0x41, 0x7f, 0x80, 0xff, r.randrange(1, 256)]) for _ in range(r.choice([0, 1, 2, 5, 9, 20])))
        put(table, 4 * k, 'I', STRINGS + len(text))
        text += s + b'\0'
    put(table, 4 * n, 'I', RAM)             # a pointer to an empty string ends the table
    if n and r.random() < 0.15:
        put(table, 0, 'I', RAM)             # an empty first name: nothing is summed
    regions = {0x765280: table, STRINGS: bytes(text) + b'\0', RAM: bytes(8), 0x879190: r.bytes(0x30)}
    if r.random() < 0.3:
        put(regions[0x879190], 0x10, 'I', 0)  # 0x8791a0 clear
    return Call(CALLS['Init'], [], regions, conv='c', ret='v')


def event_case(r):
    script = bytearray(r.bytes(0xb0))
    keys = [r.choice([1, 2, 3, 0x1234, r.getrandbits(32)]) for _ in range(4)]
    put(script, 0xac, 'I', KEYS)
    tables = bytearray(0x400)
    top = 0
    for g in range(12):
        n = r.choice([0, 0, 1, 2, 3])
        put(script, 0x1c + 12 * g, 'I', n)
        put(script, 0x1c + 12 * g + 4, 'I', TABLES + top)
        for e in range(n):
            put(tables, top + 8 * e, 'II', r.getrandbits(32), r.choice(keys + [7]))
        top += 8 * n
    regions = {SCRIPT: script, KEYS: bytes().join(k.to_bytes(4, 'little') for k in keys), TABLES: tables, OUT: r.bytes(0x10)}
    outs = [OUT + 4 * k if r.random() < 0.8 else 0 for k in range(3)]
    return Call(CALLS['Event'], [SCRIPT, r.randrange(4)] + outs, regions, conv='c', ret='a')


def case(r):
    return init_case(r) if r.choice(OPS) == 'Init' else event_case(r)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
