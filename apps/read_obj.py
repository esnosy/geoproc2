import copy
import struct
import sys


class Vec3:
    def __init__(self, x: float, y: float, z: float) -> None:
        self.x = x
        self.y = y
        self.z = z


class Triangle:
    def __init__(self, a: Vec3, b: Vec3, c: Vec3) -> None:
        self.a = a
        self.b = b
        self.c = c


class Indexed_Tri_Mesh:
    def __init__(self):
        self.vertices: list[Vec3] = []
        self.tris: list[tuple[int, int, int]] = []

    def to_tris(self) -> list[Triangle]:
        result: list[Triangle] = []
        for t in self.tris:
            a = self.vertices[t[0]]
            b = self.vertices[t[1]]
            c = self.vertices[t[2]]
            result.append(Triangle(a, b, c))
        return result


class Indexed_Mesh:
    def __init__(self) -> None:
        self.vertices: list[Vec3] = []
        self.faces: list[list[int]] = []

    def triangulate(self) -> Indexed_Tri_Mesh:
        result = Indexed_Tri_Mesh()
        result.vertices = copy.deepcopy(self.vertices)
        for f in self.faces:
            for i in range(2, len(f)):
                result.tris.append((f[0], f[i - 1], f[i]))
        return result


def write_stl_binary(tris: list[Triangle], filepath: str) -> None:
    with open(filepath, "wb") as file:
        header = bytes(80)
        file.write(header)
        file.write(struct.pack("<I", len(tris)))
        for t in tris:
            file.write(struct.pack("<fff", 0, 0, 0))
            file.write(struct.pack("<fff", t.a.x, t.a.y, t.a.z))
            file.write(struct.pack("<fff", t.b.x, t.b.y, t.b.z))
            file.write(struct.pack("<fff", t.c.x, t.c.y, t.c.z))
            file.write(struct.pack("<H", 0))


def join_meshes(meshes: list[Indexed_Mesh]) -> Indexed_Mesh:
    result = Indexed_Mesh()
    vertex_offset = 0
    for m in meshes:
        result.vertices.extend(m.vertices)
        for f in m.faces:
            new_face: list[int] = []
            for vi in f:
                new_face.append(vi + vertex_offset)
            result.faces.append(new_face)
        vertex_offset += len(m.vertices)
    return result


filepath = sys.argv[1]
meshes: list[Indexed_Mesh] = []
current_vertex_offset = 1
current_vertex_index = 1
with open(filepath, "rb") as file:
    for line in file:
        line = line.split()
        if line[0] == b"o":
            meshes.append(Indexed_Mesh())
            current_vertex_offset = current_vertex_index
        elif line[0] == b"v":
            if len(meshes) == 0:
                meshes.append(Indexed_Mesh())
            current_vertex_index += 1
            x, y, z = map(float, line[1:4])
            meshes[-1].vertices.append(Vec3(x, y, z))
        elif line[0] == b"f":
            face: list[int] = []
            for v in line[1:]:
                v = v.split(b"/")
                face.append(int(v[0]) - current_vertex_offset)
            meshes[-1].faces.append(face)

print(meshes)

for i, m in enumerate(meshes):
    tris = m.triangulate().to_tris()
    write_stl_binary(tris, f"obj_to_stl_{i}.stl")

mesh = join_meshes(meshes)
tris = mesh.triangulate().to_tris()
write_stl_binary(tris, "obj_to_stl.stl")
