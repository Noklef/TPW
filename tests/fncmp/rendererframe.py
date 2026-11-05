# The renderer's frame (src/game/render/rendererframe: Renderer_DrawView 0x576a00, Renderer_DrawFrame 0x576ec0, Engine_DrawFrame 0x44e540) against the originals: the callees recorders, those that may fail sharing one table of answers; Mat4_MoveAlongX real. DrawFrame's views are a recorder: each view's flags and window seen, its camera's position part seen through the pointer.
# usage: rendererframe.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, blob_bytes, fb, import_table, log_region, put, recorder, set_return  # noqa: E402

CHECKED, PLAIN, WINDOW, RESET, LIGHT, SCAN, CLEAR, DEV18, DEV1C, DEV20, DEV28, DEV30, ITEM, VIS, PLACE, GROUPS, ONCE = range(1, 18)
VIEW, FLAGS_T, CONTROL, LOAD, FULL, PUSH, DEVRESET, DROP, FOCUS, ATEXIT = 20, 21, 22, 23, 24, 25, 26, 27, 40, 28
THIS = {0x582f70: (WINDOW, 4), 0x56dd60: (SCAN, 0), 0x582390: (RESET, 1), 0x584be0: (LIGHT, 3), 0x576a00: (VIEW, 1), 0x56af20: (PUSH, 2), 0x56afc0: (DROP, 0)}
for a in (0x576910, 0x58cf40, 0x5863c0, 0x56f670, 0x570d90, 0x57c620, 0x577ae0, 0x58ac90, 0x58a660, 0x57c2a0, 0x57ff60):
    THIS[a] = (CHECKED, 0)
for a in (0x575fb0, 0x56ea90, 0x56bb00, 0x58d8e0, 0x581da0, 0x58a630, 0x579c70, 0x57dc80, 0x585f10, 0x577a30, 0x58ac70, 0x58a720, 0x579c80, 0x584b10, 0x583080, 0x5836b0, 0x561760, 0x5879c0):
    THIS[a] = (PLAIN, 0)
CDECL = {0x582170: (CHECKED, 0), 0x5783b0: (PLAIN, 0), 0x590750: (PLAIN, 0), 0x583790: (PLAIN, 0), 0x591220: (CONTROL, 0), 0x591240: (LOAD, 1), 0x591250: (FULL, 1), 0x5774c0: (DEVRESET, 0), 0x67a7d0: (ATEXIT, 1)}
HOOKS = {a: _recorder(k, n) for a, (k, n) in THIS.items()}
HOOKS.update({a: _recorder(k, n, cdecl=True) for a, (k, n) in CDECL.items()})


class FrameBench(Bench):
    """The original's own 0x576a00 in its direct cases, a recorder in the frame's."""

    def original(self, c):
        if c.address == 0x576a00:
            self.emu.hooks.pop(0x576a00, None)
        else:
            self.emu.hooks[0x576a00] = HOOKS[0x576a00]
        return super().original(c)


B = FrameBench([(0x576a00, 0x576eb8), (0x576ec0, 0x57745c), (0x44e540, 0x44e54c), (0x577460, 0x5774b0)], hooks=HOOKS)
SELF, OBJECTS, NODES, VIEWS, CAMERA, DEVICE_VT, ITEM_VT, CLEAR_VT, GROUPS_O, GROUPS_VB, GROUPS_VT, RENDER, PLACE_AT, MASKED, T_CHECK, T_VIEW, CAM2 = (RAM, RAM + 0x100, RAM + 0x180, RAM + 0x280, RAM + 0x400, RAM + 0x480, RAM + 0x500, RAM + 0x540, RAM + 0x580, RAM + 0x590, RAM + 0x5a0, RAM + 0x5c0,
                                                                                                                    RAM + 0x600, RAM + 0x620, RAM + 0x700, RAM + 0x800, RAM + 0x900)
