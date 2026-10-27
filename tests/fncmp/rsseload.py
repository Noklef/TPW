# A ride script's file loaded (src/game/rsse/scriptfile: 0x5587f0 RSSE_LoadScriptFile) against the original. The RSSE system on (mostly);
# the file through vfs.py's mounts and memory source (md2file.py's way), its name under the mounts with or without backslashes; its bytes
# the magic ("RSSE" mostly), a version, the six counts with their arrays (0-4 each, now and then 0x40000000: a size of 0, no array), four
# dwords, the code's count and dwords, the data's count and bytes, the whole file or cut short (the reads past it leave the record's zeroes).
# The script list's head none or a record, the id counter and count any. The thing's handle 0 or one of gThingSlots' things (as
# rideeffect.py's: their definitions' effect +0x60 0 or one of particles.py's), whose ride (+0x20) is 0 or ridebounds.py's ride (with or
# without a box; its instance's model with 0-4 parts keyed 1, 2, ... mostly with the flag 0x80) ; quiet or not. The particles as
# particles.py's Spawn cases; the sound on and started or not, its manager's object's channel (+0x24) none or one taking occluders (flags
# +0x28 0x41, an EAX buffer +0x20) into the empty occluders' list 0xfb2038, the occlusion switched off (0xfb2050).
# usage: rsseload.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, STUBS, Bench, Call, heap, import_table, log_region, put, recorder, set_return  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import particles  # noqa: E402
import ridebounds  # noqa: E402
import soundset  # noqa: E402
import vfs  # noqa: E402
from gamestring import LOCALE  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops
particles.NODES, particles.SINES = RAM + 0x60000, RAM + 0x61000   # (as trackcar_remove.py's)


def merged(ranges):   # (overlapping ranges joined: a listing line must not come twice)
    out = []
    for lo, hi in sorted(ranges):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


B = Bench(merged(vfs.RANGES + particles.RANGES + soundset.RANGES + [
    (0x5587f0, 0x559059), (0x5c4d50, 0x5c5020), (0x5f5660, 0x5f56a0), (0x67a400, 0x690000), (0x466b70, 0x466c7f), (0x44b220, 0x44b267),
    (0x412e90, 0x412eec)]))
BOX = RAM + 0x30000   # (ridebounds.py's world: the ride RAM + 0x30000, its holder +0x200, model +0x220, root +0x300)
PARTS, PREV, DEFS, THINGS, CHANNEL, SMGR, OBJ = (RAM + o for o in (0x30600, 0x31000, 0x32000, 0x33000, 0x34000, 0x34100, 0x34200))


def script(r):
    data = bytearray(b'RSSE' if r.random() < 0.95 else r.bytes(4))
    data += struct.pack('<I', r.getrandbits(32))

    def count():
        return r.choice([0, 0, 1, 2, 3, 4]) if r.random() < 0.97 else 0x40000000

    for _ in range(6):   # +0x8c, +0x54, +0x94 (a plain dword), +0x58, +0x64, +0x7c
        data += struct.pack('<I', count())
    data += r.bytes(16)
    n = r.choice([0, 1, 2, 5, 8])
    data += struct.pack('<I', n) + r.bytes(4 * n)
    m = r.choice([0, 1, 3, 10])
    data += struct.pack('<I', m) + r.bytes(m)
    return bytes(data) if r.random() < 0.8 else bytes(data[:r.randrange(len(data) + 1)])


