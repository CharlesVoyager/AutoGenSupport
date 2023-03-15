#pragma once
#include <vector>
#include <unordered_map>

using namespace std;

namespace XYZSupport
{
	class TopoEdgeStorage
	{
	public:
		vector<TopoEdge> _e;
		// public List<TopoEdge> repeatedEdges = new List<TopoEdge>();

	private:
		unordered_map<double, int> hash;	//C# AP: Hashtable hash = new Hashtable();
		int count = 0;

		//nitish stl checker
		int shared_edge_count = 0;
		int repeated_edge_count = 0;

	public:
		int Count() { return count;	}
		int get_repeated_edge_count() { return repeated_edge_count; }
		int get_shared_edge_count() { return shared_edge_count; }

		void Clear()
		{
			count = 0;
		}

		void Add(TopoEdge edge)
		{
			Add(edge, 0);
		}

		void Add(TopoEdge edge, int level)
		{
			double temp =	floor((edge.v1.pos.x + edge.v2.pos.x) / 2 * 100000) * 5915587277 +
							floor((edge.v1.pos.y + edge.v2.pos.y) / 2 * 100000) * 1500450271 + 
							floor((edge.v1.pos.z + edge.v2.pos.z) / 2 * 100000) * 3267000013;
			
			std::unordered_map<double, int>::const_iterator got = hash.find(temp);

			if (got == hash.end())
			{
				hash.insert({ temp, count });
				_e.push_back(edge);
				count++;
			}
		}

		//nitish stl checker search edge in hashtable of edges
		TopoEdge *SearchEdge(TopoVertex vertex1, TopoVertex vertex2)
		{
			double temp =	floor((vertex1.pos.x + vertex2.pos.x) / 2 * 100000) * 5915587277 +
							floor((vertex1.pos.y + vertex2.pos.y) / 2 * 100000) * 1500450271 + 
							floor((vertex1.pos.z + vertex2.pos.z) / 2 * 100000) * 3267000013;
		
			std::unordered_map<double, int>::const_iterator got = hash.find(temp);

			if (got != hash.end())
			{
				return &_e[hash[temp]];
			}
			else
				return NULL;
		}
	};
}