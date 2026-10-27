# The thing of kind 0xe and the person base constructors (src/game/world/things, person: 0x502d20, 0x518e20, 0x518e00) against the originals: things_methods.py's thing table and cells for the thing
# constructor (a thing of kind 0xe, with room for its record array), a plain object of random bytes for the person constructors.
# The thing models' constructors that add the thing to the message centre (headquarters: 0x4da920 the mechanic HQ, 0x5089c0 the staff HQ; 0x509b40 the tag system; 0x50c9b0 the UI message receiver; 0x599e70 the advisor): the thing table as for
# kind 0xe, and the message centre (0x788d3c) made already, its 29 slots messagecentre.py's maps of 0 to 4 keys at CENTRE, or not made yet (Instance makes it).
# The moving person's constructor (person: 0x4f8940): the thing table and cells as for kind 0xe with the cell one of those, the centre as for the models.
# usage: thing_ctors.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Thing,Class,Parts,MechanicHQ,StaffHQ,TagSystem,UIMsgReceiver,Advisor,Sprite)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import messagecentre  # noqa: E402
import stdtree  # noqa: E402
import things_methods  # noqa: E402
from fncmp import Bench, Call, RAM  # noqa: E402

CENTRE_RANGES = [(0x409250, 0x409260), (0x40f970, 0x40f9e0), (0x40f950, 0x40f961), (0x410ac0, 0x410b11), (0x50b350, 0x50b35d), (0x67acb0, 0x67acb1),
                 (0x67afa0, 0x67b0d0), (0x40f9e0, 0x40fa40), (0x40fe60, 0x40ff80), (0x4107b0, 0x410ac0), (0x410b20, 0x410bf0), (0x5e7720, 0x5e7750),
                 (0x405460, 0x405530), (0x415e00, 0x415e80), (0x67acf0, 0x67ada0)]
CENTRE_HOOKS = {0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None}
B = Bench([(0x4d91f0, 0x4d9290), (0x502d20, 0x502d70), (0x50afe0, 0x50b090), (0x50ba80, 0x50bac0), (0x50f1f0, 0x50f200), (0x50ffe0, 0x5100f0), (0x516270, 0x516320),
           (0x518e00, 0x518e20), (0x518e20, 0x518e70), (0x5d1750, 0x5d1751), (0x67acf0, 0x67ada0), (0x67b050, 0x67b0d0), (0x5da3c0, 0x5da3c1),
           (0x4da920, 0x4da970), (0x5089c0, 0x508a21), (0x50c620, 0x50c650), (0x509b40, 0x509bb6), (0x50c9b0, 0x50ca44), (0x599e70, 0x599f55), (0x4f8940, 0x4f8ad0), (0x4d3ed0, 0x4d3ee0), (0x5100f0, 0x510100), (0x510160, 0x510180),
           (0x516330, 0x516370)] + CENTRE_RANGES, hooks=CENTRE_HOOKS)
OBJ = RAM + 0x8000
SPRITE_CELLS = RAM + 0x80000   # (Sprite's map cells, 0x100 of them)
CENTRE = RAM + 0x20000   # the message centre's slots, then the maps' heads, nil node and nodes
# name: (address, object size)
MODELS = {'MechanicHQ': (0x4da920, 0x10), 'StaffHQ': (0x5089c0, 0x68), 'TagSystem': (0x509b40, 0x40), 'UIMsgReceiver': (0x50c9b0, 0x40), 'Advisor': (0x599e70, 0x16e8)}


