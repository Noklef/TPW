# The message centre's slot destructors (src/game/world/messagecentre: 0x40f970 one slot, 0x409250 the 29 of them) against the originals: slots of a dword and a std::map of
# tree B (core/stdtree) whose trees are stdtree.py's valid red-black trees of 0 to 4 nodes, the map counter 0x785de0 from 1 up (the last map out frees the nil node 0x785de4).
# Add (0x40f9e0) inserts a thing's handle into one slot's map (slots 0-28, or past them); Remove (0x40fa40) erases a thing's handle from the 29 maps; Save (0x40fc80) writes the maps' sizes and keys to a record stream.
# Load (0x40fd60) empties the maps, then reads the counts and keys back from a record stream (whole, or cut anywhere), duplicates among them now and then.
# Ctor (0x405460) constructs one slot over random bytes (the nil node made already or not); Instance (0x415e00) makes the centre (29 slots) or finds it made; an allocation now and then fails.
# CtorSlots (0x4a8bd0) constructs the 29 slots over random bytes, as Instance does for its block.
# Post (0x40fb10) hands a message (its slot 0 a recorder answering the type: 0x1b often, else 0-28) to the things of its type's slot (the
# thing table 0x7cfb90 of things whose handles are their slots, or now and then another key) through Thing_HandleMessage 0x50b550, the
# thiscall recorder 27 of 1 argument; type 0x1b first erases the handle of the thing its word +8 names from every map.
# Cheats (0x4053c0 CheatSystem_SetOn): the low byte 0 now and then; the message (ThingMsg15 on the stack, its vtable 0x6fd6c8 a recorder
# one answering a type 0-28 but 0x1b: the message's word +8 has a stack byte the original leaves unset; the handler logs its bytes 0-8) posted as Post's, to the centre
# there or made (an allocation now and then failing).
# usage: messagecentre.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Slot,Slots,Remove,Save,Add,Load,Ctor,Instance,CtorSlots,Post,Cheats)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import stdtree  # noqa: E402
from fncmp import LOG, Bench, Call, HEAP, RAM, _recorder, blob_bytes, heap, log_region, record_stream, set_return, vtable  # noqa: E402

B = Bench([(0x409250, 0x409260), (0x40f970, 0x40f9e0), (0x40fa40, 0x40fb03), (0x40f950, 0x40f961), (0x410ac0, 0x410b11), (0x50b350, 0x50b35d), (0x40fc80, 0x40fd5b), (0x5f5e50, 0x5f5e80), (0x5da3c0, 0x5da3c1), (0x40ff80, 0x4104e0), (0x4104b0, 0x410610), (0x410670, 0x410720), (0x410720, 0x4107a0), (0x67acb0, 0x67acb1),
           (0x67afa0, 0x67b0d0), (0x40f9e0, 0x40fa40), (0x40fe60, 0x40ff80), (0x4107b0, 0x410ac0), (0x410b20, 0x410bf0), (0x5e7720, 0x5e7750), (0x40fd60, 0x40fe50), (0x5f5e20, 0x5f5e50),
           (0x405460, 0x405530), (0x415e00, 0x415e80), (0x67acf0, 0x67ada0), (0x4a8bd0, 0x4a8beb), (0x40fb10, 0x40fc7f), (0x410610, 0x410670), (0x4053c0, 0x40545d), (0x40f6d0, 0x40f6e9)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None, 0x50b550: _recorder(27, 1)})
SLOTS, HEADS, NIL, NODES, THING = RAM, RAM + 0x400, RAM + 0x800, RAM + 0x1000, RAM + 0x3000
CHUNK, STREAM, SINK = RAM + 0x3100, RAM + 0x3200, RAM + 0x3400
COUNTER = 0x785de0   # (the nil node's pointer follows at 0x785de4)
CENTRE = 0x788d3c   # the message centre (0x415e00 makes it)
THINGS, MSG, MVT, TABLE, KEYS = RAM + 0x5000, RAM + 0x6000, RAM + 0x6100, 0x7cfb90, 100   # (stdtree.build's keys are below 100)
SLOT_COUNT, MAX_NODES = 0x1d, 4


