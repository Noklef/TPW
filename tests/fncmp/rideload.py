# The world a ride's (or the advisor's) model load runs in, for benches to import (modelslot_load.py's): Quickload_LoadRide (0x461a60;
# quickloadride.py), RideModel_Load (0x461f10; ridemodel.py) and Instance_Clone (0x4557c0; animatormesh.py) one after the other.
# texdirs.vfs_case()'s mounts, quickload tables and texture directories, with the files the load opens served by name
# (vfs.memory_files()): the located path ("folder\name"), the model (ridemodel.ride_model()) at the texture directory LoadRide sets (the
# located directory [0x79cb4c][0] + folder + "\") and its motions (ridemodel.motions()), and with the second directory in use (0x7a3d3c,
# now and then) the two qickload.txt texts (quickloadride.quick_text(): the indexes then loaded, so the motions are asked of them).
# Instance_Clone's Holder_Create and the mesh's texture directories go through the mesh env: logged (animatormesh.HOOKS, as the driver's
# AnimatorMeshEnv).
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import animatormesh  # noqa: E402
import files  # noqa: E402
import quickloadride  # noqa: E402
import ridemodel  # noqa: E402
import texdirs  # noqa: E402
import vfs  # noqa: E402
from fncmp import HEAP, LOG, heap, put  # noqa: E402

RANGES = texdirs.merged(quickloadride.RANGES + ridemodel.RANGES + animatormesh.RANGES + animatormesh.CLONE_RANGES
                        + [(0x56a770, 0x56ae60)])   # (DxResult_Message: the part's failures logged)
HOOKS = dict(animatormesh.HOOKS)
DIRECTORIES, SECOND = quickloadride.DIRECTORIES, quickloadride.SECOND
# The model's (count, offset, record size) arrays Holder_Clone copies (md2file.py's leaves: any offset).
COPIED = [(0x36, 0x50, 8), (0x38, 0x58, 0x30), (0x3a, 0x60, 0x20), (0x3c, 0x64, 0x10), (0x3e, 0x68, 8), (0x4a, 0x5c, 12)]


def world(r, name, folder, flags, sure=False):
    """The regions for loading name (bytes, its ".md2" included) from folder (bytes, not empty) with RideModel_Load's flags. Returns
    (regions, located): located whether the located path is there (LoadRide fails without it, and the texture directories stay
    texdirs.vfs_case()'s). sure: the load always works (the located path and the model there, its magic and versions right and no
    motion block) and the model clones (COPIED's arrays inside the file or none, the +0x64 one the nodes' groups when there are any),
    for callers that use the model unchecked."""
    regions, root = texdirs.vfs_case(r, [0])
    r.root = root   # (holderload.model's material names)
    log = regions[LOG]
    regions[HEAP] = heap(0x80000)
    directory = r.choice([root + b'\\', b'C:\\TP\\' + root + b'\\', b'a\\', b'd\\x\\'])
    regions[DIRECTORIES] = struct.pack('<I', DIRECTORIES + 0x40) + r.bytes(0x3c) + directory + b'\0'
    regions[0x79cb4c] = struct.pack('<I', DIRECTORIES)
    regions[0x79c9d0] = r.bytes(0x17c)
    second = r.choice([b'second', b'Data\\B', b'x'])
    regions[SECOND] = second + b'\0'
    in_use = r.random() < 0.1
    regions[files.MARKS] = put(bytearray(regions[files.MARKS]), 0, 'I', int(in_use))
    found = {}
    located = sure or r.random() < 0.95
    if located:
        found[folder + b'\\' + name] = r.bytes(r.randrange(1, 9))
    first = directory + folder + (b'' if folder.endswith(b'\\') else b'\\')
    data, total, geo = ridemodel.ride_model(r, flags)
    if sure:
        data = bytearray(data)
        put(data, 0, 'III', 0x1cd15d46, 0xdd, 0xcb)
        put(data, 0x98, 'I', 0)   # (no motion block: flags 2 refuse one)
        for count, at, size in COPIED:   # (each array the clone copies small and inside the file, or none)
            if count in (0x38, 0x3c, 0x3e, 0x4a):   # (the header's random bytes otherwise: 0x36 and 0x3a are md2file's and holderload's)
                put(data, count, 'H', r.randrange(4))
            n = struct.unpack_from('<H', data, count)[0]
            put(data, at, 'I', r.choice([0, 8]) if 8 + n * size <= len(data) else 0)
        nodes, geo = struct.unpack_from('<I', data, 0x70)[0], struct.unpack_from('<H', data, 0x44)[0]
        groups = [(struct.unpack_from('<I', data, nodes + 0xa0 * i + 0x6c)[0], struct.unpack_from('<H', data, nodes + 0xa0 * i + 0x5a)[0])
                  for i in range(geo if nodes else 0)]
        groups = [(at, n) for at, n in groups if at and n]
        if groups:   # (the +0x64 array is the nodes' groups: the clone moves their pointers by it)
            low, high = min(at for at, _ in groups), max(at + 0x10 * n for at, n in groups)
            count = (high - low + 15) // 16
            data += bytes(max(0, low + 0x10 * count - len(data)))
            put(data, 0x64, 'I', low)
            put(data, 0x3c, 'H', count)
    if sure or r.random() < 0.95:
        found[ridemodel.main_path(name, first, flags)] = data
    ridemodel.motions(r, found, first + ridemodel.base_of(name), total, geo)
    if in_use:
        found[directory + b'\\' + folder + b'\\qickload.txt'] = quickloadride.quick_text(r)
        if r.random() < 0.7:
            found[directory + b'\\' + folder + b'\\' + second + b'\\qickload.txt'] = quickloadride.quick_text(r)
    vfs.memory_files(r, log, regions, found)
    ridemodel.model_globals(r, regions, flags)
    regions[0x7b4980] = put(bytearray(regions[0x7b4980]), 0, 'I', struct.unpack('<I', regions[0x7b4980])[0] & ~8)   # (MD2_Load's flags 2: the model's)
    return regions, located
