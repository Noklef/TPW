# The input filters (src/game/input/devicefilter) against the originals: the next filter, the device, the handler table and the event queue are recorders; the device's calls answered in turn,
# its state read with the modifier keys' bytes dealt; the position's floats at the bounds, past 2^24 (FILD holds the int exactly) and NaN.
# usage: devicefilter.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, fill, heap, import_table, log_region, put, recorder, set_return, strings, vtable  # noqa: E402

ADDRESS = {'SetNext': (0x632350, 38), 'Close': (0x633200, 26), 'SetSink': (0x633250, 45), 'Key': (0x633510, 43), 'Dispatch': (0x6336d0, 21), 'ListDtor': (0x6336f0, 12), 'Subscribe': (0x633700, 66), 'Unsubscribe': (0x633750, 78),
           'Mouse': (0x6337a0, 180), 'Char': (0x633860, 68), 'HolderDtor': (0x633a50, 20), 'GetDevice': (0x633a70, 45), 'SetLevel': (0x633aa0, 185), 'Apply': (0x633b90, 92), 'ComRelease': (0x633db0, 13),
           'Modifiers': (0x634130, 160), 'Ctor': (0x634220, 197), 'MouseMsg': (0x634420, 159), 'Clamp': (0x6344f0, 107), 'Bounds': (0x634560, 55), 'Attach': (0x634720, 15), 'Detach': (0x634730, 15),
           'Const1': (0x6348e0, 6), 'Const2': (0x6348f0, 6), 'FilterDtor': (0x633160, 0x60), 'FilterDel': (0x633140, 30), 'HolderDel': (0x633a30, 30), 'KeyCtor': (0x633360, 149), 'SetMode': (0x633d10, 150),
           'SubC90': (0x633ea0, 38), 'SubCC0': (0x6345a0, 38), 'SetHolder': (0x6341d0, 54), 'ReadState': (0x633600, 191), 'DevCtor': (0x63e7c0, 84), 'DevDtor': (0x63e840, 121), 'DevDel': (0x63e820, 30),
           'ModKey': (0x633ed0, 174), 'KeyDtor': (0x633470, 147), 'CharDtor': (0x633e20, 123), 'OpenEvent': (0x6331c0, 62), 'Move': (0x634360, 190), 'Drain': (0x633280, 189), 'Poll': (0x63e970, 82),
           'BaseDel': (0x6342f0, 30), 'DevEvent': (0x633540, 19), 'SetKeys': (0x633560, 75), 'SetMice': (0x6335b0, 75), 'Const3': (0x6350d0, 6), 'Const4': (0x6350e0, 6),
           'Reopen': (0x633220, 43), 'HolderOpen': (0x633bf0, 285), 'DevOpen': (0x63e8c0, 170), 'KeyDel': (0x633400, 30), 'CharDel': (0x633e00, 30),
           'KeyDelAdj': (0x6336c0, 8), 'CharDelAdj': (0x634210, 8), 'DevDelAdj': (0x63e9d0, 8), 'ReopenOut': (0x633340, 15), 'DevOpenOut': (0x634a30, 23)}
