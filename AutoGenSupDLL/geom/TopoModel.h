#pragma once
#include <vector>
#include "TopoVertexStorage.h"
#include "TopoTriangleStorage.h"
#include "StlChecker.h"
#include "TopoTriangle.h"
#include "Submesh.h"

using namespace std;
using namespace MyTools;

namespace XYZSupport
{
	class TopoModel
	{
		/*static*/ const int MinVertexNumForProress = 2000; // large than all support part vertices number
	public:		
		enum FileFormat { Unknown, Binary, UTF8 };
		const bool debugRepair = false;
		//const float epsilon = 0.001f;

		TopoVertexStorage vertices;				// V
		TopoTriangleStorage triangles;			// V

		RHBoundingBox boundingBox;				// V
		//HashSet<TopoTriangle> intersectingTriangles = new HashSet<TopoTriangle>();
		int badEdges = 0;
		int badTriangles = 0;
		int shells = 0;
		int updatedNormals = 0;
		int loopEdges = 0;
		int manyShardEdges = 0;
		bool manifold = false;
		bool normalsOriented = false;
		bool intersectionsUpToDate = false;
		bool selectedTrunkSup;			//02102017 Added by RC Grey for tree support
		bool branchSupError;			// V //02102017 Added by RC Grey for tree support
		bool selectedDepthAdjust;		// V //02282017 Added by RC Grey for cone/branch support for collision depth adjustments
		StlChecker stlChker;

	private:
		/*static*/ const double OVERHANG_THRESHOLD = -0.3;         // 72 degree, Horizontal Level 0
		/*static*/ const double OVERHANG_THRESHOLD_H_LV_1 = -0.7;  // 45 degree
		/*static*/ const double OVERHANG_THRESHOLD_H_LV_2 = -0.85; // 32 degree
	
	public:
		bool isShowOverhangArea = false;

		bool STLErrorCheck = false;	// Charles: In support module, we don't need to load meshes data from external STL files. So, we set STLErrorCheck always false.

		void ToFile(const string filename)	// for debug purpose
		{
			TextFile textFile(filename);
			if (textFile.IsSuccess() == false)
				return;
			
			textFile.Write("boundingBox: %s\n", boundingBox.ToString().c_str());
			textFile.Write("vertices Count: %d\n", vertices.Count());

			for (int i = 0; i < vertices.Count(); i++)
				textFile.Write("_v[%d]: %s\n", i, vertices._v[i].ToString().c_str());
			
			textFile.Write("triangles Count: %d\n", triangles.Count());
			for (int i = 0; i < triangles.Count(); i++)
				textFile.Write("_t[%d]: %s\n", i, triangles._t[i].ToString().c_str());
		}
		/*	// C# AP debug function used on TopoModel class. 
		public void ToFile(string filename)
		{
			using (StreamWriter sw = new System.IO.StreamWriter(filename))
			{
				sw.WriteLine("boundingBox: " + boundingBox.ToString());
				sw.WriteLine("vertices Count: " + vertices.Count.ToString());
				int count = 0;
				foreach (TopoVertex _v in vertices.v)
				{
					sw.WriteLine("_v[" + count.ToString() + "]: " + _v.ToString());
					count++;
				}

				sw.WriteLine("triangles Count: " + triangles.Count.ToString());
				count = 0;
				foreach (TopoTriangle _t in triangles.triangles)
				{
					sw.WriteLine("_t[" + count.ToString() + "]: " + _t.ToString());
					count++;
				}
				sw.WriteLine("edgeStorage Count: " + edgeStorage.Count.ToString());
			}
		}
		*/

		void Clear()
		{
			vertices.Clear();
			triangles.Clear();
			boundingBox.Clear();
			intersectionsUpToDate = false;
		}

		void operator= (const TopoModel &other)
		{
			vertices._v.clear();
			vertices._v = other.vertices._v;	

			triangles.Clear();
			//triangles = other.triangles;
			triangles.Copy(other.triangles);

			boundingBox = other.boundingBox;
		}
		// NOTE: Convert model's coordinate to world coordicate and then add model's trangles to this object.
		void Merge(const TopoModel &model, Matrix4 trans)
		{
			int nOld = vertices.Count();
			vector<TopoVertex> vcopy;

			// Convert the coordinate of vertices to world coordinate.
			for (const auto &v : model.vertices._v)	//C#: foreach (TopoVertex v in model.vertices.v)
			{
				//model.vertices._v[i].id = i;	// Charles: AP有這行. 傳進的TopoModel的點之id已都有設定, 不須再設一次!
				TopoVertex newVert(v.id, v.pos, trans);
				addVertex(newVert);
				vcopy.push_back(newVert);
			}
			for (const auto &t : model.triangles._t)	//C#: foreach(TopoTriangle t in model.triangles)
			{
				TopoTriangle triangle(vcopy[t.vertices[0].id], vcopy[t.vertices[1].id], vcopy[t.vertices[2].id]);
				//triangle.RecomputeNormal();
				triangles.Add(triangle);
			}
			intersectionsUpToDate = false;
		}

