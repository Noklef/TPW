# The clock's month, year and tick (src/game/core/funnytime: 0x4f8770, 0x4f85a0, 0x4f8260) against the originals, on funnytime.py's clock (its imports and the
# world's ticks). FileTimeToSystemTime always succeeds (a failure leaves stack garbage in the month or the year: both sides unset, sic) and fills any year and day
# with a month of 1-12 mostly, any at times. The tick: the clock's +0x10, +0x14 and +0x18 equal to the date or not (each message and call on its own); the message
# centre made (the original's inlined making copies uninitialised stack bytes into each slot, sic); the post (0x40fb10, k 21: the message's 8 bytes logged),
# Obj7b51f0_ShowDate (0x481560, k 20), agent C's 0x4a5e10 (cdecl k 19), StatsGraph_Redraw (0x49b520, cdecl k 18) and Dialog7cb25c_Refresh (0x4a0770, cdecl k 17) as recorders.
# usage: funnyupdate.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Quarter,Sync,Tick)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, Bench, Call, _recorder, blob_bytes, fill, put, set_return  # noqa: E402
import funnytime as ft  # noqa: E402
import thing_ctors  # noqa: E402

CALLS = {'Quarter': (0x4f8770, 'a'), 'Sync': (0x4f85a0, 'v'), 'Tick': (0x4f8260, 'v')}
B = Bench(ft.RANGES + thing_ctors.CENTRE_RANGES + [(0x4f8770, 0x4f87fc), (0x4f85a0, 0x4f868b), (0x4f8260, 0x4f8562), (0x40f4c0, 0x40f580)],
          hooks={**thing_ctors.CENTRE_HOOKS, 0x40fb10: _recorder(21, 1), 0x481560: _recorder(20, 9), 0x4a5e10: _recorder(19, 1, cdecl=True),
                 0x49b520: _recorder(18, 1, cdecl=True), 0x4a0770: _recorder(17, 1, cdecl=True)})
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
K = ft.K['FileTimeToSystemTime']


def date(r, log):
    """The conversion's answer, succeeding; returns the SYSTEMTIME's year, month and day."""
    st = bytearray(r.bytes(16))
    year, month, day = r.choice([2000, 2001, r.getrandbits(16)]), r.choice([r.randint(1, 12), r.randint(1, 12), r.randint(1, 12), r.getrandbits(16)]), r.randint(1, 31)
    struct.pack_into('<HH', st, 0, year, month)
    struct.pack_into('<H', st, 6, day)
    fill(log, K, 1, bytes(st), 0)
    set_return(log, K, r.choice([1, r.getrandbits(32) | 1]))
    return year, month, day


def clock_case(r, regions):
    clock = bytearray(r.bytes(0x40))
    put(clock, 0x1c, 'I', ft.u32(r))
    regions[ft.CLOCK] = clock
    return clock


def case(r):
    name = r.deal(OPS)
    address, ret = CALLS[name]
    regions = {ft.OUT: bytearray(r.bytes(0x40))}
    log = ft.clock_regions(r, regions)
    clock = clock_case(r, regions)
    year, month, day = date(r, log)
    if name == 'Tick':
        put(clock, 0x10, 'I', r.choice([month - 1, month - 1, r.getrandbits(32)]))
        put(clock, 0x14, 'I', r.choice([day, day, r.getrandbits(32)]))
        put(clock, 0x18, 'I', r.choice([year, year, r.getrandbits(32)]))
        regions[0x7ced58] = r.bytes(0x24)
        while regions.get(0x788d3c, b'') in (b'', bytes(4)):   # (made already)
            thing_ctors.centre(r, regions, empty=True)
        blob_bytes(regions[LOG], 21, 0, 0xff)
    return Call(address, [ft.CLOCK], regions, conv='t', ret=ret)


if __name__ == '__main__':
    B.main(case)
