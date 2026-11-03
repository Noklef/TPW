# The frame's effect textures again (src/game/render/weatherfx: WeatherFx_ReloadEffects, 0x550950) against the original: a renderer or none, a queue or none, the folder's open and first file passing or failing.
# Recorders (driver.cpp's kWeatherFxReloadEnv): Quickload_Locate 1, WeatherFx_LoadTexture 2, SpriteQueue_LoadTexture 3, MemorySource_GetLock 4, the local handle's Ctor 5, Open28 6, Vfn1 7, Vfn8 8, Dtor 9 (`this` cleared on both
# sides: it is a stack address), ColourRamps_SetFromPalette 10.
# usage: weatherreload.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob_bytes, echo, fill, log_region, put, set_return, strings  # noqa: E402


def thisless(k, n):
    record = _recorder(k, n)

    def hook(e):
        e.r['ECX'] = 0
        return record(e)
    return hook


HOOKS = {0x44a280: _recorder(1, 3, cdecl=True), 0x57d7f0: _recorder(2, 2), 0x58a5c0: _recorder(3, 1), 0x619240: _recorder(4, 0), 0x5f8240: thisless(5, 0), 0x5f8510: thisless(6, 2), 0x5f8370: thisless(7, 3), 0x5f8480: thisless(8, 2),
         0x5f82a0: thisless(9, 0), 0x409db0: _recorder(10, 0, cdecl=True)}
B = Bench([(0x550950, 0x550aa0)], hooks=HOOKS)
LOCK, FX, QUEUE = RAM, RAM + 0x20, RAM + 0x40


def case(r):
    log = log_region()
    strings(log, 1, 1)
    set_return(log, 1, r.getrandbits(32))
    set_return(log, 4, LOCK)
    for k, which in ((6, 0), (7, 1)):
        echo(log, k, 0)
        fill(log, k, 0, struct.pack('<i', r.choice([0, 1, 5, -1, -0x7fffffff])), which=which)
    strings(log, 6, 0, out=1)
    strings(log, 7, 0, out=1)
    strings(log, 8, 0, out=1)
    blob_bytes(log, 7, 2, 0, inner=(0x20, 1))
    regions = {LOCK: r.bytes(0x10), LOG: log, 0x8bcbac: struct.pack('<I', FX if r.random() < 0.6 else 0), 0x8c47a0: struct.pack('<I', QUEUE if r.random() < 0.6 else 0), 0x878ec0: r.bytes(4)}
    return Call(0x550950, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
