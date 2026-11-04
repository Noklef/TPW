# The front-end frame's start (src/game/frontend/frame: 0x5d5970) against the original: its window, the singletons made and the first panels shown. All the callees are recorders; the allocator real.
# usage: framestart.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, global_heap, heap, log_region, put, set_return, vtable  # noqa: E402

BUILD, FOCUS, FIND, HANDLER, SLOTS, SEND, LABEL, CLOCK, SOUND, CAMERA, A60, A90, B08, D78, ONLINE, DIALOG = 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23
B = Bench([(0x5d5970, 0x5d5c30)] + gamestring.B_RANGES,
          hooks={0x65fd0e: _recorder(BUILD, 2), 0x65e59b: _recorder(FOCUS, 1), 0x65d7d0: _recorder(FIND, 1), 0x65e526: _recorder(HANDLER, 1), 0x5c7420: _recorder(SLOTS, 0),
                 0x658d9f: _recorder(SEND, 4, cdecl=True), 0x48b020: _recorder(LABEL, 1, cdecl=True), 0x402d70: _recorder(CLOCK, 0), 0x51e890: _recorder(SOUND, 0, cdecl=True),
                 0x5e2cc0: _recorder(CAMERA, 0), 0x5d84a0: _recorder(A60, 0), 0x5d8b00: _recorder(A90, 0), 0x5da200: _recorder(B08, 0), 0x5e3c20: _recorder(D78, 0),
                 0x5b50b0: _recorder(ONLINE, 0), 0x4a6580: _recorder(DIALOG, 0, cdecl=True)})
FRAME, OWNER, NODES, OBJECTS, USERSLOTS, SESSION, PANELS, PANEL_OBJECTS, VT = RAM, RAM + 0x80, RAM + 0x100, RAM + 0x180, RAM + 0x200, RAM + 0x400, RAM + 0x600, RAM + 0x640, RAM + 0x700


def case(r):
    log = log_region()
    table = bytearray(0x600)
    for at, stub in global_heap().items():
        table[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(0x1000)}
    n = r.choice([0, 1, 2, 3])
    nodes = bytearray(r.bytes(0x40))
    for i in range(n):
        put(nodes, 8 * i, '2I', OBJECTS + 4 * i, NODES + 8 * (i + 1) if i + 1 < n else 0)
    frame = bytearray(r.bytes(0x40))
    put(frame, 0x18, 'I', OWNER)
    regions[FRAME] = frame
    regions[OWNER] = struct.pack('<II', r.getrandbits(32), NODES if n else 0)
    regions[NODES] = nodes
    regions[OBJECTS] = struct.pack('<3I', VT, VT, VT)
    regions[VT] = vtable({0: 0, 1: 0, 2: 0, 3: 0, 4: 0, 5: 0, 6: 0, 7: 0})
    regions[PANELS] = struct.pack('<8I', *[PANEL_OBJECTS + 4 * i for i in range(8)])
    regions[PANEL_OBJECTS] = struct.pack('<8I', *[VT] * 8)
    set_return(log, BUILD, 0x1234560)
    set_return(log, FIND, 0x1234570)
    set_return(log, CLOCK, r.getrandbits(32))
    set_return(log, A60, r.getrandbits(32))
    set_return(log, A90, r.getrandbits(32))
    set_return(log, B08, r.getrandbits(32))
    set_return(log, D78, PANELS)
    userslots = bytearray(r.bytes(0x110))
    put(userslots, 0xc4, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    put(userslots, 0x60, 'i', r.choice([-1, -1, 0, 1]))
    regions[USERSLOTS] = userslots
    set_return(log, SLOTS, USERSLOTS)
    session = bytearray(r.bytes(0x110))
    put(session, 0x108, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    regions[SESSION] = session
    set_return(log, ONLINE, SESSION)
    regions[0xfaa5d8] = struct.pack('<I', r.choice([0, 0x1234500, 0x1234500]))
    regions[0xf81ebc] = struct.pack('<I', r.choice([0, USERSLOTS]))
    regions[0xf7d88c] = struct.pack('<I', r.choice([0, SESSION]))
    regions[0xf8288c] = struct.pack('<I', r.choice([0, 0, 1, r.getrandbits(32)]))
    regions[0xf83774] = r.bytes(0x14)
    regions[0xf85614] = r.bytes(4)
    return Call(0x5d5970, [FRAME], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
