"""Theme Park World .md2 meshes (version 221; NOT Quake MD2) -> Wavefront OBJ.

A .md2 is a serialised C struct: pointer fields hold file offsets, which MD2_Load
(0x46d6d0) relocates by adding the load address. Header (u32 words unless noted):
  [0x00] magic 0x1CD15D46   [0x01] version 221   [0x02] sub-version (203)
  [0x0c] flags: bit 0 = has geometry tables; 0x10/0x20 = motion data only (block at [0x26],
         see tools/md2motion.py)
  u16 0x36 material count   u16 0x42 node count (all)   u16 0x44 geometry nodes
  [0x15] materials (16 B: u32 flags, u32 texture handle, u16, u16, char *texture name)
  [0x16] positions   [0x17] normals   [0x18] UVs   [0x1a] triangles   [0x1c] nodes
  [0x20..0x25] bounding box (min xyz, max xyz floats)

Transform node (88 B; dummies/bones in the table at [0x1d], count u16 0x42 - u16 0x44):
  +0x00 flags   +0x04 parent node   +0x10 float[16] local matrix, row-major, translation in
  the last row (D3D row-vector convention: model = local * parent * ...)   +0x50 index
  +0x54 char *name
Geometry node (160 B, table at [0x1c]) = transform node + mesh data. Pointers point into
the shared tables above:
  +0x58 u16 position count   +0x5a u16 group count   +0x5c u16 triangle count   +0x5e u16 render vertex count
  +0x60 positions: SoA blocks of 4 vertices: float X[4], Y[4], Z[4]   (48 B per block)
  +0x64 normals: 3 floats each; vertex normals first, then face normals
  +0x68 UVs: SoA blocks of 4: float U[4], V[4]                           (32 B per block)
  +0x6c groups (16 B)   +0x70 triangles: u16 face_normal, v0, v1, v2
  +0x78 node bounding box (6 floats)   +0x94 u16[render vertices] -> position index

Usage:
  python -I tools/md2.py <file.md2> [out.obj]          dump / export
  python -I tools/md2.py --wad <file.wad> <out_dir>     textured OBJ+MTL+PNG for a whole wad
  python -I tools/md2.py --check <dir-or-wad> ...       validate every geometry mesh
"""
import glob
import os
import struct
import sys