GLOBALS = [0x8bd508, 0x8bcbc8, 0x8bd524, 0x8bd4ec, 0x790ab0, 0x8bd52c, 0x879d60, 0x879d54, 0x8c4b44, 0x8bcbb8, 0x8c47a0, 0x8bcbc4, 0x8bd490, 0x8bd494, 0x8bd47c, 0x8bcbac, 0x8bcc40, 0x8bcc44, 0x8bba48, 0x8bd4e4, 0x8bcbd0, 0x8bcbd4, 0x8bd550, 0x8bd518, 0x768aac, 0x8bd478, 0x87a248, 0x87a6d0, 0x879ab0, 0x87afe8]


def word(v):
    return struct.pack('<I', v & 0xffffffff)


def slots(*pairs):
    table = [0] * 16
    for i, k, n in pairs:
        table[i] = recorder(k, n)
    return struct.pack('<16I', *table)


def chain(r, data, at, most):
    """A list: nodes {the object, the next} in data; returns its head."""
    head = 0
    for _ in range(r.randrange(most + 1)):
        put(data, at, '2I', OBJECTS, head)
        head = NODES + at
        at += 8
    return head


def matrix(r):
    return struct.pack('<16I', *[fb(r.uniform(-50, 50)) for _ in range(16)])


def base(r, log):
    regions = {LOG: log, T_CHECK: struct.pack('<60I', *[r.choice([1] * 40 + [0, 7]) for _ in range(60)]), CAMERA: matrix(r), 0x87aff0: r.bytes(0x40), PLACE_AT: r.bytes(12), RENDER: r.bytes(0x40), MASKED: r.bytes(0x40),
               OBJECTS: word(ITEM_VT), ITEM_VT: slots((1, ITEM, 0), (2, VIS, 0), (3, PLACE, 0)), CLEAR_VT: slots((0, CLEAR, 0)), DEVICE_VT: slots((6, DEV18, 3), (7, DEV1C, 0), (8, DEV20, 0), (10, DEV28, 6), (12, DEV30, 0)),
               GROUPS_O: word(GROUPS_VB), GROUPS_VB: struct.pack('<2I', 0, 4), GROUPS_O + 4: word(GROUPS_VT), GROUPS_VT: slots((1, GROUPS, 0))}
    for g in GLOBALS:
        regions[g] = r.bytes(4)
    regions[0x879ab0] = word(DEVICE_VT - 0x10)
    regions[DEVICE_VT - 0x10] = word(DEVICE_VT)
    regions.update({0x87a6d0: word(RENDER), 0x87a248: word(MASKED), 0x87afe8: word(CAMERA), 0x8bba48: word(GROUPS_O), 0x8bd52c: word(CLEAR_VT - 8)})
    regions[CLEAR_VT - 8] = word(CLEAR_VT)
    regions[0x8bcbc8] = word(r.getrandbits(32) & r.getrandbits(32))
    regions[0x8bd508] = word(r.getrandbits(32) & r.getrandbits(32))
    regions[0x8bd524] = word(r.choice([0, 1, 1, 2, 4]))
    regions[0x790ab0] = bytes([r.choice([0, 0, 0, 1, 2, 4, 0x10, 0xe9])])
    regions[0x8bcbd0] = word(r.choice([640, 1024, -1]))
    regions[0x8bcbd4] = word(r.choice([480, 768, 0x1000001]))
    regions[0x8bd478] = word(fb(r.uniform(-5, 5)))
    answers(log, CHECKED, T_CHECK)
    answers(log, ITEM, T_CHECK)
    answers(log, DEV1C, T_CHECK)
    answers(log, DEV20, T_CHECK)
    answers(log, DEV28, T_CHECK)
    answers(log, CLEAR, T_CHECK)
    answers(log, VIS, T_VIEW)
    set_return(log, PLACE, r.choice([0, PLACE_AT, PLACE_AT]))
    set_return(log, SCAN, r.getrandbits(32))
    regions[T_VIEW] = struct.pack('<60I', *[r.choice([1, 1, 0, 2]) for _ in range(60)])
    return regions


