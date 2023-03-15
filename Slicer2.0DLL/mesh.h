#ifndef MESH_H
#define MESH_H

//#include "settings/settings.h"
#include "AABB3D.h"
#include "floatpoint.h"
//#include "intpoint.h"
#include <vector>
#include <map>
#include <unordered_map>

namespace cura
{
/*!
Vertex type to be used in a Mesh.

Keeps track of which faces connect to it.
*/
class MeshVertex
{
public:
	Point3 p; //!< location of the vertex
	std::vector<uint32_t> connected_faces; //!< list of the indices of connected faces
	MeshVertex(Point3 p) : p(p) { connected_faces.reserve(8); } //!< doesn't set connected_faces
};

class MeshFace
{
public:
	//int vertex_index[3] = { -1 }; //!< counter-clockwise ordering
	int vertex_index[3];
	int connected_face_index[3]; //!< same ordering as vertex_index (connected_face 0 is connected via vertex 0 and 1, etc.)
	//std::vector<int> connected_face_index[3];
};

class Mesh
{
	std::unordered_map<uint32_t, std::vector<uint32_t> > vertex_hash_map;
	AABB3D aabb;
public:
#undef min
	Point3 min() const; //!< min (in x,y and z) vertex of the bounding box
#undef max
	Point3 max() const; //!< max (in x,y and z) vertex of the bounding box
	AABB3D getAABB() const; //!< Get the axis aligned bounding box
	std::vector<MeshVertex> vertices;//!< list of all vertices in the mesh
	std::vector<MeshFace> faces; //!< list of all faces in the mesh
    std::vector<FPoint3> normals; // list of all normals of all faces
	void addFace(Point3& v0, Point3& v1, Point3& v2); //!< add a face to the mesh without settings it's connected_faces.
	void connectFace(); //!< complete the model : set the connected_face_index fields of the faces.
	void clear(); //!< clears all data
	
	Mesh() {}

private:
	mutable bool has_disconnected_faces; //!< Whether it has been logged that this mesh contains disconnected faces
	mutable bool has_overlapping_faces; //!< Whether it has been logged that this mesh contains overlapping faces
	int findIndexOfVertex(const Point3& v); //!< find index of vertex close to the given point, or create a new vertex and return its index.
	int getFaceIdxWithPoints(int idx0, int idx1, int notFaceIdx, int notFaceVertexIdx) const;
	//std::vector<int> getFaceIdxWithPoints(int idx0, int idx1, int notFaceIdx, int notFaceVertexIdx) const;
};

}//namespace cura
#endif//MESH_H