def centre(r, regions, empty=False):
    """The message centre: made (29 slots at CENTRE, as messagecentre.slot builds them, or with empty maps) mostly, else not made yet (the heap gives
    it its block; never when empty)."""
    nil = bytearray(0x20)
    struct.pack_into('<I', nil, 0x10, stdtree.BLACK)
    if empty:   # (each map's head its own first and last, the nil node its root; nothing in it, so each add allocates one node)
        slots, heads = bytearray(0x14 * messagecentre.SLOT_COUNT), bytearray(0x20 * messagecentre.SLOT_COUNT)
        for k in range(messagecentre.SLOT_COUNT):
            head = CENTRE + 0x400 + 0x20 * k
            struct.pack_into('<5I', heads, 0x20 * k, head, CENTRE + 0x800, head, r.getrandbits(32), stdtree.RED)
            struct.pack_into('<5I', slots, 0x14 * k, r.getrandbits(32), r.getrandbits(32), head, r.getrandbits(32) & 0xff, 0)
        regions.update({CENTRE: slots, CENTRE + 0x400: heads, CENTRE + 0x800: nil, messagecentre.CENTRE: struct.pack('<I', CENTRE),
                        messagecentre.COUNTER: struct.pack('<II', r.choice([1, 2, 7]), CENTRE + 0x800)})
        return
    if r.random() < 0.2:
        regions[messagecentre.CENTRE] = bytes(4)
        regions[messagecentre.COUNTER] = struct.pack('<II', r.choice([0, 1, 2]), r.choice([0, CENTRE + 0x800]))
        regions[CENTRE + 0x800] = nil
        return
    mc = messagecentre
    saved = mc.SLOTS, mc.HEADS, mc.NIL, mc.NODES
    mc.SLOTS, mc.HEADS, mc.NIL, mc.NODES = CENTRE, CENTRE + 0x400, CENTRE + 0x800, CENTRE + 0x1000
    try:
        slots, heads, nodes = bytearray(0x14 * mc.SLOT_COUNT), bytearray(0x20 * mc.SLOT_COUNT), bytearray(r.bytes(0x20 * mc.MAX_NODES * mc.SLOT_COUNT))
        for k in range(mc.SLOT_COUNT):
            mc.slot(r, k, slots, heads, nodes)
    finally:
        mc.SLOTS, mc.HEADS, mc.NIL, mc.NODES = saved
    regions.update({CENTRE: slots, CENTRE + 0x400: heads, CENTRE + 0x800: nil, CENTRE + 0x1000: nodes,
                    messagecentre.CENTRE: struct.pack('<I', CENTRE), messagecentre.COUNTER: struct.pack('<II', r.choice([1, 2, 7]), CENTRE + 0x800)})


def thing_regions(r, cells=None):
    """things_methods.py's regions for the thing constructor, without its thing (its map cells moved to cells if given)."""
    regions = dict(things_methods.construct(r).regions)
    regions.pop(things_methods.NEWTHING, None)
    if cells:
        regions[cells] = regions.pop(things_methods.NEWCELLS)
        regions[0x8023a0] = struct.pack('<I', cells)
    return regions


def case(r):
    name = r.deal(OPS)
    if name == 'Thing':
        regions = thing_regions(r)
        regions[OBJ] = r.bytes(0x1500)
        return Call(0x502d20, [OBJ], regions, conv='t', ret='a')
    if name == 'Class':
        return Call(0x518e20, [OBJ, r.getrandbits(32)], {OBJ: r.bytes(0x200)}, conv='t', ret='a')
    if name == 'Parts':
        return Call(0x518e00, [OBJ, r.getrandbits(32)], {OBJ: r.bytes(0x200)}, conv='t', ret='a')
    if name == 'Sprite':
        regions = thing_regions(r, SPRITE_CELLS)
        more = bytearray(r.bytes(0x44 * 0xbf))   # (cells 0x42-0x100 too: the nav's column takes 7 bits of the cell's)
        for k in range(0xbf):
            more[0x44 * k + 0x24:0x44 * k + 0x26] = bytes(2)
        regions[SPRITE_CELLS + 0x44 * 0x41] = more
        regions[OBJ] = r.bytes(0x200)
        regions[RAM + 0x1da708] = r.bytes(4)   # (the world's random state)
        regions[0x75c7f0] = r.bytes(0x14)       # (the words it picks from)
        while regions.get(0x788d3c, b'') in (b'', bytes(4)):   # (the centre made already: its inlined making copies uninitialised stack bytes into
            centre(r, regions)                                # each slot (sic), here the caller's, so the two differ; staffinit.py's way)
        cell = r.choice([1, 2, 3, 0x40, 0x41, 0x20, r.randrange(1, 0x41), 0x80, 0x81, 0xc0, 0x100, r.randrange(0x42, 0x101)]) | r.choice([0, 0, 0x10000, r.getrandbits(16) << 16])
        return Call(0x4f8940, [OBJ, cell, r.getrandbits(32)], regions, conv='t', ret='a')
    address, size = MODELS[name]
    regions = thing_regions(r)
    regions[OBJ] = r.bytes(size)
    centre(r, regions)
    return Call(address, [OBJ], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', 'Thing,Class,Parts,Sprite,' + ','.join(MODELS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