		TopoVertex addVertex(const TopoVertex &v)
		{
			TopoVertex retVertex = vertices.Add(v);
			boundingBox.Add(v.pos);

			return retVertex;
		}

		TopoVertex addVertex(const RHVector3 &pos)
		{
			TopoVertex newVertex(vertices.Count(), pos);
			return addVertex(newVertex);	
#if 0	// because "addVertex()" will check if the newVertex has already added or not. It does not need to call "SearchPoint()" to check vertex and then add.
			TopoVertex newVertex;
			TopoVertex *pfindVertex = vertices.SearchPoint(pos);
			if (pfindVertex == NULL)
			{
				newVertex = TopoVertex(vertices.Count(), pos); // vertex id start from 0
				addVertex(newVertex);
				return vertices._v[vertices.Count()-1];	//the last element
			}
			else
			{
				return *pfindVertex;
			}
#endif
		}

		void UpdateVertexNumbers()
		{
			for(int i=0; i<vertices.Size(); i++)	//foreach(TopoVertex v in vertices.v)
			{
				vertices._v[i].id = i;
			}
		}

		void UpdateIntersectingTriangles()
		{
			intersectionsUpToDate = true;
		}

		TopoTriangle addTriangle(double p1x, double p1y, double p1z, double p2x, double p2y, double p2z,
			double p3x, double p3y, double p3z, double nx, double ny, double nz)
		{
			RHVector3 p1 = RHVector3(p1x, p1y, p1z);
			RHVector3 p2 = RHVector3(p2x, p2y, p2z);
			RHVector3 p3 = RHVector3(p3x, p3y, p3z);

			TopoVertex v1 = addVertex(p1);
			TopoVertex v2 = addVertex(p2);
			TopoVertex v3 = addVertex(p3);

			TopoTriangle triangle(v1, v2, v3, nx, ny, nz);
			return AddTriangle(triangle);
		}

		TopoTriangle addTriangle(RHVector3 &p1, RHVector3 &p2, RHVector3 &p3, RHVector3 &normal)
		{
			TopoVertex v1 = addVertex(p1);
			TopoVertex v2 = addVertex(p2);
			TopoVertex v3 = addVertex(p3);

			TopoTriangle triangle(v1, v2, v3, normal.x, normal.y, normal.z);
			return AddTriangle(triangle);
		}

		TopoTriangle AddTriangle(TopoTriangle &triangle)
		{
			if (triangle.IsDegenerated() == false)
				triangles.Add(triangle);
			return triangle;
		}

		void removeTriangle(TopoTriangle triangle)
		{
			triangles.Remove(triangle);
		}

		void UpdateNormals()
		{
			// C# AP: commented out all the codes.
		}

		bool CheckNormals()
		{
			// C# AP: commented out all the codes and just return true;
			return true;
		}

		//double Surface()	// C# AP: has this method, but never used.
		//double Volume()	// C# AP: has this method, but never used.

		bool JoinTouchedOpenEdges(double limit)
		{
			// C# AP: commented out all the codes and just return true;
			return false;
		}

		void ResetVertexPosToBBox()
		{
			for (auto &v : vertices._v) //foreach(TopoVertex v in originalModel.vertices.v)
			{
				v.pos.x -= boundingBox.Center().x;
				v.pos.y -= boundingBox.Center().y;
				v.pos.z -= boundingBox.Center().z;
			}

			for (auto &t : triangles._t)
			{
				for (int i = 0; i < 3; i++)
				{				
					t.vertices[i].pos.x -= boundingBox.Center().x;
					t.vertices[i].pos.y -= boundingBox.Center().y;
					t.vertices[i].pos.z -= boundingBox.Center().z;
				}
			}
		}

		int Size() const { return triangles.Count() * 50 + 84; }	// calcute STL binary size in bytes

		bool IsNull() const { return triangles.Count() == 0; }
	};
}