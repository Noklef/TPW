# The C library's multibyte strings (src/game/core/crtmbstring) against the originals: LCMapStringA a COM recorder in its import slot, the locks cdecl recorders.
# usage: crt_mbstring.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, fill, import_table, log_region, set_return  # noqa: E402

LOCK, UNLOCK, LCA = 7, 8, 3
B = Bench([(0x6895b0, 0x68964e), (0x689650, 0x6896fd), (0x689400, 0x689490), (0x688f50, 0x689175), (0x689180, 0x6891af), (0x67d7f0, 0x67d82a), (0x67b5f0, 0x67b61e)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True)})
TEXT, SET = RAM, RAM + 0x100
OPS = [n for n in ('lower', 'lower', 'lower', 'pbrk', 'pbrk', 'pbrk', 'rchr', 'rchr', 'rchr') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
LEADS = (0x81, 0x82, 0xe0, 0xfc)


def text(r, longest):
    """Bytes from a small alphabet of lead bytes and others, so that matches happen; a lone lead byte at the end at times."""
    data = bytes(r.choice(LEADS + (0x41, 0x42, 0x61, 0xa0)) for _ in range(r.randrange(0, longest)))
    return data + r.choice([b'', b'', b'', bytes([r.choice(LEADS)])]) + b'\0'


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, IMPORT_TABLE: import_table(log, {'lca': (LCA, 0x6fd19c, 6, 0, 0b10100)})}
    types = bytearray(0x101)
    for i in range(1, 0x101):
        types[i] = r.choice([0, 0, 0x10, 0x20, 0x30, 0x14, 4])
    for lead in LEADS:
        types[lead + 1] = 4
    tables = bytes(types) + bytes(7) + r.bytes(0x100) + struct.pack('<2I', r.choice([0, 932, 932, 932]), r.choice([0, 0x411]))
    regions[0xfb1668] = tables
    regions[0xfb1554] = struct.pack('<I', 2)
    regions[TEXT] = text(r, 12) + bytes(4)
    if name == 'lower':
        set_return(log, LCA, r.choice([0, 2, 2, 2, 1]))
        fill(log, LCA, 4, bytes([r.randrange(256), r.randrange(256)]))
        high = r.choice(LEADS + (0x41, 0, 0xa0))
        c = r.choice([r.randrange(0x100), r.randrange(0x100), high << 8 | r.randrange(256), high << 8 | r.randrange(256), 0x10000 | high << 8, r.getrandbits(32)])
        return Call(0x6895b0, [c], regions, conv='c', ret='a')
    if name == 'pbrk':
        regions[SET] = text(r, 6) + bytes(4)
        return Call(0x689650, [TEXT, SET], regions, conv='c', ret='a')
    data = bytes(regions[TEXT])
    picks = [data[0] if data else 0, 0, 0x41, 0x8141, 0x8181, r.getrandbits(32)]
    if len(data) > 1:
        picks += [data[0] << 8 | data[1]] * 3
    return Call(0x689400, [TEXT, r.choice(picks)], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