def slot(r, k, slots, heads, nodes_blob):
    """Slot k: a random dword, then the map (allocator bytes, head, multi flag, size) whose head (in HEADS) and nodes (in NODES, 0x20 apart, colour at +0x10) make a tree."""
    n = r.choice([0, 0, 1, 2, 3, 4])
    nodes, root = stdtree.build(r, n)
    keys = sorted(nodes)
    head = HEADS + 0x20 * k
    addr = {key: NODES + 0x20 * (MAX_NODES * k + i) for i, key in enumerate(keys)}

    def ptr(key):
        return NIL if key is None else addr[key]

    for key in keys:
        base = addr[key] - NODES
        struct.pack_into('<4I', nodes_blob, base, ptr(nodes[key][0]), head if nodes[key][1] is None else addr[nodes[key][1]], ptr(nodes[key][2]), key)
        struct.pack_into('<I', nodes_blob, base + 0x10, nodes[key][3])
    if n:
        struct.pack_into('<3I', heads, 0x20 * k, addr[keys[0]], addr[root], addr[keys[-1]])
    else:
        struct.pack_into('<3I', heads, 0x20 * k, head, NIL, head)
    struct.pack_into('<II', heads, 0x20 * k + 0xc, r.getrandbits(32), stdtree.RED)
    struct.pack_into('<5I', slots, 0x14 * k, r.getrandbits(32), r.getrandbits(32), head, r.getrandbits(32) & 0xff, n)
    return keys


def made(r, op):
    """Ctor (0x405460): one slot over random bytes; Instance (0x415e00): the message centre made (29 slots) or already there; CtorSlots (0x4a8bd0): 29 slots over random bytes. The nil node made already or not,
    the map counter anything; an allocation now and then fails (the first: no centre)."""
    nil = bytearray(0x20)
    struct.pack_into('<I', nil, 0x10, stdtree.BLACK)
    regions = {COUNTER: struct.pack('<II', r.choice([0, 1, 2, 0x7fffffff, 0xffffffff]), r.choice([0, NIL])), NIL: nil,
               HEAP: heap(0x1000, r.choice([0] * 12 + [1, 1, 2, 3]) if op == 'Instance' else r.choice([0] * 14 + [1]))}
    if op == 'Ctor':
        regions[SLOTS] = r.bytes(0x14)
        return Call(0x405460, [SLOTS], regions, conv='t', ret='a')
    if op == 'CtorSlots':
        regions[SLOTS] = r.bytes(0x14 * SLOT_COUNT)
        return Call(0x4a8bd0, [SLOTS], regions, conv='t', ret='a')
    regions[CENTRE] = struct.pack('<I', 0 if r.random() < 0.8 else r.getrandbits(32))
    return Call(0x415e00, [], regions, conv='c', ret='a')