MAGIC = 0x1CD15D46
NODE_SIZE = 160
DUMMY_SIZE = 88


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
        # Transform-only nodes (dummies/bones), 88 B each: the common prefix of a geometry node.
        self.dummies = {}
        if self.flags & 1 and w[0x1d]:
            for i in range(self.u16(0x42) - self.node_count):
                o = w[0x1d] + DUMMY_SIZE * i
                self.dummies[o] = (self.u32(o + 4), struct.unpack_from('<16f', data, o + 0x10))
        self.transforms = {n.offset: (n.parent, n.matrix) for n in self.nodes}
        self.transforms.update(self.dummies)

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
        n.parent = self.u32(o + 4)
        n.matrix = struct.unpack_from('<16f', self.d, o + 0x10)
        n.npositions, n.ngroups, n.ntris, n.nverts = (self.u16(o + k) for k in (0x58, 0x5a, 0x5c, 0x5e))
        pos, nrm, uv, tri = (self.u32(o + k) for k in (0x60, 0x64, 0x68, 0x70))
        n.positions = [self._soa(pos, 3, i) for i in range(n.npositions)]
        n.uvs = [self._soa(uv, 2, i) for i in range(n.nverts)] if uv else []
        vmap = self.u32(o + 0x94)
        n.vert_pos = list(struct.unpack_from(f'<{n.nverts}H', self.d, vmap)) if vmap else list(range(n.nverts))
        n.normals_offset = nrm
        # Material groups: u32 -> [0x14] entry (8 B per material), u16 first_tri, tri_count,
        # first_vert, last_vert. Material index = (ptr - table) / 8.
        n.groups = []
        grp, mat_table = self.u32(o + 0x6c), self.u32(0x14 * 4)
        for i in range(n.ngroups if grp else 0):
            ptr, first, count = struct.unpack_from('<I2H', self.d, grp + 16 * i)
            n.groups.append(((ptr - mat_table) // 8 if ptr else None, first, count))
        n.triangles = [struct.unpack_from('<4H', self.d, tri + 8 * i) for i in range(n.ntris)] if tri else []
        n.bbox = struct.unpack_from('<6f', self.d, o + 0x78)
        return n

    def world_matrix(self, offset):
        """Node-to-model matrix: local * parent * grandparent ... (row vectors, D3D style)."""
        m = (1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)
        seen = set()
        while offset and offset in self.transforms and offset not in seen:
            seen.add(offset)
            parent, local = self.transforms[offset]
            m = tuple(sum(m[r * 4 + k] * local[k * 4 + c] for k in range(4)) for r in range(4) for c in range(4))
            offset = parent
        return m

    def world_positions(self, n):
        m = self.world_matrix(n.offset)
        return [tuple(x * m[c] + y * m[4 + c] + z * m[8 + c] + m[12 + c] for c in range(3)) for x, y, z in n.positions]

    def _soa(self, table, dims, i):
        """Element i of a structure-of-arrays table stored in blocks of 4."""
        b, j = divmod(i, 4)
        return tuple(struct.unpack_from('<f', self.d, table + 16 * dims * b + 16 * axis + 4 * j)[0]
                     for axis in range(dims))

    def material_name(self, i):
        return self.materials[i].rsplit('.', 1)[0] if i is not None and i < len(self.materials) else 'none'

    def to_obj(self, mtllib=None):
        out = [f'# Theme Park World .md2 v{self.version}, {len(self.nodes)} nodes']
        if mtllib:
            out.append(f'mtllib {mtllib}')
        vbase = tbase = 1
        for n in self.nodes:
            out.append(f'o {n.name or "node_%x" % n.offset}')
            out += [f'v {x:.6f} {y:.6f} {z:.6f}' for x, y, z in self.world_positions(n)]
            out += [f'vt {u:.6f} {1 - v:.6f}' for u, v in n.uvs]
            spans = [(m, f, c) for m, f, c in n.groups] or [(None, 0, n.ntris)]
            for mat, first, count in spans:
                if mtllib:
                    out.append(f'usemtl {self.material_name(mat)}')
                for _, *corners in n.triangles[first:first + count]:
                    if n.uvs:
                        out.append('f ' + ' '.join(f'{n.vert_pos[c] + vbase}/{c + tbase}' for c in corners))
                    else:
                        out.append('f ' + ' '.join(f'{n.vert_pos[c] + vbase}' for c in corners))
            vbase += n.npositions
            tbase += len(n.uvs)
        return '\n'.join(out) + '\n'

    def to_mtl(self):
        out = []
        for i, tex in enumerate(self.materials):
            out += [f'newmtl {self.material_name(i)}', 'Kd 1 1 1']
            if tex:
                out.append(f'map_Kd {tex.rsplit(".", 1)[0]}.png')
        return '\n'.join(out + ['newmtl none', 'Kd 0.8 0.8 0.8']) + '\n'

    def problems(self):
        """Consistency checks used to validate the format hypotheses."""
        errs = []
        for n in self.nodes:
            for t in n.triangles:
                if max(t[1:]) >= n.nverts:
                    errs.append(f'{n.name}: vertex index {max(t[1:])} >= {n.nverts}')
                    break
            for mat, first, count in n.groups:
                if (mat is not None and mat >= self.material_count) or first + count > n.ntris:
                    errs.append(f'{n.name}: bad group (material {mat}, tris {first}+{count} of {n.ntris})')
                    break
            if n.groups and sum(c for _, _, c in n.groups) != n.ntris:
                errs.append(f'{n.name}: groups cover {sum(c for _, _, c in n.groups)} of {n.ntris} triangles')
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


def export_wad(wad, out_dir):
    """Every geometry mesh in a .wad -> OBJ + MTL, and every .wct texture -> PNG."""
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    from wadtool import read_wad
    from wct2png import decode_wct, write_png
    os.makedirs(out_dir, exist_ok=True)
    for name, data, _ in read_wad(wad):
        base = os.path.basename(name.replace('\\', '/')).rsplit('.', 1)[0]
        if name.lower().endswith('.wct'):
            try:
                w, h, px, _ = decode_wct(data)
                write_png(os.path.join(out_dir, base + '.png'), w, h, px)
            except Exception as e:  # noqa: BLE001  (v0x12 textures aren't supported yet)
                print(f'{name}: texture skipped ({e})')
        elif name.lower().endswith('.md2'):
            try:
                m = Mesh(data)
            except ValueError:
                continue
            if m.version != 221 or not m.flags & 1:
                continue
            open(os.path.join(out_dir, base + '.mtl'), 'w').write(m.to_mtl())
            open(os.path.join(out_dir, base + '.obj'), 'w').write(m.to_obj(base + '.mtl'))
            print(f'{name}: {len(m.nodes)} nodes, {sum(n.ntris for n in m.nodes)} triangles')


if __name__ == '__main__':
    if sys.argv[1] == '--wad':
        export_wad(sys.argv[2], sys.argv[3])
    elif sys.argv[1] == '--check':
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
