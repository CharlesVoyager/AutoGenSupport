#pragma once
#include <vector>
#include <map>
#include "Vectors.h"
#include "RHVector3.h"

using namespace std;

namespace XYZSupport
{
	struct Segment
	{
		double p1x;
		double p1y;
		double p2x;
		double p2y;
		int meshID;
	};

	class LayerSegment
	{
	public:
		vector<Segment> Segments;
		void AddSegments()
		{
			Segment segment;
			Segments.push_back(segment);
		}
	};

	class SegmentModel
	{
	public:
		vector<LayerSegment> AllSegments;
		vector<map<Vector4d, int>> dictionary;

		void AddLayerSwgment()
		{
			LayerSegment layerSegment;
			AllSegments.push_back(layerSegment);
		}
		void GetInformation(vector<RHVector3> normallist)
		{
			for (int level = 0; level < AllSegments.size(); level++)
			{
				map<Vector4d, int> temp({ pair<Vector4d, int>() });
				dictionary.push_back(temp);
				for (int i = 0; i < AllSegments[level].Segments.size(); i++)
				{
					if (AllSegments[level].Segments[i].p1x == AllSegments[level].Segments[i].p2x && AllSegments[level].Segments[i].p1y == AllSegments[level].Segments[i].p2y)
					{
					}
					else
					{
						Vector4d line1(AllSegments[level].Segments[i].p1x, AllSegments[level].Segments[i].p1y, AllSegments[level].Segments[i].p2x, AllSegments[level].Segments[i].p2y);
						Vector4d line2(AllSegments[level].Segments[i].p2x, AllSegments[level].Segments[i].p2y, AllSegments[level].Segments[i].p1x, AllSegments[level].Segments[i].p1y);
					
						map<Vector4d, int>::const_iterator got1 = dictionary[level].find(line1);	//dictionary[level].ContainsKey(line1)
						map<Vector4d, int>::const_iterator got2 = dictionary[level].find(line2);	//dictionary[level].ContainsKey(line1)

						if (got1 == dictionary[level].end() && got2 == dictionary[level].end())
						{
							dictionary[level].insert(pair<Vector4d, int>(line1, AllSegments[level].Segments[i].meshID));
							dictionary[level].insert(pair<Vector4d, int>(line2, AllSegments[level].Segments[i].meshID));
						}
						else
						{
							int p = 0;
							p = dictionary[level][line1];

							if (normallist[p].z > normallist[AllSegments[level].Segments[i].meshID].z)
							{
								dictionary[level].erase(line1);
								dictionary[level].erase(line2);
								dictionary[level].insert(pair<Vector4d, int>(line1, AllSegments[level].Segments[i].meshID));
								dictionary[level].insert(pair<Vector4d, int>(line2, AllSegments[level].Segments[i].meshID));
							}
						}

					}
				}
			}
		}
		void ToFile(const string filename)
		{
			TextFile textFile(filename);
			if (textFile.IsSuccess() == false)
				return;

			textFile.Write("AllSegments Layer Number: %d\n", AllSegments.size());

			for (int i = 0; i < AllSegments.size(); i++)
			{
				textFile.Write("Layer: %d\n", i);
				for (int j = 0; j < AllSegments[i].Segments.size(); j++)
				{
					textFile.Write("points=%f,%f %f,%f %d\n", AllSegments[i].Segments[j].p1x,
															AllSegments[i].Segments[j].p1y,
															AllSegments[i].Segments[j].p2x,
															AllSegments[i].Segments[j].p2y,
															AllSegments[i].Segments[j].meshID);
				}
			}
		}
	};
}