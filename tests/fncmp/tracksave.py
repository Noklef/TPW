# The track rides' save (src/game/world/trackrides: TrackRides_Save 0x5428e0) against the original: the module's blocks (rides, cars, stops, the lists) built with any contents, written
# to a memory stream whose bytes are compared; the module not loaded, no rides, a chunk's allocation failing (the first two end early, a later one faults in both).
# usage: tracksave.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, RAM, Bench, Call, global_heap, heap, memory_stream, put  # noqa: E402

B = Bench([(0x5428e0, 0x5433f6), (0x543400, 0x543415), (0x543420, 0x54348b), (0x543490, 0x5434d0), (0x5434d0, 0x543552), (0x5f5bb0, 0x5f5bb9), (0x5f5bf0, 0x5f5c04), (0x5f5e50, 0x5f5e7d), (0x5da3c0, 0x5da3c1)])
STREAM, OUT, RIDES, CARS, STOPS, NODES = RAM, RAM + 0x100, RAM + 0x4000, RAM + 0x5000, RAM + 0x6000, RAM + 0x6400
GLOBALS = 0x877b58
SPACE = 0x3000


def chain(r, nodes, size, link):
    """A list of 0-3 nodes of random bytes (size each, the next at link): its head."""
    head = 0
    for _ in range(r.choice([0, 0, 1, 1, 2, 3])):
        at = NODES + len(nodes)
        node = bytearray(r.bytes(size))
        put(node, link, 'I', head)
        nodes += node
        head = at
    return head


def case(r):
    nodes = bytearray()
    used = [r.random() < 0.7 for _ in range(4)]
    used[r.randrange(4)] = True
    rides = bytearray()
    for i in range(4):
        ride = bytearray(r.bytes(0xd0))
        put(ride, 0, 'I', r.choice([0xfffffff3, 0xffffffff, r.getrandbits(32) | 1]) if used[i] else 0)
        put(ride, 0xbc, 'I', chain(r, nodes, 0x28, 0x24))
        put(ride, 0xc8, 'I', chain(r, nodes, 0x14, 0x10))
        put(ride, 0xc4, 'I', chain(r, nodes, 0x14, 0x10))
        rides += ride
    carcount = r.choice([0, 1, 2, 3, 5])
    links = len(nodes)
    nodes += r.bytes(0x10)
    stops = bytearray(r.bytes(0x1c * 5))
    for i in range(1, 5):   # (the entry at -1 only gives its +8 and +0xc)
        put(stops, 0x1c * i + 0x10, 'I', RIDES + 0xd0 * r.randrange(4))
    cars = bytearray()
    placed = 0
    for i in range(carcount + 2):
        car = bytearray(r.bytes(0xac))
        on = placed < carcount and (r.random() < 0.8 or carcount + 2 - i <= carcount - placed)
        put(car, 0, 'I', r.getrandbits(32) & ~1 | int(on))
        placed += on
        put(car, 0x9c, 'I', r.choice([RIDES + 0xd0 * r.randrange(4), RIDES + 0xd0 * r.randrange(4), r.getrandbits(32)]))
        put(car, 0x98, 'I', NODES + links)
        put(car, 0x7c, 'i', r.choice([-1, -1, 0, 1, 2, 3]))
        put(car, 0x30, 'I', chain(r, nodes, 0x14, 0x10))
        cars += car
    block = bytearray(r.bytes(0x40))
    put(block, 0, 'I', 0x4a454647 if r.random() < 0.95 else r.choice([0x4a454646, r.getrandbits(32)]))
    put(block, 8, 'I', RIDES if r.random() < 0.97 else 0)
    put(block, 0xc, 'I', 0 if r.random() < 0.03 else max(1, sum(used) - r.choice([0, 0, 0, 1])))
    put(block, 0x10, 'I', CARS)
    put(block, 0x14, 'I', carcount)
    put(block, 0x20, 'I', STOPS + 0x1c)
    regions = {STREAM: memory_stream(STREAM, OUT, SPACE), OUT: bytes(SPACE), RIDES: rides, CARS: cars, STOPS: stops, NODES: nodes, GLOBALS: block,
               HEAP: heap(size=0x2000, fail=r.choice([0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3])), **global_heap()}
    return Call(0x5428e0, [STREAM], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
