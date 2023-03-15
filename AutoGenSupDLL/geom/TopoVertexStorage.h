#pragma once
#include <unordered_set>
#include <unordered_map>
#include <list>
#include "TopoVertex.h"
#include "RHBoundingBox.h"

#ifdef __APPLE__
#define __int64 long long
#endif
namespace XYZSupport
{
	class TopoVertexStorageLeaf
	{
	public:
		RHBoundingBox box;
		list<TopoVertex> vertices;	//NOTE: We don't use "vector", because we need "remove" function in the TopoVertexStorage::RemoveTraverse().

		void Add(TopoVertex vertex)
		{
			vertices.push_back(vertex);
			box.Add(vertex.pos);
		}

		int LargestDimension()
		{
			RHVector3 size = box.Size();
			if (size.x > size.y && size.x > size.z) return 0;
			if (size.y > size.z) return 1;
			return 2;
		}

		bool SearchPoint(RHVector3 vertex)
		{
			for (list<TopoVertex>::iterator it = vertices.begin(); it != vertices.end(); it++)
			{
				if (vertex.Distance(it->pos) < 0.001f)
					return true;
			}
			return false;
		}
	};

	class TopoVertexStorage
	{
	public:
		const int maxVerticesPerNode = 50;
		TopoVertexStorage *pleft = nullptr, *pright = nullptr;
		TopoVertexStorageLeaf *pleaf = nullptr;
		vector<TopoVertex> _v;
		unordered_map<__int64, int> hash;	//C# AP: Hashtable hash = new Hashtable();

		int splitDimension = -1;
	
	private:
		int count = 0;
		double splitPosition = 0;

	public:
		int Size() const { return (int)_v.size(); }	// same as Count()

		bool IsLeaf() { return pleaf != nullptr; }

		int Count() const { return count; }

		void Clear()
		{
			pleft = pright = nullptr;
			pleaf = nullptr;
			count = 0;
		}

		void ChangeCoordinates(TopoVertex &vertex, RHVector3 newPos)
		{
			Remove(vertex);
			vertex.pos = newPos;
			Add(vertex);
		}

		TopoVertex Add(const TopoVertex &vertex)
		{
			return Add(vertex, 0);
		}

		TopoVertex Add(const TopoVertex &vertex, int level)
		{
			__int64 temp = (__int64)floor(vertex.pos.x * 100000) * 5915587277 + (__int64)floor(vertex.pos.y * 100000) * 1500450271 + (__int64)floor(vertex.pos.z * 100000) * 3267000013;

			// Check if the new vertex to add has already exist or not.
			std::unordered_map<__int64, int>::const_iterator got = hash.find(temp);
			if (got == hash.end())
			{
				hash.insert({ temp, count });
				_v.push_back(vertex);
				count++;
			}
			//else // Charles: The duplicate vertex may occur when generating supports...
			//{
			//	DbgMsg(__FUNCTION__"[ERROR]===> Found duplicate vertex: %s", vertex.pos.ToString().c_str());
			//}
			return _v[hash[temp]];
		}

#if 0
		TopoVertex *SearchPoint(RHVector3 vertex)
		{
			__int64 temp = floor(vertex.x * 100000) * 5915587277 + floor(vertex.y * 100000) * 1500450271 + floor(vertex.z * 100000) * 3267000013;
			std::unordered_map<__int64, int>::const_iterator got = hash.find(temp);
			if (got != hash.end())
				return &_v[hash[temp]];
			else 
				return nullptr;

#if 0	// DON'T use the following method. The serach speed will very slow when the number of vertex is large.
			for (int i = 0; i < _v.size(); i++)
				if (vertex == _v[i].pos) return &_v[i];
	
			return nullptr;
#endif
		}
#endif
		void Remove(TopoVertex vertex)
		{
			if (pleaf == nullptr && pleft == nullptr) return;
			if (RemoveTraverse(vertex)) count--;
		}

	private:
		bool RemoveTraverse(TopoVertex vertex)
		{
			if (IsLeaf())
			{
				pleaf->vertices.remove(vertex);	// std::list::remove ==> Return Value: none
				return true;
			}
			if (vertex.pos[splitDimension] < splitPosition)
				return pleft->RemoveTraverse(vertex);
			else
				return pright->RemoveTraverse(vertex);
		}
	};
}