#pragma once

#include <vector>
#include <list>
#include "TopoVertex.h"

using namespace std;

namespace XYZSupport
{
	class TopoTriangle;	//Don't include "TopoTriangle.h", because "TopoTraingle.h" includes "TopoEdge.h"...
	class TopoModel;	//Don't include "TopoModel.h", because "TopoModel.h" includes "TopoTriangle.h" -> inlcudes "TopoEdge.h"...

	class TopoEdge
	{
	public:
		TopoVertex v1, v2;
		RHVector3 Vec1;
		int id;
		//list<TopoTriangle *> faces;				// Charles: Never used.
		int algHelper;

		TopoEdge() {}

		TopoEdge(TopoVertex _v1, TopoVertex _v2)
		{
			v1 = _v1;
			v2 = _v2;
		}

		//niitsh stl check edge adding
		TopoEdge(int _id, TopoVertex _v1, TopoVertex _v2)
		{
			id = _id;
			v1 = _v1;
			v2 = _v2;
			Vec1 = v1.pos.Subtract(v2.pos);
			Vec1.NormalizeSafe();
		}
		bool operator==(const TopoEdge &comp)
		{
			if (v1 == comp.v1 && v2 == comp.v2 && Vec1 == comp.Vec1)
				return true;
			else
				return false;
		}

		bool isBuildOf(TopoVertex _v1, TopoVertex _v2)
		{
			return (v1 == _v1 && v2 == _v2) || (v1 == _v2 && v2 == _v1);
		}

		//int connectedFaces(){ return (int)faces.size(); }			// Charles: Never used.
		//void connectFace(TopoTriangle *pface);					// Charles: Never used.
		//void disconnectFace(TopoTriangle face, TopoModel *pmodel);// Charles: Never used.

		bool ContainsVertex(TopoVertex v)
		{
			return v1 == v || v2 == v;
		}

		double EdgeLength()
		{
			return v1.pos.Subtract(v2.pos).Length(); 
		}

		TopoTriangle* GetFaceExcept(TopoTriangle *notThis);			// Charles: Never used.

		/// <summary>
		/// Splits an edge and changes the connected triangles to maintain
		/// a topological correct system.
		/// </summary>
		/// <param name="model"></param>
		/// <param name="vertex"></param>
		//void InsertVertex(TopoModel* model, TopoVertex* vertex) //Charles: Never used.
	
		//TopoTriangle* FirstTriangle();	//Charles: Never used.

		bool ProjectPoint(RHVector3 p, double lambda, RHVector3 pProjected)
		{
			RHVector3 u = v2.pos.Subtract(v1.pos);
			lambda = p.Subtract(v1.pos).ScalarProduct(u) / u.ScalarProduct(u);
#if PRECISION_SINGLE
			pProjected.x = v1.pos.x + (float)lambda * u.x;
			pProjected.y = v1.pos.y + (float)lambda * u.y;
			pProjected.z = v1.pos.z + (float)lambda * u.z;
#else
			pProjected.x = v1.pos.x + lambda * u.x;
			pProjected.y = v1.pos.y + lambda * u.y;
			pProjected.z = v1.pos.z + lambda * u.z;
#endif
			return lambda >= 0 && lambda <= 1;
		}

		TopoVertex GetTopoVertexByIndex(int pos)
		{
			if (pos == 0) return v1;
			else return v2;
		}

		string ToString() const
		{
			return "Edge [" + v1.pos.ToString() + "," + v2.pos.ToString() + "]";
		}
	};

	class TopoEdgePair
	{
	public:
		TopoEdge edgeA, edgeB;
		double alphaBeta; // Sum of dihedral angles to a virtual shared triangle

		TopoEdgePair(TopoEdge _edgeA, TopoEdge _edgeB)
		{
			edgeA = _edgeA;
			edgeB = _edgeB;
			RHVector3 sharedPoint;
			RHVector3 p1, p2;
			if (edgeA.v1 == edgeB.v1)
			{
				sharedPoint = edgeA.v1.pos;
				p1 = edgeA.v2.pos;
				p2 = edgeB.v2.pos;
			}
			else if (edgeA.v1 == edgeB.v2)
			{
				sharedPoint = edgeA.v1.pos;
				p1 = edgeA.v2.pos;
				p2 = edgeB.v1.pos;
			}
			else if (edgeA.v2 == edgeB.v1)
			{
				sharedPoint = edgeA.v1.pos;
				p1 = edgeA.v1.pos;
				p2 = edgeB.v2.pos;
			}
			else if (edgeA.v2 == edgeB.v2)
			{
				sharedPoint = edgeA.v2.pos;
				p1 = edgeA.v1.pos;
				p2 = edgeB.v1.pos;
			}
			RHVector3 d1 = p1.Subtract(sharedPoint);
			RHVector3 d2 = p2.Subtract(sharedPoint);
			RHVector3 normal = d1.CrossProduct(d2);
			normal.NormalizeSafe();
			//alphaBeta = normal.AngleForNormalizedVectors(edgeA.faces.First.Value.normal) + normal.AngleForNormalizedVectors(edgeB.faces.First.Value.normal);
			//if (alphaBeta > Math.PI) // normal was wrong direction
				//alphaBeta = 2 * Math.PI - alphaBeta;
		}

		bool ContainsEdge(TopoEdge edge)
		{
			return edgeA == edge || edgeB == edge;
		}

		bool ContainsEdgePair(TopoEdge a, TopoEdge b)
		{
			return ContainsEdge(a) && ContainsEdge(b);
		}

		static bool ContainsListPair(vector<TopoEdgePair> list, TopoEdge a, TopoEdge b)
		{
			for (int i = 0; i < list.size(); i++)
			{
				if (list[i].ContainsEdgePair(a, b)) return true;
			}
			return false;
		}

		//public TopoTriangle BuildTriangle(TopoModel model)	// never used
	
		int Compare(TopoEdgePair pair1, TopoEdgePair pair2)
		{
			if (pair1.alphaBeta < pair2.alphaBeta)
				return -1;
			else if (pair1.alphaBeta == pair2.alphaBeta)
				return 0;
			else
				return 1;
		}

		//bool Valid();// never used

		int CompareTo(TopoEdgePair pair)
		{
			return Compare(*this, pair);
		}

		string ToString() const
		{
			return "EdgePair:" + edgeA.ToString() + " - " + edgeB.ToString() + " ab = " + to_string(alphaBeta);
		}
	};
}
