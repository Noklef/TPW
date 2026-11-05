# The CPU probe (src/game/core/cpuinfo) against the original: CpuInfo_Ctor over a fake CPU (fncmp.cpu_region(), the emulator's CPUID and EFLAGS.ID,
# the driver's stub_cpuid / stub_has_cpuid): EFLAGS.ID toggles or not; leaf 0 a vendor, leaf 1 a signature of family 0-15 (or 0) and features with
# the TSC bit or not, leaf 0x80000001 EDX with bit 31 or not. The object is random bytes (has3dNow is kept without CPUID; byte 0x1c is overwritten).
# CpuInfo_TicksPerMs over the fake CPU's stepping counter and a timeGetTime recorder answering from a table: no TSC; waits of 0 ms (no millisecond passes: the
# division faults in both), of 1 to 1000 ms and of any length; the clock wrapping past its end.
# hasTsc changing during the wait (the else arms of the original's two later tests) needs another thread: a recorder cannot write it, so those arms are not reached.
# usage: cpuinfo.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CPU, IMPORT_TABLE, LOG, RAM, Bench, Call, answers, cpu_region, import_table, log_region  # noqa: E402

B = Bench([(0x55e470, 0x55e561), (0x55e570, 0x55e5ec)])
THIS = RAM
TIMES = RAM + 0x100
TIME = 40


def ticks_per_ms(r):
    log = log_region()
    table = import_table(log, {'timeGetTime': (TIME, 0x6fd3ec, 0, 0, 0)})
    answers(log, TIME, TIMES)
    begin = r.choice([r.getrandbits(32), r.getrandbits(32), 0, 0xffffffff, 0xfffffff0])
    ms = r.choice([0, 1, 2, 5, 16, 1000, r.randrange(1, 100000), r.getrandbits(32)])
    times = [begin]
    steps = r.randrange(1, 12)
    # The clock's reads up to the end, and past it by a little or not at all.
    times += [(begin + ms * i // steps + r.choice([0, 0, 1])) & 0xffffffff for i in range(1, steps)]
    times += [(begin + ms + r.choice([0, 0, 1, 7])) & 0xffffffff] * (40 - len(times))
    obj = bytearray(r.bytes(0x40))
    struct.pack_into('<I', obj, 8, r.choice([1, 1, 1, 1, r.getrandbits(32) | 1, 0]))
    regions = {LOG: log, IMPORT_TABLE: table, THIS: obj, TIMES: struct.pack('<40I', *times),
               CPU: cpu_region(1, [], r.getrandbits(32), r.choice([1, 3, 1000, r.getrandbits(32), r.randrange(1 << 20)]))}
    return Call(0x55e570, [THIS, ms], regions, conv='t', ret='a')


def case(r):
    if r.deal(['ctor', 'ticks']) == 'ticks':
        return ticks_per_ms(r)
    sig = r.choice([0, 0, r.getrandbits(32), (r.choice([4, 5, 6, 0xf, r.getrandbits(32) & 0xf]) << 8) | (r.getrandbits(32) & 0xfffff0ff)])
    features = r.getrandbits(32) | 0x10 if r.choice([0, 1]) else r.getrandbits(32) & ~0x10
    ext = r.getrandbits(32) | 0x80000000 if r.choice([0, 1]) else r.getrandbits(32) & 0x7fffffff
    leaves = [(r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)), (sig, r.getrandbits(32), r.getrandbits(32), features), (r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), ext), (r.getrandbits(32),) * 4]
    obj = r.bytes(0x40)
    return Call(0x55e470, [THIS], {THIS: obj, CPU: cpu_region(r.choice([1, 1, 1, 0]), leaves)}, conv='t')


if __name__ == '__main__':
    B.main(case)
