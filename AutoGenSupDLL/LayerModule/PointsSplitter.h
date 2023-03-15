#pragma once
#include <string>
#include <vector>
#include "LayerModel.h"
#include "Vectors.h"
#include "SegmentModel.h"
#include "Helper.h"

using namespace std;

namespace XYZSupport
{
	class PointsSplitter
	{
		string pointsstring;
	public:
		vector<PolygonPoint> points;
		vector<Vector3d> vector3s;
		Segment segments;				// EX:  <segment points="9.75,0 10,0 0 " /> ===> must be 2 two points and 1 mesh ID. 

		void Segmentsplit()
		{
			vector<string> words = StringSplit(pointsstring, " ");	// C#: string[] words = pointsstring.Split(' ');

			vector<string> point1 = StringSplit(words[0], ",");
			vector<string> point2 = StringSplit(words[1], ",");

			segments.p1x = stod(point1[0]);
			segments.p1y = stod(point1[1]);
			segments.p2x = stod(point2[0]);
			segments.p2y = stod(point2[1]);
			segments.meshID = stoi(words[2]);
		}
		//EX: points="59, 59, 20, 69, 59, 20, 69, 59, 30, "
		void Meshsplit()
		{
			int i;
			vector<string> words = StringSplit(pointsstring, " ");	
			for (i = 0; i < words.size() / 3; i++)
			{
				vector<string> xystringx = StringSplit(words[3*i], ",");
				vector<string> xystringy = StringSplit(words[3*i+1], ",");
				vector<string> xystringz = StringSplit(words[3*i+2], ",");

				Vector3d vector3;
				vector3s.push_back(vector3);
				vector3s[i].x = stod(xystringx[0]);
				vector3s[i].y = stod(xystringy[0]);
				vector3s[i].z = stod(xystringz[0]);
			}
		}
		void Split()
		{
			int i;
			vector<string> words = StringSplit(pointsstring, " ");

			for (i = 0; i < words.size(); i++)
			{
				vector<string> xystring = StringSplit(words[i], ",");

				PolygonPoint point;
				points.push_back(point);
				points[i].x = stod(xystring[0]);	
				points[i].y = stod(xystring[1]);
			}
		}
		void Setstring(string points)
		{
			pointsstring = points;
		}
		void Pointsclear()
		{
			pointsstring = "";
			points.clear();
			vector3s.clear();
		}
	};
}