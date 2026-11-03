# One material group of a model's mesh into a batch (src/game/render/modelgroup: 0x580530) against the original: TexSet_Texture, Vec3_DotByValue, Tri_WindingTest and the clip codes run as they are;
# Model_TexCoords (k 1), FixedVerts_Modulate (k 2), Batch_FogVertices (k 4), ClipPoly_TriangleFan (k 5, stdcall), the transformer's slot 3 (k 3, a float) and the texture object's slot 3 (k 7) are
# recorders. The world: a texture none, without an id, plain or sphere mapped, a second layer now and then, faces within the group's vertices (every clip code read was written), clip flags or
# none, a code table empty, sparse or dense, the room exact now and then so that the size check and the clipped loop's limits both trip.
# usage: modelgroup.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, recorder, set_float_return, set_return, vtable  # noqa: E402

B = Bench([(0x580530, 0x5812f0), (0x67b110, 0x67b13f), (0x5812f0, 0x581315), (0x581320, 0x581403), (0x575f90, 0x575f9d), (0x561bc0, 0x561c3c)],
          hooks={0x581410: _recorder(1, 6, cdecl=True), 0x5630b0: _recorder(2, 4, cdecl=True), 0x565e70: _recorder(4, 2), 0x562ed0: _recorder(5, 5, com=True)})
MODEL, SETTINGS, ENTRIES, MATERIAL, SET, ARR, LAYER, LSET, LARR = (RAM + k for k in (0, 0x100, 0x200, 0x280, 0x2a0, 0x2c0, 0x300, 0x320, 0x340))
TEX, TEX2, OBJ, OVT, OPEN, REND, BATCH, TVT, XF, VIEW, MASK = (RAM + k for k in (0x400, 0x500, 0x600, 0x610, 0x640, 0x680, 0x6c0, 0x700, 0x780, 0x7a0, 0x7e0))
NORMALS, BLOCKS, VERTS, COLOURS, TABLE, FACES, OUT, INDICES = (RAM + k for k in (0x1000, 0x2000, 0x3000, 0x4000, 0x5000, 0x5800, 0x10000, 0x18000))
POINTS, VERT_ROOM, INDEX_ROOM = 32, 0x4000, 0x2000


def texture(r, wild):
    tex = bytearray(r.bytes(0x100))
    flags = r.getrandbits(32) & ~0x18001400
    flags |= r.choice([0, 0x1000, 0x1000, 0x1000]) | r.choice([0, 0x8000000]) | r.choice([0, 0x400]) | r.choice([0, 0x10000000]) | r.choice([0, 0x4000000])
    put(tex, 8, 'I', flags)
    put(tex, 0x1c, 'I', r.choice([0, 7, r.getrandbits(32)]))
    for off in (0x20, 0x24, 0x28, 0x2c):
        put(tex, off, 'I' if wild else 'f', r.getrandbits(32) if wild else r.uniform(-2, 2))
    put(tex, 0x3c, 'I', r.choice([0, 0, 5]))
    put(tex, 0x40, 'I', OBJ)
    put(tex, 0x68, 'I', r.getrandbits(32) & ~0x801000 | r.choice([0, 0x1000]) | r.choice([0, 0x800000]))
    put(tex, 0xc8, 'I', r.getrandbits(32) & ~0x1000 | r.choice([0, 0x1000]))
    return tex


