# A coaster's car placed while editing (src/game/coaster/placecar: CoasterEdit_PlaceCar, 0x435020) against the original: links with nodes on the square or not (some masked by byte +4), the current car on the node
# or not, a height given or not, the kind 0xff or not, the count [0x79c664] around 0-2 under [0x79c658] bit 0x40, [0x785a2c] set or not, the outs null or not. All its calls are recorders (driver.cpp's kCoasterPlaceCarEnv).
# usage: coasterplacecar.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob_array, fb, fill, log_region, put, set_float_return, set_return, strings  # noqa: E402

ENTRY, CAR, LENGTH, BRACE, RATES, COST, RESUME, TOOLS, AFTER, SWITCH, REFRESH, BOUNDS, MONEY, SOURCE, DEBIT = range(1, 16)
HOOKS = {0x432c10: _recorder(ENTRY, 6, cdecl=True), 0x434060: _recorder(CAR, 9, cdecl=True), 0x435340: _recorder(LENGTH, 7, cdecl=True), 0x433300: _recorder(BRACE, 5, cdecl=True),
         0x430ca0: _recorder(RATES, 2, cdecl=True), 0x447830: _recorder(COST, 7, cdecl=True), 0x442c70: _recorder(RESUME, 1, cdecl=True), 0x442880: _recorder(TOOLS, 1, cdecl=True),
         0x498780: _recorder(AFTER, 0, cdecl=True), 0x444770: _recorder(SWITCH, 0, cdecl=True), 0x444740: _recorder(REFRESH, 0, cdecl=True), 0x434f10: _recorder(BOUNDS, 0, cdecl=True),
         0x5195d0: _recorder(MONEY, 0), 0x6ad810: _recorder(SOURCE, 0), 0x4d01f0: _recorder(DEBIT, 1)}
B = Bench([(0x435020, 0x435333), (0x5da3c0, 0x5da3c1)], hooks=HOOKS)
COASTER, WORLD, W2, LINKS, NODES, NEW, CARS, OUTS, THECAR = RAM, RAM + 0x100, RAM + 0x180, RAM + 0x200, RAM + 0x400, RAM + 0x800, RAM + 0x900, RAM + 0x980, RAM + 0x9c0
MONEYOBJ = 0x12340
FLAGS, COUNT, NOPAY = 0x79c658, 0x79c664, 0x785a2c


def case(r):
    log = log_region()
    x, y = r.randrange(0, 0x80), r.randrange(0, 0x80)
    width = r.choice([0x40, 0x80, 0x100])
    square = width * y + x
    current = r.getrandbits(32)
    coaster = bytearray(r.bytes(0x80))
    put(coaster, 4, 'I', r.getrandbits(31))
    put(coaster, 0x44, 'I', 5)
    put(coaster, 0x64, 'I', current)
    cars = bytearray(r.bytes(0x20))
    put(cars, 0, 'I', r.choice([4, 4, 3, r.getrandbits(32)]))
    # Links of 0-2 nodes, the nodes at 0x60 each.
    links, nodes = bytearray(r.bytes(0xc0)), bytearray(r.bytes(0x60 * 4))
    n_links, used = r.choice([0, 1, 2, 2]), 0
    for li in range(n_links):
        put(links, 0x60 * li + 0x18, 'I', LINKS + 0x60 * (li + 1) if li + 1 < n_links else 0)
        n_nodes = r.choice([0, 1, 2])
        put(links, 0x60 * li + 0x4c, 'I', NODES + 0x60 * used if n_nodes else 0)
        for ni in range(n_nodes):
            base = 0x60 * used
            put(nodes, base + 4, 'B', r.choice([0, 0, 1, 4, 5, 2, 0x10]))
            put(nodes, base + 8, 'I', square if r.random() < 0.6 else r.getrandbits(32))
            put(nodes, base + 0x28, 'I', NODES + base + 0x60 if ni + 1 < n_nodes else 0)
            count = r.choice([1, 1, 2, 3])
            put(nodes, base + 0x30, 'I', count)
            for k in range(count):
                put(nodes, base + 0x34 + 4 * k, 'I', CARS)
            if r.random() < 0.4:
                put(nodes, base + 0x34 + 4 * r.randrange(count), 'I', current)
            used += 1
    regions = {COASTER: coaster, WORLD: r.bytes(0x6c) + struct.pack('<I', W2), W2: r.bytes(0x18) + struct.pack('<I', width), LINKS: links, NODES: nodes, NEW: r.bytes(0x60), CARS: cars,
               OUTS: r.bytes(8), THECAR: r.bytes(0x10), LOG: log, 0x790fec: struct.pack('<I', COASTER), 0x790fe0: struct.pack('<I', LINKS if n_links else 0), 0x7a0854: struct.pack('<I', WORLD),
               0x7cf83c: struct.pack('<I', 0x5550), FLAGS: struct.pack('<I', r.getrandbits(32) | 0x40 if r.random() < 0.7 else r.getrandbits(32) & ~0x40),
               COUNT: struct.pack('<I', r.choice([0, 1, 2, 2, 3])), NOPAY: struct.pack('<I', r.choice([0, 0, 1])), 0x6fdea4: struct.pack('<I', fb(1.0))}
    set_return(log, ENTRY, NEW)
    set_return(log, CAR, THECAR)
    set_return(log, MONEY, MONEYOBJ)
    set_float_return(log, LENGTH, 1.5)
    set_float_return(log, COST, r.choice([r.uniform(-1000, 100000), 12.5, 0.0, float('nan')]))
    strings(log, LENGTH, 0, 1 << 6)
    fill(log, LENGTH, 6, struct.pack('<I', fb(r.uniform(0, 30))), which=2)
    strings(log, BRACE, 0, 1 << 2 | 1 << 3)
    fill(log, BRACE, 2, struct.pack('<I', r.randrange(0, 0x100)), which=0)
    fill(log, BRACE, 3, struct.pack('<I', fb(r.uniform(0, 30))), which=1)
    strings(log, COST, 0, 1 << 3 | 1 << 4)
    blob_array(log, COST, 3, 1, 4, 0)
    blob_array(log, COST, 4, 1, 4, 1)
    height = r.choice([0, 0, 0, fb(r.uniform(0, 30))])
    kind = r.choice([0xff, 0x1ff, r.getrandbits(32)])
    return Call(0x435020, [x, y, height, r.choice([0, OUTS]), r.choice([0, OUTS + 4]), kind, r.getrandbits(32)], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