# the adjustors (this less the offset, then the op's function) and the wrappers returning the result: op -> (the op set up, the offset)
WRAPS = {'KeyDelAdj': ('KeyDel', 0xc), 'CharDelAdj': ('CharDel', 0xc), 'DevDelAdj': ('DevDel', 0x10), 'ReopenOut': ('Reopen', 0), 'DevOpenOut': ('DevOpen', 0)}
OPS = [n for n in ADDRESS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
LISTEN, UNLISTEN, TICK = 26, 27, 28
HOOKS = {0x5fc030: _recorder(LISTEN, 3), 0x5fc100: _recorder(UNLISTEN, 3), 0x5fba10: _recorder(TICK, 1)}
B = Bench([(a, a + n) for a, n in ADDRESS.values()] + [(0x6338b0, 0x6338c0), (0x633b80, 0x633b90), (0x6344c0, 0x6344f0), (0x5f86c0, 0x5f86c6), (0x67a830, 0x67a860), (0x67acb0, 0x67acb1), (0x67a400, 0x690000), (0x633120, 0x633140), (0x633a00, 0x633a30), (0x5fa650, 0x5fa678), (0x633940, 0x633994), (0x634310, 0x634360), (0x633b70, 0x633b80), (0x5f86a0, 0x5f86a6), (0x6793bc, 0x6793c2)], hooks=HOOKS)
DINPUT = 39
OBJ, NEXT, VT, DEV, VTC, TABLE, MSG, LIST, OUT, ANSWERS = RAM, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500, RAM + 0x600, RAM + 0x700, RAM + 0x800, RAM + 0x900, RAM + 0xa00
CLOSE, SPI, CLICK, SWAP, CREATE, LASTERROR, RESET, ENTER, LEAVE = 30, 31, 32, 33, 34, 35, 36, 37, 38
POLL = 10   # (the device's slot 0x28)
WINDOW = 0xfa7610
ACQUIRE, UNACQUIRE, STATE, LEVEL = 7, 8, 9, 13
H0, H4, H8, HC, H10 = 20, 21, 22, 23, 24


def imports(log):
    return import_table(log, {'CloseHandle': (CLOSE, 0x6fd13c, 1, 0, 0), 'SystemParametersInfoA': (SPI, 0x6fd374, 4, 0, 0b100), 'GetDoubleClickTime': (CLICK, 0x6fd32c, 0, 0, 0), 'SwapMouseButton': (SWAP, 0x6fd330, 1, 0, 0),
                              'CreateEventA': (CREATE, 0x6fd0d4, 4, 0, 0), 'GetLastError': (LASTERROR, 0x6fd138, 0, 0, 0), 'ResetEvent': (RESET, 0x6fd168, 1, 0, 0),
                              'EnterCriticalSection': (ENTER, 0x6fd140, 1, 0, 0), 'LeaveCriticalSection': (LEAVE, 0x6fd130, 1, 0, 0), 'DirectInputCreateA': (DINPUT, 0x6fd020, 4, 0, 0b100)})


def chain(r, obj, regions, slots):
    """+8: the next filter (its vtable's slots) or none."""
    put(obj, 8, 'I', 0 if r.random() < 0.25 else NEXT)
    regions[NEXT] = struct.pack('<I', VT) + r.bytes(0x20)
    regions[VT] = vtable(slots)


def device(r, regions):
    regions[DEV] = struct.pack('<I', VTC) + r.bytes(0x20)
    regions[VTC] = vtable({}, com={1: 1, 2: 1, ACQUIRE: 1, UNACQUIRE: 1, STATE: 3, LEVEL: 3})


def channels(r):
    n = r.choice([0, 1, 2, 3, 5])
    return struct.pack('<%di' % (n + 1), *[r.choice([0, 1, 2, 5, 7, -1]) for _ in range(n)], 6)


def holder(r, obj, regions, log):
    """A device holder: the level wanted and had, the windows, the device's answers."""
    level = r.choice([3, 5, 0xa])
    put(obj, 4, 'II', level, r.choice([level, level, 0xa, 5]))
    window, old = 0x11110, 0x22220
    put(obj, 0xc, 'I', 0 if r.random() < 0.08 else DEV)
    put(obj, 0x10, 'I', r.choice([0, 1, 1]))
    put(obj, 0x18, 'I', r.choice([old, old, window, 0]))
    put(obj, 0x1c, 'I', r.choice([0xffffffff, window, old, 0]))
    regions[WINDOW] = struct.pack('<I', r.choice([window, old, 0]))
    device(r, regions)
    good = [0, 0, 1, 0x4ffe0002, 0x80070005]
    set_return(log, UNACQUIRE, r.choice(good))
    set_return(log, ACQUIRE, r.choice(good))
    regions[ANSWERS] = struct.pack('<40I', *[r.choice(good) for _ in range(40)])
    answers(log, LEVEL, ANSWERS)


def position(r, low, high):
    """The bits of a float near, at or past the bounds."""
    k = r.random()
    if k < 0.15:
        return r.choice([0x7fc00000, 0xffc00000, 0x7f800000, 0xff800000, 0x7f800001])
    if k < 0.45:
        return struct.unpack('<I', struct.pack('<f', r.choice([low, high]) + r.choice([0.0, 0.5, -0.5, 1.0, -1.0])))[0]
    return struct.unpack('<I', struct.pack('<f', r.uniform(-3000, 3000)))[0]


def case(r):
    op = r.deal(OPS)
    if op in WRAPS:
        base, offset = WRAPS[op]
        c = build(r, base)
        c.address = ADDRESS[op][0]
        c.args[0] += offset
        return c
    return build(r, op)


def build(r, op):
    address = ADDRESS[op][0]
    log = log_region()
    obj = bytearray(r.bytes(0x100))
    message = bytearray(r.bytes(0x40))
    regions = {LOG: log, OBJ: obj, MSG: message, IMPORT_TABLE: imports(log)}
    if op == 'Close':
        put(obj, 8, 'I', r.choice([0, r.getrandbits(32) | 1]))
        set_return(log, CLOSE, r.getrandbits(32))
        return Call(address, [OBJ], regions, conv='t', ret='v')
    if op in ('SetSink', 'SetNext'):
        put(obj, 0xc if op == 'SetSink' else 8, 'I', r.choice([0, NEXT]))
        regions[NEXT] = struct.pack('<I', VT)
        regions[DEV] = struct.pack('<I', VT)
        regions[VT] = vtable({})
        return Call(address, [OBJ, r.choice([0, DEV])], regions, conv='t', ret='v')
    if op == 'Key':
        put(message, 0x10, 'I', r.randrange(0, 0x80))
        chain(r, obj, regions, {6: 1})
        return Call(address, [OBJ, MSG], regions, conv='t', ret='v')
    if op == 'Dispatch':
        regions[DEV] = struct.pack('<I', VT)
        regions[VT] = vtable({0: 2})
        return Call(address, [r.getrandbits(32), r.getrandbits(32), DEV], regions, conv='s', ret='v')
    if op in ('ListDtor', 'Unsubscribe', 'Subscribe'):
        regions[LIST] = channels(r)
        put(obj, 4, 'I', r.choice([0, LIST]))
        if op == 'Subscribe':
            regions[LIST + 0x40] = channels(r)
            return Call(address, [OBJ, LIST + 0x40, r.getrandbits(32)], regions, conv='t', ret='v')
        return Call(address, [OBJ], regions, conv='t', ret='v')
    if op in ('Mouse', 'Char'):
        slots = [(0, H0, 3), (4, H4, 5), (8, H8, 4), (0xc, HC, 4), (0x10, H10, 6)]
        table = bytearray(0x20)
        for at, k, n in slots:
            put(table, at, 'I', 0 if r.random() < 0.25 else recorder(k, n, cdecl=True))
        regions[TABLE] = table
        put(obj, 0xc, 'I', TABLE if op == 'Char' or r.random() < 0.9 else 0)
        put(message, 0x10, 'I', r.choice([0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0, 0xffffffff, r.getrandbits(32)]))
        chain(r, obj, regions, {4: 1, 6: 1})
        return Call(address, [OBJ, MSG], regions, conv='t', ret='v')
    if op == 'HolderDtor':
        put(obj, 0xc, 'I', r.choice([0, DEV]))
        device(r, regions)
        return Call(address, [OBJ], regions, conv='t', ret='v')
    if op == 'GetDevice':
        put(obj, 0xc, 'I', DEV)
        device(r, regions)
        regions[OUT] = r.bytes(0x10)
        return Call(address, [OBJ, OUT, r.choice([OUT + 8, OUT + 8, 0])], regions, conv='t', ret='a')
    if op in ('SetLevel', 'Apply'):
        holder(r, obj, regions, log)
        regions[OUT] = r.bytes(0x10)
        return Call(address, [OBJ, OUT], regions, conv='t', ret='a')
    if op == 'ComRelease':
        put(obj, 0, 'I', r.choice([0, DEV]))
        device(r, regions)
        return Call(address, [OBJ], regions, conv='t', ret='v')
    if op == 'Modifiers':
        put(obj, 0x1c, 'I', r.choice([NEXT - 0x10, NEXT - 0x10, NEXT - 0x10, NEXT, 0]))
        regions[NEXT] = put(bytearray(r.bytes(0x20)), 0xc, 'I', DEV)
        device(r, regions)
        set_return(log, STATE, r.choice([0, 0, 0, 1, 0x80070005]))
        strings(log, STATE, 0, out=0b100)   # (the local buffer)
        fill(log, STATE, 2, bytes(r.choice([0, 0, 0, 0x80, 0x7f, 0xff]) for _ in range(0x26)), which=0, offset=0x1d)
        fill(log, STATE, 2, bytes(r.choice([0, 0, 0, 0x80, 0x7f, 0xff]) for _ in range(0x1c)), which=1, offset=0x9d)
        return Call(address, [OBJ, NEXT], regions, conv='t', ret='v')
    if op == 'Ctor':
        fill(log, SPI, 2, r.bytes(12))
        set_return(log, SPI, r.getrandbits(32))
        set_return(log, CLICK, r.getrandbits(32))
        set_return(log, SWAP, r.choice([0, 1]))
        return Call(address, [OBJ], regions, conv='t', ret='a')
    if op == 'MouseMsg':
        what = r.choice([0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0, 0xffffffff])
        put(message, 0x10, 'I', what)
        put(message, 0x20, 'I', r.getrandbits(32))
        obj[0x60] = r.choice([0, 0, 1, 0x80])
        swapped = {0xf0: 0xf1, 0xf1: 0xf0}.get(what, what) if obj[0x60] else what
        put(obj, 0x48, 'I', r.choice([swapped, swapped, what, 0]))
        time, later = r.getrandbits(32), r.choice([0, 1, 0x7fffffff, 0x80000000, 0xffffffff, r.getrandbits(32)])
        put(message, 4, 'I', time)
        put(obj, 0x4c, 'I', time + later & 0xffffffff)
        chain(r, obj, regions, {6: 1})
        return Call(address, [OBJ, MSG], regions, conv='t', ret='v')
    if op == 'Clamp':
        low, high = r.choice([(0, 639), (-5, 5), (0, 0), (16777217, 33554435), (-16777217, 16777219), (100, -100)])
        put(obj, 0x34, 'ii', low, r.choice([low, 0]))
        put(obj, 0x3c, 'ii', high, r.choice([high, 479]))
        put(obj, 0x20, 'II', position(r, low, high), position(r, low, high))
        return Call(address, [OBJ], regions, conv='t', ret='v')
    if op == 'Bounds':
        put(message, 0, 'I', r.choice([7, 7, 6, 8, 0]))
        return Call(address, [OBJ + 0x64, r.getrandbits(32), MSG], regions, conv='t', ret='v')
    if op in ('FilterDtor', 'FilterDel', 'DevDtor', 'DevDel', 'HolderDel'):
        regions[HEAP] = heap(0x40)
        put(obj, 8, 'I', r.choice([0, r.getrandbits(32) | 1]))
        put(obj, 0xc, 'I', r.choice([0, DEV]))
        put(obj, 0x1c, 'I', r.choice([0, DEV]))
        put(obj, 0x30, 'I', r.choice([0, HEAP + 0x40, RAM + 0x7f0]))
        regions[DEV] = struct.pack('<I', VTC) + r.bytes(0x20)
        regions[VTC] = vtable({}, com={2: 1})
        set_return(log, CLOSE, r.getrandbits(32))
        if op in ('FilterDel', 'DevDel', 'HolderDel'):
            return Call(address, [OBJ, r.choice([0, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')
        return Call(address, [OBJ], regions, conv='t', ret='v')
    if op in ('KeyCtor', 'DevCtor'):
        regions[OBJ] = r.bytes(0x130)
        return Call(address, [OBJ], regions, conv='t', ret='a')
    if op in ('SubC90', 'SubCC0'):
        put(obj, 0x14, 'I', r.choice([0, LIST]))   # (the old list, its channels unsubscribed: at +0x10 + 4)
        put(obj, 0x6c, 'I', r.choice([0, LIST]))
        regions[LIST] = channels(r)
        regions[OUT] = r.bytes(0x10)
        return Call(address, [OBJ, OUT], regions, conv='t', ret='a')
    if op == 'SetMode':
        holder(r, obj, regions, log)
        regions[OUT] = r.bytes(0x10)
        return Call(address, [OBJ, OUT, r.choice([0, 1, 2, 3, 4, 5, 6, 8, 9, 0xa, 0xc, 0xd, 0xf, r.getrandbits(32)])], regions, conv='t', ret='a')
    if op == 'SetHolder':
        put(obj, 0x1c, 'I', r.choice([0, NEXT]))
        regions[NEXT - 0x10] = put(bytearray(r.bytes(0x30)), 0x1c, 'I', DEV)
        device(r, regions)
        set_return(log, STATE, r.choice([0, 0, 1, 0x80070005]))
        strings(log, STATE, 0, out=0b100)
        fill(log, STATE, 2, bytes(r.choice([0, 0, 0, 0x80, 0x7f, 0xff]) for _ in range(0x26)), which=0, offset=0x1d)
        fill(log, STATE, 2, bytes(r.choice([0, 0, 0, 0x80, 0x7f, 0xff]) for _ in range(0x1c)), which=1, offset=0x9d)
        regions[OUT] = r.bytes(0x10)
        return Call(address, [OBJ, OUT, r.choice([NEXT - 0x10, NEXT - 0x10, 0])], regions, conv='t', ret='a')
    if op == 'ReadState':
        # (One recorder for both reads and the fill does not know which: the keyboard's 0x100 bytes filled alone, the mouse's 0x10 alone, or both asked with the first failing.)
        obj = bytearray(r.bytes(0x130))
        regions[OBJ] = obj
        kind = r.deal(['keys', 'mouse', 'both', 'other'])
        put(obj, 0x11c, 'II', NEXT - 0x10 if kind in ('keys', 'both') else r.choice([0, NEXT + 0x40]), NEXT - 0x10 if kind in ('mouse', 'both') else r.choice([0, NEXT + 0x40]))
        regions[NEXT] = put(bytearray(r.bytes(0x20)), 0xc, 'I', DEV)
        device(r, regions)
        regions[ANSWERS] = struct.pack('<40I', *[0x80070005 if kind == 'both' and i == 0 else r.choice([0, 0, 1, 0x80070005]) for i in range(40)])
        answers(log, STATE, ANSWERS)
        strings(log, STATE, 0, out=0b100)
        fill(log, STATE, 2, r.bytes(0x10))
        if kind == 'keys':
            fill(log, STATE, 2, r.bytes(0x70), which=1, offset=0x10)
            fill(log, STATE, 2, r.bytes(0x70), which=2, offset=0x80)
        return Call(address, [OBJ, NEXT], regions, conv='t', ret='v')
    if op == 'ModKey':
        put(message, 0x10, 'I', r.choice([0xf, 0x1d, 0x2a, 0x36, 0x38, 0x9d, 0xb8, 0xe, 0x10, 0x1c, 0x1e, 0x39, 0xb7, 0xb9, 0xc5, 0xffffffff]))
        put(message, 0x20, 'I', r.getrandbits(32))
        put(obj, 0x20, 'I', r.getrandbits(32))
        chain(r, obj, regions, {6: 1})
        return Call(address, [OBJ, MSG], regions, conv='t', ret='v')
    if op in ('HolderOpen', 'DevOpen'):
        base = 0 if op == 'HolderOpen' else 0x10
        put(obj, base, 'I', VT)
        put(obj, base + 0xc, 'I', r.choice([0, 0x9999]))
        put(obj, base + 0x14, 'I', r.choice([0x5000, 0x5000, 0xffffffff]))
        put(obj, 8, 'I', r.choice([0, 0x1234]))
        regions[VT] = vtable({1: 0, 2: 0})
        set_return(log, 1, 0x7777)
        set_return(log, 2, 0x6666)
        regions[NEXT] = struct.pack('<I', VTC) + r.bytes(0x20)
        regions[DEV] = struct.pack('<I', VTC) + r.bytes(0x20)
        regions[VTC] = vtable({}, com={2: 1, 3: 4, 6: 3, 11: 2, 12: 2})
        failed = r.random() < 0.2
        set_return(log, DINPUT, 0x80004005 if failed else r.choice([0, 1]))
        fill(log, DINPUT, 2, struct.pack('<I', r.choice([0, NEXT]) if failed else NEXT))
        set_return(log, 3, r.choice([0, 0, 0, 1, 0x80070005]))
        strings(log, 3, 0, out=0b100)
        fill(log, 3, 2, struct.pack('<I', DEV), which=1)
        set_return(log, 11, r.choice([0, 0, 0, 0x80070057]))
        set_return(log, 12, r.choice([0, 0, 0, 0x80070006]))
        set_return(log, 6, r.choice([0, 1, 0x80070005]))
        strings(log, 6, 0b100)   # (of the property's dwords, as a C string only the first)
        set_return(log, CREATE, r.choice([0, 0x5678, 0x5678, 0x5678]))
        set_return(log, LASTERROR, r.choice([0, 5, 0x12345]))
        set_return(log, CLOSE, 1)
        regions[OUT] = r.bytes(0x10)
        regions[HEAP] = heap(0x400, fail=r.choice([0, 0, 0, 1]))
        return Call(address, [OBJ, OUT], regions, conv='t', ret='a')
    if op in ('KeyDtor', 'CharDtor', 'BaseDel', 'KeyDel', 'CharDel'):
        obj = bytearray(r.bytes(0x130))
        regions[OBJ] = obj
        regions[NEXT] = struct.pack('<I', VT)
        regions[VT] = vtable({})
        regions[LIST] = channels(r)
        regions[HEAP] = heap(0x40)
        put(obj, 8, 'I', r.choice([0, NEXT]))
        put(obj, 0x14, 'I', r.choice([0, LIST]))
        put(obj, 0x11c, 'II', r.choice([0, NEXT]), r.choice([0, NEXT]))
        if op in ('BaseDel', 'KeyDel', 'CharDel'):
            return Call(address, [OBJ, r.choice([0, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')
        return Call(address, [OBJ], regions, conv='t', ret='v')
    if op in ('OpenEvent', 'Reopen'):
        put(obj, 8, 'I', r.choice([0, 0x1234]))
        regions[OUT] = r.bytes(0x10)
        set_return(log, CREATE, r.choice([0, 0x5678, 0x5678]))
        set_return(log, LASTERROR, r.choice([0, 5, 0x12345]))
        set_return(log, CLOSE, 1)
        return Call(address, [OBJ, OUT], regions, conv='t', ret='a')
    if op == 'Move':
        low, high = r.choice([(0, 639), (-5, 5), (-3000, 3000)])
        put(obj, 0x34, 'ii', low, low)
        put(obj, 0x3c, 'ii', high, high)
        put(obj, 0x20, 'II', position(r, low, high), position(r, low, high))
        put(obj, 0x28, 'II', position(r, low, high), position(r, low, high))
        put(obj, 0x30, 'f', r.choice([1.0, 0.5, 0.3, 1.7, 2.1e-3, 3.3e7]))
        put(obj, 0x50, 'ii', r.choice([0, 3, 6, -1]), r.choice([0, 6, 10, -1]))
        put(obj, 0x58, 'ii', r.choice([0, 1, 2, 3, -1]), r.choice([0, 1, 1]))
        put(obj, 0xc, 'I', r.choice([0, NEXT + 0x40]))
        put(message, 0xc, 'I', r.choice([0, 0, 4, 4, 8, 1]))
        put(message, 0x10, 'i', r.choice([0, 1, -1, 3, 4, 6, 7, 11, -7, 0x12345, 0x7fffffff, -0x80000000, r.randrange(-500, 500)]))
        chain(r, obj, regions, {5: 1})
        return Call(address, [OBJ, MSG], regions, conv='t', ret='v')
    if op == 'Drain':
        n = r.choice([0, 1, 2, 3])
        regions[LIST] = r.bytes(0x20)
        put(obj, 0x28, 'II', 0 if r.random() < 0.2 else LIST, 0)
        if struct.unpack_from('<I', obj, 0x28)[0]:
            put(obj, 0x2c, 'I', LIST + 4 * n)
        put(obj, 0x34, 'I', 0x7700)
        put(obj, 0xc, 'I', DEV)
        regions[DEV] = struct.pack('<I', VT)
        regions[VT] = vtable({4: 1})
        return Call(address, [OBJ], regions, conv='t', ret='v')
    if op == 'Poll':
        put(obj, 0x1c, 'I', DEV)
        put(obj, 0x30, 'I', r.getrandbits(32))
        put(obj, 0, 'I', VT)
        regions[VT] = vtable({5: 1})
        regions[DEV] = struct.pack('<I', VTC) + r.bytes(0x20)
        regions[VTC] = vtable({}, com={POLL: 5, 7: 1, 8: 1, 13: 3})
        strings(log, POLL, 0, out=0b1000)
        set_return(log, POLL, r.choice([0, 0x8007001e, 0x8007001e, 1, 0x80070005]))
        set_return(log, 7, 0)
        set_return(log, 8, 0)
        set_return(log, 13, 0)
        put(obj, 0x14, 'II', 5, 5)
        put(obj, 0x20, 'I', 1)
        put(obj, 0x28, 'II', 1, 1)
        fill(log, POLL, 3, struct.pack('<I', r.choice([0, 1, 7, 0x20])))
        return Call(address, [OBJ], regions, conv='t', ret='v')
    if op in ('DevEvent', 'SetKeys', 'SetMice'):
        obj = bytearray(r.bytes(0x130))
        regions[OBJ] = obj
        regions[NEXT - 0x10] = put(bytearray(struct.pack('<I', VT) + r.bytes(0x2c)), 0x1c, 'I', DEV)
        regions[NEXT + 0x40] = struct.pack('<I', VT)
        regions[VT] = vtable({})
        device(r, regions)
        set_return(log, STATE, r.choice([0, 1, 0x80070005]))
        strings(log, STATE, 0, out=0b100)
        fill(log, STATE, 2, r.bytes(0x10))
        # (As in ReadState: the fill must fit the one read made.)
        keys = op == 'SetKeys' or op == 'DevEvent' and r.random() < 0.5
        if keys:
            fill(log, STATE, 2, r.bytes(0x70), which=1, offset=0x10)
            fill(log, STATE, 2, r.bytes(0x70), which=2, offset=0x80)
        put(obj, 0x11c, 'II', NEXT - 0x10 if op == 'DevEvent' and keys else r.choice([0, NEXT + 0x40]), NEXT - 0x10 if op == 'DevEvent' and not keys else r.choice([0, NEXT + 0x40]))
        if op == 'DevEvent':
            regions[MSG] = struct.pack('<II', r.getrandbits(32), r.choice([NEXT, NEXT, NEXT + 0x80]))
            return Call(address, [OBJ + 0xc, r.getrandbits(32), MSG], regions, conv='t', ret='v')
        regions[OUT] = r.bytes(0x10)
        return Call(address, [OBJ, OUT, r.choice([0, NEXT - 0x10, NEXT - 0x10])], regions, conv='t', ret='a')
    if op in ('Attach', 'Detach'):
        regions[DEV] = struct.pack('<I', VT)
        regions[VT] = vtable({5: 1, 6: 1})
        return Call(address, [OBJ, DEV], regions, conv='t', ret='v')
    return Call(address, [], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
