#pragma once
#include <vector>
#include "clipper.hpp"

using namespace std;
using namespace ClipperLib;

namespace XYZSupport
{
	class PolygonPoint
	{
	public:
		double x;
		double y;

		PolygonPoint()
		{
			x = 0;
			y = 0;
		}
		PolygonPoint(double x, double y)
		{
			this->x = x;
			this->y = y;
		}
		void ToDbgMsg(string name)
		{
			DbgMsg("===> %s: %f %f", name.c_str(), x, y);
		}
		string ToString() const
		{
			return to_string(x) + "," + to_string(y);
		}
	};

	class PolygonMinMaxPoint
	{
	public:
		double minX;
		double maxX;
		double minY;
		double maxY;
	};

	class LayerPolygon
	{
	public:
		vector<PolygonPoint> Points;

		void AddPoint(const PolygonPoint &polygon)
		{
			Points.push_back(polygon);
		}
	};

	class ModelLayer
	{
	public:
		vector<LayerPolygon> Polygons;
		vector<LayerPolygon> Holes;
		//debug svg to bmp
		vector<string> PolygonsColor;
		////

		void AddPolygon()
		{
			LayerPolygon Polygon;
			Polygons.push_back(Polygon);
		}

		//debug svg to bmp
		void AddPolygonColor(string PolygonColor)
		{
			PolygonsColor.push_back(PolygonColor);
		}
		////

		void AddHole()
		{
			LayerPolygon Hole;
			Holes.push_back(Hole);
		}
	};

	class LayerModel
	{
	public:
		vector<ModelLayer> Layers;
		vector<vector<vector<IntPoint>>> alllayer;

		void AddLayer()
		{
			ModelLayer layer;
			Layers.push_back(layer);
		}
		void SVGdatatoClipperdata(PolygonPoint boundarybox)
		{
			for (int layernum = 0; layernum < Layers.size(); layernum++)
			{
				vector<vector<IntPoint>> onelayer;
				for (int polynum = 0; polynum < Layers[layernum].Polygons.size(); polynum++)
				{
					vector<IntPoint> polygon;
					for (int pointnum = 0; pointnum < Layers[layernum].Polygons[polynum].Points.size(); pointnum++)
					{
						polygon.push_back(IntPoint(	static_cast<cInt>((Layers[layernum].Polygons[polynum].Points[pointnum].x + boundarybox.x) * 10000),
													static_cast<cInt>((Layers[layernum].Polygons[polynum].Points[pointnum].y + boundarybox.y) * 10000)));
					}
					onelayer.push_back(polygon);
				}
				alllayer.push_back(onelayer);
			}
		}
		void ToFile(const string filename, PolygonPoint xyMinLocation)
		{
			TextFile textFile(filename);
			if (textFile.IsSuccess() == false)
				return;

			textFile.Write("================================\n");
			textFile.Write("=    Dump LayerModel            \n");
			textFile.Write("================================\n");
			textFile.Write("xyMinLocation: %s\n", xyMinLocation.ToString().c_str());
			textFile.Write("ModelLayer size: %d\n", Layers.size());

			for (int i = 0; i < Layers.size(); i++)
			{
				textFile.Write("=== Layer %d ===\n", i);
				textFile.Write("Polygons Number: %d\n", Layers[i].Polygons.size());

				for (int j = 0; j < Layers[i].Polygons.size(); j++)
				{
					textFile.Write("[%d] ", j);
					for (int k = 0; k < Layers[i].Polygons[j].Points.size(); k++)
						textFile.Write(" %s", Layers[i].Polygons[j].Points[k].ToString().c_str());

					textFile.Write("\n");
				}
			}
			textFile.Write("================================\n");
			textFile.Write("=    Dump alllayer              \n");
			textFile.Write("================================\n");
			textFile.Write("alllayer ===> Layers Number: %d\n", alllayer.size());
			
			for (int i = 0; i < alllayer.size(); i++)
			{
				textFile.Write("=== Layer %d ===\n", i);
				textFile.Write("Polygons Number: %d\n", alllayer[i].size());

				for (int j = 0; j < alllayer[i].size(); j++)
				{
					textFile.Write("[%d] ", j);
					for (int k = 0; k < alllayer[i][j].size(); k++)
						textFile.Write(" %d,%d", alllayer[i][j][k].X, alllayer[i][j][k].Y);

					textFile.Write("\n");
				}
			}
		}
	};
}
