#pragma once
#include "clipper.hpp"
#include "MyTools.h"

using namespace ClipperLib;

namespace XYZSupport
{
	class SupportPointsListDbg
	{
	public:
		static void SupportPointsToFile(const vector<SupportPoint> &pts, const string filename)
		{
			TextFile textFile(filename);
			if(textFile.IsSuccess() == false) return;
			textFile.Write("PointsList Number: %d\n", pts.size());

			for (int i = 0; i < pts.size(); i++)
			{
				textFile.Write("SupportPoint #%d: %s\n", i, pts[i].ToString().c_str());
			}
		}

		static void DumpObjectToFile(const vector<IntPoint> &object, const string name, const string filename)
		{
			TextFile textFile(filename);
			if (textFile.IsSuccess() == false)
				return;

			textFile.Write("===================================\n");
			textFile.Write("  Dump %s\n", name.c_str());
			textFile.Write("===================================\n");
			textFile.Write("Count: %d\n", object.size());
			for (int j = 0; j < object.size(); j++)
			{
				textFile.Write("%d,%d\n", object[j].X, object[j].Y);
			}
		}

		static void DumpObjectToFile(const vector<vector<IntPoint>> &object, const string name, const string filename)
		{
			TextFile textFile(filename);
			if (textFile.IsSuccess() == false)
				return;

			textFile.Write("===================================\n");
			textFile.Write("  Dump %s\n", name.c_str());
			textFile.Write("===================================\n");
			textFile.Write("Count: %d\n", object.size());
			for (int j = 0; j < object.size(); j++)
			{
				int k = 0;
				for (k = 0; k < object[j].size()-1; k++)
					textFile.Write("%d,%d ", object[j][k].X, object[j][k].Y);

				//print last item
				textFile.Write("%d,%d", object[j][k].X, object[j][k].Y);
				textFile.Write("\n");
			}
		}

		static void DumpObjectToFile(const unordered_set<IntPoint, HashIntPoint> &object, const string name, const string filename)
		{
			TextFile textFile(filename);
			if (textFile.IsSuccess() == false)
				return;

			textFile.Write("===================================\n");
			textFile.Write("  Dump %s\n", name.c_str());
			textFile.Write("===================================\n");
			textFile.Write("Count: %d\n", object.size());
			for (auto it = object.begin(); it != object.end(); ++it)
			{
				textFile.Write("%d,%d\n", it->X, it->Y);
			}
		}
		static void DumpObjectToFile(const set<IntPoint> &object, const string name, const string filename)
		{
			TextFile textFile(filename);
			if (textFile.IsSuccess() == false)
				return;

			textFile.Write("===================================\n");
			textFile.Write("  Dump %s\n", name.c_str());
			textFile.Write("===================================\n");
			textFile.Write("Count: %d\n", object.size());
			for (auto it = object.begin(); it != object.end(); ++it)
			{
				textFile.Write("%d,%d\n", it->X, it->Y);
			}
		}
		static void DumpObjectToFile(const vector<int> &object, const string name, const string filename)
		{
			TextFile textFile(filename);
			if (textFile.IsSuccess() == false)
				return;

			textFile.Write("===================================\n");
			textFile.Write("  Dump %s\n", name.c_str());
			textFile.Write("===================================\n");
			textFile.Write("Count: %d\n", object.size());
			for (auto it = object.begin(); it != object.end(); ++it)
			{
				textFile.Write("%d\n", *it);
			}
		}
		static void DumpObjectToFile(const vector<vector<int>> &object, const string name, const string filename)
		{
			TextFile textFile(filename);
			if (textFile.IsSuccess() == false)
				return;

			textFile.Write("===================================\n");
			textFile.Write("  Dump %s\n", name.c_str());
			textFile.Write("===================================\n");
			textFile.Write("Count: %d\n", object.size());
			int index = 0;
			for (auto it = object.begin(); it != object.end(); ++it)
			{
				textFile.Write("Index: %d\n", index);	index++;
				for (auto it2 = it->begin(); it2 != it->end(); ++it2)
					textFile.Write("%d ", *it2);
			
				textFile.Write("\n");
			}
		}
		static void DumpObjectToFile(const vector<double> &object, const string name, const string filename)
		{
			TextFile textFile(filename);
			if (textFile.IsSuccess() == false)
				return;

			textFile.Write("===================================\n");
			textFile.Write("  Dump %s\n", name.c_str());
			textFile.Write("===================================\n");
			textFile.Write("Count: %d\n", object.size());
			for (auto it = object.begin(); it != object.end(); ++it)
			{
				textFile.Write("%f\n", *it);
			}
		}

		static void DumpObjectToFile(const PolyNodes &object, const string name, const string filename)
		{
			TextFile textFile(filename);
			if (textFile.IsSuccess() == false)
				return;

			textFile.Write("===================================\n");
			textFile.Write("  Dump %s\n", name.c_str());
			textFile.Write("===================================\n");
			textFile.Write("Count: %d\n", object.size());
			for (int i = 0; i < object.size(); i++)
			{
				textFile.Write("=== %d ===\n", i);
				textFile.Write("IsHole: %s\n", object[i]->IsHole()?"true":"false");
				textFile.Write("Contour:");
				for (int j = 0; j < object[i]->Contour.size(); j++)
				{
					textFile.Write(" %d,%d", object[i]->Contour[j].X, object[i]->Contour[j].Y);
				}
				textFile.Write("\n");
			}
		}

	};
	
}