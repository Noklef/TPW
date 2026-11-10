# The C library's multibyte code page set-up (src/game/core/crtmbstring): _setmbcp 0x689c60, __initmbctable 0x68a160, setSBUpLow 0x689f80, against the originals, over getSystemCP,
# setSBCS, CPtoLCID and __crtGetStringTypeA / __crtLCMapStringA (their A paths: [0xfb1890] and [0xfb1554] 2). The locks are cdecl recorders; the imports COM recorders in their slots:
# GetCPInfo fills its structure; GetStringTypeA its 0x100 words; LCMapStringA bytes 0-0x6f (the lower and upper case calls each their own; the types of bytes past them have no
# case, so the rest is never read); the bytes they are given are logged as a hash.
# usage: crt_mbctype.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, blob_array, fill, import_table, log_region, set_return  # noqa: E402

LOCK, UNLOCK = 7, 8
CPINFO, GSTA, LCMA, OEMCP, ACP = 14, 15, 16, 17, 18
B = Bench([(0x689c60, 0x689e8c), (0x689f80, 0x68a15a), (0x68a160, 0x68a16b), (0x689e90, 0x689edd), (0x689ee0, 0x689f18), (0x689f40, 0x689f72), (0x68a5d0, 0x68a708), (0x688f50, 0x689175), (0x689180, 0x6891af)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True)})
ANSWERS = RAM
TABLE = bytes.fromhex('0102040800000000a4030000608279822100000000000000a6df000000000000a1a5000000000000819fe0fc00000000407e80fc00000000a8030000c1a3daa3200000000000000000'
                      '0000000000000000000000000081fe00000000000040fe000000000000b5030000c1a3daa320000000000000000000000000000000000000000000000081fe00000000000041fe00000000'
                      '0000b6030000cfa2e4a21a00e5a2e8a25b000000000000000000000000000000000081fe000000000000407ea1fe000000005105000051da5eda20005fda6ada3200000000000000000000'
                      '0000000000000081d3d8dee0f90000317e81fe00000000')
PAGES = (932, 936, 949, 950, 1361)
KINDS = {'setmbcp': 3, 'init': 1, 'sbuplow': 2}
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k for _ in range(KINDS[k])]


def lead_bytes(r):
    pairs = b''
    for _ in range(r.choice([0, 1, 1, 2, 3, 5])):
        first = r.choice([0x81, 0xa1, 0xe0, r.randrange(1, 256)])
        pairs += bytes([first, r.choice([0xfe, 0x9f, first, max(first - 1, 0), 0, r.randrange(256)])])
    return (pairs + bytes(12))[:12]


def case(r):
    name = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, IMPORT_TABLE: import_table(log, {'info': (CPINFO, 0x6fd180, 2, 0, 0b10), 'types': (GSTA, 0x6fd1f0, 5, 0, 0b10000), 'map': (LCMA, 0x6fd19c, 6, 0, 0b10000),
                                                          'oem': (OEMCP, 0x6fd1f4, 0, 0, 0), 'acp': (ACP, 0x6fd17c, 0, 0, 0)})}
    blob_array(log, GSTA, 2, 3, 1, which=0)
    blob_array(log, LCMA, 2, 3, 1, which=1)
    regions[ANSWERS] = struct.pack('<40I', *[r.choice([1, 1, 1, 0, 2]) for _ in range(40)])
    answers(log, CPINFO, ANSWERS)
    set_return(log, GSTA, r.getrandbits(32))
    set_return(log, LCMA, r.getrandbits(32))
    set_return(log, OEMCP, r.choice([932, 437, 0, r.getrandbits(32)]))
    set_return(log, ACP, r.choice([1252, 936, 949, 0, r.getrandbits(32)]))
    fill(log, CPINFO, 1, struct.pack('<I', r.choice([1, 2, 2, 0, 3, 0x80000000])) + r.bytes(2) + lead_bytes(r) + r.bytes(2), which=0)
    types = b''.join(struct.pack('<H', r.choice([0, 1, 2, 3, 0x100, 0x201]) if i < 0x70 else r.choice([0, 0x100, 0x204, 0x8008])) for i in range(0x100))
    for w in range(5):
        fill(log, GSTA, 4, types[0x70 * w:0x70 * w + 0x70], which=1 + w, offset=0x70 * w)
    fill(log, LCMA, 4, r.bytes(0x70), which=6, records=2)
    fill(log, LCMA, 4, r.bytes(0x70), which=7, offset=0x38, records=2)
    page = r.choice([0, 0, 932, 932, 1252, r.getrandbits(32)])
    regions[0xfb1668] = r.bytes(0x208) + struct.pack('<I', page) + r.bytes(0x10) + struct.pack('<I', r.choice([0, 1]))
    regions[0xfb3b84] = r.bytes(4)
    regions[0xfb1440] = r.bytes(4)
    regions[0xfb1450] = struct.pack('<I', r.choice([932, 1252, 850]))
    regions[0xfb1554] = struct.pack('<I', 2)
    regions[0xfb1890] = struct.pack('<I', 2)
    table = bytearray(TABLE)
    if r.random() < 0.3:
        for i in range(5):
            struct.pack_into('<I', table, 8 + 0x30 * i, r.choice(PAGES + (1252, r.getrandbits(32))))
    if r.random() < 0.3:
        table[0:4] = bytes(r.choice([1, 2, 4, 8, 0x10, r.randrange(256)]) for _ in range(4))
    regions[0x781588] = bytes(table)
    if name == 'setmbcp':
        return Call(0x689c60, [r.choice([0xfffffffe, 0xfffffffd, 0xfffffffd, 0xfffffffc, 0, 0, 1252, 437, page, page, r.getrandbits(32)] + list(PAGES))], regions, conv='c', ret='a')
    if name == 'init':
        return Call(0x68a160, [], regions, conv='c', ret='a')
    return Call(0x689f80, [], regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)
