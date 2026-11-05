# Bench: Renderer_DrawCellLists 0x576910: the first list's objects drawn, the ground's textures prepared, then the visible rows' lists; 0 from any stops it.
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region, set_return, vtable  # noqa: E402

DRAW, PREPARE = 1, 2
B = Bench([(0x576910, 0x5769f1)], hooks={0x56f090: _recorder(PREPARE, 0)})
SELF, RENDER, VIEW, GRID, NODES, OBJECTS, VT, ANSWERS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x400, RAM + 0x600, RAM + 0x800, RAM + 0x880, RAM + 0x900
STRIDE = 6


def case(r):
    log = log_region()
    set_return(log, PREPARE, r.deal([1, 1, 1, 0, 7]))
    answers(log, DRAW, ANSWERS)
    pool = bytearray()

    def chain(most):
        """A list's head: most nodes or fewer, each {an object, the next}."""
        head = 0
        for _ in range(r.randrange(most + 1)):
            pool.extend(struct.pack('<II', OBJECTS + 4 * r.randrange(3), head))
            head = NODES + len(pool) - 8
        return head

    first = chain(3)
    grid = [chain(2) for _ in range(STRIDE * 4)]
    top, bottom = r.randrange(3), r.randrange(5)
    starts = [r.choice([0, 1, 2, -1 if y else 0]) for y in range(0x80)]
    ends = [r.randrange(-1, 6) for _ in range(0x80)]
    view = struct.pack('<2i', top, bottom) + struct.pack('<128b', *starts) + struct.pack('<128b', *ends)
    regions = {LOG: log, SELF: r.bytes(0x10) + struct.pack('<I', GRID) + r.bytes(0x18) + struct.pack('<I', first) + r.bytes(8) + struct.pack('<I', STRIDE) + r.bytes(8) +
               struct.pack('<I', r.deal([VIEW, VIEW, VIEW, VIEW, 0])) + r.bytes(4),
               0x87a6d0: struct.pack('<I', RENDER), RENDER + 0x30: r.bytes(4), 0x879d60: r.bytes(4), 0x8bb624: r.bytes(4), VIEW: view, GRID: struct.pack('<%dI' % len(grid), *grid),
               NODES: bytes(pool) + bytes(8), OBJECTS: struct.pack('<3I', VT, VT, VT), VT: vtable({}, 4),
               ANSWERS: struct.pack('<60I', *[r.choice([1] * 30 + [5, 0]) for _ in range(60)])}
    return Call(0x576910, [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
