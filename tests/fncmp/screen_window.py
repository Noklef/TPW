# The screen's window and DirectDraw object (src/game/gfx/screen: 0x5fa030-0x5fa63f) against the originals: the screen state (0xfa7948) random,
# its DirectDraw object and the surfaces' IDirectDrawSurfaces COM recorders (each surface its own slot, so the log tells the releases apart); the
# user32 / gdi32 / ddraw imports COM-style recorders in the import table (CreateWindowEx's twelve arguments, its class and window names hashed as
# strings; DirectDrawCreate filling its out pointer with the DirectDraw object or failing; RegisterClass's WNDCLASS hashed; EnumDisplayModes'
# callback, Screen_AddMode, whose address differs between the sides, logged as 0). The app state (0xfa7608) gives the window, the instance, the icon
# id and the window procedure hand-off (a recorder or none); the event queue (0xfa7cb0) one listener on each of channels 0, 4 and 5, its event
# (on the stack) hashed, a critical section of its own or none. The mode list (0xfa77d0) holds zero to three nodes; Screen_AddMode allocates from the heap, failing now and then.
# usage: screen_window.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetGuid,... to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, blob, fill, heap, import_table, log_region, put, recorder, set_return, strings  # noqa: E402

B = Bench([(0x5f86a0, 0x5f86f0), (0x5f8750, 0x5f87a0), (0x5f8f80, 0x5f8fa0), (0x5fa030, 0x5fa660), (0x5fbf80, 0x5fc020), (0x62f980, 0x62fa50),
           (0x6793b6, 0x6793cc), (0x67a650, 0x67a6d0), (0x680ad0, 0x6816b0), (0x683aa0, 0x683ad0), (0x687140, 0x6871c0), (0x68c320, 0x68c450)])
CALLS = {'SetGuid': 0x5fa030, 'GetDirectDraw': 0x5fa040, 'Close': 0x5fa060, 'ReleaseSurfaces': 0x5fa0e0, 'ReleaseDirectDraw': 0x5fa1b0,
         'Open': 0x5fa1e0, 'CreateDirectDraw': 0x5fa2b0, 'ListModes': 0x5fa2f0, 'AddMode': 0x5fa440, 'RegisterClass': 0x5fa490,
         'WndProc': 0x5fa540, 'SetActive': 0x5fa630}
STATE, OPEN, MODES, LOCKFLAG, SURFB, APP, Q = 0xfa7948, 0xfa9f3c, 0xfa77d0, 0xfa78e8, 0xfa7730, 0xfa7608, 0xfa7cb0
DD, DDVT, SURFS, NODES, LISTENERS, OUT, DESC, WC, GUID = (RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500, RAM + 0x520,
                                                          RAM + 0x5a0, RAM + 0x5d0)
WINDOW, OTHER_WINDOW = 0x1234, 0x5678
# the DirectDraw object's slots (k the slot): AddRef, Release, EnumDisplayModes, RestoreDisplayMode, SetCooperativeLevel
DD_ARGS = {1: 1, 2: 1, 8: 5, 19: 1, 20: 3}
SURFACE_K = [40, 41, 42, 43]   # surfaceA, surfaceB, primary, the state's +0x10: each Release on its own slot
LISTENER, APP_PROC = 60, 61
IMPORTS = {'CreateWindowEx': (50, 0x6fd398, 12, 6, 0), 'GetSystemMetrics': (51, 0x6fd338, 1, 0, 0), 'DestroyWindow': (52, 0x6fd33c, 1, 0, 0),
           'LoadIcon': (53, 0x6fd364, 2, 0, 0), 'GetStockObject': (54, 0x6fd054, 1, 0, 0), 'RegisterClass': (55, 0x6fd394, 1, 0, 0),
           'DefWindowProc': (56, 0x6fd340, 4, 0, 0), 'SetCursor': (57, 0x6fd334, 1, 0, 0), 'DirectDrawCreate': (58, 0x6fd018, 3, 0, 1 << 1),
           'EnterCriticalSection': (12, 0x6fd140, 1, 0, 0), 'LeaveCriticalSection': (13, 0x6fd130, 1, 0, 0)}


def surface(r, k):
    """A Surface wrapper's 0x50 bytes: +0x24 its IDirectDrawSurface (a COM object whose Release is slot k's recorder) or none."""
    s = bytearray(r.bytes(0x50))
    put(s, 0x24, 'I', SURFS + 0x10 * (k - 40) if r.random() < 0.7 else 0)
    return s


def modes(r, regions):
    """The mode list (an Exec-style list: head, tail 0, tailPred) with zero to three nodes {next, prev, width, height, bits}, and the class name."""
    lst = bytearray(r.bytes(0x30))
    n = r.choice([0, 0, 1, 2, 3])
    nodes = bytearray(r.bytes(0x40))   # (the queue's listener nodes follow)
    for i in range(n):
        put(nodes, 0x14 * i, '2I', NODES + 0x14 * (i + 1) if i + 1 < n else MODES + 4, NODES + 0x14 * (i - 1) if i else MODES)
    put(lst, 0, '3I', NODES if n else MODES + 4, 0, NODES + 0x14 * (n - 1) if n else MODES)
    regions[MODES], regions[NODES] = lst, nodes


