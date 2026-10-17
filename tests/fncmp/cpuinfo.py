# The CPU probe (src/game/core/cpuinfo) against the original: CpuInfo_Ctor over a fake CPU (fncmp.cpu_region(), the emulator's CPUID and EFLAGS.ID,
# the driver's stub_cpuid / stub_has_cpuid): EFLAGS.ID toggles or not; leaf 0 a vendor, leaf 1 a signature of family 0-15 (or 0) and features with
# the TSC bit or not, leaf 0x80000001 EDX with bit 31 or not. The object is random bytes (has3dNow is kept without CPUID; byte 0x1c is overwritten).
# usage: cpuinfo.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CPU, RAM, Bench, Call, cpu_region  # noqa: E402

B = Bench([(0x55e470, 0x55e561)])
THIS = RAM


def case(r):
    sig = r.choice([0, 0, r.getrandbits(32), (r.choice([4, 5, 6, 0xf, r.getrandbits(32) & 0xf]) << 8) | (r.getrandbits(32) & 0xfffff0ff)])
    features = r.getrandbits(32) | 0x10 if r.choice([0, 1]) else r.getrandbits(32) & ~0x10
    ext = r.getrandbits(32) | 0x80000000 if r.choice([0, 1]) else r.getrandbits(32) & 0x7fffffff
    leaves = [(r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)), (sig, r.getrandbits(32), r.getrandbits(32), features), (r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), ext), (r.getrandbits(32),) * 4]
    obj = r.bytes(0x40)
    return Call(0x55e470, [THIS], {THIS: obj, CPU: cpu_region(r.choice([1, 1, 1, 0]), leaves)}, conv='t')


if __name__ == '__main__':
    B.main(case)
