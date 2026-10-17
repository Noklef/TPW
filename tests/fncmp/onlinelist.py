# The online lists' requests of src/game/online/onlinelist against the originals: OutputDebugStringA is a recorder logging its text, the City methods
# (WEACITYR imports, __thiscall) recorders logging the city (ECX) and their arguments and answering a random value; the list's +0x34 / +0x3c and the
# request random.
# usage: onlinelist.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, Bench, Call, RAM, log_region, recorder, set_return, strings  # noqa: E402

# name: (address, City method's import slot, its stack arguments, list method?)
CALLS = {'Cities': (0x5bc910, 0x6fd55c, 3, 0), 'Themes': (0x5bd180, 0x6fd548, 3, 0), 'Rides': (0x5bda10, 0x6fd544, 3, 0),
         'Servers': (0x5bd5c0, 0x6fd530, 4, 0), 'InCity': (0x5bcb30, 0x6fd558, 4, 1), 'Top10': (0x5bcb90, 0x6fd554, 4, 1),
         'ByName': (0x5bcce0, 0x6fd550, 4, 1), 'ById': (0x5bce30, 0x6fd54c, 4, 1)}
B = Bench([(a, a + 0x30) for (a, _s, _n, _l) in CALLS.values()])
OUTPUT, OUTPUT_SLOT, CITY_SLOT = 0x6fd1e0, 1, 2
LIST, CITY = RAM, RAM + 0x100


def case(r):
    op = r.deal(OPS)
    address, slot, n, method = CALLS[op]
    log = log_region()
    table = bytearray(0x560)
    struct.pack_into('<I', table, OUTPUT - IMPORT_TABLE, recorder(OUTPUT_SLOT, 1, True))
    strings(log, OUTPUT_SLOT, 1)
    struct.pack_into('<I', table, slot - IMPORT_TABLE, recorder(CITY_SLOT, n))
    set_return(log, CITY_SLOT, r.getrandbits(32))
    regions = {LOG: log, IMPORT_TABLE: table, LIST: r.bytes(0x40), CITY: r.bytes(0x40)}
    request = r.getrandbits(32)
    if method:
        return Call(address, [LIST, CITY, request], regions, conv='t', ret='a')
    return Call(address, [CITY, request], regions, conv='s', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