def renderer(r, regions):
    data = bytearray(r.bytes(0x80))
    nodes = bytearray(0x100)
    at = 0
    for i in range(6):
        put(data, 0x14 + i * 4, 'I', chain(r, nodes, at, 2) if r.random() < 0.5 else 0)
        at += 0x10
    put(data, 0x2c, 'I', chain(r, nodes, at, 3) if r.random() < 0.7 else 0)
    put(data, 0x34, 'I', r.choice([0, r.getrandbits(32)]))
    put(data, 0x74, 'I', r.choice([r.getrandbits(32), struct.unpack('<I', regions[0x8bcc40])[0]]))
    put(data, 0x78, 'I', r.choice([0, recorder(ONCE, 0)]))
    regions[NODES] = nodes
    return data


def view_flags(r):
    return r.choice([r.getrandbits(32) & r.getrandbits(32) & 0x7fff, 0x7fff, r.getrandbits(32) & 0x7fff, 0x10, 0x800, 0x20])


def case(r):
    log = log_region()
    regions = base(r, log)
    data = renderer(r, regions)
    which = r.random()
    if which < 0.55:
        regions[SELF] = data
        regions[VIEWS] = struct.pack('<6I', view_flags(r), fb(0.0), fb(0.0), fb(640.0), fb(480.0), CAM2)
        regions[CAM2] = matrix(r)
        return Call(0x576a00, [SELF, VIEWS], regions, conv='t', ret='a')
    blob_bytes(log, VIEW, 0, 0xfffff, inner=(0x14, (1 << 36) - 1, 28))
    n = r.choice([0, 1, 1, 2, 3])
    put(data, 0x70, 'i', n)
    for i in range(4):
        put(data, 0x60 + i * 4, 'I', VIEWS + i * 0x18)
    views = bytearray()
    for i in range(4):
        views += struct.pack('<6I', view_flags(r), fb(r.uniform(-1, 1)), fb(r.uniform(-1, 1)), fb(r.uniform(0, 2)), fb(r.uniform(0, 2)), CAM2)
    mode = r.choice([0, 0, 4, 4, 4 | 8, 4 | 8])
    regions[0x768aac] = word(r.choice([1, 1, 2, 3, 0]))
    if r.random() < 0.4:
        # A global seen live at each view: its address in the view's first window float, which these two ways pass on as it is.
        mode = r.choice([0, 4 | 8])
        for i in range(4):
            put(views, i * 0x18 + 4, 'I', 0x8bd508 if mode == 0 else 0x8bcbc8)
        blob_bytes(log, VIEW, 0, 0xfffff, inner=(4, 0xf))
        if mode == 0 and r.random() < 0.6:
            put(data, 0x70, 'i', 1)
            regions[0x768aac] = word(1)
    regions[VIEWS] = views
    regions[CAM2] = matrix(r)
    regions[SELF] = data
    regions[0x87a680] = r.choice([bytes(regions[CAMERA]), r.bytes(0x40)]) + bytes([r.choice([0, 1, 1, 1]) | 0x80]) + r.bytes(3)
    regions[0x8bd518] = word(0x777)
    flags = struct.unpack('<I', regions[0x8bcbc8])[0]
    regions[0x8bcbc8] = word(flags & ~0xc | mode)
    regions[0x8bd550] = r.bytes(4)
    imports = import_table(log, {'GetForegroundWindow': (FOCUS, 0x6fd328, 0, 0, 0)})
    regions[IMPORT_TABLE] = imports
    set_return(log, FOCUS, r.choice([0x777, 0x777, 0x777, 0x777, 1]))
    answers(log, VIEW, T_CHECK)
    set_return(log, CONTROL, r.choice([0x27f, 0x8000 | r.getrandbits(15), 0x1ffff]))
    set_return(log, DEVRESET, r.choice([0, 1, 1, 1]))
    set_return(log, DROP, r.getrandbits(32))
    if r.random() < 0.1:
        return Call(0x44e540, [], {**regions, 0x87a6d0: word(SELF)}, conv='c', ret='a')
    return Call(0x576ec0, [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
