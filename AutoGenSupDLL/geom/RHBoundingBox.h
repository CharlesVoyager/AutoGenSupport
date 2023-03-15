#pragma once

#include <vector>
#include "RHVector3.h"
#include "TopoTriangle.h"

using namespace std;

namespace XYZSupport
{
	class RHBoundingBox
	{
		const double epsilon = 1e-7;	//static const double epsilon = 1e-7;	//0.0000007
	public:
		RHVector3 minPoint;
		RHVector3 maxPoint;

		string ToString() const
		{
			return "minPoint: " + minPoint.ToString() + " maxPoint: " + maxPoint.ToString();
		}

		RHBoundingBox() { Clear(); }

		//--- MODEL_SLA	// milton
		vector<TopoTriangle> getBoundingTri()
		{
			vector<TopoTriangle> triangles;
			vector<TopoVertex> vertices = getVertices();

			// one box contain 6 surfaces (12 triangles)
			triangles.push_back(TopoTriangle(vertices[0], vertices[1], vertices[3]));	//0
			triangles.push_back(TopoTriangle(vertices[0], vertices[3], vertices[2]));	//1

			triangles.push_back(TopoTriangle(vertices[4], vertices[6], vertices[7]));	//2
			triangles.push_back(TopoTriangle(vertices[4], vertices[7], vertices[5]));	//3

			triangles.push_back(TopoTriangle(vertices[2], vertices[3], vertices[7]));	//4
			triangles.push_back(TopoTriangle(vertices[2], vertices[7], vertices[6]));	//5

			triangles.push_back(TopoTriangle(vertices[1], vertices[5], vertices[7]));	//6
			triangles.push_back(TopoTriangle(vertices[1], vertices[7], vertices[3]));	//7

			triangles.push_back(TopoTriangle(vertices[0], vertices[4], vertices[5]));	//8
			triangles.push_back(TopoTriangle(vertices[0], vertices[5], vertices[1]));	//9

			triangles.push_back(TopoTriangle(vertices[0], vertices[2], vertices[6]));	//10
			triangles.push_back(TopoTriangle(vertices[0], vertices[6], vertices[4]));	//11
			return triangles;
		}

		vector<TopoVertex> getVertices()
		{
		   vector<TopoVertex> vertices;
		   RHVector3 MAX = maxPoint;
		   RHVector3 min = minPoint;
		   // one box contain 8 points
		   vertices.push_back(TopoVertex(0, RHVector3(min.x, min.y, min.z)));
		   vertices.push_back(TopoVertex(1, RHVector3(min.x, min.y, MAX.z)));
		   vertices.push_back(TopoVertex(2, RHVector3(min.x, MAX.y, min.z)));
		   vertices.push_back(TopoVertex(3, RHVector3(min.x, MAX.y, MAX.z)));

		   vertices.push_back(TopoVertex(4, RHVector3(MAX.x, min.y, min.z)));
		   vertices.push_back(TopoVertex(5, RHVector3(MAX.x, min.y, MAX.z)));
		   vertices.push_back(TopoVertex(6, RHVector3(MAX.x, MAX.y, min.z)));
		   vertices.push_back(TopoVertex(7, RHVector3(MAX.x, MAX.y, MAX.z)));
		   return vertices;
		}

		bool containTri(TopoTriangle triangle)
		{
			for (int i = 0; i < sizeof(triangle.vertices)/sizeof(TopoVertex); i++)
			{
				if (ContainsPoint(triangle.vertices[i].pos))
					return true;
			}
			return false;
		}

		// Milton:  Efficient AABB/triangle intersection algoirthm 
		// from http://stackoverflow.com/questions/17458562/efficient-aabb-triangle-intersection-in-c-sharp
		bool overlapTri(TopoTriangle triangle)
		{
			double triangleMin, triangleMax;
			double boxMin, boxMax;

			/*// Test the box normals (x-, y- and z-axes)
			var boxNormals = new IVector[] {
				new Vector(1,0,0),
				new Vector(0,1,0),
				new Vector(0,0,1)
			};*/
			RHVector3 boxNormals[3] = { RHVector3(1,0,0), RHVector3(0,1,0), RHVector3(0,0,1) };
			/*
			for (int i = 0; i < 3; i++)
			{
				RHVector3 n = boxNormals[i];
				Project(triangle.vertices, boxNormals[i], out triangleMin, out triangleMax);
				if (triangleMax < box.Start.Coords[i] || triangleMin > box.End.Coords[i])
					return false; // No intersection possible.
			}*/

			Project(triangle.GetVertices(), boxNormals[0], triangleMin, triangleMax);
			if (triangleMax < minPoint.x || triangleMin > maxPoint.x)
				return false;
			Project(triangle.GetVertices(), boxNormals[1], triangleMin, triangleMax);
			if (triangleMax < minPoint.y || triangleMin > maxPoint.y)
				return false;
			Project(triangle.GetVertices(), boxNormals[2], triangleMin, triangleMax);
			if (triangleMax < minPoint.z || triangleMin > maxPoint.z)
				return false;

			/*// Test the triangle normal
			double triangleOffset = triangle.Normal.Dot(triangle.A);
			Project(box.Vertices, triangle.Normal, out boxMin, out boxMax);
			if (boxMax < triangleOffset || boxMin > triangleOffset)
				return false; // No intersection possible.*/
			double triangleOffset = triangle.normal.ScalarProduct(triangle.vertices[0].pos);
			Project(getVertices(), triangle.normal, boxMin, boxMax);
			if (boxMax < triangleOffset || boxMin > triangleOffset)
				return false; // No intersection possible.

			/*// Test the nine edge cross-products
			IVector[] triangleEdges = new IVector[] {
				triangle.A.Minus(triangle.B),
				triangle.B.Minus(triangle.C),
				triangle.C.Minus(triangle.A)
			}; */
			RHVector3 triangleEdges[3] = {
				triangle.vertices[0].pos.Subtract(triangle.vertices[1].pos),
				triangle.vertices[1].pos.Subtract(triangle.vertices[2].pos),
				triangle.vertices[2].pos.Subtract(triangle.vertices[0].pos)
			};

			for (int i = 0; i < 3; i++)
				for (int j = 0; j < 3; j++)
				{
					RHVector3 axis = triangleEdges[i].CrossProduct(boxNormals[j]);
					Project(getVertices(), axis, boxMin, boxMax);
					Project(triangle.GetVertices(), axis, triangleMin, triangleMax);
					if (boxMax < triangleMin || boxMin > triangleMax)
						return false; // No intersection possible
				}
			return true;
		}

