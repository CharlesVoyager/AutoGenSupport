#pragma once
#include "TopoVertex.h"

namespace XYZSupport
{
	class TopoTriangle
	{
	public:
		TopoVertex vertices[3];
		int id = 0;			// seems no used. The AP always shows this vlaue as 0.
		RHVector3 normal;

		string ToString() const
		{
			return "[" + to_string(id) + "] " + normal.ToString() + " " + vertices[0].ToString() + " " + vertices[1].ToString() + " " + vertices[2].ToString();
		}

		TopoTriangle() {}
		TopoTriangle(int _id, const TopoVertex &v1, const TopoVertex &v2, const TopoVertex &v3)
		{
			id = _id;
			vertices[0] = v1;
			vertices[1] = v2;
			vertices[2] = v3;
			RecomputeNormal();
		}

		TopoTriangle(const TopoVertex &v1, const TopoVertex &v2, const TopoVertex &v3)
		{
			vertices[0] = v1;
			vertices[1] = v2;
			vertices[2] = v3;
			RecomputeNormal();
		}

		TopoTriangle(const TopoVertex &v1, const TopoVertex &v2, const TopoVertex &v3, bool noNormal)
		{
			vertices[0] = v1;
			vertices[1] = v2;
			vertices[2] = v3;
		}

		TopoTriangle(const TopoVertex &v1, const TopoVertex &v2, const TopoVertex &v3, double nx, double ny, double nz)
		{
			vertices[0] = v1;
			vertices[1] = v2;
			vertices[2] = v3;
			RHVector3 normalTest(nx, ny, nz);
		}

		bool operator==(const TopoTriangle &comp) const
		{
			if (vertices[0].id == comp.vertices[0].id && vertices[1].id == comp.vertices[1].id && vertices[2].id == comp.vertices[2].id)
				return true;
			else
				return false;
#if 0
			double temp =	(comp.vertices[0].pos.x + comp.vertices[0].pos.y + comp.vertices[0].pos.z) * 5915587277 +
							(comp.vertices[1].pos.x + comp.vertices[1].pos.y + comp.vertices[1].pos.z) * 1500450271 +
							(comp.vertices[2].pos.x + comp.vertices[2].pos.y + comp.vertices[2].pos.z) * 3267000013;

			double local =	(vertices[0].pos.x + vertices[0].pos.y + vertices[0].pos.z) * 5915587277 +
							(vertices[1].pos.x + vertices[1].pos.y + vertices[1].pos.z) * 1500450271 +
							(vertices[2].pos.x + vertices[2].pos.y + vertices[2].pos.z) * 3267000013;

			if (temp == local)
				return true;
			else
				return false;
#endif
		}

		bool operator!=(const TopoTriangle &comp) const
		{
			return !(*this == comp);
		}

		vector<TopoVertex> GetVertices()
		{
			vector<TopoVertex> vecVertices;

			vecVertices.push_back(vertices[0]);	
			vecVertices.push_back(vertices[1]);
			vecVertices.push_back(vertices[2]);

			return vecVertices;
		}

		bool Equals(const TopoTriangle &obj)
		{
			return (obj.vertices[0] == vertices[0] && obj.vertices[1] == vertices[1] && obj.vertices[2] == vertices[2]);
		}

		int GetHashCode()
		{
			return (int)hash<double>()(	(vertices[0].pos.x + vertices[0].pos.y + vertices[0].pos.z) * 5915587277 +
										(vertices[1].pos.x + vertices[1].pos.y + vertices[1].pos.z) * 1500450271 +
										(vertices[2].pos.x + vertices[2].pos.y + vertices[2].pos.z) * 3267000013	);
		}

		void FlipDirection()
		{
			normal.Scale((float)-1);
			TopoVertex v = vertices[0];
			vertices[0] = vertices[1];
			vertices[1] = v;
		}

		void RecomputeNormal()
		{
			RHVector3 d1 = vertices[1].pos.Subtract(vertices[0].pos);
			RHVector3 d2 = vertices[2].pos.Subtract(vertices[1].pos);
			normal = d1.CrossProduct(d2);
			normal.NormalizeSafe();
		}

		int VertexIndexFor(TopoVertex test)
		{
			if (test == vertices[0]) return 0;
			if (test == vertices[1]) return 1;
			if (test == vertices[2]) return 2;
			return -1;
		}

		double SignedVolume()
		{
			return vertices[0].pos.ScalarProduct(vertices[1].pos.CrossProduct(vertices[2].pos)) / 6.0;
		}

		double Area()
		{
			RHVector3 d1 = vertices[1].pos.Subtract(vertices[0].pos);
			RHVector3 d2 = vertices[2].pos.Subtract(vertices[1].pos);
			return 0.5 * d1.CrossProduct(d2).Length();
		}

		bool IsDegenerated()
		{
			if (vertices[0] == vertices[1] || vertices[1] == vertices[2] || vertices[2] == vertices[0])
				return true;
			return false;
		}

		/// <summary>
		/// Checks if all vertices are colinear preventing a normal computation. If point are coliniear the center vertex is
		/// moved in the direction of the edge to allow normal computations.
		/// </summary>
		/// <returns></returns>
		bool CheckIfColinear()
		{
			RHVector3 zero;
			RHVector3 d1 = vertices[1].pos.Subtract(vertices[0].pos);
			RHVector3 d2 = vertices[2].pos.Subtract(vertices[1].pos);
			//double angle = d1.Angle(d2);
			//if (angle > 0.001 && angle<Math.PI-0.001) 
			if (!d1.CrossProduct(d2).Equals(zero))
				return false;
			else
				return true;
		}