def case(r):
    log = log_region()
    stride = r.choice([0x20, 0x24, 0x28])
    clip = r.choice([0, 0, 1, 0x3f, 0x15])
    first = r.choice([0, 0, 1, 2, 5, 20])
    count = r.choice([1, 1, 2, 3, 5, 9, 16] + ([] if clip else [0]))
    first = first or (0 if count else 1)  # (last is first - 1: for first 0 the word wraps to 0xffff, 64 KB of vertices)
    faces = r.choice([0, 1, 2, 4, 8, 16]) if count else 0
    first_face = r.choice([0, 0, 3])
    group = r.randrange(4)
    colours = r.random() < 0.7

    model = bytearray(r.bytes(0x14))
    put(model, 4, 'I', SETTINGS)
    put(model, 8, 'I', r.getrandbits(32) & ~0x200003f | clip | r.choice([0, 0x2000000]))
    put(model, 0xc, 'I', VERTS)
    put(model, 0x10, 'I', COLOURS if colours else 0)

    mesh_faces = faces + r.randrange(10)
    mesh_points = count + r.randrange(22)
    b20, b24 = r.choice([0, 0, 1, 7]), r.choice([0, 0, 3, 9])
    open_room = r.choice([0, 0, 1])
    layer = r.random() < 0.3
    settings = bytearray(r.bytes(0xa0))
    put(settings, 0, 'I', r.getrandbits(32) & ~0x2000000 | (0x2000000 if layer else 0))
    put(settings, 0x5c, 'HH', mesh_faces, mesh_points)
    put(settings, 0x64, 'IIII', NORMALS, BLOCKS, ENTRIES, FACES)
    put(settings, 0x94, 'I', TABLE)
    put(settings, 0x98, 'I', LAYER if r.random() > 0.05 else 0)

    entries = bytearray(r.bytes(0x40))
    has_material = r.random() > 0.05
    put(entries, group * 16, 'I', MATERIAL if has_material else 0)
    put(entries, group * 16 + 4, 'HHHH', first_face, faces, first, (first + count - 1) & 0xffff)
    put(entries, group * 16 + 12, 'I', r.choice([0, 0, 1, 0xfffffffe]))
    number = r.randrange(4)
    material = bytearray(r.bytes(8))
    put(material, 2, 'H', number)
    put(material, 4, 'I', SET if r.random() > 0.05 else 0)
    arr = [TEX if r.random() > 0.05 else 0 for _ in range(4)]

    wild = r.random() < 0.03
    normals = bytearray(POINTS * 12)
    r.floats(normals, range(0, POINTS * 12, 4), 2.0)
    blocks = bytearray(((first + count) // 4 + 1) * 32)
    r.floats(blocks, range(0, len(blocks), 4), 4.0)
    verts = bytearray(POINTS * 16)
    r.floats(verts, range(0, POINTS * 16, 4), 50.0)
    view = bytearray(0x40)
    r.floats(view, range(0, 0x40, 4), 2.0)
    table = bytearray()
    for _ in range(first + count + 1):
        table += struct.pack('<H', r.randrange(POINTS))
    face_bytes = bytearray(r.bytes((first_face + faces + 1) * 8))
    for k in range(first_face, first_face + faces):
        put(face_bytes, k * 8 + 2, 'HHH', *(first + r.randrange(count) for _ in range(3)))

    density = r.choice([0, 0, 0.1, 0.5, 1])
    codes = bytes(r.getrandbits(8) if r.random() < density else 0 for _ in range(0x80))
    if r.random() < 0.05:
        codes = bytes([0xff] * 0x80)

    # The room: plenty, or exact for the mesh (the group's limits, ten vertices and 0x30 bytes of index short of it, can then trip).
    point_end = (mesh_points + open_room + b20) * stride + OUT
    index_end = INDICES + 2 * (mesh_faces * 3 + open_room + b24)
    kind = r.choice(['plenty', 'plenty', 'plenty', 'exact', 'exact', 'short vertices', 'short indices'])
    room_points = OUT + VERT_ROOM if kind == 'plenty' else point_end - (stride if kind == 'short vertices' else 0)
    room_indices = INDICES + INDEX_ROOM if kind == 'plenty' else index_end - (2 if kind == 'short indices' else 0)
    rend = bytearray(r.bytes(0x40))
    put(rend, 0x1c, 'I', stride)
    put(rend, 0x20, 'I', OPEN)
    put(rend, 0x28, 'II', room_points, room_indices)
    opened = bytearray(r.bytes(0x30))
    put(opened, 0x20, 'IIII', open_room, open_room, OUT, INDICES)
    batch = bytearray(r.bytes(0x40))
    put(batch, 4, 'I', r.getrandbits(32) & ~0x800000 | r.choice([0, 0x800000]))
    put(batch, 0x20, 'IIII', b20, b24, OUT, INDICES)
    put(batch, 0x30, 'f', r.uniform(-50, 50))

    set_float_return(log, 3, r.uniform(-20, 20))
    set_return(log, 5, INDICES + b24 * 2 + r.choice([0x6, 0x40, 0x200]))

    regions = {LOG: log, MODEL: model, SETTINGS: settings, ENTRIES: entries, MATERIAL: material, SET: struct.pack('<3I', 0, 0, ARR), ARR: struct.pack('<4I', *arr),
               LAYER: struct.pack('<II', 0, LSET), LSET: struct.pack('<3I', 0, 0, LARR), LARR: struct.pack('<I', TEX2 if r.random() > 0.01 else 0),
               TEX: texture(r, wild), TEX2: texture(r, wild), OBJ: struct.pack('<I', OVT), OVT: struct.pack('<4I', 0, 0, 0, recorder(7, 0)),
               OPEN: opened, REND: rend, BATCH: batch, TVT: vtable({}, cdecl={3: 5}), XF: struct.pack('<I', TVT), VIEW: view, MASK: r.bytes(0x30),
               NORMALS: normals, BLOCKS: blocks, VERTS: verts, COLOURS: r.bytes((first + count) * 4), TABLE: table, FACES: face_bytes,
               OUT: r.bytes(VERT_ROOM), INDICES: r.bytes(INDEX_ROOM),
               0x879ab0: struct.pack('<I', REND), 0x87afe8: struct.pack('<I', VIEW), 0x87a248: struct.pack('<I', MASK), 0x87b1d8: struct.pack('<I', XF), 0x8bd550: r.bytes(4),
               0x879a20: r.bytes(4), 0x879970: codes, 0x8c63b0: bytes(0x24), 0x8bcc14: struct.pack('<f', r.uniform(-0.1, 0.1))}
    return Call(0x580530, [MODEL, BATCH, group, stride], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
