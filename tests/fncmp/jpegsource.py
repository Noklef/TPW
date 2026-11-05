# The game's libjpeg data source (src/game/gfx/jpegsource: 0x631140, 0x6311b0, 0x6311c0, 0x631230) against the originals: the memory manager's alloc_small, error_exit, emit_message and the stream's read
# are recorders; error_exit returns, so the code after it runs. The reads are answered in turn, so a skip crosses several buffers.
# usage: jpegsource.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, answers, log_region, put, recorder, set_return, vtable  # noqa: E402

ADDRESS = {'Set': 0x631140, 'Init': 0x6311b0, 'Fill': 0x6311c0, 'Skip': 0x631230}
OPS = [n for n in ADDRESS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
B = Bench([(0x631140, 0x6311af), (0x6311b0, 0x6311bc), (0x6311c0, 0x63122d), (0x631230, 0x63126d)])
CINFO, ERR, MEM, SOURCE, BUFFER, STREAM, VT, ANSWERS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x1800, RAM + 0x1900, RAM + 0x1a00
READ, ALLOC, EXIT, EMIT = 2, 10, 11, 12


def case(r):
    op = r.deal(OPS)
    log = log_region()
    cinfo = bytearray(r.bytes(0x40))
    put(cinfo, 0, 'II', ERR, MEM)
    source = bytearray(r.bytes(0x28))
    put(source, 0x1c, 'II', STREAM, BUFFER)
    source[0x24] = r.choice([0, 1, 0x80])
    put(cinfo, 0x18, 'I', SOURCE)
    regions = {LOG: log, CINFO: cinfo, SOURCE: source, BUFFER: r.bytes(0x20), ERR: put(bytearray(r.bytes(0x20)), 0, 'I', recorder(EXIT, 1, cdecl=True)), MEM: struct.pack('<I', recorder(ALLOC, 3, cdecl=True)),
               STREAM: struct.pack('<I', VT), VT: vtable({2: 2})}
    put(regions[ERR], 4, 'I', recorder(EMIT, 2, cdecl=True))
    if op == 'Set':
        put(cinfo, 0x18, 'I', r.choice([0, SOURCE]))
        regions[ANSWERS] = struct.pack('<2I', SOURCE, BUFFER) + bytes(38 * 4)
        answers(log, ALLOC, ANSWERS)
        return Call(ADDRESS[op], [CINFO, STREAM], regions, conv='c', ret='v')
    if op == 'Init':
        return Call(ADDRESS[op], [CINFO], regions, conv='c', ret='v')
    if op == 'Fill':
        set_return(log, READ, r.choice([0, 0, 1, 0x1000, 0xffffffff]))
        return Call(ADDRESS[op], [CINFO], regions, conv='c', ret='b')
    avail = r.choice([0, 1, 5, 0x1000, -1])
    put(source, 4, 'i', avail)
    put(source, 0, 'I', BUFFER)
    count = r.choice([0, -1, 1, avail, avail + 1, avail + 0x1000, avail + 0x2800, -0x80000000])
    regions[ANSWERS] = struct.pack('<40I', *[r.choice([0x1000, 0x1000, 0x800, 0]) for _ in range(40)])
    answers(log, READ, ANSWERS)
    return Call(ADDRESS[op], [CINFO, count & 0xffffffff], regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)