		int NumberOfSharedVertices(TopoTriangle tri)
		{
			int sameVertices = 0;
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					if (vertices[i] == tri.vertices[j])
					{
						sameVertices++;
						break;
					}
				}
			}
			return sameVertices;
		}

		bool SameNormalOrientation(TopoTriangle test)
		{
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					if (vertices[i] == test.vertices[j] && vertices[(i + 1) % 3] == test.vertices[(j + 2) % 3])
						return true;
				}
			}
			return false;
		}

		RHVector3 Center()
		{
			{
				RHVector3 c = vertices[0].pos.Add(vertices[1].pos).Add(vertices[2].pos);
	#if PRECISION_SINGLE
				c.Scale(1.0F / 3.0F);
	#else
				c.Scale(1.0 / 3.0);
	#endif
				return c;
			}
		}

		void ToSphere(RHVector3 &center, double &radius)
		{
			center = Center();
			radius = vertices[0].pos.Distance(center);
		}

		// Checked OK. 2019/6/18
		//--- MODEL_SLA	// milton
		// ¹ê§@½×¤åªº¤èªk - Fast, Minimum Storage RayTriangle Intersection
		// t => delta, ¦pªG¦³¥´¨ìª«Åé,t>0,§_«ht<0
		bool IntersectsLineTest(RHVector3 orig, RHVector3 dir, double &t, double &u, double &v)
		{
			t = u = v = 0;
			//Debug.WriteLine("IntersectsLine orig " + orig + " dir " + dir );
			//Debug.WriteLine("IntersectsLine ver1 " + vertices[0].pos + " ver2 " + vertices[1].pos + " ver3 " + vertices[2].pos);
			RHVector3 vert0 = vertices[0].pos;
			/*  find vectors for two edges sharing vert0
				SUB(edge1, vert1, vert0)
				SUB(edge2, vert2, vert0) */
			RHVector3 edge1 = vertices[1].pos.Subtract(vert0);
			RHVector3 edge2 = vertices[2].pos.Subtract(vert0);
			/* begin calculating determinant - also used to calculate U parameter
			CROSS(pvec, dir, edge2)*/
			RHVector3 pvec = dir.CrossProduct(edge2);

			/* if determinant is near zero, ray lies in plane of triangle
			det = DOT(edge1, pvec)*/
			double det = edge1.ScalarProduct(pvec);
			//Debug.WriteLine("IntersectsLine det " + det);
			/* define TEST_CULL if culling is desired
			if (det < EPSILON)return 0*/
			if (det < 0.000001) return false;
			/* calculate distance from vert0 to ray origin
			SUB(tvec, orig, vert0)*/
			RHVector3 tvec = orig.Subtract(vert0);
			//Debug.WriteLine("IntersectsLine tvec " + tvec);
			/* calculate U parameter and test bounds
			*u = DOT(tvec, pvec)*/
			u = tvec.ScalarProduct(pvec);
			/*if (*u < 0.0 || *u > det) return 0;*/
			//Debug.WriteLine("IntersectsLine u " + u);
			if (u < 0 || u > det) return false;
			/* prepare to test V parameter
			CROSS(qvec, tvec, edge1)*/
			RHVector3 qvec = tvec.CrossProduct(edge1);
			/* calculate V parameter and test bounds
			*v = DOT(dir, qvec)*/
			v = dir.ScalarProduct(qvec);
			//Debug.WriteLine("IntersectsLine v " + v);
			/*if (*v < 0.0 || *u + *v > det) return 0*/
			if (v < 0 || (u + v) > det) return false;
			/* calculate t, scale parameters, ray intersects triangle
				*t = DOT(edge2, qvec)
				inv_det = 1.0 / det
				*t *= inv_det
				*u *= inv_det
				*v *= inv_det*/
			double inv_det = 1.0 / det;
			t = edge2.ScalarProduct(qvec);
			t *= inv_det;
			u *= inv_det;
			v *= inv_det;
			return true;
		}
		//---

	public:
		double DistanceToPlane(RHVector3 pos)
		{
			double d = vertices[0].pos.ScalarProduct(normal);
			return pos.ScalarProduct(normal) - d;
		}

	private:
		void DominantAxis(int& d1, int& d2)
		{
			double n1 = abs(normal.x);
			double n2 = abs(normal.y);
			double n3 = abs(normal.z);
			if (n1 > n2 && n1 > n3)
			{
				d1 = 1;
				d2 = 2;
			}
			else if (n2 > n3)
			{
				d1 = 0;
				d2 = 2;
			}
			else
			{
				d1 = 0;
				d2 = 1;
			}
		}
	};

	class TopoTriangleDistance
	{
	public:
		double distance;
		TopoTriangle triangle;

		//TopoTriangleDistance() {}
		TopoTriangleDistance(double dist, TopoTriangle tri)
		{
			triangle = tri;
			distance = dist;
		}

		int Compare(TopoTriangleDistance td1, TopoTriangleDistance td2)
		{
			//return -td1.distance.CompareTo(td2.distance);
			return 0;
		}

		int CompareTo(TopoTriangleDistance td)
		{
			//return -distance.CompareTo(td.distance);
			return 0;
		}
	};
}