def case(r):
    op = r.deal(OPS)
    if op in ('Ctor', 'Instance', 'CtorSlots'):
        return made(r, op)
    count = SLOT_COUNT if op in ('Slots', 'Remove', 'Save', 'Add', 'Load', 'Post', 'Cheats') else 1
    slots, heads, nodes_blob = bytearray(0x14 * count), bytearray(0x20 * count), bytearray(r.bytes(0x20 * MAX_NODES * count))
    keys = []
    for k in range(count):
        keys += slot(r, k, slots, heads, nodes_blob)
    nil = bytearray(0x20)
    struct.pack_into('<I', nil, 0x10, stdtree.BLACK)
    counter = r.choice([1, 2, count, count + 1, 100]) if op == 'Slots' else r.choice([1, 1, 2, 7])
    regions = {SLOTS: slots, HEADS: heads, NIL: nil, NODES: nodes_blob, COUNTER: struct.pack('<II', counter, NIL), HEAP: heap()}
    if op == 'Save':   # (to a record stream with room for all of it or not)
        regions[CHUNK] = struct.pack('<I', STREAM) + r.bytes(0xc)
        regions[STREAM] = record_stream(STREAM, SINK, r.choice([0, 3, 4, 0x20, 0x7a, 0x100, 0x1000]))
        regions[SINK] = r.bytes(0x400)
        return Call(0x40fc80, [SLOTS, CHUNK], regions, conv='t', ret='a')
    if op == 'Load':   # (a slot count, then per slot a count and its keys; some counts odd, some keys repeated, the stream whole or cut short)
        for k in range(SLOT_COUNT):   # (the maps' multi flag 0, as the game's are: a repeated key ends the load instead of filling the heap)
            struct.pack_into('<I', slots, 0x14 * k + 0xc, 0)
        data = bytearray(struct.pack('<I', r.choice([SLOT_COUNT, r.getrandbits(32)])))
        pool = [r.getrandbits(16) for _ in range(6)]
        for k in range(SLOT_COUNT):
            n = r.choice([0, 0, 0, 1, 1, 2, 3, 4]) if r.random() < 0.95 else r.choice([-1, 0x7fffffff, 0x80000000, 5])
            data += struct.pack('<i', n if n < 0x80000000 else n - 0x100000000)
            for _ in range(min(max(n, 0), 6)):
                data += struct.pack('<H', r.choice(pool) if r.random() < 0.08 else r.getrandbits(16))
        left = len(data) if r.random() < 0.6 else r.randrange(len(data) + 1)
        regions[CHUNK] = struct.pack('<I', STREAM) + r.bytes(0xc)
        regions[STREAM] = record_stream(STREAM, SINK, left)
        regions[SINK] = bytes(data)
        return Call(0x40fd60, [SLOTS, CHUNK], regions, conv='t', ret='a')
    if op == 'Add':   # (a thing whose handle is a key already or not; slot 0-28 mostly, else 29 and up or negative)
        handle = r.choice(keys) if keys and r.random() < 0.4 else r.getrandbits(16)
        regions[THING] = struct.pack('<HH', handle, r.getrandbits(16)) + r.bytes(8)
        index = r.randrange(SLOT_COUNT) if r.random() < 0.85 else r.choice([0x1d, 0x1e, 0x7fffffff, 0xffffffff, 0x80000000])
        return Call(0x40f9e0, [SLOTS, THING, index], regions, conv='t', ret='b')
    if op == 'Cheats':
        table, things = bytearray(r.bytes(0x14 * KEYS)), bytearray(r.bytes(0x10 * KEYS))
        for i in range(KEYS):
            struct.pack_into('<I', table, 0x14 * i, THINGS + 0x10 * i)
            struct.pack_into('<H', things, 0x10 * i, i if r.random() < 0.9 else r.randrange(KEYS))
        log = log_region()
        set_return(log, 0, r.choice([k for k in range(SLOT_COUNT) if k != 0x1b]))
        blob_bytes(log, 27, 0, 0x1ff)   # (the message on the stack: bytes 0-8 hashed, not its address; slot 0 logs no this)
        regions.update({TABLE: table, THINGS: things, LOG: log, 0x6fd6c8: vtable({}, 4, com={0: 0}), THING: r.bytes(0x10),
                        CENTRE: struct.pack('<I', SLOTS if r.random() < 0.8 else 0)})
        regions[HEAP] = heap(0x1000, r.choice([0] * 8 + [1]))
        return Call(0x4053c0, [THING, r.choice([1, 1, 2, 0xff, 0, 0x100, r.getrandbits(32)])], regions, conv='t', ret='v')
    if op == 'Post':
        table, things = bytearray(r.bytes(0x14 * KEYS)), bytearray(r.bytes(0x10 * KEYS))
        for i in range(KEYS):
            struct.pack_into('<I', table, 0x14 * i, THINGS + 0x10 * i)
            struct.pack_into('<H', things, 0x10 * i, i if r.random() < 0.9 else r.randrange(KEYS))
        log = log_region()
        typ = r.choice([0x1b] * 8 + list(range(SLOT_COUNT)))
        set_return(log, 0, typ)
        msg = bytearray(r.bytes(0x20))
        struct.pack_into('<IIH', msg, 0, MVT, r.getrandbits(32), r.choice(keys) if keys and r.random() < 0.8 else r.randrange(KEYS))
        regions.update({TABLE: table, THINGS: things, LOG: log, MSG: msg, MVT: vtable({})})
        return Call(0x40fb10, [SLOTS, MSG], regions, conv='t', ret='a')
    if op == 'Remove':   # (a thing whose u16 handle is one of the keys, mostly; the dword above it random)
        handle = r.choice(keys) if keys and r.random() < 0.8 else r.getrandbits(16)
        regions[THING] = struct.pack('<HH', handle, r.getrandbits(16)) + r.bytes(8)
        return Call(0x40fa40, [SLOTS, THING], regions, conv='t', ret='a')
    return Call(0x409250 if op == 'Slots' else 0x40f970, [SLOTS], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', 'Slot,Slots,Remove,Save,Add,Load,Ctor,Instance,CtorSlots,Post,Cheats').split(',') if k]

if __name__ == '__main__':
    B.main(case)