def queue(r):
    """The event queue: a critical section or none, channels 0, 4 and 5 one listener each (the listener's callback a recorder), the others empty."""
    q = bytearray(0x64)
    put(q, 0, 'I', r.choice([0, RAM + 0x600]))
    listeners = bytearray()
    for ch in range(6):
        if ch in (0, 4, 5) and r.random() < 0.8:
            i = len(listeners) // 8
            listeners += struct.pack('<2I', r.choice([0, 7]), recorder(LISTENER, 3, True))
            put(q, 4 + 0x10 * ch, '4I', 0x703648, NODES + 0x40 + 0xc * i, NODES + 0x40 + 0xc * i, 1)
        else:
            put(q, 4 + 0x10 * ch, '4I', 0x703648, 0, 0, 0)
    nodes = b''.join(struct.pack('<3I', LISTENERS + 8 * i, 0, 0) for i in range(len(listeners) // 8))
    return q, listeners.ljust(0x40, b'\0'), nodes


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, IMPORT_TABLE: import_table(log, IMPORTS), HEAP: heap(fail=r.choice([0, 0, 0, 1]))}
    state = bytearray(r.bytes(0x28))
    put(state, 0, '3I', r.choice([0, WINDOW, OTHER_WINDOW]), r.choice([0, WINDOW, OTHER_WINDOW]), DD if r.random() < 0.8 else 0)
    put(state, 0x10, 'I', SURFS + 0x30 if r.random() < 0.6 else 0)
    put(state, 0x24, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    regions[STATE] = state
    regions[OPEN] = struct.pack('<I', r.choice([0, 0, 1, r.getrandbits(32)]))
    modes(r, regions)
    fpu = bytearray(r.bytes(0xc))
    put(fpu, 4, '3H', r.choice([0x27f, 0x37f, 0x7f]), *([0x300, 0x300] if r.random() < 0.15 else [0, 0]))
    regions[LOCKFLAG] = r.bytes(4) + fpu + surface(r, 40)   # (the lock flag, the FPU control, surfaceA: up to the state)
    regions[SURFB] = surface(r, 41) + surface(r, 42)
    regions[DD], regions[DDVT] = struct.pack('<I', DDVT), struct.pack('<21I', *[recorder(k, DD_ARGS.get(k, 0), True) for k in range(21)])
    regions[SURFS] = b''.join(struct.pack('<4I', SURFS + 0x10 * i + 4, 0, 0, recorder(k, 1, True)) for i, k in enumerate(SURFACE_K))
    q, listeners, nodes = queue(r)
    regions[Q], regions[LISTENERS] = q, listeners
    regions[NODES] += nodes
    blob(log, LISTENER, 1, 3)   # (the event: on the stack)
    app = bytearray(r.bytes(0x18))
    put(app, 4, '4I', r.getrandbits(32), r.choice([WINDOW, OTHER_WINDOW, 0]), recorder(APP_PROC, 4, True) if r.random() < 0.3 else 0,
        r.choice([0, 0, 101, r.getrandbits(16)]))
    regions[APP] = app
    regions[0xfa76b0] = r.bytes(8)
    regions[OUT], regions[DESC], regions[WC], regions[GUID] = r.bytes(0x10), r.bytes(0x6c), r.bytes(0x28), r.bytes(0x10)
    strings(log, 8, 0, 1 << 4)   # (EnumDisplayModes' callback: Screen_AddMode, a different address on each side)
    blob(log, 55, 0, 0x3ff)      # (the WNDCLASS: the stack)
    fill(log, 58, 1, struct.pack('<I', DD))
    set_return(log, 58, r.choice([0, 0, 0, 0x80004005]))
    set_return(log, 50, r.choice([WINDOW, WINDOW, OTHER_WINDOW, 0]))
    set_return(log, 20, r.choice([0, 0, 0, 1]))
    address = CALLS[op]
    guid = r.choice([0, GUID])
    if op == 'SetGuid':
        return Call(address, [guid], regions, conv='c', ret='v')
    if op == 'GetDirectDraw':
        return Call(address, [r.choice([0, 1, r.getrandbits(32)])], regions, conv='c', ret='a')
    if op in ('Close', 'ReleaseSurfaces', 'ReleaseDirectDraw', 'RegisterClass'):
        return Call(address, [], regions, conv='c', ret='v')
    exclusive = r.choice([0, 1, r.getrandbits(32)])
    window = r.choice([0, WINDOW, OTHER_WINDOW])
    if op == 'Open':
        return Call(address, [OUT, guid, exclusive, r.choice([0, 0, WINDOW]), window], regions, conv='c', ret='a')
    if op == 'CreateDirectDraw':
        return Call(address, [OUT, guid, OUT + 4, r.getrandbits(32)], regions, conv='c', ret='a')
    if op == 'ListModes':
        return Call(address, [OUT, exclusive, window, guid], regions, conv='c', ret='a')
    if op == 'AddMode':
        return Call(address, [DESC, MODES], regions, conv='s', ret='a')
    if op == 'WndProc':
        message = r.choice([6, 6, 0x10, 0x1c, 0x1c, 0x112, 0x112, 0, r.getrandbits(32)])
        wparam = r.choice([0, 1, 2, 3, 0xf100, 0xf100, r.getrandbits(32)])
        return Call(address, [r.choice([WINDOW, OTHER_WINDOW, 0]), message, wparam, r.getrandbits(32)], regions, conv='s', ret='a')
    return Call(address, [r.choice([0, 1, r.getrandbits(32)])], regions, conv='c', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
