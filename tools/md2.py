"""Theme Park World .md2 meshes (version 221; NOT Quake MD2) -> Wavefront OBJ.

A .md2 is a serialised C struct: pointer fields hold file offsets, which MD2_Load
(0x46d6d0) relocates by adding the load address. Header (u32 words unless noted):
  [0x00] magic 0x1CD15D46   [0x01] version 221   [0x02] sub-version (203)
  [0x0c] flags: bit 0 = has geometry tables; 0x10/0x20 = motion data only (block at [0x26])
  u16 0x36 material count   u16 0x42 node count (all)   u16 0x44 geometry nodes
  [0x15] materials (16 B: u32 flags, u32 texture handle, u16, u16, char *texture name)
  [0x16] positions   [0x17] normals   [0x18] UVs   [0x1a] triangles   [0x1c] nodes
  [0x20..0x25] bounding box (min xyz, max xyz floats)

Geometry node (160 B, table at [0x1c]); all pointers point into the shared tables above:
  +0x54 char *name    +0x58 u16 vertex count    +0x5a u16 group count   +0x5c u16 triangle count
  +0x60 positions: SoA blocks of 4 vertices: float X[4], Y[4], Z[4]   (48 B per block)
  +0x64 normals: 3 floats each; vertex normals first, then face normals
  +0x68 UVs: SoA blocks of 4: float U[4], V[4]                           (32 B per block)
  +0x6c groups (16 B)   +0x70 triangles: u16 face_normal, v0, v1, v2
  +0x78 node bounding box (6 floats)

Usage:
  python -I tools/md2.py <file.md2> [out.obj]          dump / export
  python -I tools/md2.py --check <dir-or-wad> ...       validate every geometry mesh
"""
import glob
import os
import struct
import sys

MAGIC = 0x1CD15D46
NODE_SIZE = 160


class Node:
    pass


class Mesh:
    def __init__(self, data):
        self.d = data
        w = struct.unpack_from('<45I', data, 0)
        if w[0] != MAGIC:
            raise ValueError('not a .md2 mesh')
        self.version, self.subversion, self.flags = w[1], w[2], w[0xc]
        self.material_count = self.u16(0x36)
        self.node_count = self.u16(0x44)
        self.bbox = struct.unpack_from('<6f', data, 0x80)
        self.materials = []
        for i in range(self.material_count if self.flags & 1 else 0):
            name_off = self.u32(w[0x15] + 16 * i + 12)
            self.materials.append(self.cstr(name_off) if name_off else '')
        self.nodes = [self._node(w[0x1c] + NODE_SIZE * i) for i in range(self.node_count if self.flags & 1 else 0)]

    def u16(self, o):
        return struct.unpack_from('<H', self.d, o)[0]

    def u32(self, o):
        return struct.unpack_from('<I', self.d, o)[0]

    def cstr(self, o):
        return self.d[o:self.d.index(b'\0', o)].decode('latin1')

    def _node(self, o):
        n = Node()
        n.offset = o
        n.name = self.cstr(self.u32(o + 0x54)) if self.u32(o + 0x54) else ''
        n.npositions, n.ngroups, n.ntris, n.nverts = (self.u16(o + k) for k in (0x58, 0x5a, 0x5c, 0x5e))
        pos, nrm, uv, tri = (self.u32(o + k) for k in (0x60, 0x64, 0x68, 0x70))
        n.positions = [self._soa(pos, 3, i) for i in range(n.npositions)]
        n.uvs = [self._soa(uv, 2, i) for i in range(n.nverts)] if uv else []
        vmap = self.u32(o + 0x94)
        n.vert_pos = list(struct.unpack_from(f'<{n.nverts}H', self.d, vmap)) if vmap else list(range(n.nverts))
        n.normals_offset = nrm
        n.triangles = [struct.unpack_from('<4H', self.d, tri + 8 * i) for i in range(n.ntris)] if tri else []
        n.bbox = struct.unpack_from('<6f', self.d, o + 0x78)
        return n

    def _soa(self, table, dims, i):
        """Element i of a structure-of-arrays table stored in blocks of 4."""
        b, j = divmod(i, 4)
        return tuple(struct.unpack_from('<f', self.d, table + 16 * dims * b + 16 * axis + 4 * j)[0]
                     for axis in range(dims))

    def to_obj(self):
        out = [f'# Theme Park World .md2 v{self.version}, {len(self.nodes)} nodes']
        vbase = tbase = 1
        for n in self.nodes:
            out.append(f'o {n.name or "node_%x" % n.offset}')
            out += [f'v {x:.6f} {y:.6f} {z:.6f}' for x, y, z in n.positions]
            out += [f'vt {u:.6f} {1 - v:.6f}' for u, v in n.uvs]
            for _, *corners in n.triangles:
                if n.uvs:
                    out.append('f ' + ' '.join(f'{n.vert_pos[c] + vbase}/{c + tbase}' for c in corners))
                else:
                    out.append('f ' + ' '.join(f'{n.vert_pos[c] + vbase}' for c in corners))
            vbase += n.npositions
            tbase += len(n.uvs)
        return '\n'.join(out) + '\n'

    def problems(self):
        """Consistency checks used to validate the format hypotheses."""
        errs = []
        for n in self.nodes:
            for t in n.triangles:
                if max(t[1:]) >= n.nverts:
                    errs.append(f'{n.name}: vertex index {max(t[1:])} >= {n.nverts}')
                    break
            if n.vert_pos and max(n.vert_pos) >= n.npositions:
                errs.append(f'{n.name}: position index {max(n.vert_pos)} >= {n.npositions}')
            lo, hi = n.bbox[:3], n.bbox[3:]
            for p in n.positions:
                if any(p[k] < lo[k] - 1e-3 or p[k] > hi[k] + 1e-3 for k in range(3)):
                    errs.append(f'{n.name}: vertex {p} outside node bbox {n.bbox}')
                    break
        return errs


def meshes_in(path):
    if path.lower().endswith('.wad'):
        sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
        from wadtool import read_wad
        for name, data, _ in read_wad(path):
            if name.lower().endswith('.md2'):
                yield f'{path}:{name}', data
    elif os.path.isdir(path):
        for p in glob.glob(os.path.join(path, '**', '*'), recursive=True):
            if p.lower().endswith(('.md2', '.wad')):
                yield from meshes_in(p)
    else:
        yield path, open(path, 'rb').read()


if __name__ == '__main__':
    if sys.argv[1] == '--check':
        ok = bad = skipped = 0
        for arg in sys.argv[2:]:
            for name, data in meshes_in(arg):
                try:
                    m = Mesh(data)
                except ValueError:
                    continue
                if m.version != 221 or not m.flags & 1:
                    skipped += 1
                    continue
                try:
                    errs = m.problems()
                except Exception as e:  # noqa: BLE001
                    errs = [f'{type(e).__name__}: {e}']
                if errs:
                    bad += 1
                    if bad <= 15:
                        print('BAD', name, errs[0])
                else:
                    ok += 1
        print(f'{ok} geometry meshes consistent, {bad} inconsistent, {skipped} skipped (old version / motion-only)')
    else:
        m = Mesh(open(sys.argv[1], 'rb').read())
        print(f'version {m.version}.{m.subversion} flags {m.flags:#x} materials {m.materials}')
        for n in m.nodes:
            print(f'  node {n.name!r}: {n.nverts} verts, {n.ntris} tris, {n.ngroups} groups')
        if len(sys.argv) > 2:
            open(sys.argv[2], 'w').write(m.to_obj())