	private:
		void Project(vector<TopoVertex> points, RHVector3 axis, double &min, double &max)
		{
			min = std::numeric_limits<double>::infinity();				//double.PositiveInfinity
			max = -1 * std::numeric_limits<double>::infinity();			//double.NegativeInfinity

			for(int i=0; i<points.size(); i++)    // C# AP: foreach(TopoVertex p in points)
			{
				//double val = axis.Dot(p);
				double val = axis.ScalarProduct(RHVector3(points[i].pos.x, points[i].pos.y, points[i].pos.z));
				
				if (val < min) min = val;
				if (val > max) max = val;
			}
		}
		//---

	public:
		void Add(const RHVector3 &point)
		{
			if (minPoint == RHVector3(-1, -1, -1))
			{
				minPoint = point;
				maxPoint = point;
			}
			else
			{
				minPoint.StoreMinimum(point);
				maxPoint.StoreMaximum(point);
			}
		}

		void Add(double x, double y, double z)
		{
			Add(RHVector3(x, y, z));
		}

		void Add(RHBoundingBox box)
		{
			Add(box.minPoint);
			Add(box.maxPoint);
		}

		void Clear()
		{
			minPoint = RHVector3(-1, -1, -1);	// set -1 to indicate it is null
			maxPoint = RHVector3(-1, -1, -1);	// set -1 to indicate it is null
		}

		bool ContainsPoint(RHVector3 point)
		{
			return	point.x >= minPoint.x && point.x <= maxPoint.x &&
					point.y >= minPoint.y && point.y <= maxPoint.y &&
					point.z >= minPoint.z && point.z <= maxPoint.z;
		}

		bool IntersectsBox(RHBoundingBox box)
		{
			bool xOverlap = Overlap(minPoint.x, maxPoint.x, box.minPoint.x, box.maxPoint.x);
			bool yOverlap = Overlap(minPoint.y, maxPoint.y, box.minPoint.y, box.maxPoint.y);
			bool zOverlap = Overlap(minPoint.z, maxPoint.z, box.minPoint.z, box.maxPoint.z);
			return xOverlap && yOverlap && zOverlap;
		}

		void operator= (const RHBoundingBox &other)
		{
			minPoint = other.minPoint;
			maxPoint = other.maxPoint;
		}

	private:
		bool Overlap(double p1min, double p1max, double p2min, double p2max)
		{
			if (p2min > p1max + epsilon) return false;
			if (p2max + epsilon < p1min) return false;
			return true;
		}

	public:
		double xMin() const { return MinPoint().x; }

		double yMin() const { return MinPoint().y; }
		
		double zMin() const { return MinPoint().z; }
		
		double xMax() const { return MaxPoint().x; }

		double yMax() const { return MaxPoint().y; }
		
		double zMax() const { return MaxPoint().z; }
	
		RHVector3 MaxPoint() const { return maxPoint; }
	
		RHVector3 MinPoint() const { return minPoint; }

		RHVector3 Size() const { return MaxPoint().Subtract(MinPoint()); }

		RHVector3 Center() const
		{
			RHVector3 center = MaxPoint().Add(MinPoint());
#if PRECISION_SINGLE
			center.Scale(0.5F);
#else
			center.Scale(0.5);
#endif
			return center;
		}

		/// <summary>
		/// Convert the box range into bitpattern for a fast intersection test.
		/// 
		/// </summary>
		/// <param name="box"></param>
		/// <returns></returns>
		int RangeToBits(RHBoundingBox box)
		{
			double dx = (maxPoint.x - minPoint.x) / 10;
			double dy = (maxPoint.y - minPoint.y) / 10;
			double dz = (maxPoint.z - minPoint.z) / 10;
			int p = 0;
			int i;
			double px = minPoint.x;
			double px2 = px + dx;
			double vx = box.minPoint.x;
			double vx2 = box.maxPoint.x;
			double py = minPoint.y;
			double py2 = py + dy;
			double vy = box.minPoint.y;
			double vy2 = box.maxPoint.y;
			double pz = minPoint.z;
			double pz2 = pz + dz;
			double vz = box.minPoint.z;
			double vz2 = box.maxPoint.z;
			for (i = 0; i < 10; i++)
			{
				if (Overlap(px, px2, vx, vx2)) p |= 1 << i;
				if (Overlap(py, py2, vy, vy2)) p |= 1 << (10 + i);
				if (Overlap(pz, pz2, vz, vz2)) p |= 1 << (20 + i);
				px = px2;
				px2 += dx;
				py = py2;
				py2 += dy;
				pz = pz2;
				pz2 += dz;
			}
			return p;
		}

		static bool IntersectBits(int a, int b)
		{
			int r = a & b;
			if (r == 0) return false;
			if ((r & 1023) == 0) return false;
			r >>= 10;
			if ((r & 1023) == 0) return false;
			r >>= 10;
			if ((r & 1023) == 0) return false;
			return true; ;
		}
	};
}