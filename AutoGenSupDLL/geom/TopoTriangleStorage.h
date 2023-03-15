#pragma once
#include <vector>
using namespace std;

namespace XYZSupport
{
	class TopoTriangleStorage
	{
	public:
		unordered_map<__int64, int> _idDict;						// used to search the new added triangle already exists or not.
		vector<TopoTriangle> _t;									// container which stores the triangles.

	public:
		int Size() const { return (int)_t.size(); }	// same as Count()

		//TopoTriangle At(unsigned int i)	// Charles: We make _t as public. Don't need this function.

		void Add(const TopoTriangle &triangle)
		{
			__int64 temp = ((__int64)triangle.vertices[0].id) * 5915587277 + ((__int64)triangle.vertices[1].id) * 1500450271 + ((__int64)triangle.vertices[2].id) * 3267000013;	// Charles: 每個不重複的點的id都不一樣, 因此我們只要考慮三角形這三個點id的hash值就可以!

			// Check if the new triangle to add has already exist or not.
			std::unordered_map<__int64, int>::const_iterator got = _idDict.find(temp);
			if (got == _idDict.end())
			{
				_idDict.insert({ temp,  (int)_t.size() });
				_t.push_back(triangle);
			}
			//else	// Charles: Never found dplicate triangles, why???
			//{
			//	DbgMsg(__FUNCTION__"[ERROR]===> Found duplicate triangle: %s %s %s", triangle.vertices[0].ToString().c_str(), triangle.vertices[1].ToString().c_str(), triangle.vertices[2].ToString().c_str());
			//}

#if 0	// The hash function below is reference from C# AP - "GetHashCode()" in TopoTriangle.cs.
			size_t temp = std::hash<double>{}(	t.vertices[0].pos.x + t.vertices[0].pos.y + t.vertices[0].pos.z) * 5915587277 +
											(	t.vertices[1].pos.x + t.vertices[1].pos.y + t.vertices[1].pos.z) * 1500450271 +
											(	t.vertices[2].pos.x + t.vertices[2].pos.y + t.vertices[2].pos.z) * 3267000013);
#endif
		}

		void AddWithoutCheck(const TopoVertex &p1, const TopoVertex &p2, const TopoVertex &p3)
		{
			TopoTriangle t(p1, p2, p3, false);
			_t.push_back(t);
		}

		bool Remove(const TopoTriangle &triangle)
		{
			for (vector<TopoTriangle>::iterator it = _t.begin(); it != _t.end(); it++)
			{
				if (triangle == *it)
				{
					_t.erase(it);
					return true;
				}
			}
			return false;
		}

		void Copy(const TopoTriangleStorage &other)
		{
			_t.clear();
			for (int i = 0; i < other.Size(); i++)
			{
				_t.push_back(other._t[i]);
			}
		}

		void Clear()
		{
			_t.clear();
		}

#if 0	// DON'T use the method below. The search speed will be very slow when the number of triangles is large
		bool Contains(TopoTriangle &test)	// check if the triangle already exists in the _triangles vector.
		{
			for (int i = 0; i < _t.size(); i++)
			{
				if (test == _t[i]) return true;
			}
			return false;
		}
#endif
		//vector<TopoTriangle>& triangles() { return _t; }	// Charles: We make _t as public. Don't need this function.

		int Count()	const { return (int)_t.size(); }
	};

	// public class TopoTriangleNode {}; // Charles: Never used.
}