def place(regions, address, data):   # (into a region already there when it holds the address)
    for at, blob in regions.items():
        if at <= address < at + len(blob):
            b = bytearray(blob)
            b[address - at:address - at + len(data)] = data
            regions[at] = b
            return
    regions[address] = data


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x10000), LOCALE: bytes(4)}
    root = vfs.mounts(r, log, regions)
    data = script(r)
    vfs.memory_source(r, log, regions, data)
    regions[vfs.MEMDATA] = data + bytes(0x100)   # (zeroes past a cut file: no counts)
    regions[vfs.NAME] = (vfs.name_under(r, root) + r.choice([b'\\ride.rse', b'ride.rse', b'\\a\\b.rse', b'']))[:0xe0] + b'\0' + r.bytes(7)
    # the import table: vfs.py's InterlockedDecrement (k 11), global_heap()'s GlobalAlloc / GlobalFree
    table = import_table(log, {})
    put(table, 0x12c, 'I', recorder(11, 1, True))
    put(table, 0x1cc, 'II', STUBS + 0x20, 0)
    put(table, 0x26c, 'I', STUBS + 0x30)
    regions.pop(0x6fd12c, None)
    regions[IMPORT_TABLE] = table
    for address, data in particles.spawn(r).regions.items():
        if address not in (LOG, HEAP):
            place(regions, address, data)
    c = ridebounds.box(r, BOX)
    ride = c.args[0]
    ram = bytearray(c.regions[BOX])
    instances = bytearray(c.regions[0x7a4610])
    if r.random() < 0.9:
        put(instances, 4 * ride, 'I', BOX)
    n = r.randrange(5)
    put(ram, 0x220 + 0x48, 'H', n)
    put(ram, 0x220 + 0x7c, 'I', PARTS)
    parts = bytearray(r.bytes(0x14 * 5))
    for i in range(n):
        put(parts, 0x14 * i, 'I', r.getrandbits(32) & ~0x80 | (0x80 if r.random() < 0.85 else 0))
        put(parts, 0x14 * i + 4, 'I', i + 1 if r.random() < 0.85 else r.randrange(8))
    regions.update({BOX: ram, 0x7a4610: instances, PARTS: parts})
    # gThingSlots: slots 1-7 things of types 0x10..0x16, their rides the box's (or 0); gObjectDefs as rideeffect.py's, the effect at +0x60
    slots, things = bytearray(r.bytes(0x14 * 8)), bytearray(r.bytes(0x24 * 8))
    for k in range(8):
        put(slots, 0x14 * k, 'I', THINGS + 0x24 * k)
        put(things, 0x24 * k + 0xe, 'H', 0x10 + k if r.random() < 0.97 else 0x99)
        put(things, 0x24 * k + 0x20, 'I', r.choice([ride, ride, ride, 0]))
    defs, dtable = bytearray(r.bytes(0x80 * 8)), bytearray(r.bytes(0x738))
    for i, k in enumerate(r.sample(range(8), 8)):
        put(dtable, 4 * i, 'I', DEFS + 0x80 * k)
        put(defs, 0x80 * k, 'H', 0x10 + k)
        put(defs, 0x80 * k + 0x60, 'I', r.choice([0, 1, 0x68, r.randrange(0x69), r.randrange(0x69)]))
    put(dtable, 0x528, 'i', 8)
    put(dtable, 0x730, 'HHI', 0x30, 0, 0)
    regions.update({0x7cfb90: slots, THINGS: things, 0x7890a0: dtable, DEFS: defs})
    # the sound: on and started (or not), the manager's object's channel taking occluders (or none) into the empty list
    g = bytearray(r.bytes(0x20))
    put(g, 8, 'II', r.choice([1, 1, 1, 0]), SMGR)
    put(g, 0x14, 'I', r.choice([1, 1, 1, 0]))
    obj = bytearray(r.bytes(0x40))
    put(obj, 0x24, 'I', CHANNEL if r.random() < 0.85 else 0)
    chan = bytearray(r.bytes(0x80))
    put(chan, 0x28, 'I', r.getrandbits(32) | 0x41)
    put(chan, 0x20, 'I', r.choice([1, RAM + 0x34400]))
    occ = bytearray(r.bytes(0x24))
    put(occ, 4, 'III', 0, 0, 0)
    put(occ, 0x18, 'I', 0)
    regions.update({0x802bc0: g, SMGR: struct.pack('<II', r.getrandbits(32), OBJ) + r.bytes(8), OBJ: obj, CHANNEL: chan,
                    RAM + 0x34400: r.bytes(0x40), 0xfb2038: occ})
    # the RSSE globals: the checksum 0x879190, the flag 0x8791a0, the id counter 0x8791a8, the count 0x8791ac, the head 0x8791b0
    rg = bytearray(r.bytes(0x24))
    put(rg, 0x10, 'I', r.choice([1, 1, 1, 0]))
    put(rg, 0x20, 'I', r.choice([0, PREV]))
    regions.update({0x879190: rg, PREV: r.bytes(0xf4)})
    handle = r.choice([0, 0, 1, 2, 3, 5, 7, 0x10005, 0x10000])   # (only the low word counts)
    quiet = r.choice([0, 0, 0, 1, r.getrandbits(32)])
    return Call(0x5587f0, [vfs.NAME, handle, quiet], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
